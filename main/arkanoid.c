#include "arkanoid.h"
#include "font.h"
#include <math.h>

static bool is_colliding(struct Player_arkanoid a, struct Player_arkanoid b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

void game_arkanoid(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    // Инициализация объектов
    struct Player_arkanoid player = {DISPLAY_WIDTH/2 - 20, 230, 40, 10, 0xFFFF};
    struct Player_arkanoid ball = {DISPLAY_WIDTH/2, player.y - 10, 10, 10, 0xFFFF};

    int received_button = 0;

    // Настройки скорости
    uint8_t speed = 3;
    float ball_speed_x = 0;
    float ball_speed_y = 0;

    // Переменные для частичной перерисовки
    uint16_t prev_player_x = player.x;
    uint16_t prev_ball_x = ball.x;
    uint16_t prev_ball_y = ball.y;

    // Система жизней и раундов
    uint8_t lives = 5;
    uint8_t round = 1;
    bool game_started = false;
    bool ball_active = false;
    bool red_button_enabled = true;

    // Первоначальная отрисовка
    fill_screen(spi, 0x0000);
    
    // Отображение начального экрана
    const uint16_t round_text[] = {u'Р', u'А', u'У', u'Н', u'Д', u' ', u'0' + round, 0};
    draw_text(spi, DISPLAY_WIDTH/2 - 20, DISPLAY_HEIGHT/2 - 10, round_text, 0xFFFF);
    const uint16_t lives_text[] = {u'Ж', u'И', u'З', u'Н', u'И', u':', u' ', u'0' + lives, 0};
    draw_text(spi, DISPLAY_WIDTH/2 - 20, DISPLAY_HEIGHT/2 + 10, lives_text, 0xFFFF);
    
    vTaskDelay(3000 / portTICK_PERIOD_MS); // Задержка 3 секунды
    
    fill_screen(spi, 0x0000);
    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);

    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (received_button == BUTTON_LEFT) {
                player.x = (player.x > speed) ? player.x - speed : 0;
                fill_rect(spi, player.x + player.width, player.y, prev_player_x - player.x, player.height, 0x0000);
                // Обновляем позицию мяча, если он еще не запущен
                if (!ball_active) {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000); // Стираем старую позицию
                    ball.x = player.x + player.width/2 - ball.width/2;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    prev_ball_x = ball.x;
                }
            }
            else if (received_button == BUTTON_RIGHT) {
                player.x = (player.x < DISPLAY_WIDTH - player.width - speed) ?
                          player.x + speed : DISPLAY_WIDTH - player.width;
                fill_rect(spi, prev_player_x, player.y, player.x - prev_player_x, player.height, 0x0000);
                // Обновляем позицию мяча, если он еще не запущен
                if (!ball_active) {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000); // Стираем старую позицию
                    ball.x = player.x + player.width/2 - ball.width/2;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    prev_ball_x = ball.x;
                }
            }
            else if (received_button == BUTTON_RED && red_button_enabled) {
                if (!ball_active) {
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000); // Стираем старую позицию
                    // Запуск мяча
                    ball.x = player.x + player.width/2 - ball.width/2;
                    ball.y = player.y - ball.height;
                    ball_speed_x = 1.5f;
                    ball_speed_y = -1.5f;
                    prev_ball_x = ball.x;
                    prev_ball_y = ball.y;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
                    ball_active = true;
                    red_button_enabled = false;
                }
            }

            // Рисуем новую позицию
            fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
            prev_player_x = player.x;
        }

        // Движение мяча (только если он запущен)
        if (ball_active) {
            // Движение мяча
            prev_ball_x = ball.x;
            prev_ball_y = ball.y;
            ball.x += (int)ball_speed_x;
            ball.y += (int)ball_speed_y;

            if (is_colliding(player, ball)) {
                ball.y = player.y - ball.height;
                
                // Точное положение удара в пикселях относительно центра
                int hit_pixel = (ball.x + ball.width/2) - (player.x + player.width/2);
                
                // Нормализация с сохранением знака (-player.width/2 до +player.width/2)
                float hit_norm = (float)hit_pixel / (player.width/2.0f);
                
                // Базовые скорости:
                const float center_speed = 1.25f;
                const float edge_speed = 2.25f;
                const float speed_range = edge_speed - center_speed;
                
                // Линейное нарастание скорости от центра к краю
                float speed_factor = fabsf(hit_norm);
                float current_speed = center_speed + (speed_range * speed_factor);
                
                // Направление (сохраняем знак hit_norm)
                ball_speed_x = copysignf(current_speed, hit_norm);
                
                // Вертикальная скорость
                ball_speed_y = -(1.25f + speed_factor * 0.75f);
            }

            // Отскок от границ
            if (ball.y <= 0) ball_speed_y = fabs(ball_speed_y);
            if (ball.x <= 0 || ball.x + ball.width >= DISPLAY_WIDTH) {
                ball_speed_x = -ball_speed_x;
            }

            // Потеря мяча
            if (ball.y + ball.height >= DISPLAY_HEIGHT) {
                // Очищаем мяч в старой и новой позиции на всякий случай
                fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                fill_rect(spi, ball.x, ball.y, ball.width, ball.height, 0x0000);
                
                lives--;
                
                // Отображаем количество жизней
                fill_rect(spi, DISPLAY_WIDTH - 30, 10, 20, 8, 0x0000);
                const uint16_t lives_count[] = {u'0' + lives, 0};
                draw_text(spi, DISPLAY_WIDTH - 30, 10, lives_count, 0xFFFF);
                
                if (lives > 0) {
                    // Возвращаем мяч на платформу
                    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000); // Очищаем старую позицию
                    ball.x = player.x + player.width/2 - ball.width/2;
                    ball.y = player.y - ball.height;
                    ball_speed_x = 0;
                    ball_speed_y = 0;
                    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color); // Рисуем новую позицию
                    prev_ball_x = ball.x;
                    prev_ball_y = ball.y;
                    ball_active = false;
                    red_button_enabled = true;
                } else {
                    // Конец игры
                    const uint16_t game_over[] = {u'К', u'О', u'Н', u'Е', u'Ц', u' ', u'И', u'Г', u'Р', u'Ы', 0};
                    draw_text(spi, DISPLAY_WIDTH/2 - 20, DISPLAY_HEIGHT/2, game_over, 0xFFFF);
                    vTaskDelay(3000 / portTICK_PERIOD_MS);
                    return; // Выход из игры
                }
            }
            else {
                // Отрисовка мяча в новом положении только если он не потерян
                fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
                fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
