#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "main.h"
#include "image.c"
#include "esp_rom_gpio.h"
#include "freertos/stream_buffer.h"

#define STREAM_BUF_SIZE 32
StreamBufferHandle_t xStreamBuffer;


void init_gpio_display() {
	gpio_reset_pin( PIN_NUM_CS );
	gpio_set_direction( PIN_NUM_CS, GPIO_MODE_OUTPUT );
	gpio_set_level( PIN_NUM_CS, 0 );

	gpio_reset_pin( PIN_NUM_DC );
	gpio_set_direction( PIN_NUM_DC, GPIO_MODE_OUTPUT );
    gpio_set_level( PIN_NUM_DC, 0 );

	gpio_reset_pin( PIN_NUM_RST );
	gpio_set_direction( PIN_NUM_RST, GPIO_MODE_OUTPUT );
}

// Задача нажатия кнопок
void button_task(void *pvParameter) {
    int pins[] = BUTTON_PINS;
    int num_pins = 9; // 9 кнопок

    for ( ;; ) {
        for (int i = 0; i < num_pins; i++) {
            if (gpio_get_level(pins[i]) == 0) { // Если кнопка нажата (LOW, так как подтяжка к VCC)
                xStreamBufferSend(xStreamBuffer, &pins[i], sizeof(pins[i]), 0);
                vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS)); // Задержка для устранения дребезга
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Небольшая задержка для снижения нагрузки на CPU
    }
}

void init_gpio_button() {
    // Настраиваем пины как входы с подтягивающими резисторами
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // Отключаем прерывания
    io_conf.mode = GPIO_MODE_INPUT;       // Режим ввода
    io_conf.pin_bit_mask = 0;             // Очищаем маску

    // Добавляем пины в маску
    uint64_t button_pins = 0;
    int pins[] = BUTTON_PINS;
    for (int i = 0; i < sizeof(pins) / sizeof(pins[0]); i++) {
        button_pins |= (1ULL << pins[i]);
    }
    io_conf.pin_bit_mask = button_pins;

    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // Включаем подтягивающие резисторы
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
}

void spi_init(spi_device_handle_t *spi) {
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 153600, //320*240*2
    };

    esp_err_t ret = spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CHANNEL);
    if (ret != ESP_OK) {
        ESP_LOGE("SPI", "Failed to initialize bus: %s", esp_err_to_name(ret));
    }

    spi_device_interface_config_t spicfg = {
        .clock_speed_hz = SPI_MASTER_FREQ_26M,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7,
        .flags = SPI_DEVICE_NO_DUMMY,
        .mode = 0,
        .pre_cb = NULL,
        .post_cb = NULL,
    };

    ret = spi_bus_add_device(HSPI_HOST, &spicfg, spi);
    if (ret != ESP_OK) { printf("SPI bus add device failed!\n");
    }
}

void send_command(spi_device_handle_t spi, uint8_t cmd) {
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));
    trans.length = 8;
    trans.tx_buffer = &cmd;
    gpio_set_level(PIN_NUM_DC, 0);
    esp_err_t ret = spi_device_transmit(spi, &trans);
    if (ret != ESP_OK) {
        printf("Error sending command: %s (0x%02X)\n", esp_err_to_name(ret), cmd);
    }
}

void send_command_no_dc(spi_device_handle_t spi, uint8_t cmd) {
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));
    trans.length = 8;
    trans.tx_buffer = &cmd;
    gpio_set_level(PIN_NUM_DC, 1);
    esp_err_t ret = spi_device_transmit(spi, &trans);
    if (ret != ESP_OK) {
        printf("Error sending command: %s (0x%02X)\n", esp_err_to_name(ret), cmd);
    }
}

void send_data(spi_device_handle_t spi, const uint8_t *data, size_t length) {
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));
    trans.length = length*8; // Длина в битах
    trans.tx_buffer = data; // Указатель на данные для передачи
    gpio_set_level(PIN_NUM_DC, 1);
    esp_err_t ret = spi_device_transmit(spi, &trans);
    if (ret != ESP_OK) {
        printf("Error sending data: %s\n", esp_err_to_name(ret));
    }
}

void send_data16b(spi_device_handle_t spi, const uint16_t *data, size_t length) {
    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));
    trans.length = length*16; // Длина в битах
    trans.tx_buffer = data; // Указатель на данные для передачи
    gpio_set_level(PIN_NUM_DC, 1);
    esp_err_t ret = spi_device_transmit(spi, &trans);
    if (ret != ESP_OK) {
        printf("Error sending data: %s\n", esp_err_to_name(ret));
    }
}

