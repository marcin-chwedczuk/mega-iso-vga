#ifndef CONSOLE_H
#define CONSOLE_H

void console_puts(const char* str, int fc, int bc);
void console_set_cursor(int row, int col);
void console_clear(void);

#endif