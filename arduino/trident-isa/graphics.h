#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

void gfx_draw_rect(int x, int y, int w, int h, uint8_t c);
void gfx_draw_circle(int16_t x0, int16_t y0, int16_t r, uint8_t color);

#endif