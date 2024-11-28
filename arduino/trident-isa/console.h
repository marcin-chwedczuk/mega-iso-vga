#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

#define TEXT_BLACK         0x0
#define TEXT_BLUE          0x1
#define TEXT_GREEN         0x2
#define TEXT_CYAN          0x3
#define TEXT_RED           0x4
#define TEXT_MAGENTA       0x5
#define TEXT_BROWN         0x6
#define TEXT_LIGHT_GRAY    0x7
#define TEXT_DARK_GRAY     0x8
#define TEXT_LIGHT_BLUE    0x9
#define TEXT_LIGHT_GREEN   0xA
#define TEXT_LIGHT_CYAN    0xB
#define TEXT_LIGHT_RED     0xC
#define TEXT_LIGHT_MAGENTA 0xD
#define TEXT_YELLOW        0xE
#define TEXT_WHITE         0xF

#define BACKGROUND_BLACK         (0x0 << 4)
#define BACKGROUND_BLUE          (0x1 << 4)
#define BACKGROUND_GREEN         (0x2 << 4)
#define BACKGROUND_CYAN          (0x3 << 4)
#define BACKGROUND_RED           (0x4 << 4)
#define BACKGROUND_MAGENTA       (0x5 << 4)
#define BACKGROUND_BROWN         (0x6 << 4)
#define BACKGROUND_LIGHT_GRAY    (0x7 << 4)

#define TEXT_BLINK  0b10000000

void console_blinking_text(bool enable);
void console_clear(uint8_t attribs);
void console_puts(const char* str, uint8_t attribs);
void console_set_cursor(int row, int col);
void console_draw_frame(int x, int y, int width, int height, uint8_t attribs);

#endif