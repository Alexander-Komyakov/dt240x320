#pragma once
#include "display.h"
#include <stdint.h>
#include "driver/spi_master.h"


extern const uint8_t font_5x8[][5];
void draw_char(spi_device_handle_t spi, uint16_t x, uint16_t y, char c, uint16_t color);
void draw_text(spi_device_handle_t spi, uint16_t x, uint16_t y, const char *text, uint16_t color);
