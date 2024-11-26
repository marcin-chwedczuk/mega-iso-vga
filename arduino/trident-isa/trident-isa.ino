
// References:
// https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
// https://chromium.googlesource.com/chromiumos/third_party/seabios/+/refs/heads/0.14.811.B/vgasrc/vgaio.c


// ISA Address is mapped to pins [30..53] (addr 0 - 23), which addr0 at pin 30
// and addr19 at pin 50

// ISA Data is mapped to pins A[0..7] with A0 being mapped to data0

#include "font.h"

#define PIN_ALE 4
#define PIN_MEM_16BIT 5
#define PIN_MEMW 3
#define PIN_MEMR 2
#define PIN_IOW 10
#define PIN_IOR 11
#define PIN_RESET 12
#define PIN_WAITSTATE 13

void ale(bool active) { digitalWrite(PIN_ALE, active ? HIGH : LOW); }

bool mem16bit() { return !!digitalRead(PIN_MEM_16BIT); }

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

void VgaIoWriteIx(uint32_t addr, uint16_t valIx) {
  writeIO(addr, valIx & 0xFFu);
  writeIO(addr + 1, (valIx >> 8) & 0xFFu);
}

uint8_t VgaIoReadIx(uint32_t addr, uint8_t ix) {
  uint16_t data = 0;
  writeIO(addr, ix);
  return readIO(addr + 1);
}

// VGA Stuff 

#define	VGA_AC_INDEX		    0x3C0
#define	VGA_AC_WRITE		    0x3C0
#define	VGA_AC_READ		      0x3C1
#define	VGA_MISC_WRITE	    0x3C2
#define VGA_SEQ_INDEX		    0x3C4
#define VGA_SEQ_DATA		    0x3C5
#define	VGA_DAC_READ_INDEX	0x3C7
#define	VGA_DAC_WRITE_INDEX	0x3C8
#define	VGA_DAC_DATA		    0x3C9
#define	VGA_MISC_READ		    0x3CC
#define VGA_GC_INDEX 		    0x3CE
#define VGA_GC_DATA 		    0x3CF
#define VGA_VIDEO_ENABLE    0x3C3
/*			COLOR emulation		MONO emulation */
#define VGA_CRTC_INDEX		  0x3D4		/* 0x3B4 */
#define VGA_CRTC_DATA		    0x3D5		/* 0x3B5 */
#define	VGA_INSTAT_READ		  0x3DA // Status register

#define	VGA_NUM_SEQ_REGS	  5
#define	VGA_NUM_CRTC_REGS	  25
#define	VGA_NUM_GC_REGS		  9
#define	VGA_NUM_AC_REGS		  21
#define	VGA_NUM_REGS		    (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)



void loadFont() {
  VgaIoWriteIx(VGA_GC_INDEX, 0x0005);
  VgaIoWriteIx(VGA_GC_INDEX, 0x0406);
  VgaIoWriteIx(VGA_SEQ_INDEX, 0x0402);

  VgaIoWriteIx(VGA_SEQ_INDEX, 0x0604);

  uint32_t ptr = 0xA0000;
  for (int i = 0; i < 1024 * 4; i++) {
    if (!(i % 16) && (i > 0)) {
      ptr += 16;
    }

    writeMemoryByte(ptr++, Font8x16[i + 1]);
  }

  VgaIoWriteIx(VGA_SEQ_INDEX, 0x0302);

  VgaIoWriteIx(VGA_SEQ_INDEX, 0x0204);
  VgaIoWriteIx(VGA_GC_INDEX, 0x1005);
  VgaIoWriteIx(VGA_GC_INDEX, 0x0E06);
}

#define IoPortOutB writeIO
#define IoPortInB readIO

//*********************************************************************
void sub_2EA(void) {
  VgaIoWriteIx(VGA_SEQ_INDEX, 0x000B); // Set oldmode
}
//*********************************************************************
uint8_t sub_292(void) {
  sub_2EA();                         // Set oldmode
  return (VgaIoReadIx(VGA_SEQ_INDEX, 0x0D)); // Old mode VGA_SEQ_INDEX.0x0D read
}
//*********************************************************************

