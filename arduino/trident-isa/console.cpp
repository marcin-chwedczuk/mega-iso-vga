
#include "console.h"
#include "vga.h"

int current_row = 0;
int current_col = 0;

void console_clear() {
  for (int row = 0; row < 25; row++) {
    for (int col = 0; col < 80; col++) {
      vga_set_character(row, col, ' ', 0, 0);
    }
  }
  console_set_cursor(0, 0);
}

void console_set_cursor(int row, int col) {
  current_row = row;
  current_col = col;
  vga_set_coursor_pos(row, col);
}

void console_puts(const char* str, int fc, int bc) {
  while (*str) {
    if (*str == '\n') {
      current_row++;
      current_col = 0;
    } else {
      vga_set_character(current_row, current_col, *str, fc, bc);
      
      current_col++;
      // TODO: remove hardcoded value
      if (current_col >= 80) {
        // TODO: Support screen wrap
        current_row++;
        current_col = 0;
      }
    }

    str++;
  }
}