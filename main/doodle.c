#include "doodle.h"
#include "font.h"


void draw_platform(spi_device_handle_t spi, const Platform* p, 
                     const Image* platform_normal) {
    Image temp_image = {
        .x = p->x,
        .y = p->y,
        .width = platform_normal->width,
        .height = platform_normal->height,
        .size_image = platform_normal->width*platform_normal->height,
        .pixels = platform_normal->pixels
    };
    draw_image(spi, &temp_image);
}

void game_doodle(spi_device_handle_t spi) {
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    Platform platforms[MAX_PLATFORM];
    for (uint8_t i = 0; i < 10; i++) {
        platforms[i].x = 10*i;
        platforms[i].y = 0;
        platforms[i].type = 0;
        platforms[i].visible = 1;
    }

    for (uint8_t i = 10; i < MAX_PLATFORM; i++) {
        platforms[i].x = 10*i;
        platforms[i].y = 60;
        platforms[i].type = 0;
        platforms[i].visible = 1;
    }

    fill_screen(spi, 0xFFFF);
    draw_image(spi, &image_doodle_hero);

    int received_button;
    uint8_t speed = 3;
    uint8_t speed_jump = 2;
    uint8_t limit_jump = 50;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10);
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
        for (uint8_t i = 0; i < MAX_PLATFORM; i++) {
            draw_platform(spi, &platforms[i], &image_platform);
        }
        draw_image(spi, &image_doodle_hero);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
