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
