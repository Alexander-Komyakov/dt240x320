#include "flappy.h"

void game_flappy(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));
    draw_image(spi, &image_flappy_ground);
    draw_image(spi, &image_flappy_background);
    fill_rect(spi, 140, 0, 180, 240, 0x4DF9);

    //draw_image(spi, &image_flappy_up);
    draw_image_composite(spi, &image_flappy_up, &image_flappy_background, 1, 0x195E);
    draw_image(spi, &image_flappy_down);
    draw_image(spi, &image_flappy_mid);
    //draw_image(spi, &image_flappy_pipe);
    image_flappy_pipe.x -= 100;
    image_flappy_pipe.y -= 25;
    draw_image_part(spi, &image_flappy_pipe, 100, 25, 95, 26);

    image_flappy_pipe_up.y -= 25;
    draw_image_part(spi, &image_flappy_pipe_up, 0, 25, 100, 26);
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
