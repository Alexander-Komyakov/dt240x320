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
restart_game:
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));
    srand(xTaskGetTickCount());

    uint8_t i;
    Platform platforms[MAX_PLATFORM];
    for (i = 0; i < 10; i++) {
        platforms[i].x = 32*i;
        platforms[i].y = rand() % 184;
        platforms[i].prev_x = 32*i-3;
        platforms[i].prev_y = 0;
        platforms[i].type = 0;
        platforms[i].visible = 1;
    }
    for (i = 10; i < 20; i++) {
        platforms[i].x = 0;
        platforms[i].y = 0;
        platforms[i].prev_x = 0;
        platforms[i].prev_y = 0;
        platforms[i].type = 0;
        platforms[i].visible = 0;
    }

    image_doodle_hero.x = DISPLAY_WIDTH-80;
    image_doodle_hero.y = DISPLAY_HEIGHT/2;
    fill_screen(spi, 0xFFFF);
    draw_image(spi, &image_doodle_hero);

    int received_button;
    uint8_t speed = 3;
    uint8_t speed_jump_up = 2;
    uint8_t speed_jump_down = 3;
    uint8_t limit_jump = 60;

    uint8_t count_new_platform = 0;
    uint8_t overlap_count = 0;

    // счетчик пустых линий платформ для генератора
    uint8_t count_zero_platform = 0;

    uint16_t death_limit = DISPLAY_WIDTH;
    uint16_t player_score = 0;
    save_nvs_u16("doodle", 300);
    uint16_t player_record = load_nvs_u8("doodle");

    // буфер для отправки числа на экран
    uint16_t p_text[5];
    uint16_t p_pos = 0;

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
                player_score++;
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
                uint8_t rand_count_platform = 0;
                uint8_t rand_type_platform = 0;
                if (count_new_platform >= NEW_PLATFORM) {
                    rand_count_platform = rand() % 100;
                    rand_count_platform = (rand_count_platform < 10) ? 1 : (rand_count_platform < 20) ? 2: 0;
                    count_zero_platform = rand_count_platform == 0 ? count_zero_platform + 1     : 0;
                    if (count_zero_platform >= 3) {
                        rand_count_platform = 1;
                        count_zero_platform = 0;
                    }
                    count_new_platform = 0;
                    // генерируем новые платформы
                    for (i = 0; i < MAX_PLATFORM; i++) {
                        if (count_new_platform == rand_count_platform) {
                            break;
                        }
                        if (platforms[i].visible == 0) {
                            count_new_platform += 1;
                            rand_type_platform = rand() % 100;
                            if (rand_count_platform == 1) {
                                platforms[i].type = (rand_type_platform < 30) ? 0 : ((rand_type_platform < 90) ? 2 : 3);
                                // DISPLAY_HEIGHT / 2 - image_platform.height (240/2) - 56 = 184
                                platforms[i].y = rand() % 184;
                            } else {
                                // Левая или правая части экрана
                                platforms[i].type = (rand_type_platform < 60) ? 0 : 3;
                                platforms[i].y = (rand() % 64) + (120*(count_new_platform-1));
                            }
                            platforms[i].visible = 1;
                            platforms[i].x = DISPLAY_WIDTH-image_platform.width;
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
                // 8 размер ног
                // 7 - отступ от края до ноги дудлика
                // 18 - отступ от края до ноги со стороны носа
                if (check_collision_rect(image_doodle_hero.x+3, image_doodle_hero.y + 7, 8, image_doodle_hero.height - 18,
                                platforms[i].x+8, platforms[i].y+15, image_platform.width-12, image_platform.height-21) && platforms[i].visible) {
                    limit_jump = 90;
                    if (platforms[i].type == 3) {
                        platforms[i].visible = 0;
                        fill_rect(spi, platforms[i].x, platforms[i].y, image_platform.width, image_platform.height, 0xFFFF);
                    }
                }
            }
        } else {
            // смерть
            limit_jump = 0;
            if (death_limit <= 5) {
                fill_rect(spi, DISPLAY_WIDTH-30, 0, 30, DISPLAY_HEIGHT, 0xFFFF);
                draw_text(spi, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2-24, u"СЧЕТ", 0xCCCC, 1);
                // Формируем текст для игрока
                if (player_score >= 10000) p_text[p_pos++] = u'0' + (player_score / 10000 % 10);
                if (player_score >= 1000) p_text[p_pos++] = u'0' + (player_score / 1000 % 10);
                if (player_score >= 100) p_text[p_pos++] = u'0' + (player_score / 100 % 10);
                if (player_score >= 10 || p_pos > 0) p_text[p_pos++] = u'0' + ((player_score / 10) % 10);
                p_text[p_pos++] = u'0' + (player_score % 10);
                p_text[p_pos] = 0;
                draw_text(spi, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2+6, p_text, 0xCCCC, 1);
                if (player_score > player_record) {
                    draw_text(spi, DISPLAY_WIDTH/2-10, DISPLAY_HEIGHT/2-39, u"НОВЫЙ РЕКОРД!", 0xCCCC, 1);
                    save_nvs_u16("doodle", player_score);
                } else {
                    draw_text(spi, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2-18, u"РЕКОРД", 0xCCCC, 1);
                    if (player_record >= 10000) p_text[p_pos++] = u'0' + (player_record / 10000 % 10);
                    if (player_record >= 1000) p_text[p_pos++] = u'0' + (player_record / 1000 % 10);
                    if (player_record >= 100) p_text[p_pos++] = u'0' + (player_record / 100 % 10);
                    if (player_record >= 10 || p_pos > 0) p_text[p_pos++] = u'0' + ((player_record / 10) % 10);
                    p_text[p_pos++] = u'0' + (player_record % 10);
                    p_text[p_pos] = 0;
                    draw_text(spi, DISPLAY_WIDTH/2, DISPLAY_HEIGHT/2+30, p_text, 0xCCCC, 1);
                }
                vTaskDelay(3000 / portTICK_PERIOD_MS);
                goto restart_game;
            } else {
                death_limit -= 3;
                for (i = 0; i < MAX_PLATFORM; i++) {
                    if (platforms[i].visible == 1) {
                        if (platforms[i].x < DISPLAY_WIDTH-image_platform.width) {
                            platforms[i].x += 3;
                        } else {
                            platforms[i].visible = 0;
                            fill_rect(spi, DISPLAY_WIDTH-image_platform.width, platforms[i].y, image_platform.width, image_platform.height, 0xFFFF);
                        }
                    }
                }
            }
        }
        // Проверка столкновений с платформами для прозрачности
        overlap_count = 0;
        // пересечение максимум с 6 платформами
        Image overlap_images[6];
        for (i = 0; i < MAX_PLATFORM; i++) {
            if (!platforms[i].visible) {
                continue;
            }
            if (platforms[i].type == 1) {
                if (platforms[i].y < DISPLAY_HEIGHT-image_platform.height)
                    platforms[i].y += 2;
                else {
                    platforms[i].type = 2;
                }
            } else if (platforms[i].type == 2) {
                if (platforms[i].y >= 2) {
                    platforms[i].y -= 2;
                } else {
                    platforms[i].type = 1;
                }
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
                if (platforms[i].type == 3) {
                    overlap_images[overlap_count-1].pixels = image_platform_white.pixels;
                } else if (platforms[i].type == 1 || platforms[i].type == 2) {
                    overlap_images[overlap_count-1].pixels = image_platform_blue.pixels;
                }
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
                if (platforms[i].type == 3) {
                    temp_image.pixels = image_platform_white.pixels;
                } else if (platforms[i].type == 1 || platforms[i].type == 2) {
                    temp_image.pixels = image_platform_blue.pixels;
                }
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
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
