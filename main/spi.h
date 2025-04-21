#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5
#define PIN_NUM_DC 2
#define PIN_NUM_RST 4

#define SPI_DMA_CHANNEL 1
#define SPI_SPEED        2600000


void init_gpio_display();
void spi_init(spi_device_handle_t *spi);
void send_command(spi_device_handle_t spi, uint8_t cmd);
void send_command_no_dc(spi_device_handle_t spi, uint8_t cmd);
void send_data(spi_device_handle_t spi, const uint8_t *data, size_t length);
void send_data16b(spi_device_handle_t spi, const uint16_t *data, size_t length);
void reset_display(void);