uint8_t sub_4D9(void) {
  uint8_t al;

  al = sub_292() & 0x0E;
  if (al != 0x0C)
    return (1); // return no zero
  al = IoPortInB(VGA_MISC_READ) & 0x67;
  if (al != 0x67)
    return (1); // return no zero
  return (0);
}

//*********************************************************************
uint8_t sub_26A() {
  VgaIoReadIx(VGA_SEQ_INDEX, 0x0B); // New mode set
  VgaIoWriteIx(VGA_SEQ_INDEX, (((VgaIoReadIx(VGA_SEQ_INDEX, 0x0E) | 0x80) ^ 2) << 8) + 0x0E);
  return (VgaIoReadIx(VGA_SEQ_INDEX, 0x0C));
}
//*********************************************************************
void sub_179(void) {
  // SP BP manipulate
  VgaIoWriteIx(VGA_SEQ_INDEX, (((sub_26A() | 0x42) & 0xFE) << 8) + 0x0C);
  VgaIoWriteIx(VGA_SEQ_INDEX, ((VgaIoReadIx(VGA_SEQ_INDEX, 0x0F) | 0x80) << 8) + 0x0F);
  VgaIoWriteIx(VGA_SEQ_INDEX, (((VgaIoReadIx(VGA_SEQ_INDEX, 0x0E) & 0x7F) ^ 2) << 8) + 0x0E);
}
//*********************************************************************
void sub_51A(void) {
  uint8_t al, bh;
  bh = (sub_26A() | 0x80) & 0xFE;
  VgaIoWriteIx(VGA_SEQ_INDEX, 0x2407);
  IoPortOutB(VGA_MISC_WRITE, 0x01);
  if (!((al = VgaIoReadIx(VGA_CRTC_INDEX, 0x28)) & 0x0C)) {
    al |= 0x04;
    VgaIoWriteIx(VGA_CRTC_INDEX, (al << 8) + 0x28);
  }
  VgaIoWriteIx(VGA_SEQ_INDEX, ((VgaIoReadIx(VGA_SEQ_INDEX, 0x0F) & 0x7F) << 8) + 0x0F);
  VgaIoWriteIx(VGA_SEQ_INDEX, (bh << 8) + 0x0C);
  VgaIoWriteIx(VGA_SEQ_INDEX, (((VgaIoReadIx(VGA_SEQ_INDEX, 0x0E) & 0x7F) ^ 2) << 8) + 0x0E);
  if (VgaIoReadIx(VGA_SEQ_INDEX, 0x0F) & 0x08) {
    sub_179();
  }
  sub_2EA(); // Old mode
  VgaIoWriteIx(VGA_SEQ_INDEX, 0x200D);
  VgaIoWriteIx(VGA_SEQ_INDEX, 0xA00E);
  VgaIoReadIx(VGA_SEQ_INDEX, 0x0B); // New mode
  VgaIoWriteIx(VGA_SEQ_INDEX, 0x020E);
  if (!((al = VgaIoReadIx(VGA_GC_INDEX, 0x06)) & 0x0C)) {
    VgaIoWriteIx(VGA_GC_INDEX, (((al & 0xF3) | 0x04) << 8) + 0x06);
  }
  VgaIoWriteIx(VGA_SEQ_INDEX, 0x000D);
  al = VgaIoReadIx(VGA_CRTC_INDEX, 0x1E);
  VgaIoWriteIx(VGA_CRTC_INDEX, 0x001E);
}
//*********************************************************************

