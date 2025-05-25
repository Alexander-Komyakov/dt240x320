#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/stream_buffer.h"
#include "core/button.h"
#include "display.h"
#include "image_structure.h"
#include <stdlib.h>
#include <stdio.h>


void game_pong(spi_device_handle_t spi);

struct Player
{
	int16_t x;
	uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t color;
};
