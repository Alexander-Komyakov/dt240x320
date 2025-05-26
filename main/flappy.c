#include "flappy.h"

void game_flappy(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));
    draw_image(spi, &image_flappy_ground);

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
