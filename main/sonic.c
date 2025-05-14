#include "sonic.h"

// Структура с нулевыми значениями x, y. Переопределяется в цикле при использовании task_animation
Image current_fighter = {0};
Image current_sonic = {0};
Image current_pikachu = {0};

// Буфер для композитного изображения
static uint16_t *composite_buffer = NULL;
static size_t composite_buffer_size = 0;

// Функция инициализации буфера
void init_composite_buffer(uint16_t width, uint16_t height) {
    size_t needed_size = width * height * sizeof(uint16_t);
    if (composite_buffer_size < needed_size) {
        if (composite_buffer) free(composite_buffer);
        composite_buffer = heap_caps_malloc(needed_size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
        composite_buffer_size = needed_size;
    }
}

// Функция композитной отрисовки обоих объектов
void draw_composite_scene(spi_device_handle_t spi) {
    // Определяем границы всей сцены
    int16_t scene_left = MIN(current_fighter.x, MIN(current_sonic.x, current_pikachu.x));
    int16_t scene_right = MAX(current_fighter.x + current_fighter.width, 
                            MAX(current_sonic.x + current_sonic.width,
                               current_pikachu.x + current_pikachu.width));
    int16_t scene_top = MIN(current_fighter.y, MIN(current_sonic.y, current_pikachu.y));
    int16_t scene_bottom = MAX(current_fighter.y + current_fighter.height, 
                             MAX(current_sonic.y + current_sonic.height,
                                current_pikachu.y + current_pikachu.height));

    uint16_t scene_width = scene_right - scene_left;
    uint16_t scene_height = scene_bottom - scene_top;

    // Инициализируем буфер
    init_composite_buffer(scene_width, scene_height);
    if (!composite_buffer) return;

    // Заполняем фоном (белый цвет)
    memset(composite_buffer, 0xFF, scene_width * scene_height * sizeof(uint16_t));

	// В указанных циклах рисование происходит поочередно.
	// Это значит что чтобы отобразить передний слой, нужно указывать писать его в конце.

    // Рисуем pikachu (нижний слой)
    for (int16_t y = 0; y < current_pikachu.height; y++) {
        for (int16_t x = 0; x < current_pikachu.width; x++) {
            uint16_t pixel = current_pikachu.pixels[y * current_pikachu.width + x];
            if (pixel != TRANSPARENT_COLOR) {
                int16_t scene_x = current_pikachu.x - scene_left + x;
                int16_t scene_y = current_pikachu.y - scene_top + y;
                if (scene_x >= 0 && scene_x < scene_width && 
                    scene_y >= 0 && scene_y < scene_height) {
                    composite_buffer[scene_y * scene_width + scene_x] = pixel;
                }
            }
        }
    }

    // Рисуем sonic (нижний слой)
    for (int16_t y = 0; y < current_sonic.height; y++) {
        for (int16_t x = 0; x < current_sonic.width; x++) {
            uint16_t pixel = current_sonic.pixels[y * current_sonic.width + x];
            if (pixel != TRANSPARENT_COLOR) {
                int16_t scene_x = current_sonic.x - scene_left + x;
                int16_t scene_y = current_sonic.y - scene_top + y;
                if (scene_x >= 0 && scene_x < scene_width && 
                    scene_y >= 0 && scene_y < scene_height) {
                    composite_buffer[scene_y * scene_width + scene_x] = pixel;
                }
            }
        }
    }

    // Рисуем fighter поверх (верхний слой)
    for (int16_t y = 0; y < current_fighter.height; y++) {
        for (int16_t x = 0; x < current_fighter.width; x++) {
            uint16_t pixel = current_fighter.pixels[y * current_fighter.width + x];
            if (pixel != TRANSPARENT_COLOR) {
                int16_t scene_x = current_fighter.x - scene_left + x;
                int16_t scene_y = current_fighter.y - scene_top + y;
                if (scene_x >= 0 && scene_x < scene_width && 
                    scene_y >= 0 && scene_y < scene_height) {
                    composite_buffer[scene_y * scene_width + scene_x] = pixel;
                }
            }
        }
    }


    // Отправляем на дисплей всю сцену целиком
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {
        scene_left >> 8,
        scene_left & 0xFF,
        (scene_left + scene_width - 1) >> 8,
        (scene_left + scene_width - 1) & 0xFF
    };
    send_data(spi, col_data, 4);

    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {
        scene_top >> 8,
        scene_top & 0xFF,
        (scene_top + scene_height - 1) >> 8,
        (scene_top + scene_height - 1) & 0xFF
    };
    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);
    send_data16b(spi, composite_buffer, scene_width * scene_height);
}

// Единая задача для анимации обоих объектов
void task_animation(void *pvParameters) {
    spi_device_handle_t spi = (spi_device_handle_t)pvParameters;
    const Image *fighter_frames[] = {&image_fighter1, &image_fighter2};
    const Image *sonic_frames[] = {&image_sonic1, &image_sonic2, &image_sonic3, 
                                  &image_sonic4, &image_sonic5, &image_sonic6};
    const Image *pikachu_frames[] = {&image_pikachu1, &image_pikachu2};

    uint8_t fighter_frame = 0;
    uint8_t sonic_frame = 0;
	uint8_t pikachu_frame = 0;

    while (1) {
        // Обновляем кадры
        current_fighter = *fighter_frames[fighter_frame];
        current_fighter.x = FIGHTER_X;
        current_fighter.y = FIGHTER_Y;

        current_sonic = *sonic_frames[sonic_frame];
        current_sonic.x = SONIC_X;
        current_sonic.y = SONIC_Y;

		current_pikachu = *pikachu_frames[pikachu_frame];
        current_pikachu.x = PIKACHU_X;
        current_pikachu.y = PIKACHU_Y;

        // Рисуем всю сцену
        draw_composite_scene(spi);

        // Переключаем кадры // Равносильно старой записи - fighter_frame = (fighter_frame + 1) % 2;
		fighter_frame = (fighter_frame + 1) % (sizeof(fighter_frames) / sizeof(fighter_frames[0]));
		sonic_frame = (sonic_frame + 1) % (sizeof(sonic_frames) / sizeof(sonic_frames[0]));
		pikachu_frame = (pikachu_frame + 1) % (sizeof(pikachu_frames) / sizeof(pikachu_frames[0]));

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void sonic(spi_device_handle_t spi) {
    init_gpio_display();
    reset_display();
    spi_init(&spi);

    const size_t data = 1;
    send_command(spi, CMD_SOFTWARE_RESET);
    send_command(spi, CMD_SLEEP_OUT);
    send_command(spi, CMD_SET_RGB);
    send_data(spi, (uint8_t[]){0x05}, data);
    send_command(spi, CMD_DISPLAY_ON);
    send_command(spi, CMD_NORMAL_MODE);

    uint8_t madctl_value = 0xB0;
    send_command(spi, CMD_MADCTL);
    send_data(spi, &madctl_value, 1);

    fill_screen(spi, 0xFFFF);

    // Запускаем одну задачу вместо двух
    xTaskCreate(task_animation, "animation_task", 4096, spi, 1, NULL);
}
