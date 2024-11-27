#include "vga.h"
#include "isa.h"


void VgaIoWriteIx(uint32_t addr, uint16_t valIx) {
  isa_outb(addr, valIx & 0xFFu);
  isa_outb(addr + 1, (valIx >> 8) & 0xFFu);
}

uint8_t VgaIoReadIx(uint32_t addr, uint8_t ix) {
  uint16_t data = 0;
  isa_outb(addr, ix);
  return isa_inb(addr + 1);
}