uint8_t Pal16[192] =
    //  R    G    B
    {
        0x00, 0x00, 0x00, // 0x00
        0x00, 0x00, 0x2A, // 0x01
        0x00, 0x2A, 0x00, // 0x02
        0x00, 0x2A, 0x2A, // 0x03
        0x2A, 0x00, 0x00, // 0x04
        0x2A, 0x00, 0x2A, // 0x05
        0x2A, 0x2A, 0x00, // 0x06
        0x2A, 0x2A, 0x2A, // 0x07
        0x00, 0x00, 0x15, // 0x08
        0x00, 0x00, 0x3F, // 0x09
        0x00, 0x2A, 0x15, // 0x0A
        0x00, 0x2A, 0x3F, // 0x0B
        0x2A, 0x00, 0x15, // 0x0C
        0x2A, 0x00, 0x3F, // 0x0D
        0x2A, 0x2A, 0x15, // 0x0E
        0x2A, 0x2A, 0x3F, // 0x0F
        0x00, 0x15, 0x00, // 0x10
        0x00, 0x15, 0x2A, // 0x11
        0x00, 0x3F, 0x00, // 0x12
        0x00, 0x3F, 0x2A, // 0x13
        0x2A, 0x15, 0x00, // 0x14
        0x2A, 0x15, 0x2A, // 0x15
        0x2A, 0x3F, 0x00, // 0x16
        0x2A, 0x3F, 0x2A, // 0x17
        0x00, 0x15, 0x15, // 0x18
        0x00, 0x15, 0x3F, // 0x19
        0x00, 0x3F, 0x15, // 0x1A
        0x00, 0x3F, 0x3F, // 0x1B
        0x2A, 0x15, 0x15, // 0x1C
        0x2A, 0x15, 0x3F, // 0x1D
        0x2A, 0x3F, 0x15, // 0x1E
        0x2A, 0x3F, 0x3F, // 0x1F
        0x15, 0x00, 0x00, // 0x20
        0x15, 0x00, 0x2A, // 0x21
        0x15, 0x2A, 0x00, // 0x22
        0x15, 0x2A, 0x2A, // 0x23
        0x3F, 0x00, 0x00, // 0x24
        0x3F, 0x00, 0x2A, // 0x25
        0x3F, 0x2A, 0x00, // 0x26
        0x3F, 0x2A, 0x2A, // 0x27
        0x15, 0x00, 0x15, // 0x28
        0x15, 0x00, 0x3F, // 0x29
        0x15, 0x2A, 0x15, // 0x2A
        0x15, 0x2A, 0x3F, // 0x2B
        0x3F, 0x00, 0x15, // 0x2C
        0x3F, 0x00, 0x3F, // 0x2D
        0x3F, 0x2A, 0x15, // 0x2E
        0x3F, 0x2A, 0x3F, // 0x2F
        0x15, 0x15, 0x00, // 0x30
        0x15, 0x15, 0x2A, // 0x31
        0x15, 0x3F, 0x00, // 0x32
        0x15, 0x3F, 0x2A, // 0x33
        0x3F, 0x15, 0x00, // 0x34
        0x3F, 0x15, 0x2A, // 0x35
        0x3F, 0x3F, 0x00, // 0x36
        0x3F, 0x3F, 0x2A, // 0x37
        0x15, 0x15, 0x15, // 0x38
        0x15, 0x15, 0x3F, // 0x39
        0x15, 0x3F, 0x15, // 0x3A
        0x15, 0x3F, 0x3F, // 0x3B
        0x3F, 0x15, 0x15, // 0x3C
        0x3F, 0x15, 0x3F, // 0x3D
        0x3F, 0x3F, 0x15, // 0x3E
        0x3F, 0x3F, 0x3F  // 0X3F
};

