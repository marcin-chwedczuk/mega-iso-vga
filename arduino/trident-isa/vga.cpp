#include "vga.h"
#include "isa.h"


void VgaIoWriteIx(uint32_t addr, uint16_t valIx) {
  writeIO(addr, valIx & 0xFFu);
  writeIO(addr + 1, (valIx >> 8) & 0xFFu);
}

uint8_t VgaIoReadIx(uint32_t addr, uint8_t ix) {
  uint16_t data = 0;
  writeIO(addr, ix);
  return readIO(addr + 1);
}