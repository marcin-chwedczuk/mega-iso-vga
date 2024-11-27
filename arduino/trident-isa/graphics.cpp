
#include "graphics.h"
#include "vga.h"

// Functions were adapted from:
// https://github.com/adafruit/Adafruit-GFX-Library
// 

// X axis goes left to right, Y axis goes from top to the bottom of the screen.

void gfx_draw_rect(int x, int y, int w, int h, uint8_t c) {
  for (int i = x; i < x + w; i++)
    for (int j = y; j < y + h; j++)
      vga_set_pixel(i, j, c);
}

void gfx_draw_circle(int16_t x0, int16_t y0, int16_t r, uint8_t color) {

  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;


  vga_set_pixel(x0, y0 + r, color);
  vga_set_pixel(x0, y0 - r, color);
  vga_set_pixel(x0 + r, y0, color);
  vga_set_pixel(x0 - r, y0, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    vga_set_pixel(x0 + x, y0 + y, color);
    vga_set_pixel(x0 - x, y0 + y, color);
    vga_set_pixel(x0 + x, y0 - y, color);
    vga_set_pixel(x0 - x, y0 - y, color);
    vga_set_pixel(x0 + y, y0 + x, color);
    vga_set_pixel(x0 - y, y0 + x, color);
    vga_set_pixel(x0 + y, y0 - x, color);
    vga_set_pixel(x0 - y, y0 - x, color);
  }
}