#include "display.h"
#include "spi.h"


void draw_image_composite_slave(spi_device_handle_t spi, const Image *main_image, const Image *overlap_images) {
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {
        main_image->x >> 8,
        main_image->x & 0xFF,
        (main_image->x + main_image->width - 1) >> 8,
        (main_image->x + main_image->width - 1) & 0xFF
    };
    send_data(spi, col_data, 4);

    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {
        0,
        main_image->y & 0xFF,
        0,
        ((main_image->y + main_image->height) > DISPLAY_HEIGHT)
            ? (main_image->height - (DISPLAY_HEIGHT - main_image->y)) & 0xFF
            : (main_image->y + main_image->height) & 0xFF
    };

    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);

    // Создаем DMA буфер
    uint16_t *dma_buffer = heap_caps_malloc(
        main_image->size_image * sizeof(uint16_t),
        MALLOC_CAP_DMA | MALLOC_CAP_32BIT
    );
    // Проверяем доступность памяти 
    if (!dma_buffer) {
        ESP_LOGE("DMA", "Не хватило памяти!");
        return;
    }

    memcpy(dma_buffer, main_image->pixels, main_image->size_image * sizeof(uint16_t));

    // Вычисляем область пересечения
    int16_t x_start = MAX(main_image->x, overlap_images->x);
    int16_t x_end = MIN(main_image->x + main_image->width, overlap_images->x + overlap_images->width);
    int16_t y_start = MAX(main_image->y, overlap_images->y);
    int16_t y_end = MIN(main_image->y + main_image->height, overlap_images->y + overlap_images->height);
    
    if (x_start >= x_end || y_start >= y_end) return;
    
    for (int16_t y = y_start; y < y_end; y++) {
        for (int16_t x = x_start; x < x_end; x++) {
            uint16_t main_x = x - main_image->x;
            uint16_t main_y = y - main_image->y;
            uint16_t main_idx = main_y * main_image->width + main_x;
            
            uint16_t overlap_x = x - overlap_images->x;
            uint16_t overlap_y = y - overlap_images->y;
            
            // Полная проверка границ:
            if (overlap_x < overlap_images->width && 
                overlap_y < overlap_images->height &&
                (overlap_y * overlap_images->width + overlap_x) < overlap_images->size_image) 
            {
                if (overlap_images->pixels[overlap_y * overlap_images->width + overlap_x] != 0xFFFF) {
                    dma_buffer[main_idx] = overlap_images->pixels[overlap_y * overlap_images->width + overlap_x];
                }
            }
        }
    }

    send_data16b(spi, dma_buffer, main_image->size_image);
    free(dma_buffer);
}

void draw_image_composite(spi_device_handle_t spi, const Image *main_image, const Image *overlap_images, uint8_t overlap_count) {
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {
        main_image->x >> 8,
        main_image->x & 0xFF,
        (main_image->x + main_image->width - 1) >> 8,
        (main_image->x + main_image->width - 1) & 0xFF
    };
    send_data(spi, col_data, 4);

    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {
        0,
        main_image->y & 0xFF,
        0,
        ((main_image->y + main_image->height) > DISPLAY_HEIGHT)
            ? (main_image->height - (DISPLAY_HEIGHT - main_image->y)) & 0xFF
            : (main_image->y + main_image->height) & 0xFF
    };

    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);

    // Создаем DMA буфер
    uint16_t *dma_buffer = heap_caps_malloc(
        main_image->size_image * sizeof(uint16_t),
        MALLOC_CAP_DMA | MALLOC_CAP_32BIT
    );
    // Проверяем доступность памяти 
    if (!dma_buffer) {
        ESP_LOGE("DMA", "Не хватило памяти!");
        return;
    }

    memcpy(dma_buffer, main_image->pixels, main_image->size_image * sizeof(uint16_t));


    for (uint8_t i = 0; i < overlap_count; i++) {
        const Image* overlap = &overlap_images[i]; // Важно: берём текущий элемент
        
        // Вычисляем область пересечения
        int16_t x_start = MAX(main_image->x, overlap->x);
        int16_t x_end = MIN(main_image->x + main_image->width, overlap->x + overlap->width);
        int16_t y_start = MAX(main_image->y, overlap->y);
        int16_t y_end = MIN(main_image->y + main_image->height, overlap->y + overlap->height);
    
        if (x_start >= x_end || y_start >= y_end) continue;
    
        for (int16_t y = y_start; y < y_end; y++) {
            for (int16_t x = x_start; x < x_end; x++) {
                uint16_t main_x = x - main_image->x;
                uint16_t main_y = y - main_image->y;
                uint16_t main_idx = main_y * main_image->width + main_x;
                
                uint16_t overlap_x = x - overlap->x;
                uint16_t overlap_y = y - overlap->y;
                
                // Полная проверка границ:
                if (overlap_x < overlap->width && 
                    overlap_y < overlap->height &&
                    (overlap_y * overlap->width + overlap_x) < overlap->size_image) 
                {
                    if (dma_buffer[main_idx] == 0xFFFF) {
                        dma_buffer[main_idx] = overlap->pixels[overlap_y * overlap->width + overlap_x];
                    }
                }
            }
        }
    }

    send_data16b(spi, dma_buffer, main_image->size_image);
    free(dma_buffer);
}

