#pragma once
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint16_t x, y, width, height;
    size_t size_image;
    const uint16_t *pixels;
} Image;


