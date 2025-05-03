#include "arkanoid.h"
#include "font.h"
#include <math.h>
#include <stdlib.h>

#define BRICK_ROWS 5
#define BRICK_COLS 10
#define BRICK_WIDTH 30
#define BRICK_HEIGHT 10
#define BRICK_MARGIN 2
#define BRICK_TOP_MARGIN 30
#define SPEED_INCREASE_INTERVAL 15
#define MIN_DISTANCE_FROM_BOTTOM 30

struct Brick {
    uint16_t x, y;
    uint16_t width, height;
    uint16_t color;
    bool active;
};

static bool is_colliding(struct Player_arkanoid a, struct Player_arkanoid b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

void generate_random_bricks(struct Brick bricks[BRICK_ROWS][BRICK_COLS]) {
    uint16_t brick_colors[BRICK_ROWS] = {0xF800, 0x07E0, 0x001F, 0xFFE0, 0xF81F};
    
    // Очищаем все кирпичи
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            bricks[row][col].active = false;
        }
    }
    
    // Генерируем случайные фигуры
    int shapes = 3 + rand() % 3; // От 3 до 5 фигур
    for (int s = 0; s < shapes; s++) {
        int shape_type = rand() % 3;
        int start_row = rand() % BRICK_ROWS;
        int start_col = rand() % (BRICK_COLS - 2); // -2 чтобы фигура поместилась
        
        switch (shape_type) {
            case 0: // Горизонтальная линия (3-5 кирпичей)
                {
                    int length = 3 + rand() % 3;
                    for (int i = 0; i < length && (start_col + i) < BRICK_COLS; i++) {
                        bricks[start_row][start_col + i].active = true;
                    }
                }
                break;
            case 1: // Вертикальная линия (2-4 кирпича)
                {
                    int height = 2 + rand() % 3;
                    for (int i = 0; i < height && (start_row + i) < BRICK_ROWS; i++) {
                        bricks[start_row + i][start_col].active = true;
                    }
                }
                break;
            case 2: // Квадрат 2x2 или 3x3
                {
                    int size = (rand() % 2) ? 2 : 3;
                    for (int i = 0; i < size && (start_row + i) < BRICK_ROWS; i++) {
                        for (int j = 0; j < size && (start_col + j) < BRICK_COLS; j++) {
                            bricks[start_row + i][start_col + j].active = true;
                        }
                    }
                }
                break;
        }
    }
    
    // Устанавливаем координаты и цвета для активных кирпичей
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            if (bricks[row][col].active) {
                bricks[row][col].x = col * (BRICK_WIDTH + BRICK_MARGIN) + BRICK_MARGIN;
                // Убедимся, что кирпичи не слишком близко к низу
                int max_y = DISPLAY_HEIGHT - MIN_DISTANCE_FROM_BOTTOM - BRICK_HEIGHT;
                bricks[row][col].y = MIN_DISTANCE_FROM_BOTTOM + 
                                    (row * (BRICK_HEIGHT + BRICK_MARGIN)) % 
                                    (max_y - BRICK_TOP_MARGIN);
                bricks[row][col].width = BRICK_WIDTH;
                bricks[row][col].height = BRICK_HEIGHT;
                bricks[row][col].color = brick_colors[row];
            }
        }
    }
}

void draw_bricks(spi_device_handle_t spi, struct Brick bricks[BRICK_ROWS][BRICK_COLS]) {
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            if (bricks[row][col].active) {
                fill_rect(spi, bricks[row][col].x, bricks[row][col].y, 
                         bricks[row][col].width, bricks[row][col].height, 
                         bricks[row][col].color);
            }
        }
    }
}

bool all_bricks_destroyed(struct Brick bricks[BRICK_ROWS][BRICK_COLS]) {
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            if (bricks[row][col].active) {
                return false;
            }
        }
    }
    return true;
}

void show_round_screen(spi_device_handle_t spi, uint8_t round, uint8_t lives, float speed) {
    fill_screen(spi, 0x0000);
    
    const uint16_t round_text[] = {u'Р', u'А', u'У', u'Н', u'Д', u' ', u'0' + round, 0};
    draw_text(spi, DISPLAY_WIDTH/2 - 20, DISPLAY_HEIGHT/2 - 20, round_text, 0xFFFF);
    
    const uint16_t lives_text[] = {u'Ж', u'И', u'З', u'Н', u'И', u':', u'0' + lives, 0};
    draw_text(spi, DISPLAY_WIDTH/2 - 20, DISPLAY_HEIGHT/2, lives_text, 0xFFFF);
    
    vTaskDelay(3000 / portTICK_PERIOD_MS);
}

