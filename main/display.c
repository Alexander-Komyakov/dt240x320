#include "display.h"
#include "spi.h"

void draw_border(spi_device_handle_t spi, const Image *my_image, uint8_t border_size, uint16_t color) {
    uint16_t half_size = border_size / 2;
    fill_rect(spi, my_image->x, 36, my_image->width, half_size, color);
    fill_rect(spi, my_image->x, 100, my_image->width, half_size, color);
    fill_rect(spi, my_image->x-half_size, my_image->y-half_size, half_size, my_image->height+border_size, color);
    fill_rect(spi, my_image->x+my_image->width, my_image->y-half_size, half_size, my_image->height+border_size, color);
}

void draw_image(spi_device_handle_t spi, const Image *my_image) {
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
