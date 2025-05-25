#include "physics.h"


bool check_collision_rect(uint16_t x1, uint16_t y1, uint16_t w1, uint16_t h1,
                     uint16_t x2, uint16_t y2, uint16_t w2, uint16_t h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 &&
            y1 < y2 + h2 && y1 + h1 > y2);
}
