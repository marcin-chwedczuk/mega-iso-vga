#ifndef FONT_H
#define FONT_H

#include <stdint.h>

// Externs

extern  unsigned char Font8x16[];
extern  unsigned char Font8x8[];

// Prototypes

void SetFont8x16(void);

void SetFont8x8(void);

void  FontsRead( uint8_t  *biosfont,uint8_t uint8_tsperchar );

uint8_t VgaIoReadIx(uint32_t addr, uint8_t ix);
void VgaIoWriteIx(uint32_t addr, uint16_t valIx);
void writeMemoryByte(uint32_t address, uint8_t data);

#endif