void draw_image_part(spi_device_handle_t spi, const Image *my_image,
                    uint16_t src_x, uint16_t src_y,
                    uint16_t part_width, uint16_t part_height) {

    // Установка области вывода по X (столбцы)
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {
        (my_image->x + src_x) >> 8,
        (my_image->x + src_x) & 0xFF,
        (my_image->x + src_x + part_width - 1) >> 8,
        (my_image->x + src_x + part_width - 1) & 0xFF
    };
    send_data(spi, col_data, 4);

    // Установка области вывода по Y (строки)
    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {
        (my_image->y + src_y) >> 8,           // Старший байт начального Y
        (my_image->y + src_y) & 0xFF,         // Младший байт начального Y
        (my_image->y + src_y + part_height - 1) >> 8,  // Старший байт конечного Y
        (my_image->y + src_y + part_height - 1) & 0xFF // Младший байт конечного Y
    };
    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);

    uint16_t *dma_buffer = heap_caps_malloc(part_width * part_height * sizeof(uint16_t), MALLOC_CAP_DMA);

    // Построчное копирование с правильными смещениями
    for (uint16_t y = 0; y < part_height; y++) {
        uint32_t src_offset = (src_y + y) * my_image->width + src_x;
        uint32_t dst_offset = y * part_width;
        memcpy(dma_buffer + dst_offset,
               my_image->pixels + src_offset,
               part_width * sizeof(uint16_t));
    }

    send_data16b(spi, dma_buffer, part_width * part_height);
    free(dma_buffer);
}

void draw_border(spi_device_handle_t spi, const Image *my_image, uint8_t border_size, uint16_t color) {
    uint16_t half_size = border_size / 2;
    fill_rect(spi, my_image->x, my_image->y-half_size, my_image->width, half_size, color);
    fill_rect(spi, my_image->x, my_image->y+my_image->height, my_image->width, half_size, color);
    fill_rect(spi, my_image->x-half_size, my_image->y-half_size, half_size, my_image->height+border_size, color);
    fill_rect(spi, my_image->x+my_image->width, my_image->y-half_size, half_size, my_image->height+border_size, color);
}

