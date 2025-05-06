#include "doodle.h"
#include "font.h"

void game_doodle(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    fill_screen(spi, 0xFFFF);
    draw_image(spi, &image_doodle_hero);

    int received_button;
    uint8_t speed = 1;
    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (received_button == BUTTON_UP) {
                if (image_doodle_hero.y > speed) {
                    image_doodle_hero.y = image_doodle_hero.y - speed;
                } else {
                    image_doodle_hero.y = DISPLAY_HEIGHT + 15;
                }
                draw_image(spi, &image_doodle_hero);
            }
            else if (received_button == BUTTON_DOWN) {
                if (image_doodle_hero.y < DISPLAY_HEIGHT + 15) {
                    image_doodle_hero.y = image_doodle_hero.y + speed;
                } else {
                    image_doodle_hero.y = 0;
                }
                draw_image(spi, &image_doodle_hero);
            }
            vTaskDelay(5 / portTICK_PERIOD_MS);
        }
    }
}
