#include "flappy.h"

void game_flappy(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));
    draw_image(spi, &image_flappy_background);
    fill_rect(spi, 140, 0, 180, 240, 0x4DF9);

    fill_rect(spi, 0, 0, 38, 240, 0xD6B1);
    //draw_image(spi, &image_flappy_up);
    draw_image_composite(spi, &image_flappy_up, &image_flappy_background, 1, 0x195E);
    draw_image(spi, &image_flappy_down);
    draw_image(spi, &image_flappy_mid);
    //image_flappy_pipe.x -= 100;
    //image_flappy_pipe.y -= 25;
    //draw_image_part(spi, &image_flappy_pipe, 100, 25, 95, 26);
    image_flappy_pipe_up.x = 0;
    image_flappy_pipe_up.y = 0;
    //draw_image_part(spi, &image_flappy_pipe, 95, 26, 100, 25);

    //image_flappy_pipe_up.y -= 25;
    //draw_image_part(spi, &image_flappy_pipe_up, 0, 25, 100, 26);
    uint16_t i = 50;

    while (1) {
        if (i == 0) { i = 50; };
        // Обработка ввода игрока
        //image_flappy_ground.y = i;
        //draw_image(spi, &image_flappy_ground);
        if (i % 2) {
            image_flappy_ground.y = i/2;
            draw_image_part(spi, &image_flappy_ground, 0, 0, 22, 240-(i/2));
            image_flappy_ground.y = 0;
            draw_image_part(spi, &image_flappy_ground, 0, 240-(i/2), 22, (i/2));
        } else {
            if (image_flappy_pipe.y == 0) image_flappy_pipe.y = 189;
            image_flappy_pipe.y--;
            draw_image_part(spi, &image_flappy_pipe, 0, 0, 195, 51);
        }
        i--;
        //if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
        //    if (received_button == BUTTON_UP) {
        //    }
        //    else if (received_button == BUTTON_DOWN) {
        //    }

        //}
        //ssa = 0+i;
        //vertical_scroll(spi, &tfa, &vsa, &bfa, &ssa);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