void reset_display(void) {
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(50 / portTICK_PERIOD_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(50 / portTICK_PERIOD_MS);
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

void fill_rect(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    send_command(spi, CMD_COLUMN);
    uint8_t col_data[4] = {x >> 8, x & 0xFF, (x + width) >> 8, (x + width) & 0xFF};
    send_data(spi, col_data, 4);

    send_command(spi, CMD_ROW);
    uint8_t row_data[4] = {0, y & 0xFF, 0, (y + height) & 0xFF};
    send_data(spi, row_data, 4);

    send_command(spi, CMD_SET_PIXEL);

    uint32_t buf_size = ((width + sizeof(uint32_t)) * (height + sizeof(uint32_t)) * 2);
    uint8_t *pixel_buf = heap_caps_malloc(buf_size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);

    for (int i = 0; i < (buf_size / 2) + sizeof(uint32_t); i++) {
        pixel_buf[i * 2] = color >> 8;   // Старший байт
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

void app_main(void)
{
    xStreamBuffer = xStreamBufferCreate(STREAM_BUF_SIZE, sizeof(int));

    init_gpio_display();
    reset_display();
    spi_device_handle_t spi;

    spi_init(&spi);

    const size_t data = 1;
    send_command(spi, CMD_SOFTWARE_RESET);
    send_command(spi, CMD_SLEEP_OUT);
    send_command(spi, CMD_SET_RGB);
    send_data(spi, (uint8_t[]){0x05}, data);  //16-bit/pixel 65K-Colors(RGB 5-6-5-bit Input)

    send_command(spi, CMD_DISPLAY_ON);
    send_command(spi, CMD_NORMAL_MODE);

    uint8_t madctl_value = 0xB0;
    send_command(spi, CMD_MADCTL);
    send_data(spi, &madctl_value, 1);

    init_gpio_button();
    // Создаем задачу для обработки нажатий
    xTaskCreate(button_task, "button_task", 2048, NULL, 1, NULL);

//    fill_screen(spi, 0xAAAA);

/*
    Fill more rectangle
*/
/*
    fill_rect(spi, 0, 0, 32, 40, 0x0000);
    fill_rect(spi, 0, 40, 32, 40, 0x1111);
    fill_rect(spi, 0, 80, 32, 40, 0x2222);
    fill_rect(spi, 0, 120, 32, 40, 0x3333);
    fill_rect(spi, 32, 0, 32, 40, 0x4444);
    fill_rect(spi, 32, 40, 32, 40, 0x5555);
    fill_rect(spi, 32, 80, 32, 40, 0x6666);
    fill_rect(spi, 32, 120, 32, 40, 0x7777);
    fill_rect(spi, 64, 0, 32, 40, 0x8888);
    fill_rect(spi, 64, 40, 32, 40, 0x9999);
    fill_rect(spi, 64, 80, 32, 40, 0xAAAA);
    fill_rect(spi, 64, 120, 32, 40, 0xBBBB);
    fill_rect(spi, 96, 0, 32, 40, 0xCCCC);
    fill_rect(spi, 96, 40, 32, 40, 0xDDDD);
    fill_rect(spi, 96, 80, 32, 40, 0xEEEE);
    fill_rect(spi, 96, 120, 32, 40, 0xFFFF);
*/

    int received = 0;
	while (1)
	{
        xStreamBufferReceive(xStreamBuffer, &received, sizeof(received), 0);
        //if(xStreamBufferReceive(xStreamBuffer, &received, sizeof(received), portMAX_DELAY) > 0)
        //{
        //    // Обработка числа
        //}
        fill_rect(spi, 96+received, 120, 32, 40, 0x0000);
    	draw_image(spi, &my_image_1);
        fill_rect(spi, 96+received, 120, 32, 40, 0x0000);
    	draw_image(spi, &my_image_2);
        fill_rect(spi, 96+received, 120, 32, 40, 0x0000);
    	draw_image(spi, &my_image_3);
        fill_rect(spi, 96+received, 120, 32, 40, 0x0000);
    	draw_image(spi, &my_image_4);
        fill_rect(spi, 96+received, 120, 32, 40, 0x0000);
    	draw_image(spi, &my_image_5);
        fill_rect(spi, 96+received, 120, 32, 40, 0x0000);
	}

/*

    Vertical Scrolling

*/
/*
    uint16_t tfa = 0;
    uint16_t vsa = 320;
    uint16_t bfa = 0;
    uint16_t ssa = 0;
    uint16_t i = 1;
    while (1) {
        if (i == 320) { i = 1; }; i++;
        ssa = 0+i;
        vertical_scroll(spi, &tfa, &vsa, &bfa, &ssa);
        //fill_screen(spi, 0xCCCC+i);
        vTaskDelay(10 / portTICK_PERIOD_MS); // Задержка для видимости прокрутки
        printf("ok\n");
    }
*/
}

