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
#include <math.h>
#include "font.h"


// Константы позиций спрайтов
#define FIGHTER_X 10
#define FIGHTER_Y 50
#define SONIC_X 70
#define SONIC_Y 50
#define PIKACHU_X 140
#define PIKACHU_Y 50

// Белый цвет
#define TRANSPARENT_COLOR 0xFFFF


void sonic(spi_device_handle_t spi);