void draw_image(spi_device_handle_t spi, const Image *my_image) {
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {my_image->x >> 8, my_image->x & 0xFF, (my_image->x - 1 + my_image->width) >> 8, (my_image->x - 1 + my_image->width) & 0xFF};
    send_data(spi, col_data, 4);

    send_command(spi, CMD_ROW);
    // Правки для скролинга
    // если разрешено рисовать объект от конца экрана к началу
    uint8_t row_data[4] = {
        0, 
        my_image->y & 0xFF,
        0, 
        ((my_image->y + my_image->height) > DISPLAY_HEIGHT) 
            ? (my_image->height - (DISPLAY_HEIGHT - my_image->y)) & 0xFF
            : (my_image->y + my_image->height) & 0xFF
    };
    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);

    // Создаем DMA буфер
    uint16_t *dma_buffer = heap_caps_malloc(
        my_image->size_image * sizeof(uint16_t),
        MALLOC_CAP_DMA | MALLOC_CAP_32BIT
    );
    // Проверяем доступность памяти
    if (!dma_buffer) {
        ESP_LOGE("DMA", "Не хватило памяти!");
        return;
    }

    // Копируем данные из flash в RAM
    memcpy(dma_buffer, my_image->pixels, my_image->size_image * sizeof(uint16_t));
    send_data16b(spi, dma_buffer, my_image->size_image);
    free(dma_buffer);
}

void draw_image_background(spi_device_handle_t spi, const Image *my_image, const uint16_t *background) {
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {my_image->x >> 8, my_image->x & 0xFF, (my_image->x - 1 + my_image->width) >> 8, (my_image->x - 1 + my_image->width) & 0xFF};
    send_data(spi, col_data, 4);

    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {0, my_image->y & 0xFF, 0, (my_image->y - 1 + my_image->height) & 0xFF};
    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);

    // Создаем DMA буфер
    uint16_t *dma_buffer = heap_caps_malloc(
        my_image->size_image * sizeof(uint16_t),
        MALLOC_CAP_DMA
    );
    // Проверяем доступность памяти
    if (!dma_buffer) {
        ESP_LOGE("DMA", "Не хватило памяти!");
        return;
    }

    for (int i = 0; i < my_image->size_image; i++) {
        if (my_image->pixels[i] == 0xFFFF) {
            // Если пиксель прозрачный, берем фон
            int bg_x = my_image->x + (i % my_image->width);
            int bg_y = my_image->y + (i / my_image->width);
            dma_buffer[i] = background[bg_y * DISPLAY_WIDTH + bg_x];
        } else {
            // Иначе берем пиксель из спрайта
            dma_buffer[i] = my_image->pixels[i];
        }
    }

    send_data16b(spi, dma_buffer, my_image->size_image);
    free(dma_buffer);
}

void fill_rect(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {x >> 8, x & 0xFF, (x + width - 1) >> 8, (x + width - 1) & 0xFF};
    send_data(spi, col_data, 4);

    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {0, y & 0xFF, 0, (y + height - 1) & 0xFF};
    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);

    uint32_t pixel_count = width * height;
    uint32_t buf_size = pixel_count * 2; // 2 bytes per pixel
    uint8_t *pixel_buf = heap_caps_malloc(buf_size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
    
    if (!pixel_buf) {
        ESP_LOGE("FILL_RECT", "Failed to allocate memory");
        return;
    }

    for (uint32_t i = 0; i < pixel_count; i++) {
        pixel_buf[i * 2] = color >> 8;     // Старший байт
        pixel_buf[i * 2 + 1] = color & 0xFF; // Младший байт
    }

    send_data(spi, pixel_buf, buf_size);
    free(pixel_buf);
}

void fill_screen(spi_device_handle_t spi, uint16_t color) {
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {0, 0, DISPLAY_WIDTH >> 8, DISPLAY_WIDTH & 0xFF};
    send_data(spi, col_data, 4);

    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {0, 0, DISPLAY_HEIGHT >> 8, DISPLAY_HEIGHT & 0xFF};
    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);

    uint32_t buf_size = 240 * 320 * 2; // 240x320 пикселей, 2 байта на пиксель
    uint8_t *pixel_buf = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);

    for (int i = 0; i < 240 * 320; i++) {
        pixel_buf[i * 2] = color >> 8;   // Старший байт
        pixel_buf[i * 2 + 1] = color & 0xFF; // Младший байт
    }

    send_data(spi, pixel_buf, buf_size);

    free(pixel_buf);
}

