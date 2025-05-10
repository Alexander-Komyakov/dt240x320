#pragma once
#include <stdint.h>
#include <stdbool.h>

bool check_collision_rect(uint16_t x1, uint16_t y1, uint16_t w1, uint16_t h1,
                     uint16_t x2, uint16_t y2, uint16_t w2, uint16_t h2);
