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

    uint8_t i;
    Platform platforms[MAX_PLATFORM];
    for (i = 0; i < 10; i++) {
        platforms[i].x = 32*i;
        platforms[i].y = 30;
        platforms[i].prev_x = 32*i-3;
        platforms[i].prev_y = 27;
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
    uint8_t overlap_count = 0;

    while (1) {
        // Обработка ввода игрока
        if (xStreamBufferReceive(xStreamBuffer, &received_button, sizeof(received_button), 0) > 0) {
            if (received_button == BUTTON_UP) {
                if (image_doodle_hero.y >= speed) {
                    image_doodle_hero.y = image_doodle_hero.y - speed;
                } else {
                    image_doodle_hero.y = DISPLAY_HEIGHT + 15;
                }
            }
            else if (received_button == BUTTON_DOWN) {
                if (image_doodle_hero.y + speed < DISPLAY_HEIGHT + 15) {
                    image_doodle_hero.y = image_doodle_hero.y + speed;
                } else {
                    image_doodle_hero.y = 0;
                }
            }
        }
        if (limit_jump != 0) {
            // если дудлик ниже середины экрана
            if (image_doodle_hero.x <= (DISPLAY_WIDTH / 2)) {
                image_doodle_hero.x += speed_jump_up;
            } else {
                // движение всех платформ вниз
                for (i = 0; i < MAX_PLATFORM; i++) {
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
                    for (i = 0; i < MAX_PLATFORM; i++) {
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
            for (i = 0; i < MAX_PLATFORM; i++) {
                // 3 - белая рамка картинки
                // 7 - отступ от края до ноги дудлика
                // 18 - отступ от края до ноги со стороны носа
                if (check_collision_rect(image_doodle_hero.x, image_doodle_hero.y + 7, image_doodle_hero.width - 3, image_doodle_hero.height - 18,
                                platforms[i].x+8, platforms[i].y+15, image_platform.width-3, image_platform.height-21))
                        limit_jump = 80;
            }
        } else {
            limit_jump = 80;
        }
        // Проверка столкновений с платформами для прозрачности
        overlap_count = 0;
        // пересечение максимум с 6 платформами
        Image *overlap_images = (Image*)malloc(4*sizeof(Image));
        if (!overlap_images) {
            perror("malloc failed");
        }
        for (i = 0; i < MAX_PLATFORM; i++) {
            if (!platforms[i].visible) {
                continue;
            }
            if (check_collision_rect(image_doodle_hero.x, image_doodle_hero.y, image_doodle_hero.width, image_doodle_hero.height,
                                     platforms[i].x, platforms[i].y, image_platform.width, image_platform.height)) {
                overlap_images[overlap_count++] = (Image){
                    .x = platforms[i].x,
                    .y = platforms[i].y,
                    .width = image_platform.width,
                    .height = image_platform.height,
                    .size_image = image_platform.size_image,
                    .pixels = image_platform.pixels
                };
                // если столкнулись, то рисуем платформу за игроком
                draw_image_composite_slave(spi, &overlap_images[overlap_count-1], &image_doodle_hero);
            } else if (platforms[i].prev_x != platforms[i].x || 
                       platforms[i].prev_y != platforms[i].y) {
                // если не столкнулись и платформа сдвинулась, то просто рисуем
                Image temp_image = {
                    .x = platforms[i].x,      
                    .y = platforms[i].y,
                    .width = image_platform.width,
                    .height = image_platform.height,
                    .size_image = image_platform.size_image,
                    .pixels = image_platform.pixels
                };  
                draw_image(spi, &temp_image);
            }
            if(overlap_count >= MAX_PLATFORM) break;
        }
        if (overlap_count == 0) {
            draw_image(spi, &image_doodle_hero);
        } else {
            draw_image_composite(spi, &image_doodle_hero, overlap_images, overlap_count);
        }
        for (i = 0; i < MAX_PLATFORM; i++) {
            platforms[i].prev_x = platforms[i].x;
            platforms[i].prev_y = platforms[i].y;
        }
        free(overlap_images);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
