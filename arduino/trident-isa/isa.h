#ifndef ISA_H
#define ISA_H

#include <stdint.h>

void isa_arduino_setup(void);

void isa_write_byte(uint32_t address, uint8_t data);
void isa_write_word(uint32_t address, uint16_t data);
uint8_t isa_read_byte(uint32_t address);
uint16_t isa_read_word(uint32_t address);

void isa_outb(uint32_t portAddress, uint8_t data);
uint8_t isa_inb(uint32_t portAddress);

#endif