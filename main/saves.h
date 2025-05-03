#pragma once
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"


void nvs_init(nvs_handle_t *nvs_handle);
void nvs_save_uint8_t(nvs_handle_t nvs_handle, uint8_t number, char* strnumber);
uint8_t nvs_load_uint8_t(nvs_handle_t nvs_handle, char* strnumber);
