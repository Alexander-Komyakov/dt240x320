#include "dacha.h"

void game_dacha(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

	draw_image(spi, &image_dacha);

	int received_button;

    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
			
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