void fill_screen_gradient(spi_device_handle_t spi, uint16_t color_start, uint16_t color_end) {
    // Устанавливаем область заполнения (весь экран)
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {0, 0, DISPLAY_WIDTH >> 8, DISPLAY_WIDTH & 0xFF};
    send_data(spi, col_data, 4);
    
    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {0, 0, DISPLAY_HEIGHT >> 8, DISPLAY_HEIGHT & 0xFF};
    send_data(spi, row_data, 4);
    
    send_command(spi, CMD_SET_PIXEL);

    // Разбираем начальный и конечный цвета на компоненты
    uint8_t r1 = (color_start >> 11) & 0x1F;
    uint8_t g1 = (color_start >> 5) & 0x3F;
    uint8_t b1 = color_start & 0x1F;
    
    uint8_t r2 = (color_end >> 11) & 0x1F;
    uint8_t g2 = (color_end >> 5) & 0x3F;
    uint8_t b2 = color_end & 0x1F;

    // Выделяем буфер для всех пикселей
    uint32_t buf_size = DISPLAY_WIDTH * DISPLAY_HEIGHT * 2;
    uint8_t *pixel_buf = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);

    // Заполняем буфер градиентом
    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        // Вычисляем коэффициенты интерполяции для текущей строки
        float ratio = (float)y / (DISPLAY_HEIGHT - 1);
        uint8_t r = r1 + (r2 - r1) * ratio;
        uint8_t g = g1 + (g2 - g1) * ratio;
        uint8_t b = b1 + (b2 - b1) * ratio;
        
        // Формируем цвет для всей строки
        uint16_t line_color = (r << 11) | (g << 5) | b;
        
        // Заполняем строку в буфере
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            int idx = (y * DISPLAY_WIDTH + x) * 2;
            pixel_buf[idx] = line_color >> 8;
            pixel_buf[idx + 1] = line_color & 0xFF;
        }
    }

    send_data(spi, pixel_buf, buf_size);
    free(pixel_buf);
}

void draw_pixel(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t color) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) return;
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {x >> 8, x & 0xFF, x >> 8, x & 0xFF}; // Начало и конец
    send_data(spi, col_data, 4);

    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {y >> 8, y & 0xFF, y >> 8, y & 0xFF}; // Начало и конец
    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);
    uint8_t pixel_data[2] = {color >> 8, color & 0xFF}; // Цвет в формате RGB565
    send_data(spi, pixel_data, 2);
}


void vertical_scroll(spi_device_handle_t spi, uint16_t* tfa, uint16_t* vsa, uint16_t* bfa, uint16_t* ssa) {
  send_command(spi, CMD_SCRLAR);

  send_command_no_dc(spi, ((*tfa >> 8) & 0xFF));
  send_command_no_dc(spi, (*tfa & 0xFF));
  send_command_no_dc(spi, ((*vsa >> 8) & 0xFF));
  send_command_no_dc(spi, (*vsa & 0xFF));
  send_command_no_dc(spi, ((*bfa >> 8) & 0xFF));
  send_command_no_dc(spi, (*bfa & 0xFF));

  send_command(spi, CMD_VSCSAD);
  send_command_no_dc(spi, ((*ssa >> 8) & 0xFF));
  send_command_no_dc(spi, (*ssa & 0xFF));
}

void init_display(spi_device_handle_t spi) {
    const size_t data = 1;
    send_command(spi, CMD_SOFTWARE_RESET);
    send_command(spi, CMD_SLEEP_OUT);
    send_command(spi, CMD_SET_RGB);
    send_data(spi, (uint8_t[]){0x05}, data);  //16-bit/pixel 65K-Colors(RGB 5-6-5-bit Input)

    send_command(spi, CMD_DISPLAY_ON);
    send_command(spi, CMD_NORMAL_MODE);

    uint8_t madctl_value = 0x70;
    send_command(spi, CMD_MADCTL);
    send_data(spi, &madctl_value, 1);
}
