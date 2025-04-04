#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint16_t x, y, width, height;
    size_t size_image;
    const uint16_t *pixels;
} Image;

extern const uint16_t my_image_pixels_1[320 * 240];
extern const Image my_image_1;

extern const uint16_t my_image_pixels_2[320 * 240];
extern const Image my_image_2;

extern const uint16_t my_image_pixels_3[320 * 240];
extern const Image my_image_3;

extern const uint16_t my_image_pixels_4[320 * 240];
extern const Image my_image_4;

extern const uint16_t my_image_pixels_5[320 * 240];
extern const Image my_image_5;