uint8_t Pal[768] = {
    0,  0,  0,  0,  0,  42, 0,  42, 0,  0,  42, 42, 42, 0,  0,  42, 0,  42, 42,
    21, 0,  42, 42, 42, 21, 21, 21, 21, 21, 63, 21, 63, 21, 21, 63, 63, 63, 21,
    21, 63, 21, 63, 63, 63, 21, 63, 63, 63, 0,  0,  0,  5,  5,  5,  8,  8,  8,
    11, 11, 11, 14, 14, 14, 17, 17, 17, 20, 20, 20, 24, 24, 24, 28, 28, 28, 32,
    32, 32, 36, 36, 36, 40, 40, 40, 45, 45, 45, 50, 50, 50, 56, 56, 56, 63, 63,
    63, 0,  0,  63, 16, 0,  63, 31, 0,  63, 47, 0,  63, 63, 0,  63, 63, 0,  47,
    63, 0,  31, 63, 0,  16, 63, 0,  0,  63, 16, 0,  63, 31, 0,  63, 47, 0,  63,
    63, 0,  47, 63, 0,  31, 63, 0,  16, 63, 0,  0,  63, 0,  0,  63, 16, 0,  63,
    31, 0,  63, 47, 0,  63, 63, 0,  47, 63, 0,  31, 63, 0,  16, 63, 31, 31, 63,
    39, 31, 63, 47, 31, 63, 55, 31, 63, 63, 31, 63, 63, 31, 55, 63, 31, 47, 63,
    31, 39, 63, 31, 31, 63, 39, 31, 63, 47, 31, 63, 55, 31, 63, 63, 31, 55, 63,
    31, 47, 63, 31, 39, 63, 31, 31, 63, 31, 31, 63, 39, 31, 63, 47, 31, 63, 55,
    31, 63, 63, 31, 55, 63, 31, 47, 63, 31, 39, 63, 45, 45, 63, 49, 45, 63, 54,
    45, 63, 58, 45, 63, 63, 45, 63, 63, 45, 58, 63, 45, 54, 63, 45, 49, 63, 45,
    45, 63, 49, 45, 63, 54, 45, 63, 58, 45, 63, 63, 45, 58, 63, 45, 54, 63, 45,
    49, 63, 45, 45, 63, 45, 45, 63, 49, 45, 63, 54, 45, 63, 58, 45, 63, 63, 45,
    58, 63, 45, 54, 63, 45, 49, 63, 0,  0,  28, 7,  0,  28, 14, 0,  28, 21, 0,
    28, 28, 0,  28, 28, 0,  21, 28, 0,  14, 28, 0,  7,  28, 0,  0,  28, 7,  0,
    28, 14, 0,  28, 21, 0,  28, 28, 0,  21, 28, 0,  14, 28, 0,  7,  28, 0,  0,
    28, 0,  0,  28, 7,  0,  28, 14, 0,  28, 21, 0,  28, 28, 0,  21, 28, 0,  14,
    28, 0,  7,  28, 14, 14, 28, 17, 14, 28, 21, 14, 28, 24, 14, 28, 28, 14, 28,
    28, 14, 24, 28, 14, 21, 28, 14, 17, 28, 14, 14, 28, 17, 14, 28, 21, 14, 28,
    24, 14, 28, 28, 14, 24, 28, 14, 21, 28, 14, 17, 28, 14, 14, 28, 14, 14, 28,
    17, 14, 28, 21, 14, 28, 24, 14, 28, 28, 14, 24, 28, 14, 21, 28, 14, 17, 28,
    20, 20, 28, 22, 20, 28, 24, 20, 28, 26, 20, 28, 28, 20, 28, 28, 20, 26, 28,
    20, 24, 28, 20, 22, 28, 20, 20, 28, 22, 20, 28, 24, 20, 28, 26, 20, 28, 28,
    20, 26, 28, 20, 24, 28, 20, 22, 28, 20, 20, 28, 20, 20, 28, 22, 20, 28, 24,
    20, 28, 26, 20, 28, 28, 20, 26, 28, 20, 24, 28, 20, 22, 28, 0,  0,  16, 4,
    0,  16, 8,  0,  16, 12, 0,  16, 16, 0,  16, 16, 0,  12, 16, 0,  8,  16, 0,
    4,  16, 0,  0,  16, 4,  0,  16, 8,  0,  16, 12, 0,  16, 16, 0,  12, 16, 0,
    8,  16, 0,  4,  16, 0,  0,  16, 0,  0,  16, 4,  0,  16, 8,  0,  16, 12, 0,
    16, 16, 0,  12, 16, 0,  8,  16, 0,  4,  16, 8,  8,  16, 10, 8,  16, 12, 8,
    16, 14, 8,  16, 16, 8,  16, 16, 8,  14, 16, 8,  12, 16, 8,  10, 16, 8,  8,
    16, 10, 8,  16, 12, 8,  16, 14, 8,  16, 16, 8,  14, 16, 8,  12, 16, 8,  10,
    16, 8,  8,  16, 8,  8,  16, 10, 8,  16, 12, 8,  16, 14, 8,  16, 16, 8,  14,
    16, 8,  12, 16, 8,  10, 16, 11, 11, 16, 12, 11, 16, 13, 11, 16, 15, 11, 16,
    16, 11, 16, 16, 11, 15, 16, 11, 13, 16, 11, 12, 16, 11, 11, 16, 12, 11, 16,
    13, 11, 16, 15, 11, 16, 16, 11, 15, 16, 11, 13, 16, 11, 12, 16, 11, 11, 16,
    11, 11, 16, 12, 11, 16, 13, 11, 16, 15, 11, 16, 16, 11, 15, 16, 11, 13, 16,
    11, 12, 16, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  63, 63, 63};

