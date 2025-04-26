#include "pong.h"

void game_pong(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

	struct Player player = {30, 80, 30, 70, 0xFFFF}; // 0xFFFF - белый цвет
	struct Player bot = {260, 80, 30, 70, 0xFFFF};

    fill_screen(spi, 0x0000);
	fill_rect(spi, player.x, player.y, player.width, player.height, player.color);
	fill_rect(spi, bot.x, bot.y, bot.width, bot.height, bot.color);

	uint8_t speed = 1;

	int received_button = 0;
	int old_coordinates = player.y;
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
        vTaskDelay(10 / portTICK_PERIOD_MS);  // 10 (~100 FPS)
    }
}


