#pragma once
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>

#define MAX_STR_LEN 32


typedef struct {
    char key[16];
    union {
        uint8_t  u8;
        uint16_t u16;
        uint32_t u32;
        char str[MAX_STR_LEN];
    } value;
    uint8_t type;
} nvs_data_t;

void nvs_init();
void storage_task(void* arg);
void save_nvs_u8(const char* key, uint8_t value);
void save_nvs_u16(const char* key, uint16_t value);
void save_nvs_u32(const char* key, uint32_t value);
void save_nvs_str(const char* key, const char* value);
uint8_t load_nvs_u8(const char* strnumber);
uint16_t load_nvs_u16(const char* strnumber);
uint32_t load_nvs_u32(const char* strnumber);
bool load_nvs_str(const char* key, char* out_value);
