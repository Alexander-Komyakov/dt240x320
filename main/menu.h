#pragma once
#include "core/display.h"
#include "core/button.h"
#include "core/saves.h"

#define DEBOUNCE_TIME_MS 250
#define MENU_BACKGROUND_COLOR 0xAAAA
#define MENU_GAME_COLOR 0xCCCC
#define MENU_BORDER 8


uint8_t menu(spi_device_handle_t spi);
