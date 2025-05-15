#include "sonic.h"

// Структура с нулевыми значениями x, y
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

// Функция композитной отрисовки (теперь объявлена в sonic.h)
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

    // Заполняем фоном
    for (int16_t y = 0; y < scene_height; y++) {
        for (int16_t x = 0; x < scene_width; x++) {
            int16_t bg_x = scene_left + x;
            int16_t bg_y = scene_top + y;
            if (bg_x >= 0 && bg_x < DISPLAY_WIDTH && bg_y >= 0 && bg_y < DISPLAY_HEIGHT) {
                composite_buffer[y * scene_width + x] = image_background.pixels[bg_y * DISPLAY_WIDTH + bg_x];
            } else {
                composite_buffer[y * scene_width + x] = 0xFFFF;
            }
        }
    }

    // Рисуем pikachu
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

    // Рисуем sonic
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

    // Рисуем fighter поверх
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

    // Отправляем на дисплей
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

// Функция для вращения экрана
void rotate_screen_horizontal(spi_device_handle_t spi, uint16_t angle) {
    // Простейшая реализация горизонтального скроллинга
    static uint16_t offset = 0;
    offset = (offset + 5) % DISPLAY_WIDTH; // Шаг скроллинга
    
    // Создаем временный буфер для одной строки (для экономии памяти)
    uint16_t line_buffer[DISPLAY_WIDTH];
    
    // Устанавливаем область вывода
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {0, 0, (DISPLAY_WIDTH-1) >> 8, (DISPLAY_WIDTH-1) & 0xFF};
    send_data(spi, col_data, 4);
    
    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {0, 0, (DISPLAY_HEIGHT-1) >> 8, (DISPLAY_HEIGHT-1) & 0xFF};
    send_data(spi, row_data, 4);
    
    send_command(spi, CMD_SET_PIXEL);
    
    // Заполняем буфер со смещением
    for (uint16_t y = 0; y < DISPLAY_HEIGHT; y++) {
        for (uint16_t x = 0; x < DISPLAY_WIDTH; x++) {
            uint16_t src_x = (x + offset) % DISPLAY_WIDTH;
            line_buffer[x] = image_background.pixels[y * DISPLAY_WIDTH + src_x];
        }
        // Отправляем строку (в реальном коде нужно отправлять весь буфер)
        send_data16b(spi, line_buffer, DISPLAY_WIDTH);
    }
}

// Единая задача для анимации
void task_animation(void *pvParameters) {
    spi_device_handle_t spi = (spi_device_handle_t)pvParameters;
    const Image *fighter_frames[] = {&image_fighter1, &image_fighter2};
    const Image *sonic_frames[] = {&image_sonic1, &image_sonic2, &image_sonic3, 
                                  &image_sonic4, &image_sonic5, &image_sonic6};
    const Image *pikachu_frames[] = {&image_pikachu1, &image_pikachu2};

    uint8_t fighter_frame = 0;
    uint8_t sonic_frame = 0;
    uint8_t pikachu_frame = 0;
    uint8_t rotation_counter = 0;

    while (1) {
        // Каждый 10-й кадр делаем вращение
        if (rotation_counter++ >= 10) {
            rotate_screen_horizontal(spi, 5);
            rotation_counter = 0;
            continue;
        }

        // Стандартная анимация
        current_fighter = *fighter_frames[fighter_frame];
        current_fighter.x = FIGHTER_X;
        current_fighter.y = FIGHTER_Y;

        current_sonic = *sonic_frames[sonic_frame];
        current_sonic.x = SONIC_X;
        current_sonic.y = SONIC_Y;

        current_pikachu = *pikachu_frames[pikachu_frame];
        current_pikachu.x = PIKACHU_X;
        current_pikachu.y = PIKACHU_Y;

        draw_composite_scene(spi);

        fighter_frame = (fighter_frame + 1) % (sizeof(fighter_frames)/sizeof(fighter_frames[0]));
        sonic_frame = (sonic_frame + 1) % (sizeof(sonic_frames)/sizeof(sonic_frames[0]));
        pikachu_frame = (pikachu_frame + 1) % (sizeof(pikachu_frames)/sizeof(pikachu_frames[0]));

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

    draw_image(spi, &image_background);
    xTaskCreate(task_animation, "animation_task", 4096, spi, 1, NULL);
}
