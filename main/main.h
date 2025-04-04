#include "driver/spi_master.h"

#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5
#define PIN_NUM_DC 2
#define PIN_NUM_RST 4

#define SPI_DMA_CHANNEL 1
#define SPI_SPEED        2600000

#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240

#define CMD_SET_PIXEL 0x2C      // Установка пикселя
#define CMD_COLUMN 0x2A         // Установка колонки
#define CMD_ROW 0x2B            // Установка строки
#define CMD_SLEEP_OUT 0x11      // Выход из сна
#define CMD_SOFTWARE_RESET 0x01 // Выход из сна
#define CMD_SET_RGB 0x3A        // Установка RGB режима
#define CMD_DISPLAY_ON 0x29     // Включение дисплея
#define CMD_NORMAL_MODE 0x13    // Нормальный режим
#define CMD_MADCTL 0x36         // Установка MADCTL
#define CMD_SCRLAR 0x33         // Установка площади скрола
#define CMD_VSCSAD 0x37         // Стартовый адрес вертикального скрола

void init_gpio_display();
void spi_init(spi_device_handle_t *spi);
void send_command(spi_device_handle_t spi, uint8_t cmd);
void send_command_no_dc(spi_device_handle_t spi, uint8_t cmd);
void send_data(spi_device_handle_t spi, const uint8_t *data, size_t length);
void reset_display(void);
void fill_rect(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void fill_screen(spi_device_handle_t spi, uint16_t color);
void draw_pixel(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t color);
void vertical_scroll(spi_device_handle_t spi, uint16_t* tfa, uint16_t* vsa, uint16_t* bfa, uint16_t* ssa);
void app_main(void);