//*********************************************************************
void setpal(uint8_t color, uint8_t r, uint8_t g, uint8_t b) {
  IoPortOutB(VGA_DAC_WRITE_INDEX, color);
  IoPortOutB(VGA_DAC_DATA, r);
  IoPortOutB(VGA_DAC_DATA, g);
  IoPortOutB(VGA_DAC_DATA, b);
}

//*********************************************************************

void setpalette16() {
  uint16_t j = 0;
  uint16_t i;
  for (i = 0; i < 192; i += 3) {
    setpal(j, Pal16[i], Pal16[i + 1], Pal16[i + 2]);
    j++;
  }
}

//*********************************************************************

void setpalette256() {
  uint16_t j = 0;
  uint16_t i;
  for (i = 0; i < 768; i += 3) {
    setpal(j, Pal[i], Pal[i + 1], Pal[i + 2]);
    j++;
  }
}

//*********************************************************************

void TRSubsEnable(void) {
  IoPortOutB(VGA_VIDEO_ENABLE, 0x00);
  IoPortOutB(0x46E8, 0x16);
  IoPortOutB(0x46E9, 0x00);
  IoPortOutB(0x102, 0x01);
  IoPortOutB(0x103, 0x00);
  IoPortOutB(0x46E8, 0x0E);
  IoPortOutB(0x46E9, 0x00);
  IoPortOutB(0x4AE8, 0x00);
  IoPortOutB(0x4AE9, 0x00);
  //   IoPortOutB(VGA_MISC_WRITE,0x23);
}

typedef struct _VMODE_ST {
  uint8_t mode;            // Videomode Number
  uint16_t width;          // Width in pixels
  uint16_t height;         // Height in pixels
  uint16_t width_uint8_ts; // Number of uint8_ts per screen
  uint16_t colors;         // Number of colors
  uint16_t attrib;         // Videomode attributes
} VMODE_ST;

VMODE_ST Mode; // Mode info

// Same as Mode 02H
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

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

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
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

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


// Video mode numbers

#define MODE03H 0x03
#define MODE12H 0x12
#define MODE13H 0x13

#define TVU_TEXT 0x0001
#define TVU_GRAPHICS 0x0002
#define TVU_MONOCHROME 0x0004
#define TVU_PLANAR 0x0008
#define TVU_UNCHAINED 0x0010

void ModeSet(uint8_t *dataptr) {

  uint8_t i;

  IoPortOutB(VGA_MISC_WRITE,
             0x67); // Before acess registers must be set address sheme

  IoPortOutB(VGA_MISC_WRITE, *dataptr);
  dataptr++;

  IoPortOutB(VGA_INSTAT_READ, *dataptr);
  dataptr++;

  for (i = 0; i < 5; i++) {
    IoPortOutB(VGA_SEQ_INDEX, i);
    IoPortOutB(VGA_SEQ_INDEX + 1, *dataptr);
    dataptr++;
  }

  VgaIoWriteIx(VGA_CRTC_INDEX, 0x0E11);

  i = VgaIoReadIx(VGA_CRTC_INDEX, 0x11);

  for (i = 0; i < 25; i++) {
    VgaIoWriteIx(VGA_CRTC_INDEX, ((*dataptr) << 8) + i);
    dataptr++;
  }

  for (i = 0; i < 9; i++) {
    VgaIoWriteIx(VGA_GC_INDEX, ((*dataptr) << 8) + i);
    dataptr++;
  }

  i = IoPortInB(VGA_INSTAT_READ);

  for (i = 0; i < 21; i++) {
    IoPortInB(VGA_INSTAT_READ);
    IoPortOutB(VGA_AC_INDEX, i);
    IoPortOutB(VGA_AC_INDEX, *dataptr);
    dataptr++;
  }
  VgaIoWriteIx(VGA_GC_INDEX, 0x3A0B);
  IoPortInB(VGA_INSTAT_READ);
  IoPortOutB(VGA_AC_INDEX, 0x20);
  IoPortOutB(0x3C6, 0xFF);
}

