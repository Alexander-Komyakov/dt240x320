#include "sonic.h"

// Структура с нулевыми значениями x, y
Image current_fighter = {0};
Image current_sonic = {0};
Image current_pikachu = {0};

// Буфер для композитного изображения
static uint16_t *composite_buffer = NULL;
static size_t composite_buffer_size = 0;

// Инициализация буфера
void init_composite_buffer(uint16_t width, uint16_t height) {
    size_t needed_size = width * height * sizeof(uint16_t);
    if (composite_buffer_size < needed_size) {
        if (composite_buffer) free(composite_buffer);
        composite_buffer = heap_caps_malloc(needed_size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
        composite_buffer_size = needed_size;
    }
}

// Отрисовка персонажа на композитном буфере
void draw_character(const Image *character) {
    for (int y = 0; y < character->height; y++) {
        for (int x = 0; x < character->width; x++) {
            uint16_t pixel = character->pixels[y * character->width + x];
            if (pixel != TRANSPARENT_COLOR) {
                int buf_x = character->x + x;
                int buf_y = character->y + y;
                if (buf_x >= 0 && buf_x < DISPLAY_WIDTH && 
                    buf_y >= 0 && buf_y < DISPLAY_HEIGHT) {
                    composite_buffer[buf_y * DISPLAY_WIDTH + buf_x] = pixel;
                }
            }
        }
    }
}

// Создание композитного кадра (фон + персонажи)
void prepare_composite_frame(uint16_t scroll_offset) {
    // Копируем фон со смещением
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            int src_x = (x + scroll_offset) % DISPLAY_WIDTH;
            composite_buffer[y * DISPLAY_WIDTH + x] = image_background.pixels[y * DISPLAY_WIDTH + src_x];
        }
    }

    // Рисуем персонажей поверх фона
//    draw_character(&current_pikachu);
//    draw_character(&current_sonic);
    draw_character(&current_fighter);
}

// Функция вращения экрана
void rotate_display(spi_device_handle_t spi, uint16_t speed) {
    static uint16_t scroll_offset = 0;
    scroll_offset = (scroll_offset + speed) % DISPLAY_WIDTH;

    // Подготавливаем кадр со смещением
    prepare_composite_frame(scroll_offset);

    // Отправляем на дисплей
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {0, 0, (DISPLAY_WIDTH-1) >> 8, (DISPLAY_WIDTH-1) & 0xFF};
    send_data(spi, col_data, 4);

    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {0, 0, (DISPLAY_HEIGHT-1) >> 8, (DISPLAY_HEIGHT-1) & 0xFF};
    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);
    send_data16b(spi, composite_buffer, DISPLAY_WIDTH * DISPLAY_HEIGHT);
}

// Задача анимации
void task_animation(void *pvParameters) {
    spi_device_handle_t spi = (spi_device_handle_t)pvParameters;
    const Image *fighter_frames[] = {&image_fighter1, &image_fighter2};
//    const Image *sonic_frames[] = {&image_sonic1, &image_sonic2, &image_sonic3, 
//                                  &image_sonic4, &image_sonic5, &image_sonic6};
//    const Image *pikachu_frames[] = {&image_pikachu1, &image_pikachu2};

    uint8_t frames[] = {0, 0, 0};
    const uint8_t frame_counts[] = {
        sizeof(fighter_frames)/sizeof(fighter_frames[0]),
//        sizeof(sonic_frames)/sizeof(sonic_frames[0]),
//        sizeof(pikachu_frames)/sizeof(pikachu_frames[0])
    };

    // Инициализация буфера
    init_composite_buffer(DISPLAY_WIDTH, DISPLAY_HEIGHT);

    while (1) {
        // Обновляем персонажей
        current_fighter = *fighter_frames[frames[0]];
        current_fighter.x = FIGHTER_X;
        current_fighter.y = FIGHTER_Y;

//        current_sonic = *sonic_frames[frames[1]];
//        current_sonic.x = SONIC_X;
//        current_sonic.y = SONIC_Y;

//        current_pikachu = *pikachu_frames[frames[2]];
//        current_pikachu.x = PIKACHU_X;
//        current_pikachu.y = PIKACHU_Y;

        // Вращаем экран
        rotate_display(spi, 3); // Скорость вращения

        // Переключаем кадры анимации
//        for (int i = 0; i < 3; i++) {
		for (int i = 0; i < 1; i++) {
            frames[i] = (frames[i] + 1) % frame_counts[i];
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void sonic(spi_device_handle_t spi) {
    init_gpio_display();
    reset_display();
    spi_init(&spi);

    // Инициализация дисплея
    send_command(spi, CMD_SOFTWARE_RESET);
    send_command(spi, CMD_SLEEP_OUT);
    send_command(spi, CMD_SET_RGB);
    send_data(spi, (uint8_t[]){0x05}, 1);
    send_command(spi, CMD_DISPLAY_ON);
    send_command(spi, CMD_NORMAL_MODE);

    uint8_t madctl_value = 0xB0;
    send_command(spi, CMD_MADCTL);
    send_data(spi, &madctl_value, 1);

    xTaskCreate(task_animation, "animation_task", 4096, spi, 1, NULL);
}
