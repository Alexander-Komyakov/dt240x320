#include "flappy.h"

void game_flappy(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));
    fill_rect(spi, 60, 0, 260, 240, 0x4DF9);

    fill_rect(spi, 0, 0, 38, 240, 0xD6B1);

    uint8_t pipe_count = 2;
    Image* images_flappy_pipe[pipe_count];
    images_flappy_pipe[1] = &(Image) {
        .x = 60,
        .y = 239,
        .width = 195,
        .height = 51,
        .size_image = 9945, 
        .pixels = image_flappy_pipe_pixels
    };
    images_flappy_pipe[0] = &(Image) {
        .x = 260,
        .y = 239,
        .width = 195,
        .height = 51,
        .size_image = 9945, 
        .pixels = image_flappy_pipe_up_pixels
    };

    uint16_t flappy_counter = 0;
    uint16_t parity_counter = 50;
    uint16_t pipe_counter[pipe_count];
    uint16_t pipe_width[pipe_count];
    pipe_width[0] = 60;
    pipe_width[1] = 60;
    pipe_counter[0] = images_flappy_pipe[0]->height;
    pipe_counter[1] = images_flappy_pipe[1]->height;


    while (1) {
        if (parity_counter == 0) { parity_counter = 50; };
        if (flappy_counter == 18) { flappy_counter = 0; };
        draw_image(spi, &image_flappy_up);
        flappy_counter++;
        image_flappy_up.pixels = (flappy_counter <= 6) ? image_flappy_up_pixels : (flappy_counter <= 12) ? image_flappy_mid_pixels : image_flappy_down_pixels;
        if (parity_counter % 2) {
            image_flappy_up.pixels = image_flappy_mid_pixels;
            image_flappy_ground.y = parity_counter/2;
            draw_image_part(spi, &image_flappy_ground, 0, 0, 22, 240-(parity_counter/2));
            image_flappy_ground.y = 0;
            draw_image_part(spi, &image_flappy_ground, 0, 240-(parity_counter/2), 22, (parity_counter/2));
        } else {
            for (uint16_t i = 0; i < pipe_count; i++) {
                if (images_flappy_pipe[i]->y == 0) {
                    draw_image_part(spi, images_flappy_pipe[i], (images_flappy_pipe[i]->width - pipe_width[i])*i, images_flappy_pipe[i]->height-pipe_counter[i], pipe_width[i], pipe_counter[i]);
                    if (pipe_counter[i] == 0) {
                        images_flappy_pipe[i]->y = 239;
                        pipe_counter[i] = images_flappy_pipe[i]->height;
                    }
                    pipe_counter[i]--;
                } else if (images_flappy_pipe[i]->y > 189) {
                    draw_image_part(spi, images_flappy_pipe[i], (images_flappy_pipe[i]->width - pipe_width[i])*i, 0, pipe_width[i], DISPLAY_HEIGHT-images_flappy_pipe[i]->y);
                    images_flappy_pipe[i]->y--;
                } else  {
                    draw_image_part(spi, images_flappy_pipe[i], (images_flappy_pipe[i]->width - pipe_width[i])*i, 0, pipe_width[i], 51);
                    images_flappy_pipe[i]->y--;
                }

            }
        }
        parity_counter--;
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