//*********************************************************************

void SetVideoMode(uint8_t mode) {

  Mode.mode = mode;
  if (mode == MODE03H) // 80 x 25 x 16
  {
    ModeSet(mode03h);
    setpalette16();
    // FontsRead(Font8x16,16);
    loadFont();

    Mode.width = 80;
    Mode.height = 25;
    Mode.width_uint8_ts = 2000;
    Mode.colors = 16;
    Mode.attrib = TVU_TEXT;
  } else if (mode == MODE12H) // 640 x 480 x 16
  {
    ModeSet(mode12h);
    setpalette16();

    Mode.width = 640;
    Mode.height = 480;
    Mode.width_uint8_ts = 38400u;
    Mode.colors = 16;
    Mode.attrib = TVU_GRAPHICS | TVU_PLANAR;
  } else if (mode == MODE13H) // 320 x 200 x 256
  {
    ModeSet(mode13h);
    setpalette256();

    Mode.width = 320;
    Mode.height = 200;
    Mode.width_uint8_ts = 64000u;
    Mode.colors = 256;
    Mode.attrib = TVU_GRAPHICS;
  }
}

void TR9000i_Init(void) {
  TRSubsEnable();
  VgaIoWriteIx(VGA_SEQ_INDEX, 0x000B); //  Force old_mode_registers
  unsigned int chp =
      IoPortInB(VGA_SEQ_DATA); //  Read chip ID and switch to new_mode_registers}
  unsigned int old = VgaIoReadIx(VGA_SEQ_INDEX, 0x0E);
  IoPortOutB(VGA_SEQ_DATA, 0x00);
  unsigned int value = IoPortInB(VGA_SEQ_DATA) & 0x0F;
  IoPortOutB(VGA_SEQ_DATA, old);
  Serial.print("detected chip: ");
  Serial.println(chp, HEX);
  IoPortOutB(VGA_SEQ_DATA, old ^ 2);

  uint16_t i = 0;
  IoPortOutB(VGA_VIDEO_ENABLE, 0x00);
  if (!sub_4D9()) {
    Serial.println("Initialization failed.");
    return;
  }

  do {
    IoPortOutB(VGA_DAC_DATA, 0x00);
    i++;
  } while (i < 768);

  IoPortOutB(VGA_MISC_WRITE, 0x23);
  sub_51A();
  //  IoPortOutB(VGA_CRTC_INDEX,0x1F);
  //  IoPortOutB(VGA_CRTC_DATA,0x81);

  //  IoPortOutB(VGA_CRTC_INDEX,0x25);
  //  IoPortOutB(VGA_CRTC_DATA,0xFF);

  // if(((sub_292()&0x0E)==0x0C)&& IoPortInB(VGA_MISC_READ)==0x67));
}
//*********************************************************************

#define SEGMENT_OFFSET(seg, off) ((((uint32_t)(seg) << 8)) + (uint32_t)off)

uint32_t VidMemBase = SEGMENT_OFFSET(0xB800, 0);

uint16_t CursorScreen = 0;
uint8_t CurrentAttrib = 0x0F;
void SetHwCursor(uint16_t pos) {
  VgaIoWriteIx(VGA_CRTC_INDEX, ((CursorScreen / 256) << 8) + 0x0E);
  VgaIoWriteIx(VGA_CRTC_INDEX, ((CursorScreen % 256) << 8) + 0x0F);
}

