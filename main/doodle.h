#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/stream_buffer.h"
#include "button.h"
#include "display.h"
#include "physics.h"
#include "image_structure.h"
#include <stdlib.h>
#include <stdio.h>


#define MAX_PLATFORM 20
#define NEW_PLATFORM 32

typedef struct {
    uint16_t prev_x;
    uint8_t prev_y;  
    uint16_t x;
    uint8_t y;  
    uint8_t type;  // 00=обычная, 01=движ, 10=пружина
    uint8_t visible;
} Platform;


void game_doodle(spi_device_handle_t spi);
void draw_platform(spi_device_handle_t spi, const Platform* p, const Image* platform_normal);
