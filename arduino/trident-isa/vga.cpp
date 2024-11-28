#include "vga.h"
#include "isa.h"
#include "palette.h"
#include "font.h"

VMODE_ST Mode; 

uint8_t mode03h[62] = {
    // MISC reg,  STATUS reg,    SEQ regs
    0x67, 0x00, 0x03, 0x00, 0x03, 0x00, 0x02,
    // CRTC regs
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F, 0x00, 0x4F, 0x0E, 0x0F,
    0x00, 0x00, 0x00, 0x01, 0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF,
    // GRAPHICS regs
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00, 0xFF,
    // ATTRIBUTE CONTROLLER regs
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x3E, 0x3F, 0x04, 0x00, 0x0F, 0x08, 0x00};

uint8_t mode12h[62] = {
    // MISC reg,  STATUS reg,    SEQ regs
    0xE3, 0x00, 0x03, 0x01, 0x0F, 0x00, 0x06,
    // CRTC regs
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E, 0x00, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x59, 0xEA, 0x8C, 0x0DF, 0x28, 0x00, 0x0E7, 0x04, 0xE3,
    0xFF,
    // GRAPHICS regs
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x0F, 0xFF,
    // ATTRIBUTE CONTROLLER regs
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x3E, 0x3F, 0x01, 0x00, 0x0F, 0x00, 0x00};

uint8_t mode13h[62] = {
    // MISC reg,  STATUS reg,    SEQ regs
    0x63, 0x00, 0x03, 0x01, 0x0F, 0x00, 0x0E,
    // CRTC regs
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0x0A3,
    0xFF,
    // GRAPHICS regs
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    // ATTRIBUTE CONTROLLER regs
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00};

void VgaIoWriteIx(uint32_t addr, uint16_t valIx) {
  isa_outb(addr, valIx & 0xFFu);
  isa_outb(addr + 1, (valIx >> 8) & 0xFFu);
}

uint8_t VgaIoReadIx(uint32_t addr, uint8_t ix) {
  uint16_t data = 0;
  isa_outb(addr, ix);
  return isa_inb(addr + 1);
}

inline void VgaMemoryWriteW(uint32_t addr, uint16_t data) { 
  isa_write_word(addr, data); 
}

void vga_load_font() {
  VgaIoWriteIx(VGA_GC_INDEX, 0x0005);
  VgaIoWriteIx(VGA_GC_INDEX, 0x0406);
  VgaIoWriteIx(VGA_SEQ_INDEX, 0x0402);

  VgaIoWriteIx(VGA_SEQ_INDEX, 0x0604);

  uint32_t ptr = VIDEO_MEMORY_GRAPH;
  for (int i = 0; i < 1024 * 4; i++) {
    if (!(i % 16) && (i > 0)) {
      ptr += 16;
    }

    isa_write_byte(ptr++, Font8x16[i + 1]);
  }

  VgaIoWriteIx(VGA_SEQ_INDEX, 0x0302);

  VgaIoWriteIx(VGA_SEQ_INDEX, 0x0204);
  VgaIoWriteIx(VGA_GC_INDEX, 0x1005);
  VgaIoWriteIx(VGA_GC_INDEX, 0x0E06);
}

void vga_set_mode_registers(uint8_t *dataptr) {
  isa_outb(VGA_MISC_WRITE, 0x67); // Before acess registers must be set address sheme

  isa_outb(VGA_MISC_WRITE, *dataptr);
  dataptr++;

  isa_outb(VGA_INSTAT_READ, *dataptr);
  dataptr++;

  for (int i = 0; i < 5; i++) {
    isa_outb(VGA_SEQ_INDEX, i);
    isa_outb(VGA_SEQ_INDEX + 1, *dataptr);
    dataptr++;
  }

  VgaIoWriteIx(VGA_CRTC_INDEX, 0x0E11);
  VgaIoReadIx(VGA_CRTC_INDEX, 0x11);

  for (int i = 0; i < 25; i++) {
    VgaIoWriteIx(VGA_CRTC_INDEX, ((*dataptr) << 8) + i);
    dataptr++;
  }

  for (int i = 0; i < 9; i++) {
    VgaIoWriteIx(VGA_GC_INDEX, ((*dataptr) << 8) + i);
    dataptr++;
  }

  isa_inb(VGA_INSTAT_READ);

  for (int i = 0; i < 21; i++) {
    isa_inb(VGA_INSTAT_READ);
    isa_outb(VGA_AC_INDEX, i);
    isa_outb(VGA_AC_INDEX, *dataptr);
    dataptr++;
  }
  VgaIoWriteIx(VGA_GC_INDEX, 0x3A0B);
  isa_inb(VGA_INSTAT_READ);
  isa_outb(VGA_AC_INDEX, 0x20);
  isa_outb(VGA_PEL_MASK, 0xFF);
}

