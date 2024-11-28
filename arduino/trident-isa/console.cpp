

#include "console.h"
#include "vga.h"

int current_row = 0;
int current_col = 0;

void console_blinking_text(bool enable) {
  vga_set_blinking(enable);
}

void console_clear(uint8_t attribs) {
  vga_screen_clear(attribs);
  console_set_cursor(0, 0);
}

void console_set_cursor(int row, int col) {
  current_row = row;
  current_col = col;
  vga_set_coursor_pos(row, col);
}

void console_put_character(char c, uint8_t attribs) {
  const VMODE_ST* mode = vga_get_current_mode();
  
  vga_set_character(current_row, current_col, c, attribs);

  // Set cursor to the next place were we will write text
  current_col++;
  if (current_col >= mode->width) {
    current_row++;
    current_col = 0;
  }
}

void console_puts(const char* str, uint8_t attribs) {
  const VMODE_ST* mode = vga_get_current_mode();

  while (*str) {
    if (*str == '\n') {
      current_row++;
      current_col = 0;
    } else if (*str == '\r') {
      current_col = 0;
    } else {
      if (current_col >= mode->width || current_row >= mode->height) {
        continue;
      }

      current_col++;
      if (current_col >= mode->width) {
        current_row++;
        current_col = 0;
      }
      vga_set_character(current_row, current_col, *str, attribs);
    }

    str++;
  }

  // Set cursor to the next place were we will write text
  current_col++;
  if (current_col >= mode->width) {
    current_row++;
    current_col = 0;
  }
  console_set_cursor(current_row, current_col);
}

void console_draw_frame(int row, int col, int width, int height, uint8_t attribs) {
    // Box-drawing characters from CP437
    const char HORIZONTAL = 0xCD; // ─
    const char VERTICAL = 0xBA;   // │
    const char TOP_LEFT = 0xC9;   // ┌
    const char TOP_RIGHT = 0xBB;  // ┐
    const char BOTTOM_LEFT = 0xC8; // └
    const char BOTTOM_RIGHT = 0xBC; // ┘

    // Draw top border
    console_set_cursor(row, col);
    console_put_character(TOP_LEFT, attribs);
    for (int i = 0; i < width - 2; i++) {
        console_put_character(HORIZONTAL, attribs);
    }
    console_put_character(TOP_RIGHT, attribs);

    // Draw sides
    for (int i = 1; i < height - 1; i++) {
        console_set_cursor(row + i, col);
        console_put_character(VERTICAL, attribs);

        // Fill background
        for (int j = 0; j < width-2; j++) {
          console_put_character(' ', attribs);
        }

        // console_set_cursor(row + i, col + width - 1);
        console_put_character(VERTICAL, attribs);
    }

    // Draw bottom border
    console_set_cursor(row + height - 1, col);
    console_put_character(BOTTOM_LEFT, attribs);
    for (int i = 0; i < width - 2; i++) {
        console_put_character(HORIZONTAL, attribs);
    }
    console_put_character(BOTTOM_RIGHT, attribs);
}