void game_arkanoid(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    srand(xTaskGetTickCount());
    
    struct Brick bricks[BRICK_ROWS][BRICK_COLS];
    struct Player_arkanoid player = {DISPLAY_WIDTH/2 - 20, 230, 40, 10, 0xFFFF};
    struct Player_arkanoid ball = {DISPLAY_WIDTH/2, player.y - 10, 10, 10, 0xFFFF};

    int received_button = 0;

    uint8_t speed = 3;
    float ball_speed_x = 0;
    float ball_speed_y = 0;
    float base_speed = 1.5f;
    uint8_t speed_hits = 0; // Счетчик ударов для увеличения скорости

    uint16_t prev_player_x = player.x;
    uint16_t prev_ball_x = ball.x;
    uint16_t prev_ball_y = ball.y;

    uint8_t lives = 5;
    uint8_t round = 1;
    bool ball_active = false;
    bool red_button_enabled = true;
    bool game_paused = false;

    // Первый раунд - стандартное расположение кирпичей
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            bricks[row][col].x = col * (BRICK_WIDTH + BRICK_MARGIN) + BRICK_MARGIN;
            bricks[row][col].y = row * (BRICK_HEIGHT + BRICK_MARGIN) + BRICK_TOP_MARGIN;
            bricks[row][col].width = BRICK_WIDTH;
            bricks[row][col].height = BRICK_HEIGHT;
            bricks[row][col].color = (row == 0) ? 0xF800 : 
                                   (row == 1) ? 0x07E0 :
                                   (row == 2) ? 0x001F :
                                   (row == 3) ? 0xFFE0 : 0xF81F;
            bricks[row][col].active = true;
        }
    }
    
    show_round_screen(spi, round, lives, base_speed);
    
    fill_screen(spi, 0x0000);
    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
    draw_bricks(spi, bricks);

    while (1) {
        // Проверка нажатия белой кнопки для паузы/продолжения
        if (gpio_get_level(BUTTON_WHITE) == 0) {
            vTaskDelay(200 / portTICK_PERIOD_MS); // Дебаунс
            if (gpio_get_level(BUTTON_WHITE) == 0) {
                game_paused = !game_paused; // Переключаем состояние паузы
                
                if (game_paused) {
                    // Отрисовка меню паузы
                    fill_screen(spi, 0x0000);
                    draw_text(spi, DISPLAY_WIDTH/2 - 10, DISPLAY_HEIGHT/2 - 20, 
                             u"ПАУЗА", 0xFFFF);
                    draw_text(spi, DISPLAY_WIDTH/2 - 115, DISPLAY_HEIGHT/2 + 30, 
                             u"ЧТОБЫ ПРОДОЛЖИТЬ НАЖМИТЕ БЕЛУЮ КНОПКУ", 0xFFFF);
                } else {
                    // Восстановление игры - перерисовываем все объекты
                    fill_screen(spi, 0x0000);
                    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    draw_bricks(spi, bricks);
                }
                
                // Ждем отпускания кнопки
                while (gpio_get_level(BUTTON_WHITE) == 0) {
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                }
            }
        }

        // Проверка нажатия желтой кнопки для автозавершения раунда
        if (gpio_get_level(BUTTON_YELLOW) == 0) {
            vTaskDelay(200 / portTICK_PERIOD_MS); // Дебаунс
            if (gpio_get_level(BUTTON_YELLOW) == 0) {
                // Деактивируем все кирпичи
                for (int row = 0; row < BRICK_ROWS; row++) {
                    for (int col = 0; col < BRICK_COLS; col++) {
                        bricks[row][col].active = false;
                    }
                }
                // Ждем отпускания кнопки
                while (gpio_get_level(BUTTON_YELLOW) == 0) {
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                }
            }
        }

        // Если игра на паузе - пропускаем основной игровой цикл
        if (game_paused) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (received_button == BUTTON_LEFT) {
                player.x = (player.x > speed) ? player.x - speed : 0;
                fill_rect(spi, player.x + player.width, player.y, prev_player_x - player.x, player.height, 0x0000);
                if (!ball_active) {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                    ball.x = player.x + player.width/2 - ball.width/2;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    prev_ball_x = ball.x;
                }
            }
            else if (received_button == BUTTON_RIGHT) {
                player.x = (player.x < DISPLAY_WIDTH - player.width - speed) ?
                          player.x + speed : DISPLAY_WIDTH - player.width;
                fill_rect(spi, prev_player_x, player.y, player.x - prev_player_x, player.height, 0x0000);
                if (!ball_active) {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                    ball.x = player.x + player.width/2 - ball.width/2;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    prev_ball_x = ball.x;
                }
            }
            else if (received_button == BUTTON_RED && red_button_enabled) {
                if (!ball_active) {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                    ball.x = player.x + player.width/2 - ball.width/2;
                    ball.y = player.y - ball.height;
                    ball_speed_x = base_speed;
                    ball_speed_y = -base_speed;
                    prev_ball_x = ball.x;
                    prev_ball_y = ball.y;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    ball_active = true;
                    red_button_enabled = false;
                }
            }

            fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
            prev_player_x = player.x;
        }

        if (ball_active) {
            prev_ball_x = ball.x;
            prev_ball_y = ball.y;
            ball.x += (int)ball_speed_x;
            ball.y += (int)ball_speed_y;

            for (int row = 0; row < BRICK_ROWS; row++) {
                for (int col = 0; col < BRICK_COLS; col++) {
                    if (bricks[row][col].active && 
                        is_colliding(ball, (struct Player_arkanoid){
                            bricks[row][col].x, 
                            bricks[row][col].y, 
                            bricks[row][col].width, 
                            bricks[row][col].height, 
                            0})) {
                        
                        bricks[row][col].active = false;
                        fill_rect(spi, bricks[row][col].x, bricks[row][col].y, 
                                 bricks[row][col].width, bricks[row][col].height, 
                                 0x0000);
                        
                        if (ball.x + ball.width < bricks[row][col].x + bricks[row][col].width/2) {
                            ball_speed_x = -fabs(ball_speed_x);
                        } else if (ball.x > bricks[row][col].x + bricks[row][col].width/2) {
                            ball_speed_x = fabs(ball_speed_x);
                        } else {
                            ball_speed_y = -ball_speed_y;
                        }
                    }
                }
            }

            if (is_colliding(player, ball)) {
                ball.y = player.y - ball.height;
                int hit_pixel = (ball.x + ball.width/2) - (player.x + player.width/2);
                float hit_norm = (float)hit_pixel / (player.width/2.0f);
                const float center_speed = 1.25f;
                const float edge_speed = 2.25f;
                const float speed_range = edge_speed - center_speed;
                float speed_factor = fabsf(hit_norm);
                float current_speed = center_speed + (speed_range * speed_factor);
                ball_speed_x = copysignf(current_speed, hit_norm);
                ball_speed_y = -(1.25f + speed_factor * 0.75f);
                
                speed_hits++;
                if (speed_hits % SPEED_INCREASE_INTERVAL == 0) {
                    base_speed += 0.25f;
                    printf("Speed increased to %.2f after %d hits\n", base_speed, speed_hits);
                }
            }

            if (ball.y <= 0) ball_speed_y = fabs(ball_speed_y);
            if (ball.x <= 0 || ball.x + ball.width >= DISPLAY_WIDTH) {
                ball_speed_x = -ball_speed_x;
            }

            if (ball.y + ball.height >= DISPLAY_HEIGHT) {
                fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                fill_rect(spi, ball.x, ball.y, ball.width, ball.height, 0x0000);
                
                lives--;
                fill_rect(spi, DISPLAY_WIDTH - 30, 10, 20, 8, 0x0000);
                const uint16_t lives_count[] = {u'0' + lives, 0};
                draw_text(spi, DISPLAY_WIDTH - 30, 10, lives_count, 0xFFFF);
                
                if (lives > 0) {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                    ball.x = player.x + player.width/2 - ball.width/2;
                    ball.y = player.y - ball.height;
                    ball_speed_x = 0;
                    ball_speed_y = 0;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    prev_ball_x = ball.x;
                    prev_ball_y = ball.y;
                    ball_active = false;
                    red_button_enabled = true;
                    // Сброс скорости только при потере жизни
                    base_speed = 1.5f;
                    speed_hits = 0;
                } else {
                    const uint16_t game_over[] = {u'К', u'О', u'Н', u'Е', u'Ц', u' ', u'И', u'Г', u'Р', u'Ы', 0};
                    draw_text(spi, DISPLAY_WIDTH/2 - 20, DISPLAY_HEIGHT/2, game_over, 0xFFFF);
                    vTaskDelay(3000 / portTICK_PERIOD_MS);
                    return;
                }
            }
            else {
                if (all_bricks_destroyed(bricks)) {
                    round++;
                    // Сохраняем скорость между раундами, но сбрасываем счетчик
                    speed_hits = 0;
                    
                    generate_random_bricks(bricks);
                    show_round_screen(spi, round, lives, base_speed);
                    
                    player.x = DISPLAY_WIDTH/2 - 20;
                    ball.x = player.x + player.width/2 - ball.width/2;
                    ball.y = player.y - ball.height;
                    prev_ball_x = ball.x;
                    prev_ball_y = ball.y;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);

                    ball_speed_x = 0;
                    ball_speed_y = 0;
                    ball_active = false;
                    red_button_enabled = true;
                    
                    fill_screen(spi, 0x0000);
                    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    draw_bricks(spi, bricks);
                }
                else {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                }
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
