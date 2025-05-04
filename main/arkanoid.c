#include "arkanoid.h"

// Массив цветов кирпичей
const uint16_t brick_colors[BRICK_ROWS] = {
    0xF800, 0x07E0, 0x001F, 0xFFE0, 
    0xF81F, 0x07FF, 0xAFE5, 0xFF07
};

static bool is_colliding(GameObject a, GameObject b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

void print_bricks_matrix(Brick bricks[BRICK_ROWS][BRICK_COLS]) {
    printf("Матрица кирпичей:\n");
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            printf("%c", bricks[row][col].active ? 'X' : '.');
        }
        printf("\n");
    }
}

void generate_random_bricks(Brick bricks[BRICK_ROWS][BRICK_COLS]) {
    // Очистка поля
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            bricks[row][col].active = false;
        }
    }

    // Генерация 3-5 фигур
    int shapes = 3 + rand() % 3;
    printf("\n=== Новый раунд ===\nГенерируем %d фигур:\n", shapes);
    
    for (int s = 0; s < shapes; s++) {
        int start_row = rand() % (BRICK_ROWS - 4);
        int start_col = rand() % (BRICK_COLS - 4);
        ShapeType shape_type = rand() % (SHAPE_TOTAL_COUNT - 1);

        switch (shape_type) {
            case SHAPE_HORIZONTAL: {
                int length = 3 + rand() % 5;
                printf("%d. Горизонтальная линия (длина %d) в [%d,%d]\n", 
                      s+1, length, start_row, start_col);
                for (int i = 0; i < length && (start_col + i) < BRICK_COLS; i++) {
                    bricks[start_row][start_col + i].active = true;
                }
                break;
            }
            
            case SHAPE_VERTICAL: {
                int height = 2 + rand() % 4;
                printf("%d. Вертикальная линия (высота %d) в [%d,%d]\n",
                      s+1, height, start_row, start_col);
                for (int i = 0; i < height && (start_row + i) < BRICK_ROWS; i++) {
                    bricks[start_row + i][start_col].active = true;
                }
                break;
            }
            
            case SHAPE_SQUARE: {
                int size = 2 + rand() % 2;
                printf("%d. Квадрат %dx%d в [%d,%d]\n",
                      s+1, size, size, start_row, start_col);
                for (int i = 0; i < size && (start_row + i) < BRICK_ROWS; i++) {
                    for (int j = 0; j < size && (start_col + j) < BRICK_COLS; j++) {
                        bricks[start_row + i][start_col + j].active = true;
                    }
                }
                break;
            }
            
            case SHAPE_SNAKE: {
                int length = 5 + rand() % 6;
                printf("%d. Змейка (длина %d) в [%d,%d]\n",
                      s+1, length, start_row, start_col);
                int dir = rand() % 4;
                int row = start_row, col = start_col;
                
                for (int i = 0; i < length; i++) {
                    if (row >= 0 && row < BRICK_ROWS && col >= 0 && col < BRICK_COLS) {
                        bricks[row][col].active = true;
                        if (rand() % 10 < 3) dir = rand() % 4;
                        
                        switch (dir) {
                            case 0: row--; break;
                            case 1: col++; break;
                            case 2: row++; break;
                            case 3: col--; break;
                        }
                    }
                }
                break;
            }
            
            case SHAPE_SPIRAL: {
                int size = 3 + rand() % 3;
                printf("%d. Спираль (размер %d) в [%d,%d]\n",
                      s+1, size, start_row, start_col);
                int row = start_row, col = start_col;
                int dir = 1;
                int steps = 1, step_count = 0;
                
                for (int i = 0; i < size*size; i++) {
                    if (row >= 0 && row < BRICK_ROWS && col >= 0 && col < BRICK_COLS) {
                        bricks[row][col].active = true;
                    }
                    
                    switch (dir) {
                        case 0: row--; break;
                        case 1: col++; break;
                        case 2: row++; break;
                        case 3: col--; break;
                    }
                    
                    if (++step_count == steps) {
                        dir = (dir + 1) % 4;
                        step_count = 0;
                        if (dir % 2 == 0) steps++;
                    }
                }
                break;
            }
            
            case SHAPE_SNOWFLAKE: {
                int arms = 3 + rand() % 4;
                printf("%d. Снежинка (%d луча) в [%d,%d]\n",
                      s+1, arms, start_row+2, start_col+2);
                int center_row = start_row + 2;
                int center_col = start_col + 2;
                
                for (int a = 0; a < arms; a++) {
                    float angle = 2 * M_PI * a / arms;
                    for (int r = 0; r < 3; r++) {
                        int row = center_row + (int)(r * sin(angle));
                        int col = center_col + (int)(r * cos(angle));
                        
                        if (row >= 0 && row < BRICK_ROWS && col >= 0 && col < BRICK_COLS) {
                            bricks[row][col].active = true;
                        }
                    }
                }
                break;
            }
            
            default:
                printf("%d. Неизвестная фигура\n", s+1);
                break;
        }
    }
    printf("==================\n");
    print_bricks_matrix(bricks);

    // Установка параметров кирпичей
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            if (bricks[row][col].active) {
                bricks[row][col].x = col * (BRICK_WIDTH + BRICK_MARGIN) + BRICK_MARGIN;
                bricks[row][col].y = BRICK_TOP_MARGIN + row * (BRICK_HEIGHT + BRICK_MARGIN);
                bricks[row][col].width = BRICK_WIDTH;
                bricks[row][col].height = BRICK_HEIGHT;
                bricks[row][col].color = brick_colors[row % BRICK_ROWS];
            }
        }
    }
}

