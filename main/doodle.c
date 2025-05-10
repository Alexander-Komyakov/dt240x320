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
        platforms[i].x = 32*i;
        platforms[i].y = 30;
        platforms[i].prev_x = 32*i;
        platforms[i].prev_y = 30;
        platforms[i].type = 0;
        platforms[i].visible = 1;
    }

    for (uint8_t i = 10; i < MAX_PLATFORM; i++) {
        platforms[i].x = 32*(i-10);
        platforms[i].y = 120;
        platforms[i].prev_x = 32*(i-10);
        platforms[i].prev_y = 120;
        platforms[i].type = 0;
        platforms[i].visible = 1;
    }

    fill_screen(spi, 0xFFFF);
    draw_image(spi, &image_doodle_hero);

    int received_button;
    uint8_t speed = 3;
    uint8_t speed_jump_up = 2;
    uint8_t speed_jump_down = 3;
    uint8_t limit_jump = 50;

    uint8_t count_new_platform = 0;

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
        for (uint8_t i = 0; i < MAX_PLATFORM; i++) {
            platforms[i].prev_x = platforms[i].x;
            platforms[i].prev_y = platforms[i].y;
        }
        if (limit_jump != 0) {
            // если дудлик ниже середины экрана
            if (image_doodle_hero.x <= (DISPLAY_WIDTH / 2)) {
                image_doodle_hero.x += speed_jump_up;
            } else {
                // движение всех платформ вниз
                for (uint8_t i = 0; i < MAX_PLATFORM; i++) {
                    if (platforms[i].visible == 1) {
                        if (platforms[i].x > speed_jump_up) {
                            platforms[i].x -= speed_jump_up;
                        } else {
                            platforms[i].visible = 0;
                            fill_rect(spi, platforms[i].x, platforms[i].y, image_platform.width, image_platform.height, 0xFFFF);
                        }
                    }
                }

                // генерация новых платформ
                count_new_platform += speed_jump_up;
                if (count_new_platform >= NEW_PLATFORM) {
                    // переиспользуем переменную как счетчик для количества платформ занимаемых
                    count_new_platform = 0;
                    // генерируем новые платформы
                    for (uint8_t i = 0; i < MAX_PLATFORM; i++) {
                        if (platforms[i].visible == 0) {
                            count_new_platform += 1;
                            platforms[i].visible = 1;
                            platforms[i].x = DISPLAY_WIDTH-image_platform.width;
                            platforms[i].y = count_new_platform * 60;
                            if (count_new_platform == 2) {
                                break;
                            }
                        }
                    }
                    // сбрасываем счетчик
                    count_new_platform = 0;
                }
            }

            limit_jump -= speed_jump_up;
        } else if (image_doodle_hero.x - speed_jump_down > 0) {
            image_doodle_hero.x -= speed_jump_down;
            // проверка столкновения
            for (uint8_t i = 0; i < MAX_PLATFORM; i++) {
                // 3 - белая рамка картинки
                // 7 - отступ от края до ноги дудлика
                // 18 - отступ от края до ноги со стороны носа
                if (check_collision_rect(image_doodle_hero.x, image_doodle_hero.y + 7, image_doodle_hero.width - 3, image_doodle_hero.height - 18,
                                platforms[i].x, platforms[i].y, image_platform.width, image_platform.height))
                        limit_jump = 80;
            }
        } else {
            limit_jump = 80;
        }
        for (uint8_t i = 0; i < MAX_PLATFORM; i++) {
            if (platforms[i].visible) {
                if (platforms[i].prev_x != platforms[i].x || 
                    platforms[i].prev_y != platforms[i].y) {
                    fill_rect(spi, platforms[i].x + image_platform.width, platforms[i].prev_y, platforms[i].prev_x - platforms[i].x, image_platform.height, 0xFFFF);
                }
                draw_platform(spi, &platforms[i], &image_platform);
            }
        }
        draw_image(spi, &image_doodle_hero);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
