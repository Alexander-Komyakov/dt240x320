#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/stream_buffer.h"
#include "button.h"
#include "display.h"
#include "image_structure.h"


void game_pong(spi_device_handle_t spi);

struct Player
{
	uint16_t x;
	uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t color;
};

