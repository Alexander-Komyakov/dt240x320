#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "freertos/stream_buffer.h"
#include "button.h"
#include "display.h"
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
//    SHAPE_SQUARE,
    SHAPE_SNAKE,
//    SHAPE_SPIRAL,
//    SHAPE_SNOWFLAKE,
    SHAPE_TOTAL_COUNT
} ShapeType;


// Максимальное количество блоков в фигуре
#define MAX_BRICKS 20  

// Типы фигур
/*
typedef enum {
    SHAPE_SNAKE_CLASSIC,  // Классическая змейка
    SHAPE_SNAKE_SHORT,    // Компактная змейка
    SHAPE_SQUARE,         // Квадрат 3x3
    SHAPE_LADDER,         // Лесенка
    SHAPE_TOTAL_COUNT
} ShapeType;
*/

// Шаблоны фигур (формат: {row_offset, col_offset})
//const int shape_templates[SHAPE_TOTAL_COUNT][MAX_BRICKS][2] = {
//extern const int shape_templates[SHAPE_TOTAL_COUNT][MAX_BRICKS][2];
/*
    // Классическая змейка (17 блоков)
    {
        {0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {0,5},  // Горизонтальная часть 1
        {1,5}, {2,5},                                // Вертикальная часть 1
        {2,4}, {2,3}, {2,2}, {2,1}, {2,0},           // Горизонтальная часть 2
        {3,0}, {4,0},                                // Вертикальная часть 2
        {4,1}, {4,2}                                 // Хвост
    },
    // Компактная змейка (5 блоков)
    {
        {0,0}, {0,1}, {1,1}, {1,2}, {2,2}, {-1,-1}  // -1,-1 — маркер конца
    },
    // Квадрат 3x3 (9 блоков)
    {
        {0,0}, {0,1}, {0,2},
        {1,0}, {1,1}, {1,2},
        {2,0}, {2,1}, {2,2}, {-1,-1}
    },
    // Лесенка (6 блоков)
    {
        {0,0}, {1,0}, {1,1}, {2,1}, {2,2}, {3,2}, {-1,-1}
    }
};

*/

// Структуры (остаются без изменений)
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
void show_round_screen(spi_device_handle_t spi, uint8_t round, uint8_t lives, float speed);
bool check_collision(GameObject a, GameObject b);
