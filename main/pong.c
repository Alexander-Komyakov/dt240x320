#include "pong.h"

void game_pong(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

	struct Player player = {30, 80, 30, 70, 0xFFFF}; // 0xFFFF - белый цвет
	struct Player bot = {260, 80, 30, 70, 0xFFFF};

    fill_screen(spi, 0x0000);
	fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
	fill_rect(spi, bot.x, bot.y, bot.width, bot.height, bot.color);

//	struct Player ball = {60, 110, 10, 10, 0xFFFF}; // При указанных позициях 60 по x - граница где мяч должен касаться прямоугольников
//	fill_rect(spi, ball.x, ball.y, ball.width, ball.height, ball.color);
//	uint8_t ball_speed = 2;

	uint8_t speed = 2;
    int8_t bot_speed = 1; // Скорость движения бота - за каждое движение +1 пиксель

	int received_button = 0;
	uint16_t old_coordinates = player.y;
	uint16_t old_bot_coordinates = bot.y;

	while (1) {
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
			if (received_button == BUTTON_UP) {
				player.y -= speed;
			}
			if (received_button == BUTTON_DOWN) {
			    player.y += speed;
			}

			// Ограничиваем координаты
			player.y = ((uint16_t) (player.y + speed) <= speed) ? 0 : (player.y > DISPLAY_HEIGHT - player.height) ? DISPLAY_HEIGHT - player.height : player.y;
			fill_rect(spi, player.x, player.y, player.width, player.height, player.color);


			if (player.y < old_coordinates) {
				fill_rect(spi, player.x, player.y + player.height, player.width, old_coordinates - player.y, 0x0000);
			}
			if (player.y > old_coordinates) {
				fill_rect(spi, player.x, old_coordinates, player.width, player.y - old_coordinates, 0x0000);
			}
			old_coordinates = player.y;
		}




			// Движение бота
			int old_bot_y = bot.y;
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




        vTaskDelay(10 / portTICK_PERIOD_MS);  // 10 (~100 FPS)
    }
}