void VgaMemoryWriteW(uint32_t addr, uint16_t data) { writeMemory(addr, data); }

void TextClear(uint8_t attrib) {
  uint16_t i, ilim = Mode.width_uint8_ts;
  uint32_t address = VidMemBase;

  for (i = 0; i < ilim; i++) {
    VgaMemoryWriteW(address, (attrib << 8) + ' ');
    address++;
    //    VgaMemoryWriteB(address,attrib);
    address++;
  }
  CurrentAttrib = attrib;
  CursorScreen = 0;
  SetHwCursor(CursorScreen);
}

void pixel12H(uint16_t x, uint16_t y, uint8_t color) {
  VgaIoWriteIx(VGA_GC_INDEX, ((1 << (((x % 256) & 7) ^ 7)) << 8) + 0x08);
  VgaIoWriteIx(VGA_SEQ_INDEX, (color << 8) + 0x02);
  readMemoryByte(0xA00000 + (y * 80) + (x >> 3));
  writeMemoryByte(0xA00000 + (y * 80) + (x >> 3), 0xFF);
}

void Pixel13H(uint16_t x, uint16_t y, uint8_t color);

void pixel(uint16_t x, uint16_t y, uint8_t color) {
  int width = Mode.width;

  if (Mode.mode == MODE13H)
    Pixel13H(x, y, color);
  else if (Mode.attrib & TVU_UNCHAINED) {

    //     IoPortOutB(VGA_SEQ_DATA,1<<((x%256)&3));
    VgaIoWriteIx(VGA_SEQ_INDEX, ((1 << ((x % 256) & 3)) << 8) + 0x02);

    writeMemoryByte(0xA00000 + (x / 4) + (y * (width / 4)), color);
  } else if (Mode.attrib & TVU_PLANAR) {

    //     IoPortOutB(VGA_GC_INDEX,0x08);
    //     IoPortOutB(VGA_GC_DATA,1<<(((x%256)&7)^7));
    //     IoPortOutB(VGA_GC_DATA,rv);
    VgaIoWriteIx(VGA_GC_INDEX, ((1 << (((x % 256) & 7) ^ 7)) << 8) + 0x08);
    //     IoPortOutB(VGA_GC_DATA,0x0F);
    //     IoPortOutB(VGA_SEQ_INDEX,0x02);
    //     IoPortOutB(VGA_SEQ_DATA,color);
    VgaIoWriteIx(VGA_SEQ_INDEX, (color << 8) + 0x02);

    readMemoryByte(0xA00000 + (y * 80) + (x >> 3));

    writeMemoryByte(0xA00000 + (y * 80) + (x >> 3), 0xFF);

    //     IoPortOutB(VGA_SEQ_INDEX,0x02);
    //     IoPortOutB(VGA_SEQ_DATA,0x0F);
    //     VgaIoWriteIx(VGA_SEQ_INDEX,(0x0F<<8)+0x02);

    //     IoPortOutB(VGA_GC_INDEX,0x08);
    //     IoPortOutB(VGA_GC_DATA,0xFF);
    //     VgaIoWriteIx(VGA_GC_INDEX,(0xFF<<8)+0x08);
  }
}

void SetScrPage(uint8_t page) // Set desired screen page
{
  uint32_t addr;

  addr = 2000 * (page & 0x07);
  VgaIoWriteIx(VGA_CRTC_INDEX, ((addr / 256) << 8) + 0x0C);
  VgaIoWriteIx(VGA_CRTC_INDEX, ((addr % 256) << 8) + 0x0D);
  VidMemBase = 0xB08000 + (addr << 1); // Error here?
}
//*********************************************************************
void CursorOn(void) {
  VgaIoWriteIx(VGA_CRTC_INDEX, ((VgaIoReadIx(VGA_CRTC_INDEX, 0x0A) & (0xFF - 0x20)) << 8) + 0x0A);
}

void rect(int x, int y, int w, int h, uint8_t c) {
  for (int i = x; i < x + w; i++)
    for (int j = y; j < y + h; j++)
      pixel(i, j, c);
}

