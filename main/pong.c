#include "pong.h"

void game_pong(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));
    // Передвижение персонажа
    int received_button = 0;

    image_kunglao_1.x = 100;
    image_kunglao_1.y = 100;
    uint8_t speed = 1;
    draw_image(spi, &image_background_forest_1);
    draw_image_background(spi, &image_kunglao_1, image_background_forest_pixels_1);
    while (1) {
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (received_button == BUTTON_UP) {
                image_kunglao_1.y -= speed;
            }
            if (received_button == BUTTON_DOWN) {
                image_kunglao_1.y += speed;
            }
            if (received_button == BUTTON_LEFT) {
                image_kunglao_1.x -= speed;
            }
            if (received_button == BUTTON_RIGHT) {
                image_kunglao_1.x += speed;
            }

            // Ограничиваем координаты
            image_kunglao_1.x = ((uint16_t) (image_kunglao_1.x + speed) <= speed) ? 0 : (image_kunglao_1.x > DISPLAY_WIDTH - image_kunglao_1.width) ? DISPLAY_WIDTH - image_kunglao_1.width : image_kunglao_1.x;
            image_kunglao_1.y = ((uint16_t) (image_kunglao_1.y + speed) <= speed) ? 0 : (image_kunglao_1.y > DISPLAY_HEIGHT - image_kunglao_1.height) ? DISPLAY_HEIGHT - image_kunglao_1.height : image_kunglao_1.y;
            draw_image_background(spi, &image_kunglao_1, image_background_forest_pixels_1);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);  // 10 (~100 FPS)
    }
}
