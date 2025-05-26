#include "flappy.h"

void game_flappy(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));
    draw_image(spi, &image_flappy_ground);
    draw_image(spi, &image_flappy_background);
    fill_rect(spi, 140, 0, 180, 240, 0x4DF9);

    draw_image(spi, &image_flappy_up);
    draw_image(spi, &image_flappy_down);
    draw_image(spi, &image_flappy_mid);

    while (1) {
        // Обработка ввода игрока
        //if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
        //    if (received_button == BUTTON_UP) {
        //    }
        //    else if (received_button == BUTTON_DOWN) {
        //    }

        //}
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
