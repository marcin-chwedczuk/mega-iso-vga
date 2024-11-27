
#include "isa.h"
#include <Arduino.h>


void ale(bool active) { digitalWrite(PIN_ALE, active ? HIGH : LOW); }

void memw(bool active) { digitalWrite(PIN_MEMW, active ? LOW : HIGH); }
void memr(bool active) { digitalWrite(PIN_MEMR, active ? LOW : HIGH); }

void iow(bool active) { digitalWrite(PIN_IOW, active ? LOW : HIGH); }
void ior(bool active) { digitalWrite(PIN_IOR, active ? LOW : HIGH); }

void waitForDevice() {
  delay(0); // initial delay
  while (!digitalRead(PIN_WAITSTATE)) {
    // wait
  }
}

void setAddress(uint32_t address) {
  for (int pin = 30; pin < 50; pin++) {
    uint8_t bit = (address & 1u);
    digitalWrite(pin, bit ? HIGH : LOW);
    address >>= 1;
  }
}

void enableDataOutput(bool enable) {
  int dataPins[] = {A0, A1, A2, A3, A4, A5, A6, A7};
  for (int i = 0; i < 8; i++) {
    pinMode(dataPins[i], enable ? OUTPUT : INPUT);
  }
}

void setData(uint8_t data) {
  int dataPins[] = {A0, A1, A2, A3, A4, A5, A6, A7};
  for (int i = 0; i < 8; i++) {
    uint16_t bit = (data & 1u);
    digitalWrite(dataPins[i], bit ? HIGH : LOW);
    data >>= 1;
  }
}

uint8_t readData() {
  int dataPins[] = {A0, A1, A2, A3, A4, A5, A6, A7};
  uint16_t data = 0;
  for (int i = 0; i < 8; i++) {
    data = (data << 1) | digitalRead(dataPins[7 - i]);
  }
  return data;
}

void writeMemoryByte(uint32_t address, uint8_t data) {
  setAddress(address);
  ale(true);
  delay(0);
  ale(false); // lock upper address bits

  enableDataOutput(true);
  setData(data);
  delay(0);

  memw(true);
  waitForDevice();
  memw(false);

  enableDataOutput(false);
}

void writeMemory(uint32_t address, uint16_t data) {
  writeMemoryByte(address, data & 0xFFu);
  writeMemoryByte(address + 1, ((data >> 8) & 0xFF));
}

uint8_t readMemoryByte(uint32_t address) {
  setAddress(address);
  ale(true);
  delay(0);
  ale(false); // lock upper address bits

  enableDataOutput(false);
  delay(0);

  memr(true);
  delay(0);
  waitForDevice();
  uint8_t data = readData();
  memr(false);

  return data;
}

uint16_t readMemory(uint32_t address) {
  return readMemoryByte(address) |
         (((uint16_t)readMemoryByte(address + 1)) << 8);
}

void writeIO(uint32_t portAddress, uint16_t data) {
  setAddress(portAddress);
  ale(true);
  delay(0);
  ale(false); // lock upper address bits

  enableDataOutput(true);
  setData(data);
  delay(0);

  iow(true);
  delay(0);
  waitForDevice();
  iow(false);

  enableDataOutput(false);
}

uint16_t readIO(uint32_t portAddress) {
  // mem16bit(true);

  setAddress(portAddress);
  ale(true);
  delay(0);
  ale(false); // lock upper address bits

  enableDataOutput(false);

  ior(true);
  delay(0);
  waitForDevice();

  uint16_t data = readData();

  ior(false);

  return data;

  // mem16bit(false);
}

