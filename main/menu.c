#include "menu.h"


uint8_t menu(spi_device_handle_t spi, nvs_handle_t nvs_handle) {
    fill_screen_gradient(spi, 0xBBBB, 0xFFFF);

    uint8_t current_game = nvs_load_uint8_t(nvs_handle, "menu");

    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);
    draw_image(spi, &image_pong_preview);

    image_pong_preview.x = 127;
    draw_image(spi, &image_doodle_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.x = 224;
    draw_image(spi, &image_arkanoid_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.y = 140;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.x = 127;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    image_pong_preview.x = 30;
    draw_image(spi, &image_pong_preview);
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);

    if (current_game == 0) {
        image_pong_preview.x = 30;
        image_pong_preview.y = 40;
    } else if (current_game == 1) {
        image_pong_preview.x = 127;
        image_pong_preview.y = 40;
    } else if (current_game == 2) {
        image_pong_preview.x = 224;
        image_pong_preview.y = 40;
    } else if (current_game == 3) {
        image_pong_preview.x = 30;
        image_pong_preview.y = 140;
    } else if (current_game == 4) {
        image_pong_preview.x = 127;
        image_pong_preview.y = 140;
    } else if (current_game == 5) {
        image_pong_preview.x = 224;
        image_pong_preview.y = 140;
    }
    draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_GAME_COLOR);
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    int received_button = 0;

    uint32_t last_button_time = 0;
    int last_button = 99;
    uint32_t current_time;

    uint8_t old_current_game = current_game;

    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
            // Если кнопка изменилась или прошло достаточно времени с последнего нажатия
            if (received_button != last_button || (current_time - last_button_time) > DEBOUNCE_TIME_MS) {
                last_button_time = current_time;
                last_button = received_button;
                if (received_button == BUTTON_UP) {
                    if (current_game >= 3) {
                        current_game -= 3;
                        draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);
                        image_pong_preview.y = 40;
                    }
                } else if (received_button == BUTTON_DOWN) {
                    if (current_game <= 2) {
                        current_game += 3;
                        draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);
                        image_pong_preview.y = 140;
                    }
                } else if (received_button == BUTTON_RIGHT) {
                    if (current_game != 2 && current_game != 5) {
                        current_game += 1;
                        draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);
                        image_pong_preview.x += 97;
                    }
                } else if (received_button == BUTTON_LEFT) {
                    if (current_game != 0 && current_game != 3) {
                        current_game -= 1;
                        draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_BACKGROUND_COLOR);
                        image_pong_preview.x -= 97;
                    }
                } else {
                    nvs_save_uint8_t(nvs_handle, current_game, "menu");
                    return current_game;
                }
                draw_border(spi, &image_pong_preview, MENU_BORDER, MENU_GAME_COLOR);
                old_current_game = current_game;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
