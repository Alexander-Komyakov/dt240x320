#include "dacha.h"

void game_dacha(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

	draw_image(spi, &image_dacha);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
	draw_image(spi, &image_dacha_evening);

	int received_button;

    // Переменные для скроллинга
    uint16_t ssa = 0;
    uint16_t speed = 0;
    uint16_t iter_scroll = 1;
    while (1) {
        // Обработка ввода игрока
		speed = 0;
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
		    speed = 1;
        }



        // Скроллинг
        if (iter_scroll == 0) { iter_scroll = 320; }; iter_scroll-=speed;
        ssa = 0+iter_scroll;
        vertical_scroll(spi, 0, 320, 0, ssa);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
