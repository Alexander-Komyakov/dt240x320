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

void sonic(spi_device_handle_t spi);
