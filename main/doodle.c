#include "doodle.h"
#include "font.h"

void game_doodle(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    fill_screen(spi, 0xFFFF);
    draw_image_part(spi, &image_doodle_hero, 20, 20, 20, 20);

    int received_button;
    uint8_t speed = 3;
    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (received_button == BUTTON_UP) {
                image_doodle_hero.y = (image_doodle_hero.y > speed) ? image_doodle_hero.y - speed : 0;
                draw_image_part(spi, &image_doodle_hero, 20, 20, 20, 20);
            }
            else if (received_button == BUTTON_DOWN) {
                image_doodle_hero.y = (image_doodle_hero.y < DISPLAY_HEIGHT - image_doodle_hero.height - speed) ?
                          image_doodle_hero.y + speed : DISPLAY_HEIGHT - image_doodle_hero.height;
                draw_image_part(spi, &image_doodle_hero, 20, 20, 20, 20);
            }

            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}
