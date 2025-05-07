#include "doodle.h"
#include "font.h"

void game_doodle(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    fill_screen(spi, 0xFFFF);
    draw_image(spi, &image_doodle_hero);
    draw_image(spi, &image_platform);

    int received_button;
    uint8_t speed = 3;
    uint8_t speed_jump = 1;
    uint8_t limit_jump = 80;
    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (received_button == BUTTON_UP) {
                if (image_doodle_hero.y >= speed) {
                    image_doodle_hero.y = image_doodle_hero.y - speed;
                } else {
                    image_doodle_hero.y = DISPLAY_HEIGHT + 15;
                }
                draw_image(spi, &image_doodle_hero);
            }
            else if (received_button == BUTTON_DOWN) {
                if (image_doodle_hero.y + speed < DISPLAY_HEIGHT + 15) {
                    image_doodle_hero.y = image_doodle_hero.y + speed;
                } else {
                    image_doodle_hero.y = 0;
                }
                draw_image(spi, &image_doodle_hero);
            }
        }
        if (limit_jump != 0) {
            image_doodle_hero.x += speed_jump;
            limit_jump -= speed_jump;
        } else if (image_doodle_hero.x > 0) {
            image_doodle_hero.x -= speed_jump;
        } else {
            limit_jump = 80;
        }
        draw_image(spi, &image_platform);
        draw_image(spi, &image_doodle_hero);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