void vga_set_mode(uint8_t mode) {

  Mode.mode = mode;
  if (mode == MODE03H) // 80 x 25 x 16
  {
    vga_set_mode_registers(mode03h);
    palette_load_16();
    vga_load_font();

    Mode.width = 80;
    Mode.height = 25;
    Mode.width_uint8_ts = 2000;
    Mode.colors = 16;
    Mode.attrib = TVU_TEXT;
  } else if (mode == MODE12H) // 640 x 480 x 16
  {
    vga_set_mode_registers(mode12h);
    palette_load_16();

    Mode.width = 640;
    Mode.height = 480;
    Mode.width_uint8_ts = 38400u;
    Mode.colors = 16;
    Mode.attrib = TVU_GRAPHICS | TVU_PLANAR;
  } else if (mode == MODE13H) // 320 x 200 x 256
  {
    vga_set_mode_registers(mode13h);
    palette_load_256();

    Mode.width = 320;
    Mode.height = 200;
    Mode.width_uint8_ts = 64000u;
    Mode.colors = 256;
    Mode.attrib = TVU_GRAPHICS;
  }
}

void vga_set_coursor_pos(uint8_t row, uint8_t col) {
  uint16_t pos = 80*row + col;

  // Set the high byte of cursor position
  isa_outb(VGA_CRTC_INDEX, 0x0F);
  isa_outb(VGA_CRTC_DATA, (uint8_t)(pos & 0xFF));

  // Set the low byte of cursor position
  isa_outb(VGA_CRTC_INDEX, 0x0E);
  isa_outb(VGA_CRTC_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_cursor_visible(bool visible) {
    isa_outb(VGA_CRTC_INDEX, 0x0A);
    uint8_t reg = isa_inb(VGA_CRTC_DATA);

    if (visible) {
        // Clear the 5th bit to enable the cursor
        reg &= ~0x20;
    } else {
        // Set the 5th bit to disable the cursor
        reg |= 0x20;
    }

    isa_outb(VGA_CRTC_INDEX, 0x0A);
    isa_outb(VGA_CRTC_DATA, reg);
}

void vga_set_character(uint8_t row, uint8_t col, char character, int fgColor, int bgColor) {
    uint32_t offset = VIDEO_MEMORY_CTEXT + (row * Mode.width + col) * 2;
    isa_write_byte(offset, character);
    isa_write_byte(offset+1, (bgColor << 4) | (fgColor & 0x0F));
}

void vga_set_pixel(uint16_t x, uint16_t y, uint8_t color) {
  int width = Mode.width;

  if (Mode.mode == MODE13H) {
    VgaIoWriteIx(VGA_SEQ_INDEX, ((1 << ((x % 256) & 3)) << 8) + 0x02);
    isa_write_byte(0xA0000 + (x + (y * Mode.width)), color);
  } else if (Mode.attrib & TVU_PLANAR) {
    VgaIoWriteIx(VGA_GC_INDEX, ((1 << (((x % 256) & 7) ^ 7)) << 8) + 0x08);
    VgaIoWriteIx(VGA_SEQ_INDEX, (color << 8) + 0x02);
    isa_read_byte(0xA0000 + (y * 80) + (x >> 3));
    isa_write_byte(0xA0000 + (y * 80) + (x >> 3), 0xFF);
  }
}

void vga_mode12h_screen_clear(uint8_t color) {
  uint32_t address = VIDEO_MEMORY_GRAPH;

  VgaIoWriteIx(0x3CE,0xFF08);
  isa_outb(0x3C4,0x02);
  for(uint16_t i = 0; i < Mode.width_uint8_ts / 2; i++) {
    isa_write_word(address,0x0000);
    isa_outb(0x3C5,color);
    isa_write_word(address,0xFFFF);
    isa_outb(0x3C5,0x0F);
    address += 2;
   }
}
