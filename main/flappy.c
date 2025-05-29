#include "flappy.h"

void game_flappy(spi_device_handle_t spi) {
restart_game:
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    fill_rect(spi, 60, 0, 260, 240, 0x4DF9);
    fill_rect(spi, 0, 0, 38, 240, 0xD6B1);
    uint16_t player_record = load_nvs_u16("flappy");
    uint16_t player_score = 0;

    int received_button;
    image_flappy_up.x = 150;
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
        .x = 160,
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

    // буфер для отправки числа на экран
    uint16_t p_text[3];
    uint16_t p_pos = 0;

    uint8_t limit_jump = 0;

    uint16_t random_pipe_height = 30 + (esp_random() % 131);
    pipe_width[0] = random_pipe_height;
    images_flappy_pipe[0]->x = DISPLAY_WIDTH - pipe_width[0];
    pipe_width[1] = DISPLAY_WIDTH - 130 - pipe_width[0];

    pipe_counter[0] = images_flappy_pipe[0]->height;
    pipe_counter[1] = images_flappy_pipe[1]->height;

    while (1) {
        image_flappy_up.x--;
        if (limit_jump != 0) {
            if (image_flappy_up.x < DISPLAY_WIDTH-4-image_flappy_up.width) {
                image_flappy_up.x += 2;
            }
            limit_jump -= 2;
        }
        fill_rect(spi, image_flappy_up.x+image_flappy_up.width, image_flappy_up.y, 1, image_flappy_up.height, 0x4DF9);
        fill_rect(spi, image_flappy_up.x-1, image_flappy_up.y, 1, image_flappy_up.height, 0x4DF9);
        if (parity_counter == 0) { parity_counter = 50; };
        if (flappy_counter == 18) { flappy_counter = 0; };
        if (check_collision_rect(image_flappy_up.x, image_flappy_up.y, image_flappy_up.width, image_flappy_up.height,
                                 images_flappy_pipe[0]->x, images_flappy_pipe[0]->y, pipe_width[0], images_flappy_pipe[0]->height) ||
            check_collision_rect(image_flappy_up.x, image_flappy_up.y, image_flappy_up.width, image_flappy_up.height,
                                 60, images_flappy_pipe[1]->y, pipe_width[1], images_flappy_pipe[1]->height) ||
            image_flappy_up.x <= 60) {
            fill_screen(spi, 0xFFFF);
            draw_text(spi, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2-24, u"СЧЕТ", 0xCCCC, 1);
            player_score = (player_score != 0) ? player_score / 2 : 0;
            // Формируем текст для игрока
            if (player_score >= 100) p_text[p_pos++] = u'0' + (player_score / 100 % 10);
            if (player_score >= 10 || p_pos > 0) p_text[p_pos++] = u'0' + ((player_score / 10) % 10);
            p_text[p_pos++] = u'0' + (player_score % 10);
            p_text[p_pos] = 0;
            draw_text(spi, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2+6, p_text, 0xCCCC, 1);
            if (player_score > player_record) {
                draw_text(spi, DISPLAY_WIDTH/2-10, DISPLAY_HEIGHT/2-39, u"НОВЫЙ РЕКОРД!", 0xCCCC, 1);
                save_nvs_u16("flappy", player_score);
            } else {
                draw_text(spi, DISPLAY_WIDTH/2-10, DISPLAY_HEIGHT/2-30, u"РЕКОРД", 0xCCCC, 1);
                p_pos = 0;
                if (player_record >= 100) p_text[p_pos++] = u'0' + (player_record / 100 % 10);
                if (player_record >= 10 || p_pos > 0) p_text[p_pos++] = u'0' + ((player_record / 10) % 10);
                p_text[p_pos++] = u'0' + (player_record % 10);
                p_text[p_pos] = 0;
                draw_text(spi, DISPLAY_WIDTH/2-10, DISPLAY_HEIGHT/2+12, p_text, 0xCCCC, 1);
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
            goto restart_game;
        }
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
                        random_pipe_height = 30 + (esp_random() % 131);
                        pipe_width[0] = random_pipe_height;
                        images_flappy_pipe[0]->x = DISPLAY_WIDTH - pipe_width[0];
                        pipe_width[1] = DISPLAY_WIDTH - 130 - pipe_width[0];
                        images_flappy_pipe[i]->y = 239;
                        pipe_counter[i] = images_flappy_pipe[i]->height;
                        player_score++;
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
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (limit_jump == 0) {
                limit_jump = 50;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