void draw_bricks(spi_device_handle_t spi, Brick bricks[BRICK_ROWS][BRICK_COLS]) {
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

bool all_bricks_destroyed(Brick bricks[BRICK_ROWS][BRICK_COLS]) {
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
    unsigned int seed = xTaskGetTickCount();
    printf("Инициализация генератора случайных чисел: seed=%u\n", seed);
    srand(seed);
    
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));
    
    Brick bricks[BRICK_ROWS][BRICK_COLS];
    GameObject player = {DISPLAY_WIDTH/2 - 20, 230, 40, 10, 0xFFFF};
    GameObject ball = {DISPLAY_WIDTH/2, player.y - 10, 10, 10, 0xFFFF};

    int received_button = 0;
    uint8_t speed = 3;
    float ball_speed_x = 0;
    float ball_speed_y = 0;
    float base_speed = 1.5f;
    uint8_t speed_hits = 0;
    uint16_t prev_player_x = player.x;
    uint16_t prev_ball_x = ball.x;
    uint16_t prev_ball_y = ball.y;
    uint8_t lives = 5;
    uint8_t round = load_nvs_u8("round");
    bool ball_active = false;
    bool red_button_enabled = true;
    bool game_paused = false;

    // Первый раунд - стандартное расположение
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            bricks[row][col].x = col * (BRICK_WIDTH + BRICK_MARGIN) + BRICK_MARGIN;
            bricks[row][col].y = row * (BRICK_HEIGHT + BRICK_MARGIN) + BRICK_TOP_MARGIN;
            bricks[row][col].width = BRICK_WIDTH;
            bricks[row][col].height = BRICK_HEIGHT;
            bricks[row][col].color = brick_colors[row];
            bricks[row][col].active = true;
        }
    }
    
    show_round_screen(spi, round, lives, base_speed);
    fill_screen(spi, 0x0000);
    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
    draw_bricks(spi, bricks);


    while (1) {
        // Обработка паузы
        if (gpio_get_level(BUTTON_WHITE) == 0) {
            vTaskDelay(200 / portTICK_PERIOD_MS);
            if (gpio_get_level(BUTTON_WHITE) == 0) {
                game_paused = !game_paused;
                
                if (game_paused) {
                    fill_screen(spi, 0x0000);
                    draw_text(spi, DISPLAY_WIDTH/2 - 10, DISPLAY_HEIGHT/2 - 20, 
                             u"ПАУЗА", 0xFFFF);
                    draw_text(spi, DISPLAY_WIDTH/2 - 115, DISPLAY_HEIGHT/2 + 30, 
                             u"ЧТОБЫ ПРОДОЛЖИТЬ НАЖМИТЕ БЕЛУЮ КНОПКУ", 0xFFFF);
                } else {
                    fill_screen(spi, 0x0000);
                    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    draw_bricks(spi, bricks);
                }
                
                while (gpio_get_level(BUTTON_WHITE) == 0) {
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                }
            }
        }

        // Пропуск раунда
        if (gpio_get_level(BUTTON_YELLOW) == 0) {
            vTaskDelay(200 / portTICK_PERIOD_MS);
            if (gpio_get_level(BUTTON_YELLOW) == 0) {
                for (int row = 0; row < BRICK_ROWS; row++) {
                    for (int col = 0; col < BRICK_COLS; col++) {
                        bricks[row][col].active = false;
                    }
                }
                while (gpio_get_level(BUTTON_YELLOW) == 0) {
                    vTaskDelay(50 / portTICK_PERIOD_MS);
                }
            }
        }

        if (game_paused) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }

        // Основной игровой цикл
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

            // Обработка столкновений
            for (int row = 0; row < BRICK_ROWS; row++) {
                for (int col = 0; col < BRICK_COLS; col++) {
                    if (bricks[row][col].active && 
                        is_colliding(ball, (GameObject){
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
                }
            }

            // Границы экрана
            if (ball.y <= 0) ball_speed_y = fabs(ball_speed_y);
            if (ball.x <= 0 || ball.x + ball.width >= DISPLAY_WIDTH) {
                ball_speed_x = -ball_speed_x;
            }

            // Проверка проигрыша
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
                    save_nvs_u8("round", round);
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
