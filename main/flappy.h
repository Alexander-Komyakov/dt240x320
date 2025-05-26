#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/stream_buffer.h"
#include "core/button.h"
#include "core/display.h"
#include "core/image_structure.h"
#include <stdlib.h>
#include <stdio.h>


void game_flappy(spi_device_handle_t spi);
