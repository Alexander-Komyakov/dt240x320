#include "pong.h"

void game_pong(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    struct Player player = {10, 100, 10, 40, 0xFFFF}; // 0xFFFF - белый цвет
    struct Player bot = {300, 100, 10, 40, 0xFFFF};
    struct Player ball = {20, 115, 10, 10, 0xFFFF}; // При указанных позициях 60 по x - граница где мяч должен касаться прямоугольников

    fill_screen(spi, 0x0000);
    fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
    fill_rect(spi, bot.x, bot.y, bot.width, bot.height, bot.color);
    fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);

    uint8_t speed = 2;
    int8_t bot_speed = 1; // Скорость движения бота - за каждое движение +1 пиксель
    int8_t ball_speed_x = 2;
    int8_t ball_speed_y = 2;

    int received_button = 0;
    uint16_t old_player_y = player.y;
    uint16_t old_bot_y = bot.y;
    uint16_t old_ball_x = ball.x;
    uint16_t old_ball_y = ball.y;

    while (1) {
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (received_button == BUTTON_UP) {
                player.y -= speed;
            }
            else if (received_button == BUTTON_DOWN) {
                player.y += speed;
            }

            // Ограничиваем координаты
            player.y = ((uint16_t) (player.y + speed) <= speed) ? 0 : (player.y > DISPLAY_HEIGHT - player.height) ? DISPLAY_HEIGHT - player.height : player.y;
            fill_rect(spi, player.x, player.y, player.width, player.height, player.color);


            if (player.y < old_player_y) {
                fill_rect(spi, player.x, player.y + player.height, player.width,
                         old_player_y - player.y, 0x0000);
            }
            else if (player.y > old_player_y) {
                fill_rect(spi, player.x, old_player_y, player.width,
                         player.y - old_player_y, 0x0000);
            }
            old_player_y = player.y;
        }

        // Движение бота
        old_bot_y = bot.y;
        bot.y += bot_speed;

        if (bot.y <= 0) {
            bot.y = 0;
            bot_speed = 1;
        } 
        else if (bot.y + bot.height >= DISPLAY_HEIGHT) {
            bot.y = DISPLAY_HEIGHT - bot.height;
            bot_speed = -1;
        }

        // Отрисовка с частичной очисткой
        fill_rect(spi, bot.x, bot.y, bot.width, bot.height, bot.color);
        if (bot.y < old_bot_y) {
            fill_rect(spi, bot.x, bot.y + bot.height, bot.width, 
                     old_bot_y - bot.y, 0x0000);
        } 
        else if (bot.y > old_bot_y) {
            fill_rect(spi, bot.x, old_bot_y, bot.width, 
                     bot.y - old_bot_y, 0x0000);
        }
 
        printf("ball.x: %d, ball_speed_x: %d\n", ball.x, ball_speed_x);
        ball.x += ball_speed_x;
        // Стираем за шариком
        if (ball.x < old_ball_x) {
            fill_rect(spi, ball.x + ball.width, ball.y, old_ball_x - ball.x, 
                     ball.height, 0x0000);
        } 
        else if (ball.x > old_ball_x) {
            fill_rect(spi, old_ball_x, ball.y, ball.x - old_ball_x, 
                     ball.height, 0x0000);
        }
        // Движение шарика
        if (ball.x >= DISPLAY_WIDTH - ball.width) {
            ball_speed_x = -ball_speed_x;
        }
        else if (ball.x <= 0) {
            ball_speed_x = -ball_speed_x;
        }
        old_ball_x = ball.x;
        old_ball_y = ball.y;
        fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);

        vTaskDelay(10 / portTICK_PERIOD_MS);  // 10 (~100 FPS)
    }
}


