#include "dacha.h"

void game_dacha(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    fill_rect(spi, 60, 0, 260, 240, 0x4DF9);
    fill_rect(spi, 0, 0, 38, 240, 0xD6B1);
	int received_button;

    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
			
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
