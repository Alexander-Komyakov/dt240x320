#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/stream_buffer.h"
#include "core/button.h"
#include "display.h"
#include "physics.h"
#include "image_structure.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "font.h"
//#include "saves.h" // сохранение

// Константы игры
#define BRICK_ROWS 8       // Увеличили для сложных фигур
#define BRICK_COLS 12
#define BRICK_WIDTH 24
#define BRICK_HEIGHT 8
#define BRICK_MARGIN 2
#define BRICK_TOP_MARGIN 30
#define SPEED_INCREASE_INTERVAL 15
#define MIN_DISTANCE_FROM_BOTTOM 30

// Типы фигур
typedef enum {
    HORIZONTAL_LINE,
    VERTICAL_LINE,
    SHAPE_SQUARE,
    SHAPE_SNAKE,
    SHAPE_SPIDER,
    SHAPE_LADDER,
    SHAPE_CROSS,
    SHAPE_TRIANGLE,
    SHAPE_SNOWFLAKE,
    SHAPE_TOTAL_COUNT
} ShapeType;

// Структуры
typedef struct {
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t color;
} GameObject;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t color;
    bool active;
    bool unbreakable;  // New field for unbreakable blocks
} Brick;

// Прототипы функций
void game_arkanoid(spi_device_handle_t spi);
void generate_random_bricks(Brick bricks[BRICK_ROWS][BRICK_COLS]);
void draw_bricks(spi_device_handle_t spi, Brick bricks[BRICK_ROWS][BRICK_COLS]);
bool all_bricks_destroyed(Brick bricks[BRICK_ROWS][BRICK_COLS]);
void show_round_screen(spi_device_handle_t spi, uint16_t round, uint8_t lives, float speed);
