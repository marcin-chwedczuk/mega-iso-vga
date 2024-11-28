
#include "isa.h"
#include <Arduino.h>

// ISA Address is mapped to pins [30..50) (addr 0 - 23), with addr0 at pin 30
// and addr19 at pin 49

// ISA Data is mapped to pins A[0..7] with A0 being mapped to data0

#define PIN_RESET 12
#define PIN_ALE 4
#define PIN_MEMW 3
#define PIN_MEMR 2
#define PIN_IOW 10
#define PIN_IOR 11
#define PIN_WAITSTATE 13

// Private functions:
void isa_ale(bool active);
void isa_memw(bool active);
void isa_memr(bool active);
void isa_iow(bool active);
void isa_ior(bool active);
void isa_wait_device_ready();
void isa_set_address(uint32_t address);
void isa_enable_output_lines(bool enable);
void isa_set_data(uint8_t data);
uint8_t isa_read_data(void);

void isa_arduino_setup() {
  // Setup reset pin, raise RESET line
  pinMode(PIN_RESET, OUTPUT);
  digitalWrite(PIN_RESET, HIGH);

  pinMode(PIN_ALE, OUTPUT);
  digitalWrite(PIN_ALE, LOW);

  // Setup memory management pins
  pinMode(PIN_MEMW, OUTPUT);
  digitalWrite(PIN_MEMW, HIGH);

  pinMode(PIN_MEMR, OUTPUT);
  digitalWrite(PIN_MEMR, HIGH);

  pinMode(PIN_IOW, OUTPUT);
  digitalWrite(PIN_IOW, HIGH);

  pinMode(PIN_IOR, OUTPUT);
  digitalWrite(PIN_IOR, HIGH);

  // Setup address bus pins on pins [30..50)
  DDRA = 0xFF;
  PORTA = 0x00;
  DDRC = 0xFF;
  PORTC = 0x00;
  DDRL = 0xFF;
  PORTL = 0x00;
  /*
  for (int pin = 30; pin < 50; pin++) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  */

  // Setup data lines
  isa_enable_output_lines(false);

  // "Unreset" device
  delay(1);
  digitalWrite(PIN_RESET, LOW);
}

void isa_ale(bool active) {
  digitalWrite(PIN_ALE, active ? HIGH : LOW);
}

void isa_memw(bool active) {
  digitalWrite(PIN_MEMW, active ? LOW : HIGH);
}
void isa_memr(bool active) {
  digitalWrite(PIN_MEMR, active ? LOW : HIGH);
}

void isa_iow(bool active) {
  digitalWrite(PIN_IOW, active ? LOW : HIGH);
}
void isa_ior(bool active) {
  digitalWrite(PIN_IOR, active ? LOW : HIGH);
}

void isa_wait_device_ready() {
  delay(0);  // initial delay
  while (!digitalRead(PIN_WAITSTATE)) {
    // wait
  }
}

void isa_set_address(uint32_t address) {
  // PORTL[3..0] | PORTC[7..0] | PORTA[7...0]

  /*
  for (int pin = 30; pin < 50; pin++) {
    uint8_t bit = (address & 1u);
    digitalWrite(pin, bit ? HIGH : LOW);
    address >>= 1;
  }*/

  PORTA = (uint8_t)((address >> 0)  & 0xFF);
  PORTC = (uint8_t)((address >> 8)  & 0xFF);
  PORTL = (uint8_t)((address >> 16) & 0xFF);
}

void isa_enable_output_lines(bool enable) {
  DDRF = enable ? 0xFF : 0x00;
  if (!enable) {
    PORTF = 0x00; // Disable pull'ups
  }

  /*
  int dataPins[] = { A0, A1, A2, A3, A4, A5, A6, A7 };
  for (int i = 0; i < 8; i++) {
    pinMode(dataPins[i], enable ? OUTPUT : INPUT);
  }
  */
}

void isa_set_data(uint8_t data) {
  /*
  int dataPins[] = { A0, A1, A2, A3, A4, A5, A6, A7 };
  for (int i = 0; i < 8; i++) {
    uint16_t bit = (data & 1u);
    digitalWrite(dataPins[i], bit ? HIGH : LOW);
    data >>= 1;
  }
  */

  PORTF = data;
}

uint8_t isa_read_data() {
  /*
  int dataPins[] = { A0, A1, A2, A3, A4, A5, A6, A7 };
  uint16_t data = 0;
  for (int i = 0; i < 8; i++) {
    data = (data << 1) | digitalRead(dataPins[7 - i]);
  }
  return data;
  */
  return PINF;
}

void isa_write_byte(uint32_t address, uint8_t data) {
  isa_set_address(address);
  isa_ale(true);
  // delay(0);
  isa_ale(false);  // lock upper address bits

  isa_enable_output_lines(true);
  isa_set_data(data);
  // delay(0);

  isa_memw(true);
  isa_wait_device_ready();
  isa_memw(false);

  isa_enable_output_lines(false);
}

void isa_write_word(uint32_t address, uint16_t data) {
  isa_write_byte(address, data & 0xFFu);
  isa_write_byte(address + 1, ((data >> 8) & 0xFFu));
}

uint8_t isa_read_byte(uint32_t address) {
  isa_set_address(address);
  isa_ale(true);
  // delay(0);
  isa_ale(false);  // lock upper address bits

  isa_enable_output_lines(false);
  // delay(0);

  isa_memr(true);
  // delay(0);
  isa_wait_device_ready();
  uint8_t data = isa_read_data();
  isa_memr(false);

  return data;
}

uint16_t isa_read_word(uint32_t address) {
  return isa_read_byte(address) | (((uint16_t)isa_read_byte(address + 1)) << 8);
}

void isa_outb(uint32_t portAddress, uint8_t data) {
  isa_set_address(portAddress);
  isa_ale(true);
  // delay(0);
  isa_ale(false);  // lock upper address bits

  isa_enable_output_lines(true);
  isa_set_data(data);
  // delay(0);

  isa_iow(true);
  // delay(0);
  isa_wait_device_ready();
  isa_iow(false);

  isa_enable_output_lines(false);
}

uint8_t isa_inb(uint32_t portAddress) {
  isa_set_address(portAddress);
  isa_ale(true);
  // delay(0);
  isa_ale(false);  // lock upper address bits

  isa_enable_output_lines(false);

  isa_ior(true);
  // delay(0);
  isa_wait_device_ready();
  uint8_t data = isa_read_data();
  isa_ior(false);

  return data;
}
