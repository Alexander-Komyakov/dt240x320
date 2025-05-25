#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/stream_buffer.h"
#include "core/button.h"
#include "core/display.h"
#include "core/physics.h"
#include "core/saves.h"
#include "image_structure.h"
#include <stdlib.h>
#include <stdio.h>


#define MAX_PLATFORM 20
#define NEW_PLATFORM 20

typedef struct {
    uint16_t prev_x;
    uint8_t prev_y;  
    uint16_t x;
    uint8_t y;  
    uint8_t type;  // 00=обычная, 01=движ, 10=пружина
    uint8_t visible;
} Platform;


void game_doodle(spi_device_handle_t spi);
