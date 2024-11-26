#ifndef ISA_H
#define ISA_H

#include <stdint.h>

// TODO: Hide those const
#define PIN_ALE 4
#define PIN_MEM_16BIT 5
#define PIN_MEMW 3
#define PIN_MEMR 2
#define PIN_IOW 10
#define PIN_IOR 11
#define PIN_RESET 12
#define PIN_WAITSTATE 13

void ale(bool active);
bool mem16bit();

void memw(bool active);
void memr(bool active);

void iow(bool active);
void ior(bool active);

void waitForDevice();

void setAddress(uint32_t address);

void enableDataOutput(bool enable);
void setData(uint8_t data);
uint8_t readData();

void writeMemoryByte(uint32_t address, uint8_t data);
void writeMemory(uint32_t address, uint16_t data);
uint8_t readMemoryByte(uint32_t address);
uint16_t readMemory(uint32_t address);

void writeIO(uint32_t portAddress, uint16_t data);
uint16_t readIO(uint32_t portAddress);

#endif