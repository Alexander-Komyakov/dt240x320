#include "arkanoid.h"
#include "font.h"
#include <math.h>

// Функция проверки столкновения
static bool is_colliding(struct Player_arkanoid a, struct Player_arkanoid b) {
    return (a.x < b.x + b.width &&
            a.x + a.width > b.x &&
            a.y < b.y + b.height &&
            a.y + a.height > b.y);
}

void game_arkanoid(spi_device_handle_t spi) {
//restart_game:
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    // Инициализация объектов
    struct Player_arkanoid player = {DISPLAY_WIDTH/2 - 20, 230, 40, 10, 0xFFFF};
    struct Player_arkanoid ball = {DISPLAY_WIDTH/2, player.y - 10, 10, 10, 0xFFFF};

    int received_button = 0;

    // Настройки скорости
    uint8_t speed = 3;
	// Пока укажем значение нулевыми, Чтобы мяч не двигался. Стояли значения 3 и 3
//    int8_t ball_speed_x = 1;
//    int8_t ball_speed_y = 1;
	float ball_speed_x = 1.5f;
	float ball_speed_y = 1.5f;

    // Переменные для частичной перерисовки
    uint16_t prev_player_x = player.x;
    uint16_t prev_ball_x = ball.x;
    uint16_t prev_ball_y = ball.y;

    // Переменные для обработки столкновений
//    int hit_pos;

    // Первоначальная отрисовка
    fill_screen(spi, 0x0000);
    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);

    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (received_button == BUTTON_LEFT) {
                player.x = (player.x > speed) ? player.x - speed : 0;
                fill_rect(spi, player.x + player.width, player.y , prev_player_x - player.x, player.height, 0x0000);
            }
            else if (received_button == BUTTON_RIGHT) {
                player.x = (player.x < DISPLAY_WIDTH - player.width - speed) ?
                          player.x + speed : DISPLAY_WIDTH - player.width;
                fill_rect(spi, prev_player_x, player.y, player.x - prev_player_x, player.height, 0x0000);
            }

            // Рисуем новую позицию
            fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
            prev_player_x = player.x;
        }

        // Движение мяча
        prev_ball_x = ball.x;
        prev_ball_y = ball.y;
        ball.x += (int)ball_speed_x;
        ball.y += (int)ball_speed_y;

        // Проверка столкновений с платформой
//	    if (is_colliding(player, ball)) {
//	        ball.y = player.y - ball.height;
//	        ball_speed_y = -fabs(ball_speed_y);
//	        hit_pos = (ball.x + ball.width/2) - (player.x + player.width/2);
//	        ball_speed_x = hit_pos * 0.2f;
//
//		    printf("hit_pos: %d | speed_x: %.1f | speed_y: %.1f\n", 
//				hit_pos, ball_speed_x, ball_speed_y);
//	    }

		if (is_colliding(player, ball)) {
		    ball.y = player.y - ball.height;
		    
		    //  Точное положение удара в пикселях относительно центра
		    int hit_pixel = (ball.x + ball.width/2) - (player.x + player.width/2);
		    
		    //  Нормализация с сохранением знака (-player.width/2 до +player.width/2)
		    float hit_norm = (float)hit_pixel / (player.width/2.0f);
		    
		    //  Базовые скорости:
		    const float center_speed = 1.25f;  // Скорость по центру
		    const float edge_speed = 2.25f;    // Скорость у края
		    const float speed_range = edge_speed - center_speed;
		    
		    //  Линейное нарастание скорости от центра к краю
		    float speed_factor = fabsf(hit_norm); // 0.0 в центре, 1.0 у края
		    float current_speed = center_speed + (speed_range * speed_factor);
		    
		    //  Направление (сохраняем знак hit_norm)
		    ball_speed_x = copysignf(current_speed, hit_norm);
		    
		    //  Вертикальная скорость (можно сделать зависимой от удара)
//		    ball_speed_y = -1.5f;
		    ball_speed_y = -(1.25f + speed_factor * 0.75f);
		    
		    printf("Pixel: %d | Norm: %.2f | SpeedX: %.2f | SpeedY: %.2f\n", 
		          hit_pixel, hit_norm, ball_speed_x, ball_speed_y);
		}

	    // Отскок от границ
	    if (ball.y <= 0) ball_speed_y = fabs(ball_speed_y);
	    if (ball.x <= 0 || ball.x + ball.width >= DISPLAY_WIDTH) {
	        ball_speed_x = -ball_speed_x;
	    }

	    // Потеря мяча
	    if (ball.y + ball.height >= DISPLAY_HEIGHT) {
	        ball.x = player.x + player.width/2 - ball.width/2;
	        ball.y = player.y - ball.height;
	        ball_speed_y = -1.25f;
	    }

	    // Отрисовка
	    fill_rect(spi, prev_ball_x, prev_ball_y, ball.width, ball.height, 0x0000);
	    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);

        vTaskDelay(10 / portTICK_PERIOD_MS);

		}
}