void DrawCircle(uint16_t x, uint16_t y, uint16_t rad, uint8_t color) {
  long Hiba; // hiba valtozo
  long X;
  long Y;
  long DU; // hiba modosito, ha csak X lepett
  long DD; // hiba modosito, ha X es Y is lepett

  Hiba = 1L - rad;
  X = 0;
  Y = (long)rad;
  DU = 3L;
  DD = 5L - (2L * rad);

  // kezdopont kirajzol
  pixel(X + x, Y + y, color);
  // pixel12H(X+x,Y+y,color);

  //  KorCikkInterpolal:

  //  DoEvents

  //  'Vege a 90 foktol 45 fokigtarto resznek?
  while (!(X > Y)) {
    //'x mindig lep
    X = X + 1;
    if (Hiba < 0) {
      // az x-heztartozo felso y a jobb
      // azaz az aktualison marad
      Hiba = Hiba + DU;
      DU = DU + 2;
      DD = DD + 2;
    } else {
      // az x-heztartozo also y a jobb
      // azzaz y mar lephet egyet lefele
      Y = Y - 1;
      Hiba = Hiba + DD;
      DU = DU + 2;
      DD = DD + 4;
    }
    // aktualis kirajzol
    pixel(X + x, Y + y, color);
    // pixel12H(X+x, Y+y,color);
  }
}

void Clear13H(uint8_t Color) {

  uint16_t i;
  uint32_t address = 0xA0000;

  i = 0;
  do {
    writeMemoryByte(address, Color);
    address++;
    i++;
  } while (i);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  // Setup reset pin, raise RESET line
  pinMode(PIN_RESET, OUTPUT);
  digitalWrite(PIN_RESET, HIGH);

  pinMode(PIN_WAITSTATE, INPUT_PULLUP);

  pinMode(PIN_MEM_16BIT, INPUT);

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

  // Setup Address bus pins
  for (int pin = 30; pin <= 53; pin++) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  // Setup data lines
  enableDataOutput(false);

  // "Unreset" device
  delay(1);
  digitalWrite(PIN_RESET, LOW);

  delay(3000);
  TR9000i_Init();
  SetVideoMode(MODE03H);

  // TextClear(0x0F);

  SetScrPage(0);
  SetHwCursor(8);
  CursorOn();

  // Clear13H(0x00);
  // for (int i = 0; i < 300; i++) for (int j = 0; i < 300; i++) pixel(i, i,
  // 0x01);

  Serial.println("dumped");
}

void Pixel13H(uint16_t x, uint16_t y, uint8_t color) {
  //     IoPortOutB(VGA_SEQ_DATA,1<<((x%256)&3));
  VgaIoWriteIx(VGA_SEQ_INDEX, ((1 << ((x % 256) & 3)) << 8) + 0x02);
  //   XOR DI,DI

  writeMemoryByte(0xA0000 + (x + (y * Mode.width)), color);
}

#define outb writeIO
#define inb readIO

void update_cursor(uint16_t pos) {
  // uint16_t pos = y * mode.width + x;

  outb(VGA_CRTC_INDEX, 0x0F);
  outb(VGA_CRTC_DATA, (uint8_t)(pos & 0xFF));
  outb(VGA_CRTC_INDEX, 0x0E);
  outb(VGA_CRTC_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

uint32_t abc = 0xB8000, ii = 0, attr = 0x17;
bool done = false;

void loop() {
  if (!done) {
    // clear screen
    abc = 0xB8000;
    for (int i = 0; i < 25; i++) {
      for (int j = 0; j < 80; j++) {
        writeMemory(abc, 0x0000);
        abc += 2;
      }
    }

    abc = 0xB8000;
    update_cursor(0);
    const char *str = "Hello world!";
    while (*str) {
      writeMemory(abc, (attr << 8) + (*str));

      str++;
      abc += 2;
    }
  }
  done = true;
  /*
  writeMemory(abc, (attr << 8) + (ii & 0xFF));
  abc += 2;
  update_cursor(ii);  ii +=1;

  if (ii > (80 * 25)) {
    ii = 0; attr++; abc = 0xB8000;
  }*/
}
