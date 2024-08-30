

// ISA Address is mapped to pins [30..53] (addr 0 - 23), which addr0 at pin 30 and addr19 at pin 50

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

void ale(bool active) {
  digitalWrite(PIN_ALE, active ? HIGH : LOW);
}

bool mem16bit() {
  return !!digitalRead(PIN_MEM_16BIT);
}

void memw(bool active) {
  digitalWrite(PIN_MEMW, active ? LOW : HIGH);
}
void memr(bool active) {
  digitalWrite(PIN_MEMR, active ? LOW : HIGH);
}

void iow(bool active) {
  digitalWrite(PIN_IOW, active ? LOW : HIGH);
}
void ior(bool active) {
  digitalWrite(PIN_IOR, active ? LOW : HIGH);
}

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
  int dataPins[] = {A0,A1,A2,A3,A4,A5,A6,A7};
  for (int i = 0; i < 8; i++) {
    pinMode(dataPins[i], enable ? OUTPUT : INPUT);
  }
}

void setData(uint8_t data) {
  int dataPins[] = {A0,A1,A2,A3,A4,A5,A6,A7};
  for (int i = 0; i < 8; i++) {
    uint16_t bit = (data & 1u);
    digitalWrite(dataPins[i], bit ? HIGH : LOW);
    data >>= 1;
  }
}

uint8_t readData() {
  int dataPins[] = {A0,A1,A2,A3,A4,A5,A6,A7};
  uint16_t data = 0;
  for (int i = 0; i < 8; i++) {
    data = (data<<1) | digitalRead(dataPins[7 - i]);
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
  writeMemoryByte(address+1, ((data >> 8) & 0xFF));
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
  return readMemoryByte(address) | ( ((uint16_t)readMemoryByte(address+1)) << 8 );
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
  return readIO(addr+1);
}



void loadFont() {
  VgaIoWriteIx(0x03ce, 0x0005);
  VgaIoWriteIx(0x03ce, 0x0406);
  VgaIoWriteIx(0x03c4, 0x0402);
  
  VgaIoWriteIx(0x03c4, 0x0604);

  uint32_t ptr = 0xA0000;
  for (int i = 0; i < 1024*4; i++) {
    if (!(i % 16) && (i > 0)) {
      ptr += 16;
    }

    writeMemoryByte(ptr++, Font8x16[i+1]);
  }

  VgaIoWriteIx(0x03c4, 0x0302);

    VgaIoWriteIx(0x03c4, 0x0204);
      VgaIoWriteIx(0x03ce, 0x1005);
        VgaIoWriteIx(0x03ce,0x0E06 );
}


#define IoPortOutB writeIO
#define IoPortInB readIO


//*********************************************************************
void  sub_2EA(void)
{
  VgaIoWriteIx(0x3C4,0x000B);            // Set oldmode
}
//*********************************************************************
uint8_t  sub_292(void)
{
  sub_2EA();                              // Set oldmode
  return(VgaIoReadIx(0x3C4,0x0D));        // Old mode 0x3C4.0x0D read
}
//*********************************************************************

uint8_t  sub_4D9(void)
{
  uint8_t al;

  al = sub_292()&0x0E;
  if(al != 0x0C) return(1);             // return no zero
  al = IoPortInB(0x3CC)&0x67;
  if(al != 0x67) return(1);             // return no zero
  return(0);
}

//*********************************************************************
uint8_t  sub_26A()
{
  VgaIoReadIx(0x3C4,0x0B);              // New mode set
  VgaIoWriteIx(0x3C4,(((VgaIoReadIx(0x3C4,0x0E)|0x80)^2)<<8)+0x0E);
  return (VgaIoReadIx(0x3C4,0x0C));
}
//*********************************************************************
void  sub_179(void)
{
  // SP BP manipulate
  VgaIoWriteIx(0x3C4,(((sub_26A()|0x42)&0xFE)<<8)+0x0C);
  VgaIoWriteIx(0x3C4,((VgaIoReadIx(0x3C4,0x0F)|0x80)<<8)+0x0F);
  VgaIoWriteIx(0x3C4,(((VgaIoReadIx(0x3C4,0x0E)&0x7F)^2)<<8)+0x0E);
}
//*********************************************************************
void sub_51A(void)
{
  uint8_t  al,bh;
  bh = (sub_26A()|0x80)&0xFE;
  VgaIoWriteIx(0x3C4,0x2407);
  IoPortOutB(0x3C2,0x01);
  if(!((al = VgaIoReadIx(0x3D4,0x28))&0x0C))
  {
    al |= 0x04;
    VgaIoWriteIx(0x3D4,(al<<8)+0x28);
  }
  VgaIoWriteIx(0x3C4,((VgaIoReadIx(0x3C4,0x0F)&0x7F)<<8)+0x0F);
  VgaIoWriteIx(0x3C4,(bh<<8)+0x0C);
  VgaIoWriteIx(0x3C4,(((VgaIoReadIx(0x3C4,0x0E)&0x7F)^2)<<8)+0x0E);
  if(VgaIoReadIx(0x3C4,0x0F)&0x08)
  {
    sub_179();
  }
  sub_2EA();                  // Old mode
  VgaIoWriteIx(0x3C4,0x200D);
  VgaIoWriteIx(0x3C4,0xA00E);
  VgaIoReadIx(0x3C4,0x0B);    // New mode
  VgaIoWriteIx(0x3C4,0x020E);
  if(!((al = VgaIoReadIx(0x3CE,0x06))&0x0C))
  {
     VgaIoWriteIx(0x3CE,(((al&0xF3)|0x04)<<8)+0x06);
  }
  VgaIoWriteIx(0x3C4,0x000D);
  al = VgaIoReadIx(0x3D4,0x1E);
  VgaIoWriteIx(0x3D4,0x001E);
}
//*********************************************************************

 uint8_t Pal16[192] =
//  R    G    B
{
  0x00,0x00,0x00,       // 0x00
  0x00,0x00,0x2A,       // 0x01
  0x00,0x2A,0x00,       // 0x02
  0x00,0x2A,0x2A,       // 0x03
  0x2A,0x00,0x00,       // 0x04
  0x2A,0x00,0x2A,       // 0x05
  0x2A,0x2A,0x00,       // 0x06
  0x2A,0x2A,0x2A,       // 0x07
  0x00,0x00,0x15,       // 0x08
  0x00,0x00,0x3F,       // 0x09
  0x00,0x2A,0x15,       // 0x0A
  0x00,0x2A,0x3F,       // 0x0B
  0x2A,0x00,0x15,       // 0x0C
  0x2A,0x00,0x3F,       // 0x0D
  0x2A,0x2A,0x15,       // 0x0E
  0x2A,0x2A,0x3F,       // 0x0F
  0x00,0x15,0x00,       // 0x10
  0x00,0x15,0x2A,       // 0x11
  0x00,0x3F,0x00,       // 0x12
  0x00,0x3F,0x2A,       // 0x13
  0x2A,0x15,0x00,       // 0x14
  0x2A,0x15,0x2A,       // 0x15
  0x2A,0x3F,0x00,       // 0x16
  0x2A,0x3F,0x2A,       // 0x17
  0x00,0x15,0x15,       // 0x18
  0x00,0x15,0x3F,       // 0x19
  0x00,0x3F,0x15,       // 0x1A
  0x00,0x3F,0x3F,       // 0x1B
  0x2A,0x15,0x15,       // 0x1C
  0x2A,0x15,0x3F,       // 0x1D
  0x2A,0x3F,0x15,       // 0x1E
  0x2A,0x3F,0x3F,       // 0x1F
  0x15,0x00,0x00,       // 0x20
  0x15,0x00,0x2A,       // 0x21
  0x15,0x2A,0x00,       // 0x22
  0x15,0x2A,0x2A,       // 0x23
  0x3F,0x00,0x00,       // 0x24
  0x3F,0x00,0x2A,       // 0x25
  0x3F,0x2A,0x00,       // 0x26
  0x3F,0x2A,0x2A,       // 0x27
  0x15,0x00,0x15,       // 0x28
  0x15,0x00,0x3F,       // 0x29
  0x15,0x2A,0x15,       // 0x2A
  0x15,0x2A,0x3F,       // 0x2B
  0x3F,0x00,0x15,       // 0x2C
  0x3F,0x00,0x3F,       // 0x2D
  0x3F,0x2A,0x15,       // 0x2E
  0x3F,0x2A,0x3F,       // 0x2F
  0x15,0x15,0x00,       // 0x30
  0x15,0x15,0x2A,       // 0x31
  0x15,0x3F,0x00,       // 0x32
  0x15,0x3F,0x2A,       // 0x33
  0x3F,0x15,0x00,       // 0x34
  0x3F,0x15,0x2A,       // 0x35
  0x3F,0x3F,0x00,       // 0x36
  0x3F,0x3F,0x2A,       // 0x37
  0x15,0x15,0x15,       // 0x38
  0x15,0x15,0x3F,       // 0x39
  0x15,0x3F,0x15,       // 0x3A
  0x15,0x3F,0x3F,       // 0x3B
  0x3F,0x15,0x15,       // 0x3C
  0x3F,0x15,0x3F,       // 0x3D
  0x3F,0x3F,0x15,       // 0x3E
  0x3F,0x3F,0x3F        // 0X3F
};



 uint8_t Pal[768] = {
 0,  0,  0,  0,  0, 42,  0, 42,  0,  0, 42, 42, 42,  0,  0, 42,  0, 42,
42, 21,  0, 42, 42, 42, 21, 21, 21, 21, 21, 63, 21, 63, 21, 21, 63, 63,
63, 21, 21, 63, 21, 63, 63, 63, 21, 63, 63, 63,  0,  0,  0,  5,  5,  5,
 8,  8,  8, 11, 11, 11, 14, 14, 14, 17, 17, 17, 20, 20, 20, 24, 24, 24,
28, 28, 28, 32, 32, 32, 36, 36, 36, 40, 40, 40, 45, 45, 45, 50, 50, 50,
56, 56, 56, 63, 63, 63,  0,  0, 63, 16,  0, 63, 31,  0, 63, 47,  0, 63,
63,  0, 63, 63,  0, 47, 63,  0, 31, 63,  0, 16, 63,  0,  0, 63, 16,  0,
63, 31,  0, 63, 47,  0, 63, 63,  0, 47, 63,  0, 31, 63,  0, 16, 63,  0,
 0, 63,  0,  0, 63, 16,  0, 63, 31,  0, 63, 47,  0, 63, 63,  0, 47, 63,
 0, 31, 63,  0, 16, 63, 31, 31, 63, 39, 31, 63, 47, 31, 63, 55, 31, 63,
63, 31, 63, 63, 31, 55, 63, 31, 47, 63, 31, 39, 63, 31, 31, 63, 39, 31,
63, 47, 31, 63, 55, 31, 63, 63, 31, 55, 63, 31, 47, 63, 31, 39, 63, 31,
31, 63, 31, 31, 63, 39, 31, 63, 47, 31, 63, 55, 31, 63, 63, 31, 55, 63,
31, 47, 63, 31, 39, 63, 45, 45, 63, 49, 45, 63, 54, 45, 63, 58, 45, 63,
63, 45, 63, 63, 45, 58, 63, 45, 54, 63, 45, 49, 63, 45, 45, 63, 49, 45,
63, 54, 45, 63, 58, 45, 63, 63, 45, 58, 63, 45, 54, 63, 45, 49, 63, 45,
45, 63, 45, 45, 63, 49, 45, 63, 54, 45, 63, 58, 45, 63, 63, 45, 58, 63,
45, 54, 63, 45, 49, 63,  0,  0, 28,  7,  0, 28, 14,  0, 28, 21,  0, 28,
28,  0, 28, 28,  0, 21, 28,  0, 14, 28,  0,  7, 28,  0,  0, 28,  7,  0,
28, 14,  0, 28, 21,  0, 28, 28,  0, 21, 28,  0, 14, 28,  0,  7, 28,  0,
 0, 28,  0,  0, 28,  7,  0, 28, 14,  0, 28, 21,  0, 28, 28,  0, 21, 28,
 0, 14, 28,  0,  7, 28, 14, 14, 28, 17, 14, 28, 21, 14, 28, 24, 14, 28,
28, 14, 28, 28, 14, 24, 28, 14, 21, 28, 14, 17, 28, 14, 14, 28, 17, 14,
28, 21, 14, 28, 24, 14, 28, 28, 14, 24, 28, 14, 21, 28, 14, 17, 28, 14,
14, 28, 14, 14, 28, 17, 14, 28, 21, 14, 28, 24, 14, 28, 28, 14, 24, 28,
14, 21, 28, 14, 17, 28, 20, 20, 28, 22, 20, 28, 24, 20, 28, 26, 20, 28,
28, 20, 28, 28, 20, 26, 28, 20, 24, 28, 20, 22, 28, 20, 20, 28, 22, 20,
28, 24, 20, 28, 26, 20, 28, 28, 20, 26, 28, 20, 24, 28, 20, 22, 28, 20,
20, 28, 20, 20, 28, 22, 20, 28, 24, 20, 28, 26, 20, 28, 28, 20, 26, 28,
20, 24, 28, 20, 22, 28,  0,  0, 16,  4,  0, 16,  8,  0, 16, 12,  0, 16,
16,  0, 16, 16,  0, 12, 16,  0,  8, 16,  0,  4, 16,  0,  0, 16,  4,  0,
16,  8,  0, 16, 12,  0, 16, 16,  0, 12, 16,  0,  8, 16,  0,  4, 16,  0,
 0, 16,  0,  0, 16,  4,  0, 16,  8,  0, 16, 12,  0, 16, 16,  0, 12, 16,
 0,  8, 16,  0,  4, 16,  8,  8, 16, 10,  8, 16, 12,  8, 16, 14,  8, 16,
16,  8, 16, 16,  8, 14, 16,  8, 12, 16,  8, 10, 16,  8,  8, 16, 10,  8,
16, 12,  8, 16, 14,  8, 16, 16,  8, 14, 16,  8, 12, 16,  8, 10, 16,  8,
 8, 16,  8,  8, 16, 10,  8, 16, 12,  8, 16, 14,  8, 16, 16,  8, 14, 16,
 8, 12, 16,  8, 10, 16, 11, 11, 16, 12, 11, 16, 13, 11, 16, 15, 11, 16,
16, 11, 16, 16, 11, 15, 16, 11, 13, 16, 11, 12, 16, 11, 11, 16, 12, 11,
16, 13, 11, 16, 15, 11, 16, 16, 11, 15, 16, 11, 13, 16, 11, 12, 16, 11,
11, 16, 11, 11, 16, 12, 11, 16, 13, 11, 16, 15, 11, 16, 16, 11, 15, 16,
11, 13, 16, 11, 12, 16,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
 0,  0,  0,  0,  0,  0,  0,  0,  0, 63, 63, 63
};


//*********************************************************************
void setpal(uint8_t color, uint8_t r, uint8_t g, uint8_t b)
{
//   asm {
   // Send color
//   MOV AX,color
//   MOV DX,03C8H
//   OUT DX,AL
   IoPortOutB(0x03C8,color);


   // Write R value
//   MOV DX,03C9H
//   MOV AL,r
//   OUT DX,AL
   IoPortOutB(0x03C9,r);
   // Write G value
//   MOV DX,03C9H
//  MOV AL,g
//   OUT DX,AL
   IoPortOutB(0x03C9,g);
   // Write B value
//   MOV DX,03C9H
//   MOV AL,b
//   OUT DX,AL
   IoPortOutB(0x03C9,b);
}

//*********************************************************************

void setpalette4()
{
   setpal( 0,  0,  0,  0);
   setpal( 1,  0, 42, 42);
   setpal( 2, 42,  0, 42);
   setpal( 3, 63, 63, 63);
}

//*********************************************************************

void setpalette16()
{
   uint16_t j = 0;
   uint16_t i;
   for (i = 0; i < 192; i+=3)
   {
      setpal(j, Pal16[i], Pal16[i+1], Pal16[i+2]);
      j++;
   }
}

//*********************************************************************

void setpalette256()
{
   uint16_t j = 0;
   uint16_t i;
   for (i = 0; i < 768; i+=3)
   {
      setpal(j, Pal[i], Pal[i+1], Pal[i+2]);
      j++;
   }

}

//*********************************************************************


void TRSubsEnable(void)
{
   IoPortOutB(0x03C3,0x00);
   IoPortOutB(0x46E8,0x16);
   IoPortOutB(0x46E9,0x00);
   IoPortOutB(0x0102,0x01);
   IoPortOutB(0x0103,0x00);
   IoPortOutB(0x46E8,0x0E);
   IoPortOutB(0x46E9,0x00);
   IoPortOutB(0x4AE8,0x00);
   IoPortOutB(0x4AE9,0x00);
//   IoPortOutB(0x3C2,0x23);
}

typedef struct _VMODE_ST
{
   uint8_t mode;                       // Videomode Number
   uint16_t width;                      // Width in pixels
   uint16_t height;                     // Height in pixels
   uint16_t width_uint8_ts;                // Number of uint8_ts per screen
   uint16_t colors;                     // Number of colors
   uint16_t attrib;                     // Videomode attributes
}VMODE_ST;

VMODE_ST Mode;                // Mode info

// Same as Mode 01H
 uint8_t mode00h[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x67,      0x00,          0x03,0x08,0x03,0x00,0x02,
// CRTC regs
0x2D,0x27,0x28,0x90,0x2B,0xA0,0xBF,0x1F,0x00,0x4F,0x0D,0x0F,0x00,0x00,0x00,0x00,
0x9C,0x8E,0x8F,0x14,0x1F,0x96,0xB9,0xA3,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,0xFF,
// ATTRIBUTE CONTROLLER regs

//0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
//0x04,0x00,0x0F,0x08,0x00
0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x04,0x00,0x0F,0x08,0x00

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Same as Mode 02H
 uint8_t mode03h[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x67,      0x00,          0x03,0x00,0x03,0x00,0x02,
// CRTC regs
0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,0x00,0x4F,0x0E,0x0F,0x00,0x00,0x00,0x01,
0x9C,0x8E,0x8F,0x28,0x1F,0x96,0xB9,0xA3,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x04,0x00,0x0F,0x08,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Same as Mode 05,
 uint8_t mode04h[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x63,      0x00,          0x03,0x09,0x03,0x00,0x02,
// CRTC regs
0x2D,0x27,0x28,0x90,0x2B,0x80,0xBF,0x1F,0x00,0x0C1,0x00,0x00,0x00,0x00,0x00,
0x31,0x9C,0x8E,0x8F,0x14,0x00,0x96,0xB9,0x0A2,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x30,0x0F,0x00,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x3B,0x3D,0x3F,0x02,0x04,0x06,0x07,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
0x01,0x00,0x03,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t mode06h[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x63,      0x00,          0x03,0x01,0x01,0x00,0x06,
// CRTC regs
0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,0x00,0x0C1,0x00,0x00,0x00,0x00,0x00,
0x00,0x9C,0x8E,0x8F,0x28,0x00,0x96,0xB9,0x0C2,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x00,0x0D,0x00,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
0x01,0x00,0x01,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t mode07h[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x66,     0x00,          0x03,0x00,0x03,0x00,0x02,
// CRTC regs
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x10,0x0A,0x00,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x0E,0x00,0x0F,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t mode0Dh[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x63,     0x00,          0x03,0x09,0x0F,0x00,0x06,
// CRTC regs
0x2D,0x27,0x28,0x90,0x2B,0x80,0x0BF,0x1F,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,
0x31,0x9C,0x8E,0x8F,0x14,0x00,0x96,0xB9,0xE3,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x10,0x05,0x00,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x01,0x00,0x0F,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t mode0Eh[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x63,      0x00,          0x03,0x01,0x0F,0x00,0x06,
// CRTC regs
0x5F,0x4F,0x50,0x82,0x54,0x80,0x0BF,0x1F,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,
0x59,0x9C,0x8E,0x8F,0x28,0x00,0x96,0xB9,0xE3,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x0F,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x01,0x00,0x0F,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t mode0Fh[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x0A2,     0x00,          0x03,0x01,0x0F,0x00,0x06,
// CRTC regs
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x08,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x18,0x00,0x00,
0x0B,0x00,0x05,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t mode10h[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x0A3,     0x00,          0x03,0x01,0x0F,0x00,0x06,
// CRTC regs
0x5F,0x4F,0x50,0x82,0x54,0x80,0x0BF,0x1F,0x00,0x40,0x00,0x00,0x00,0x00,0x00,
0x00,0x83,0x85,0x5D,0x28,0x0F,0x63,0x0BA,0xE3,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x0F,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x01,0x00,0x0F,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t mode11h[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0xE3,     0x00,          0x03,0x01,0x0F,0x00,0x06,
// CRTC regs
0x5F,0x4F,0x50,0x82,0x54,0x80,0x0B,0x3E,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x59,
0xEA,0x8C,0x0DF,0x28,0x0F,0x0E7,0x004,0x0C3,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x08,0x3F,0x3F,0x18,0x18,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
0x01,0x00,0x0F,0x00,0x00

};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t mode12h[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0xE3,     0x00,          0x03,0x01,0x0F,0x00,0x06,
// CRTC regs
0x5F,0x4F,0x50,0x82,0x54,0x80,0x0B,0x3E,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x59,
0xEA,0x8C,0x0DF,0x28,0x00,0x0E7,0x04,0xE3,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x0F,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x01,0x00,0x0F,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t mode13h[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x63,      0x00,          0x03,0x01,0x0F,0x00,0x0E,
// CRTC regs
0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x00,
0x9C,0x0E,0x8F,0x28,0x40,0x96,0xB9,0x0A3,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x41,0x00,0x0F,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t modeC4[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x63,      0x00,          0x03,0x01,0x0F,0x00,0x06,
// CRTC regs
0x5F,0x4F,0x50,0x82,0x54,0x80,0xBF,0x1F,0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x00,
0x9C,0x0E,0x8F,0x28,0x00,0x96,0xB9,0xE3,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x40,0x05,0x0F,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x41,0x00,0x0F,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t modeJ[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0xa3,      0x00,          0x03,0x01,0x03,0x00,0x02,
// CRTC regs
0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,0x00,0x47,0x0E,0x0F,0x00,0x00,0x00,
0x00,0x83,0x85,0x57,0x28,0x1F,0x60,0xB8,0xA3,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x0C,0x00,0x0F,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t modeK[62] = {
// MISC reg,  STATUS reg,    SEQ regs
   0x63,      0x00,          0x03,0x01,0x03,0x00,0x02,
// CRTC regs
0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,0x00,0x47,0x0E,0x0F,0x00,0x00,0x00,
0x00,0x9C,0x8E,0x8F,0x28,0x1F,0x96,0xB9,0xA3,0xFF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,0xFF,
// ATTRIBUTE CONTROLLER regs
0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x0C,0x00,0x0F,0x00,0x00
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

 uint8_t modeL[62] = {
// MISC reg,  STATUS reg,    SEQ regs
    0x67,       0x00,        0x03,0x08,0x03,0x00,0x02,
// CRTC regs
0x2D,0x27,0x28,0x90,0x2B,0x0A0,0x0BF,0x1F,0x00,0x47,0x06,0x07,0x00,0x00,0x00,
0x31,0x83,0x85,0x57,0x14,0x1F,0x60,0x0B8,0x0A3,0x0FF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,0x0FF,
// ATTRIBUTE CONTROLLER regs
0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,0x39,0x38,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x0C,0x00,0x0F,0x00,0x00,
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


 uint8_t modeM[62] = {
// MISC reg,  STATUS reg,    SEQ regs
0x67,0x00,0x03,0x08,0x03,0x00,0x02,
// CRTC regs
0x2D,0x27,0x28,0x90,0x2B,0x0A0,0x0BF,0x1F,0x00,0x47,0x06,0x07,0x00,0x00,0x00,0x31,
0x9C,0x8E,0x8F,0x14,0x1F,0x96,0x0B9,0x0A3,0x0FF,
// GRAPHICS regs
0x00,0x00,0x00,0x00,0x00,0x10,0x0E,0x00,0x0FF,
// ATTRIBUTE CONTROLLER regs
0x00,0x01,0x02,0x03,0x04,0x05,0x14,0x07,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x0C,0x00,0x0F,0x00,0x00,
};
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// VGA register port addresses
#define ATTRCON_ADDR			0x03C0
#define MISC_ADDR         0x03C2
#define VGAENABLE_ADDR    0x03C3
#define SEQ_ADDR          0x03C4
#define GRACON_ADDR       0x03CE
#define CRTC_ADDR         0x03D4
#define STATUS_ADDR       0x03DA

// Video mode numbers
#define MODE00H						0x00
#define MODE01H						0x00
#define MODE02H						0x03
#define MODE03H						0x03
#define MODE04H						0x04
#define MODE05H						0x05
#define MODE06H						0x06
#define MODE07H						0x07
#define MODE0DH						0x0D
#define MODE0EH						0x0E
#define MODE0FH						0x0F
#define MODE10H						0x10
#define MODE11H						0x11
#define MODE12H						0x12
#define MODE13H						0x13
#define CHAIN4						0x14
#define MODE_X						0x15
#define MODE_A						0x16
#define MODE_B						0x17
#define MODE_C						0x18
#define MODE_D						0x19
#define MODE_E						0x1A
#define MODE_F						0x1B
#define MODE_G						0x1C
#define MODE_H						0x1D
#define MODE_I						0x1E
#define MODE_J						0x1F
#define MODE_K						0x20
#define MODE_L						0x21
#define MODE_M						0x22
#define MODE10AH          0x23

#define TVU_TEXT					0x0001
#define TVU_GRAPHICS			0x0002
#define TVU_MONOCHROME		0x0004
#define TVU_PLANAR				0x0008
#define TVU_UNCHAINED			0x0010


void  ModeSet(uint8_t  *dataptr)
{

   uint8_t i;
//   data  uint8_t rr,wr;
//   asm {
//   MOV SI, regs

   // Send MISC regs
//   MOV DX,MISC_ADDR
//   MOV AL,[SI]
//   OUT DX,AL
//   INC SI
    IoPortOutB(MISC_ADDR,0x67);      // Before acess registers must be set address sheme

//      IoPortOutB(0x03D4,0x11);
//      i = IoPortInB(0x03D5);

//      IoPortOutB(0x03D4,0x11);
//      IoPortOutB(0x03D5,0xA5);

//     IoPortOutB(0x03D4,0x11);
//     i = IoPortInB(0x03D5);

    IoPortOutB(MISC_ADDR,*dataptr);
    dataptr++;

//   MOV DX,STATUS_ADDR
//   MOV AL,[SI]
//   OUT DX,AL
//   INC SI
    IoPortOutB(STATUS_ADDR,*dataptr);
    dataptr++;

   // Send SEQ regs
//   MOV CX,0
//REG_LOOP:
//   MOV DX,SEQ_ADDR
//   MOV AL,CL
//   OUT DX,AL

//   MOV DX,SEQ_ADDR
//   INC DX
//   MOV AL,[SI]
//   OUT DX,AL

//   INC SI
//   INC CX
//   CMP CL,5
//   JL REG_LOOP
    for(i=0;i<5;i++)
    {
      IoPortOutB(SEQ_ADDR,i);
      IoPortOutB(SEQ_ADDR+1,*dataptr);
      dataptr++;
    }

   // Clear Protection bits
//   MOV AH,0EH
//   MOV AL,11H
//   AND AH,7FH
//   MOV DX,CRTC_ADDR
//   OUT DX,AX

     VgaIoWriteIx(CRTC_ADDR,0x0E11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0x0E);
     i = VgaIoReadIx(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    i = IoPortInB(CRTC_ADDR+1);


//   // Send CRTC regs
//   MOV CX,0
//REG_LOOP2:
//   MOV DX,CRTC_ADDR
//   MOV AL,CL
//   OUT DX,AL

//   MOV DX,CRTC_ADDR
//   INC DX
//   MOV AL,[SI]
//   OUT DX,AL

//   INC SI
//   INC CX
//   CMP CL,25
//   JL REG_LOOP2

    for(i=0;i<25;i++)
    {
      VgaIoWriteIx(CRTC_ADDR,((*dataptr)<<8)+i);
//      IoPortOutB(CRTC_ADDR,i);
//      IoPortOutB(CRTC_ADDR+1,*dataptr);
      dataptr++;
    }

   // Send GRAPHICS regs
//   MOV CX,0
//REG_LOOP3:
//   MOV DX,GRACON_ADDR
//   MOV AL,CL
//   OUT DX,AL

//   MOV DX,GRACON_ADDR
//   INC DX
//   MOV AL,[SI]
//   OUT DX,AL

//   INC SI
//   INC CX
//   CMP CL,9
//   JL REG_LOOP3
    for(i=0;i<9;i++)
    {
      VgaIoWriteIx(GRACON_ADDR,((*dataptr)<<8)+i);
//      IoPortOutB(GRACON_ADDR,i);
//      IoPortOutB(GRACON_ADDR+1,*dataptr);
      dataptr++;
    }



//   MOV DX,STATUS_ADDR
//   IN AL,DX
   i = IoPortInB(STATUS_ADDR);

// Send ATTRCON regs
//   MOV CX,0
//REG_LOOP4:
//   MOV DX,ATTRCON_ADDR
//   IN AX,DX
//
//   MOV AL,CL
//   OUT DX,AL

//   MOV AL,[SI]
//   OUT DX,AL

//   INC SI
//   INC CX
//   CMP CL,21
//   JL REG_LOOP4

//   MOV AL,20H
//   OUT DX,AL
//   }
    for(i=0;i<21;i++)
    {
      IoPortInB(0x03DA);
      IoPortOutB(ATTRCON_ADDR,i);
      IoPortOutB(ATTRCON_ADDR,*dataptr);
      dataptr++;
    }
   VgaIoWriteIx(0x3CE,0x3A0B);
   IoPortInB(0x03DA);
   IoPortOutB(ATTRCON_ADDR,0x20);
   IoPortOutB(0x3C6,0xFF);
}

//*********************************************************************

void SetVideoMode(uint8_t mode)
{

   Mode.mode = mode;
   if (mode == MODE00H)                        // 40 x 25 x 16
   {
      ModeSet(mode00h);
      setpalette16();
      FontsRead(Font8x16,16);

      Mode.width = 40;
      Mode.height = 25;
      Mode.width_uint8_ts = 1000;
      Mode.colors = 16;
      Mode.attrib = TVU_TEXT;
   }
   else if (mode == MODE03H)                   // 80 x 25 x 16
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
   }
   else if (mode == MODE04H)                   // 320 x 200 x 4
   {
      ModeSet(mode04h);
      setpalette4();

      Mode.width = 320;
      Mode.height = 200;
      Mode.width_uint8_ts = 8192;
      Mode.colors = 4;
      Mode.attrib = TVU_GRAPHICS;
   }
   else if (mode == MODE06H)                    // 640 x 200 x 2
   {
      ModeSet(mode06h);

      Mode.width = 640;
      Mode.height = 200;
      Mode.width_uint8_ts = 8192;
      Mode.colors = 2;
      Mode.attrib = TVU_GRAPHICS;
   }
   else if (mode == MODE07H)                    // 80 x 25 x 2
   {
      ModeSet(mode07h);

      Mode.width = 80;
      Mode.height = 25;
      Mode.width_uint8_ts = 2000;
      Mode.colors = 2;
      Mode.attrib = TVU_TEXT | TVU_MONOCHROME;
   }
   else if (mode == MODE0DH)                    // 320 x 200 x 16
   {
      ModeSet(mode0Dh);
      setpalette16();

      Mode.width = 320;
      Mode.height = 200;
      Mode.width_uint8_ts = 8000;
      Mode.colors = 16;
      Mode.attrib = TVU_GRAPHICS | TVU_PLANAR;
   }
   else if (mode == MODE0EH)                    // 640 x 200 x 16
   {
      ModeSet(mode0Eh);
      setpalette16();

      Mode.width = 640;
      Mode.height = 200;
      Mode.width_uint8_ts = 16000;
      Mode.colors = 16;
      Mode.attrib = TVU_GRAPHICS | TVU_PLANAR;
   }
   else if (mode == MODE0FH)                    // 640 x 350 x 2
   {
      ModeSet(mode0Fh);

      Mode.width = 640;
      Mode.height = 350;
      Mode.width_uint8_ts = 28000;
      Mode.colors = 2;
      Mode.attrib = TVU_GRAPHICS | TVU_MONOCHROME;
   }
   else if (mode == MODE10H)                    // 640 x 350 x 16
   {
      ModeSet(mode10h);
      setpalette16();

      Mode.width = 640;
      Mode.height = 350;
      Mode.width_uint8_ts = 28000;
      Mode.colors = 16;
      Mode.attrib = TVU_GRAPHICS | TVU_PLANAR;
   }
   else if (mode == MODE11H)                    // 640 x 480 x 2
   {
      ModeSet(mode11h);

      Mode.width = 640;
      Mode.height = 480;
      Mode.width_uint8_ts = 38400u;
      Mode.colors = 2;
      Mode.attrib = TVU_GRAPHICS | TVU_PLANAR;
   }
   else if (mode == MODE12H)                    // 640 x 480 x 16
   {
      ModeSet(mode12h);
      setpalette16();

      Mode.width = 640;
      Mode.height = 480;
      Mode.width_uint8_ts = 38400u;
      Mode.colors = 16;
      Mode.attrib = TVU_GRAPHICS | TVU_PLANAR;
   }
   else if (mode == MODE13H)                    // 320 x 200 x 256
   {
      ModeSet(mode13h);
      setpalette256();

      Mode.width = 320;
      Mode.height = 200;
      Mode.width_uint8_ts = 64000u;
      Mode.colors = 256;
      Mode.attrib = TVU_GRAPHICS;
   }
   else if (mode == CHAIN4)                     // unchained 320 x 200 x 256
   {
      ModeSet(modeC4);
      setpalette256();

      Mode.width = 320;
      Mode.height = 200;
      Mode.width_uint8_ts = 16000;
      Mode.colors = 256;
      Mode.attrib = TVU_GRAPHICS | TVU_UNCHAINED;
   }
   else if (mode == MODE_X)                     // unchained 320 x 240 x 256
   {
      ModeSet(modeC4);

//    outportb(MISC_ADDR,0xE3);
      IoPortOutB(MISC_ADDR,0xE3);

      // turn off write protect
//    outport(CRTC_ADDR,0x2C11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0x2C);
      VgaIoWriteIx(CRTC_ADDR,0x2C11);

      // vertical total
//    outport(CRTC_ADDR,0x0D06);
//    IoPortOutB(CRTC_ADDR,0x06);
//    IoPortOutB(CRTC_ADDR+1,0x0D);
      VgaIoWriteIx(CRTC_ADDR,0x0D06);
      // overflow register
//    outport(CRTC_ADDR,0x3E07);
//    IoPortOutB(CRTC_ADDR,0x07);
//    IoPortOutB(CRTC_ADDR+1,0x3E);
      VgaIoWriteIx(CRTC_ADDR,0x3E07);
      // vertical retrace start
//    outport(CRTC_ADDR,0xEA10);
//    IoPortOutB(CRTC_ADDR,0x10);
//    IoPortOutB(CRTC_ADDR+1,0xEA);
      VgaIoWriteIx(CRTC_ADDR,0xEA10);
      // vertical retrace end AND wr.prot
//    outport(CRTC_ADDR,0xAC11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0xAC);
      VgaIoWriteIx(CRTC_ADDR,0xAC11);
      // vertical display enable end
//    outport(CRTC_ADDR,0xDF12);
//    IoPortOutB(CRTC_ADDR,0x12);
//    IoPortOutB(CRTC_ADDR+1,0xDF);
      VgaIoWriteIx(CRTC_ADDR,0xDF12);
      // start vertical blanking
//    outport(CRTC_ADDR,0xE715);
//    IoPortOutB(CRTC_ADDR,0x15);
//    IoPortOutB(CRTC_ADDR+1,0xE7);
      VgaIoWriteIx(CRTC_ADDR,0xE715);
      // end vertical blanking
//    outport(CRTC_ADDR,0x0616);
//    IoPortOutB(CRTC_ADDR,0x16);
//    IoPortOutB(CRTC_ADDR+1,0x06);
      VgaIoWriteIx(CRTC_ADDR,0x0616);
      setpalette256();
      Mode.width = 320;
      Mode.height = 240;
      Mode.width_uint8_ts = 19200;
      Mode.colors = 256;
      Mode.attrib = TVU_GRAPHICS | TVU_UNCHAINED;
   }
   else if (mode == MODE_A)                     // unchained 320 x 350 x 256
   {
      ModeSet(modeC4);

      // turn off double scanning mode
//    outportb(CRTC_ADDR,9);
//    outportb(CRTC_ADDR+1,inportb(CRTC_ADDR+1) & ~0x1F);
      IoPortOutB(CRTC_ADDR,0x09);
      IoPortOutB(CRTC_ADDR+1,IoPortInB(CRTC_ADDR+1) & ~0x1F);

      // change the vertical resolution flags to 350
//    outportb(MISC_ADDR,(inportb(0x3CC) & ~0xC0) | 0x80);
      IoPortOutB(MISC_ADDR,(IoPortInB(0x3CC) & ~0xC0) | 0x80);

      // turn off write protect
//    outport(CRTC_ADDR,0x2C11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0x2C);
      VgaIoWriteIx(CRTC_ADDR,0x2C11);
      // vertical total
//    outport(CRTC_ADDR,0xBF06);
//    IoPortOutB(CRTC_ADDR,0x06);
//    IoPortOutB(CRTC_ADDR+1,0xBF);
      VgaIoWriteIx(CRTC_ADDR,0xBF06);
      // overflow register
//    outport(CRTC_ADDR,0x1F07);
//    IoPortOutB(CRTC_ADDR,0x07);
//    IoPortOutB(CRTC_ADDR+1,0x1F);
      VgaIoWriteIx(CRTC_ADDR,0x1F07);
      // vertical retrace start
//    outport(CRTC_ADDR,0x8310);
//    IoPortOutB(CRTC_ADDR,0x10);
//    IoPortOutB(CRTC_ADDR+1,0x83);
      VgaIoWriteIx(CRTC_ADDR,0x8310);
      // vertical retrace end AND wr.prot
//    outport(CRTC_ADDR,0x8511);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0x85);
      VgaIoWriteIx(CRTC_ADDR,0x8511);
      // vertical display enable end
//    outport(CRTC_ADDR,0x5D12);
//    IoPortOutB(CRTC_ADDR,0x12);
//    IoPortOutB(CRTC_ADDR+1,0x5D);
      VgaIoWriteIx(CRTC_ADDR,0x5D12);
      // start vertical blanking
//    outport(CRTC_ADDR,0x6315);
//    IoPortOutB(CRTC_ADDR,0x15);
//    IoPortOutB(CRTC_ADDR+1,0x63);
      VgaIoWriteIx(CRTC_ADDR,0x6315);
      // end vertical blanking
//    outport(CRTC_ADDR,0xBA16);
//    IoPortOutB(CRTC_ADDR,0x16);
//    IoPortOutB(CRTC_ADDR+1,0xBA);
      VgaIoWriteIx(CRTC_ADDR,0xBA16);

      setpalette256();
      Mode.width = 320;
      Mode.height = 350;
      Mode.width_uint8_ts = 28000u;
      Mode.colors = 256;
      Mode.attrib = TVU_GRAPHICS | TVU_UNCHAINED;
   }
   else if (mode == MODE_B)                     // unchained 320 x 400 x 256
   {
      ModeSet(modeC4);
      // turn off double scanning mode
//    outportb(CRTC_ADDR,9);
//    outportb(CRTC_ADDR+1,inportb(CRTC_ADDR+1) & ~0x1F);
      IoPortOutB(CRTC_ADDR,0x09);
      IoPortOutB(CRTC_ADDR+1,IoPortInB(CRTC_ADDR+1) & ~0x1F);

      // change the vertical resolution flags to 400
//      outportb(MISC_ADDR,(inportb(0x3CC) & ~0xC0) | 0x40);
      IoPortOutB(MISC_ADDR+1,(IoPortInB(0x3CC) & ~0xC0) | 0x40);

      setpalette256();
      Mode.width = 320;
      Mode.height = 400;
      Mode.width_uint8_ts = 32000;
      Mode.colors = 256;
      Mode.attrib = TVU_GRAPHICS | TVU_UNCHAINED;
   }
   else if (mode == MODE_C)                     // unchained 320 x 480 x 256
   {
      ModeSet(modeC4);

      // turn off double scanning mode
//    outportb(CRTC_ADDR,9);
//    outportb(CRTC_ADDR+1,inportb(CRTC_ADDR+1) & ~0x1F);
      IoPortOutB(CRTC_ADDR,0x09);
      IoPortOutB(CRTC_ADDR+1,IoPortInB(CRTC_ADDR+1) & ~0x1F);

      // change the vertical resolution flags to 480
//    outportb(MISC_ADDR,(inportb(0x3CC) & ~0xC0) | 0xC0);
      IoPortOutB(MISC_ADDR,(IoPortInB(0x3CC) & ~0xC0) | 0xC0);

      // turn off write protect
//    outport(CRTC_ADDR,0x2C11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0x2C);
      VgaIoWriteIx(CRTC_ADDR,0x2C11);
      // vertical total
//    outport(CRTC_ADDR,0x0D06);
//    IoPortOutB(CRTC_ADDR,0x06);
//    IoPortOutB(CRTC_ADDR+1,0x0D);
      VgaIoWriteIx(CRTC_ADDR,0x0D06);
      // overflow register
//    outport(CRTC_ADDR,0x3E07)
//    IoPortOutB(CRTC_ADDR,0x07);
//    IoPortOutB(CRTC_ADDR+1,0x3E);
      VgaIoWriteIx(CRTC_ADDR,0x3E07);
      // vertical retrace start
//    outport(CRTC_ADDR,0xEA10);
//    IoPortOutB(CRTC_ADDR,0x10);
//    IoPortOutB(CRTC_ADDR+1,0xEA);
      VgaIoWriteIx(CRTC_ADDR,0xEA10);
      // vertical retrace end AND wr.prot
//    outport(CRTC_ADDR,0xAC11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0xAC);
      VgaIoWriteIx(CRTC_ADDR,0xAC11);
      // vertical display enable end
//    outport(CRTC_ADDR,0xDF12);
//    IoPortOutB(CRTC_ADDR,0x12);
//    IoPortOutB(CRTC_ADDR+1,0xDF);
      VgaIoWriteIx(CRTC_ADDR,0xDF12);
      // start vertical blanking
//    outport(CRTC_ADDR,0xE715);
//    IoPortOutB(CRTC_ADDR,0x15);
//    IoPortOutB(CRTC_ADDR+1,0xE7);
      VgaIoWriteIx(CRTC_ADDR,0xE715);
      // end vertical blanking
//    outport(CRTC_ADDR,0x0616);
//    IoPortOutB(CRTC_ADDR,0x16);
//    IoPortOutB(CRTC_ADDR+1,0x06);
      VgaIoWriteIx(CRTC_ADDR,0x0616);
      setpalette256();
      Mode.width = 320;
      Mode.height = 480;
      Mode.width_uint8_ts = 38400u;
      Mode.colors = 256;
      Mode.attrib = TVU_GRAPHICS | TVU_UNCHAINED;
   }
   else if (mode == MODE_D)                     // unchained 360 x 200 x 256
   {
      ModeSet(mode13h);

      // Turn off Chain 4
//    outport(SEQ_ADDR,0x0604);
//    IoPortOutB(SEQ_ADDR,0x04);
//    IoPortOutB(SEQ_ADDR+1,0x06);
      VgaIoWriteIx(SEQ_ADDR,0x0604);
      // Activate a synchronous reset
//    outport(SEQ_ADDR,0x0100);
//    IoPortOutB(SEQ_ADDR,0x00);
//    IoPortOutB(SEQ_ADDR+1,0x01);
      VgaIoWriteIx(SEQ_ADDR,0x0100);
      // Select 28 mhz pixel clock
//    outportb(MISC_ADDR,0xE7);
      IoPortOutB(MISC_ADDR,0xE7);

      // Release synchronous reset
//    outport(SEQ_ADDR,0x0300);
//    IoPortOutB(SEQ_ADDR,0x00);
//    IoPortOutB(SEQ_ADDR+1,0x03);
      VgaIoWriteIx(SEQ_ADDR,0x0300);

      // change the vertical resolution flags to 400
//    outportb(MISC_ADDR,(inportb(0x3CC) & ~0xC0) | 0x40);
      IoPortOutB(MISC_ADDR,(IoPortInB(0x3CC) & ~0xC0) | 0x40);

      // turn off write protect
//    outport(CRTC_ADDR,0x2C11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0x2C);
      VgaIoWriteIx(CRTC_ADDR,0x2C11);
//    outport(CRTC_ADDR,0x6B00);
//    IoPortOutB(CRTC_ADDR,0x00);
//    IoPortOutB(CRTC_ADDR+1,0x6B);
      VgaIoWriteIx(CRTC_ADDR,0x6B00);
//    outport(CRTC_ADDR,0x5901);
//    IoPortOutB(CRTC_ADDR,0x01);
//    IoPortOutB(CRTC_ADDR+1,0x59);
      VgaIoWriteIx(CRTC_ADDR,0x5901);
//    outport(CRTC_ADDR,0x5A02);
//    IoPortOutB(CRTC_ADDR,0x02);
//    IoPortOutB(CRTC_ADDR+1,0x5A);
      VgaIoWriteIx(CRTC_ADDR,0x5A02);
//    outport(CRTC_ADDR,0x8E03);
//    IoPortOutB(CRTC_ADDR,0x03);
//    IoPortOutB(CRTC_ADDR+1,0x8E);
      VgaIoWriteIx(CRTC_ADDR,0x8E03);
//    outport(CRTC_ADDR,0x5E04);
//    IoPortOutB(CRTC_ADDR,0x04);
//    IoPortOutB(CRTC_ADDR+1,0x5E);
      VgaIoWriteIx(CRTC_ADDR,0x5E04);
//    outport(CRTC_ADDR,0x8A05);
//    IoPortOutB(CRTC_ADDR,0x05);
//    IoPortOutB(CRTC_ADDR+1,0x8A);
      VgaIoWriteIx(CRTC_ADDR,0x8A05);
//    outport(CRTC_ADDR,0x0008);
//    IoPortOutB(CRTC_ADDR,0x08);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x0008);
//    outport(CRTC_ADDR,0xC009);
//    IoPortOutB(CRTC_ADDR,0x09);
//    IoPortOutB(CRTC_ADDR+1,0xC0);
      VgaIoWriteIx(CRTC_ADDR,0xC009);
//    outport(CRTC_ADDR,0x000A);
//    IoPortOutB(CRTC_ADDR,0x0A);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000A);
//    outport(CRTC_ADDR,0x000B);
//    IoPortOutB(CRTC_ADDR,0x0B);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000B);
//    outport(CRTC_ADDR,0x000C);
//    IoPortOutB(CRTC_ADDR,0x0C);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000C);
//    outport(CRTC_ADDR,0x000D);
//    IoPortOutB(CRTC_ADDR,0x0D);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000D);
//    outport(CRTC_ADDR,0x000E);
//    IoPortOutB(CRTC_ADDR,0x0E);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000E);
//    outport(CRTC_ADDR,0x000F);
//    IoPortOutB(CRTC_ADDR,0x0F);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000F);
//    outport(CRTC_ADDR,0xAC11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0xAC);
      VgaIoWriteIx(CRTC_ADDR,0xAC11);
//    outport(CRTC_ADDR,0x2D13);
//    IoPortOutB(CRTC_ADDR,0x13);
//    IoPortOutB(CRTC_ADDR+1,0x2D);
      VgaIoWriteIx(CRTC_ADDR,0x2D13);
//    outport(CRTC_ADDR,0x0014);
//    IoPortOutB(CRTC_ADDR,0x14);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x0014);
//    outport(CRTC_ADDR,0xE317);
//    IoPortOutB(CRTC_ADDR,0x17);
//    IoPortOutB(CRTC_ADDR+1,0xE3);
      VgaIoWriteIx(CRTC_ADDR,0xE317);
//    outport(CRTC_ADDR,0xFF18);
//    IoPortOutB(CRTC_ADDR,0x18);
//    IoPortOutB(CRTC_ADDR+1,0xFF);
      VgaIoWriteIx(CRTC_ADDR,0xFF18);
      setpalette256();
      Mode.width = 360;
      Mode.height = 200;
      Mode.width_uint8_ts = 18000u;
      Mode.colors = 256;
      Mode.attrib = TVU_GRAPHICS | TVU_UNCHAINED;
   }
   else if (mode == MODE_E)                     // unchained 360 x 240 x 256
   {
      ModeSet(mode13h);

      // Turn off Chain 4
//    outport(SEQ_ADDR,0x0604);
//    IoPortOutB(SEQ_ADDR,0x04);
//    IoPortOutB(SEQ_ADDR+1,0x06);
      VgaIoWriteIx(SEQ_ADDR,0x0604);
      // Activate a synchronous reset
//    outport(SEQ_ADDR,0x0100);
//    IoPortOutB(SEQ_ADDR,0x00);
//    IoPortOutB(SEQ_ADDR+1,0x01);
      VgaIoWriteIx(SEQ_ADDR,0x0100);
      // Select 28 mhz pixel clock
//    outportb(MISC_ADDR,0xE7);
      IoPortOutB(MISC_ADDR,0xE7);

      // Release synchronous reset
//    outport(SEQ_ADDR,0x0300);
//    IoPortOutB(SEQ_ADDR,0x00);
//    IoPortOutB(SEQ_ADDR+1,0x03);
      VgaIoWriteIx(SEQ_ADDR,0x0300);

      // change the vertical resolution flags to 480
//    outportb(MISC_ADDR,(inportb(0x3CC) & ~0xC0) | 0xC0);
      IoPortOutB(MISC_ADDR,(IoPortInB(0x3CC) & ~0xC0) | 0xC0);

      // turn off write protect
//    outport(CRTC_ADDR,0x2C11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0x2C);
      VgaIoWriteIx(CRTC_ADDR,0x2C11);
//    outport(CRTC_ADDR,0x6B00);
//    IoPortOutB(CRTC_ADDR,0x00);
//    IoPortOutB(CRTC_ADDR+1,0x6B);
      VgaIoWriteIx(CRTC_ADDR,0x6B00);
//    outport(CRTC_ADDR,0x5901);
//    IoPortOutB(CRTC_ADDR,0x01);
//    IoPortOutB(CRTC_ADDR+1,0x59);
      VgaIoWriteIx(CRTC_ADDR,0x5901);
//    outport(CRTC_ADDR,0x5A02);
//    IoPortOutB(CRTC_ADDR,0x02);
//    IoPortOutB(CRTC_ADDR+1,0x5A);
      VgaIoWriteIx(CRTC_ADDR,0x5A02);
//    outport(CRTC_ADDR,0x8E03);
//    IoPortOutB(CRTC_ADDR,0x03);
//    IoPortOutB(CRTC_ADDR+1,0x8E);
      VgaIoWriteIx(CRTC_ADDR,0x8E03);
//    outport(CRTC_ADDR,0x5E04);
//    IoPortOutB(CRTC_ADDR,0x04);
//    IoPortOutB(CRTC_ADDR+1,0x5E);
      VgaIoWriteIx(CRTC_ADDR,0x5E04);
//    outport(CRTC_ADDR,0x8A05);
//    IoPortOutB(CRTC_ADDR,0x05);
//    IoPortOutB(CRTC_ADDR+1,0x8A);
      VgaIoWriteIx(CRTC_ADDR,0x8A05);
//    outport(CRTC_ADDR,0x0D06);
//    IoPortOutB(CRTC_ADDR,0x06);
//    IoPortOutB(CRTC_ADDR+1,0x0D);
      VgaIoWriteIx(CRTC_ADDR,0x0D06);
//    outport(CRTC_ADDR,0x3E07);
//    IoPortOutB(CRTC_ADDR,0x07);
//    IoPortOutB(CRTC_ADDR+1,0x3E);
      VgaIoWriteIx(CRTC_ADDR,0x3E07);
//    outport(CRTC_ADDR,0x0008);
//    IoPortOutB(CRTC_ADDR,0x08);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x0008);
//    outport(CRTC_ADDR,0xC009);
//    IoPortOutB(CRTC_ADDR,0x09);
//    IoPortOutB(CRTC_ADDR+1,0xC0);
      VgaIoWriteIx(CRTC_ADDR,0xC009);
//    outport(CRTC_ADDR,0x000A);
//    IoPortOutB(CRTC_ADDR,0x0A);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000A);
//    outport(CRTC_ADDR,0x000B);
//    IoPortOutB(CRTC_ADDR,0x0B);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000B);
//    outport(CRTC_ADDR,0x000C);
//    IoPortOutB(CRTC_ADDR,0x0C);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000C);
//    outport(CRTC_ADDR,0x000D);
//    IoPortOutB(CRTC_ADDR,0x0D);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000D);
//    outport(CRTC_ADDR,0x000E);
//    IoPortOutB(CRTC_ADDR,0x0E);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000E);
//    outport(CRTC_ADDR,0x000F);
//    IoPortOutB(CRTC_ADDR,0x0F);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000F);
//    outport(CRTC_ADDR,0xEA10);
//    IoPortOutB(CRTC_ADDR,0x10);
//    IoPortOutB(CRTC_ADDR+1,0xEA);
      VgaIoWriteIx(CRTC_ADDR,0xEA10);
//    outport(CRTC_ADDR,0xAC11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0xAC);
      VgaIoWriteIx(CRTC_ADDR,0xAC11);
//    outport(CRTC_ADDR,0xDF12);
//    IoPortOutB(CRTC_ADDR,0x12);
//    IoPortOutB(CRTC_ADDR+1,0xDF);
      VgaIoWriteIx(CRTC_ADDR,0xDF12);
//    outport(CRTC_ADDR,0x2D13);
//    IoPortOutB(CRTC_ADDR,0x13);
//    IoPortOutB(CRTC_ADDR+1,0x2D);
      VgaIoWriteIx(CRTC_ADDR,0x2D13);
//    outport(CRTC_ADDR,0x0014);
//    IoPortOutB(CRTC_ADDR,0x14);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x0014);
//    outport(CRTC_ADDR,0xE715);
//    IoPortOutB(CRTC_ADDR,0x15);
//    IoPortOutB(CRTC_ADDR+1,0xE7);
      VgaIoWriteIx(CRTC_ADDR,0xE715);
//    outport(CRTC_ADDR,0x0616);
//    IoPortOutB(CRTC_ADDR,0x16);
//    IoPortOutB(CRTC_ADDR+1,0x06);
      VgaIoWriteIx(CRTC_ADDR,0x0616);
//    outport(CRTC_ADDR,0xE317);
//    IoPortOutB(CRTC_ADDR,0x17);
//    IoPortOutB(CRTC_ADDR+1,0xE3);
      VgaIoWriteIx(CRTC_ADDR,0xE317);
//    outport(CRTC_ADDR,0xFF18);
//    IoPortOutB(CRTC_ADDR,0x18);
//    IoPortOutB(CRTC_ADDR+1,0xFF);
      VgaIoWriteIx(CRTC_ADDR,0xFF18);

      setpalette256();
      Mode.width = 360;
      Mode.height = 240;
      Mode.width_uint8_ts = 21600;
      Mode.colors = 256;
      Mode.attrib = TVU_GRAPHICS | TVU_UNCHAINED;
   }
   else if (mode == MODE_F)                     // unchained 360 x 350 x 256
   {
      ModeSet(mode13h);

      // Turn off Chain 4
//    outport(SEQ_ADDR,0x0604);
//    IoPortOutB(SEQ_ADDR,0x04);
//    IoPortOutB(SEQ_ADDR+1,0x06);
      VgaIoWriteIx(SEQ_ADDR,0x0604);
      // Activate a synchronous reset
//    outport(SEQ_ADDR,0x0100);
//    IoPortOutB(SEQ_ADDR,0x00);
//    IoPortOutB(SEQ_ADDR+1,0x01);
      VgaIoWriteIx(SEQ_ADDR,0x0100);
      // Select 28 mhz pixel clock
//    outportb(MISC_ADDR,0xE7);
      IoPortOutB(MISC_ADDR,0xE7);

      // Release synchronous reset
//    outport(SEQ_ADDR,0x0300);
//    IoPortOutB(SEQ_ADDR,0x00);
//    IoPortOutB(SEQ_ADDR+1,0x03);
      VgaIoWriteIx(SEQ_ADDR,0x0300);

      // change the vertical resolution flags to 350
//    outportb(MISC_ADDR,(inportb(0x3CC) & ~0xC0) | 0x80);
      IoPortOutB(MISC_ADDR,(IoPortInB(0x3CC) & ~0xC0) | 0x80);

      // turn off write protect
//    outport(CRTC_ADDR,0x2C11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0x2C);
      VgaIoWriteIx(CRTC_ADDR,0x2C11);
//    outport(CRTC_ADDR,0x6B00);
//    IoPortOutB(CRTC_ADDR,0x00);
//    IoPortOutB(CRTC_ADDR+1,0x6B);
      VgaIoWriteIx(CRTC_ADDR,0x6B00);
//    outport(CRTC_ADDR,0x5901);
//    IoPortOutB(CRTC_ADDR,0x01);
//    IoPortOutB(CRTC_ADDR+1,0x59);
      VgaIoWriteIx(CRTC_ADDR,0x5901);
//    outport(CRTC_ADDR,0x5A02);
//    IoPortOutB(CRTC_ADDR,0x02);
//    IoPortOutB(CRTC_ADDR+1,0x5A);
      VgaIoWriteIx(CRTC_ADDR,0x5A02);
//    outport(CRTC_ADDR,0x8E03);
//    IoPortOutB(CRTC_ADDR,0x03);
//    IoPortOutB(CRTC_ADDR+1,0x8E);
      VgaIoWriteIx(CRTC_ADDR,0x8E03);
//    outport(CRTC_ADDR,0x5E04);
//    IoPortOutB(CRTC_ADDR,0x04);
//    IoPortOutB(CRTC_ADDR+1,0x5E);
      VgaIoWriteIx(CRTC_ADDR,0x5E04);
//    outport(CRTC_ADDR,0x8A05);
//    IoPortOutB(CRTC_ADDR,0x05);
//    IoPortOutB(CRTC_ADDR+1,0x8A);
      VgaIoWriteIx(CRTC_ADDR,0x8A05);
//    outport(CRTC_ADDR,0xBF06);
//    IoPortOutB(CRTC_ADDR,0x06);
//    IoPortOutB(CRTC_ADDR+1,0xBF);
      VgaIoWriteIx(CRTC_ADDR,0xBF06);
//    outport(CRTC_ADDR,0x1F07);
//    IoPortOutB(CRTC_ADDR,0x07);
//    IoPortOutB(CRTC_ADDR+1,0x1F);
      VgaIoWriteIx(CRTC_ADDR,0x1F07);
//    outport(CRTC_ADDR,0x0008);
//    IoPortOutB(CRTC_ADDR,0x08);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x0008);
//    outport(CRTC_ADDR,0x4009);
//    IoPortOutB(CRTC_ADDR,0x09);
//    IoPortOutB(CRTC_ADDR+1,0x40);
      VgaIoWriteIx(CRTC_ADDR,0x4009);
//    outport(CRTC_ADDR,0x000A);
//    IoPortOutB(CRTC_ADDR,0x0A);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000A);
//    outport(CRTC_ADDR,0x000B);
//    IoPortOutB(CRTC_ADDR,0x0B);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000B);
//    outport(CRTC_ADDR,0x000C);
//    IoPortOutB(CRTC_ADDR,0x0C);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000C);
//    outport(CRTC_ADDR,0x000D);
//    IoPortOutB(CRTC_ADDR,0x0D);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000D);
//    outport(CRTC_ADDR,0x000E);
//    IoPortOutB(CRTC_ADDR,0x0E);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000E);
//    outport(CRTC_ADDR,0x000F);
//    IoPortOutB(CRTC_ADDR,0x0F);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000F);
//    outport(CRTC_ADDR,0x8310);
//    IoPortOutB(CRTC_ADDR,0x10);
//    IoPortOutB(CRTC_ADDR+1,0x83);
      VgaIoWriteIx(CRTC_ADDR,0x8310);
//    outport(CRTC_ADDR,0x8511);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0x85);
      VgaIoWriteIx(CRTC_ADDR,0x8511);
//    outport(CRTC_ADDR,0x5D12);
//    IoPortOutB(CRTC_ADDR,0x12);
//    IoPortOutB(CRTC_ADDR+1,0x5D);
      VgaIoWriteIx(CRTC_ADDR,0x5D12);
//    outport(CRTC_ADDR,0x2D13);
//    IoPortOutB(CRTC_ADDR,0x13);
//    IoPortOutB(CRTC_ADDR+1,0x2D);
      VgaIoWriteIx(CRTC_ADDR,0x2D13);
//    outport(CRTC_ADDR,0x0014);
//    IoPortOutB(CRTC_ADDR,0x14);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x0014);
//    outport(CRTC_ADDR,0x6315);
//    IoPortOutB(CRTC_ADDR,0x15);
//    IoPortOutB(CRTC_ADDR+1,0x63);
      VgaIoWriteIx(CRTC_ADDR,0x6315);
//    outport(CRTC_ADDR,0xBA16);
//    IoPortOutB(CRTC_ADDR,0x16);
//    IoPortOutB(CRTC_ADDR+1,0xBA);
      VgaIoWriteIx(CRTC_ADDR,0xBA16);
//    outport(CRTC_ADDR,0xE317);
//    IoPortOutB(CRTC_ADDR,0x17);
//    IoPortOutB(CRTC_ADDR+1,0xE3);
      VgaIoWriteIx(CRTC_ADDR,0xE317);
//    outport(CRTC_ADDR,0xFF18);
//    IoPortOutB(CRTC_ADDR,0x18);
//    IoPortOutB(CRTC_ADDR+1,0xFF);
      VgaIoWriteIx(CRTC_ADDR,0xFF18);

      setpalette256();
      Mode.width = 360;
      Mode.height = 350;
      Mode.width_uint8_ts = 31500;
      Mode.colors = 256;
      Mode.attrib = TVU_GRAPHICS | TVU_UNCHAINED;
   }
   else if (mode == MODE_G)                     // unchained 360 x 400 x 256
   {
      ModeSet(mode13h);

      // Turn off Chain 4
//    outport(SEQ_ADDR,0x0604);
//    IoPortOutB(SEQ_ADDR,0x04);
//    IoPortOutB(SEQ_ADDR+1,0x06);
      VgaIoWriteIx(SEQ_ADDR,0x0604);
      // Activate a synchronous reset
//    outport(SEQ_ADDR,0x0100);
//    IoPortOutB(SEQ_ADDR,0x00);
//    IoPortOutB(SEQ_ADDR+1,0x01);
      VgaIoWriteIx(SEQ_ADDR,0x0100);
      // Select 28 mhz pixel clock
//    outportb(MISC_ADDR,0xE7);
      IoPortOutB(MISC_ADDR,0xE7);

      // Release synchronous reset
//    outport(SEQ_ADDR,0x0300);
//    IoPortOutB(SEQ_ADDR,0x00);
//    IoPortOutB(SEQ_ADDR+1,0x03);
      VgaIoWriteIx(SEQ_ADDR,0x0300);
      // change the vertical resolution flags to 400
//    outportb(MISC_ADDR,(inportb(0x3CC) & ~0xC0) | 0x40);
      IoPortOutB(MISC_ADDR,(IoPortInB(0x3CC) & ~0xC0) | 0x40);

      // turn off write protect
//    outport(CRTC_ADDR,0x2C11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0x2C);
      VgaIoWriteIx(CRTC_ADDR,0x2C11);
//    outport(CRTC_ADDR,0x6B00);
//    IoPortOutB(CRTC_ADDR,0x00);
//    IoPortOutB(CRTC_ADDR+1,0x6B);
      VgaIoWriteIx(CRTC_ADDR,0x6B00);
//    outport(CRTC_ADDR,0x5901);
//    IoPortOutB(CRTC_ADDR,0x01);
//    IoPortOutB(CRTC_ADDR+1,0x59);
      VgaIoWriteIx(CRTC_ADDR,0x5901);
//    outport(CRTC_ADDR,0x5A02);
//    IoPortOutB(CRTC_ADDR,0x02);
//    IoPortOutB(CRTC_ADDR+1,0x5A);
      VgaIoWriteIx(CRTC_ADDR,0x5A02);
//    outport(CRTC_ADDR,0x8E03);
//    IoPortOutB(CRTC_ADDR,0x03);
//    IoPortOutB(CRTC_ADDR+1,0x8E);
      VgaIoWriteIx(CRTC_ADDR,0x8E03);
//    outport(CRTC_ADDR,0x5E04);
//    IoPortOutB(CRTC_ADDR,0x04);
//    IoPortOutB(CRTC_ADDR+1,0x5E);
      VgaIoWriteIx(CRTC_ADDR,0x5E04);
//    outport(CRTC_ADDR,0x8A05);
//    IoPortOutB(CRTC_ADDR,0x05);
//    IoPortOutB(CRTC_ADDR+1,0x8A);
      VgaIoWriteIx(CRTC_ADDR,0x8A05);
//    outport(CRTC_ADDR,0x0008);
//    IoPortOutB(CRTC_ADDR,0x08);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x0008);
//    outport(CRTC_ADDR,0x4009);
//    IoPortOutB(CRTC_ADDR,0x09);
//    IoPortOutB(CRTC_ADDR+1,0x40);
      VgaIoWriteIx(CRTC_ADDR,0x4009);
//    outport(CRTC_ADDR,0x000A);
//    IoPortOutB(CRTC_ADDR,0x0A);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000A);
//    outport(CRTC_ADDR,0x000B);
//    IoPortOutB(CRTC_ADDR,0x0B);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000B);
//    outport(CRTC_ADDR,0x000C);
//    IoPortOutB(CRTC_ADDR,0x0C);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000C);
//    outport(CRTC_ADDR,0x000D);
//    IoPortOutB(CRTC_ADDR,0x0D);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000D);
//    outport(CRTC_ADDR,0x000E);
//    IoPortOutB(CRTC_ADDR,0x0E);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000E);
//    outport(CRTC_ADDR,0x000F);
//    IoPortOutB(CRTC_ADDR,0x0F);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000F);
//    outport(CRTC_ADDR,0xAC11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0xAC);
      VgaIoWriteIx(CRTC_ADDR,0xAC11);
//    outport(CRTC_ADDR,0x2D13);
//    IoPortOutB(CRTC_ADDR,0x13);
//    IoPortOutB(CRTC_ADDR+1,0x2D);
      VgaIoWriteIx(CRTC_ADDR,0x2D13);
//    outport(CRTC_ADDR,0x0014);
//    IoPortOutB(CRTC_ADDR,0x14);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x0014);
//    outport(CRTC_ADDR,0xE317);
//    IoPortOutB(CRTC_ADDR,0x17);
//    IoPortOutB(CRTC_ADDR+1,0xE3);
      VgaIoWriteIx(CRTC_ADDR,0xE317);
//    outport(CRTC_ADDR,0xFF18);
//    IoPortOutB(CRTC_ADDR,0x18);
//    IoPortOutB(CRTC_ADDR+1,0xFF);
      VgaIoWriteIx(CRTC_ADDR,0xFF18);

      setpalette256();
      Mode.width = 360;
      Mode.height = 400;
      Mode.width_uint8_ts = 36000u;
      Mode.colors = 256;
      Mode.attrib = TVU_GRAPHICS | TVU_UNCHAINED;
   }
   else if (mode == MODE_H)                     // unchained 360 x 480 x 256
   {
      ModeSet(mode13h);

      // Turn off Chain 4
//    outport(SEQ_ADDR,0x0604);
//    IoPortOutB(SEQ_ADDR,0x04);
//    IoPortOutB(SEQ_ADDR+1,0x06);
      VgaIoWriteIx(SEQ_ADDR,0x0604);
      // Activate a synchronous reset
//    outport(SEQ_ADDR,0x0100);
//    IoPortOutB(SEQ_ADDR,0x00);
//    IoPortOutB(SEQ_ADDR+1,0x01);
      VgaIoWriteIx(SEQ_ADDR,0x0100);
      // Select 28 mhz pixel clock
//    outportb(MISC_ADDR,0xE7);
      IoPortOutB(MISC_ADDR,0xE7);

      // Release synchronous reset
//    outport(SEQ_ADDR,0x0300);
//    IoPortOutB(SEQ_ADDR,0x00);
//    IoPortOutB(SEQ_ADDR+1,0x03);
      VgaIoWriteIx(SEQ_ADDR,0x0300);

      // change the vertical resolution flags to 480
//    outportb(MISC_ADDR,(inportb(0x3CC) & ~0xC0) | 0xC0);
      IoPortOutB(MISC_ADDR,(IoPortInB(0x3CC) & ~0xC0) | 0xC0);

      // turn off write protect
//    outport(CRTC_ADDR,0x2C11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0x2C);
      VgaIoWriteIx(CRTC_ADDR,0x2C11);
//    outport(CRTC_ADDR,0x6B00);
//    IoPortOutB(CRTC_ADDR,0x00);
//    IoPortOutB(CRTC_ADDR+1,0x6B);
      VgaIoWriteIx(CRTC_ADDR,0x6B00);
//    outport(CRTC_ADDR,0x5901);
//    IoPortOutB(CRTC_ADDR,0x01);
//    IoPortOutB(CRTC_ADDR+1,0x59);
      VgaIoWriteIx(CRTC_ADDR,0x5901);
//    outport(CRTC_ADDR,0x5A02);
//    IoPortOutB(CRTC_ADDR,0x02);
//    IoPortOutB(CRTC_ADDR+1,0x5A);
      VgaIoWriteIx(CRTC_ADDR,0x5A02);
//    outport(CRTC_ADDR,0x8E03);
//    IoPortOutB(CRTC_ADDR,0x03);
//    IoPortOutB(CRTC_ADDR+1,0x8E);
      VgaIoWriteIx(CRTC_ADDR,0x8E03);
//    outport(CRTC_ADDR,0x5E04);
//    IoPortOutB(CRTC_ADDR,0x04);
//    IoPortOutB(CRTC_ADDR+1,0x5E);
      VgaIoWriteIx(CRTC_ADDR,0x5E04);
//    outport(CRTC_ADDR,0x8A05);
//    IoPortOutB(CRTC_ADDR,0x05);
//    IoPortOutB(CRTC_ADDR+1,0x8A);
      VgaIoWriteIx(CRTC_ADDR,0x8A05);
//    outport(CRTC_ADDR,0x0D06);
//    IoPortOutB(CRTC_ADDR,0x06);
//    IoPortOutB(CRTC_ADDR+1,0x0D);
      VgaIoWriteIx(CRTC_ADDR,0x0D06);
//    outport(CRTC_ADDR,0x3E07);
//    IoPortOutB(CRTC_ADDR,0x07);
//    IoPortOutB(CRTC_ADDR+1,0x3E);
      VgaIoWriteIx(CRTC_ADDR,0x3E07);
//    outport(CRTC_ADDR,0x0008);
//    IoPortOutB(CRTC_ADDR,0x08);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x0008);
//    outport(CRTC_ADDR,0x4009);
//    IoPortOutB(CRTC_ADDR,0x09);
//    IoPortOutB(CRTC_ADDR+1,0x40);
      VgaIoWriteIx(CRTC_ADDR,0x4009);
//    outport(CRTC_ADDR,0x000A);
//    IoPortOutB(CRTC_ADDR,0x0A);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000A);
//    outport(CRTC_ADDR,0x000B);
//    IoPortOutB(CRTC_ADDR,0x0B);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000B);
//    outport(CRTC_ADDR,0x000C);
//    IoPortOutB(CRTC_ADDR,0x0C);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000C);
//    outport(CRTC_ADDR,0x000D);
//    IoPortOutB(CRTC_ADDR,0x0D);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000D);
//    outport(CRTC_ADDR,0x000E);
//    IoPortOutB(CRTC_ADDR,0x0E);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000E);
//    outport(CRTC_ADDR,0x000F);
//    IoPortOutB(CRTC_ADDR,0x0F);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x000F);
//    outport(CRTC_ADDR,0xEA10);
//    IoPortOutB(CRTC_ADDR,0x10);
//    IoPortOutB(CRTC_ADDR+1,0xEA);
      VgaIoWriteIx(CRTC_ADDR,0xEA10);
//    outport(CRTC_ADDR,0xAC11);
//    IoPortOutB(CRTC_ADDR,0x11);
//    IoPortOutB(CRTC_ADDR+1,0xAC);
      VgaIoWriteIx(CRTC_ADDR,0xAC11);
//    outport(CRTC_ADDR,0xDF12);
//    IoPortOutB(CRTC_ADDR,0x12);
//    IoPortOutB(CRTC_ADDR+1,0xDF);
      VgaIoWriteIx(CRTC_ADDR,0xDF12);
//    outport(CRTC_ADDR,0x2D13);
//    IoPortOutB(CRTC_ADDR,0x13);
//    IoPortOutB(CRTC_ADDR+1,0x2D);
      VgaIoWriteIx(CRTC_ADDR,0x2D13);
//    outport(CRTC_ADDR,0x0014);
//    IoPortOutB(CRTC_ADDR,0x14);
//    IoPortOutB(CRTC_ADDR+1,0x00);
      VgaIoWriteIx(CRTC_ADDR,0x0014);
//    outport(CRTC_ADDR,0xE715);
//    IoPortOutB(CRTC_ADDR,0x15);
//    IoPortOutB(CRTC_ADDR+1,0xE7);
      VgaIoWriteIx(CRTC_ADDR,0xE715);
//    outport(CRTC_ADDR,0x0616);
//    IoPortOutB(CRTC_ADDR,0x16);
//    IoPortOutB(CRTC_ADDR+1,0x06);
      VgaIoWriteIx(CRTC_ADDR,0x0616);
//    outport(CRTC_ADDR,0xE317);
//    IoPortOutB(CRTC_ADDR,0x17);
//    IoPortOutB(CRTC_ADDR+1,0xE3);
      VgaIoWriteIx(CRTC_ADDR,0xE317);
//    outport(CRTC_ADDR,0xFF18);
//    IoPortOutB(CRTC_ADDR,0x18);
//    IoPortOutB(CRTC_ADDR+1,0xFF);
      VgaIoWriteIx(CRTC_ADDR,0xFF18);

      setpalette256();
      Mode.width = 360;
      Mode.height = 480;
      Mode.width_uint8_ts = 43200u;
      Mode.colors = 256;
      Mode.attrib = TVU_GRAPHICS | TVU_UNCHAINED;
   }
   else if (mode == MODE_I)                     // 640 x 400 x 16
   {
      ModeSet(mode10h);
//      asm {

//         MOV DX,03CCH
//         IN AL,DX
//         AND AL,03FH
//         OR AL,40H

//         MOV DX,03C2H
//         OUT DX,AL
           IoPortOutB(0x3C2,(IoPortInB(0x3CC)&0x3F)|0x40);

//         MOV DX,CRTC_ADDR
//         MOV AX,9C10H
//         OUT DX,AX
//         IoPortOutB(CRTC_ADDR,0x10);
//         IoPortOutB(CRTC_ADDR+1,0x9C);
           VgaIoWriteIx(CRTC_ADDR,0x9C10);

//         MOV AX,8311H
//         OUT DX,AX
//         IoPortOutB(CRTC_ADDR,0x11);
//         IoPortOutB(CRTC_ADDR+1,0x83);
           VgaIoWriteIx(CRTC_ADDR,0x8311);
//         MOV AX,8F12H
//         OUT DX,AX
//         IoPortOutB(CRTC_ADDR,0x12);
//         IoPortOutB(CRTC_ADDR+1,0x8F);
           VgaIoWriteIx(CRTC_ADDR,0x8F12);

//         MOV AX,9615H
//         OUT DX,AX
//         IoPortOutB(CRTC_ADDR,0x15);
//         IoPortOutB(CRTC_ADDR+1,0x96);
           VgaIoWriteIx(CRTC_ADDR,0x9615);

//         MOV AX,0B916H
//         OUT DX,AX
//         IoPortOutB(CRTC_ADDR,0x16);
//         IoPortOutB(CRTC_ADDR+1,0xB9);
           VgaIoWriteIx(CRTC_ADDR,0xB916);

//      }
      setpalette16();
      Mode.width = 640;
      Mode.height = 400;
      Mode.width_uint8_ts = 32000;
      Mode.colors = 16;
      Mode.attrib = TVU_GRAPHICS | TVU_PLANAR;
   }
   else if (mode == MODE_J)                    // 80 x 43 x 16
   {
      ModeSet(modeJ);
      FontsRead(Font8x8,8);

      Mode.width = 80;
      Mode.height = 43;
      Mode.width_uint8_ts = 3440;
      Mode.colors = 16;
      Mode.attrib = TVU_TEXT;
   }
   else if (mode == MODE_K)                    // 80 x 50 x 16
   {
      ModeSet(modeK);
      FontsRead(Font8x8,8);

      Mode.width = 80;
      Mode.height = 50;
      Mode.width_uint8_ts = 4000;
      Mode.colors = 16;
      Mode.attrib = TVU_TEXT;
   }
   else if (mode == MODE_L)                    // 40 x 43 x 16
   {
      ModeSet(modeL);
      FontsRead(Font8x8,8);

      Mode.width = 40;
      Mode.height = 43;
      Mode.width_uint8_ts = 4000;
      Mode.colors = 16;
      Mode.attrib = TVU_TEXT;
   }
   else if (mode == MODE_M)                    // 40 x 50 x 16
   {
      ModeSet(modeM);
      FontsRead(Font8x8,8);

      Mode.width = 40;
      Mode.height = 50;
      Mode.width_uint8_ts = 4000;
      Mode.colors = 16;
      Mode.attrib = TVU_TEXT;
   }

}


void  TR9000i_Init(void)
{
    TRSubsEnable();
  VgaIoWriteIx(0x3C4,0x000B);     //  Force old_mode_registers
  unsigned int chp = IoPortInB(0x3C5);         //  Read chip ID and switch to new_mode_registers}
  unsigned int old = VgaIoReadIx(0x3C4,0x0E);
  IoPortOutB(0x3C5,0x00);
  unsigned int value = IoPortInB(0x3C5) & 0x0F;
  IoPortOutB(0x3C5,old);
  Serial.print("detected chip: ");
  Serial.println(chp, HEX);
      IoPortOutB(0x3C5,old^2);


  uint16_t  i=0;
  IoPortOutB(0x03C3,0x00);
  if(!sub_4D9()) {
    Serial.println("Initialization failed.");
    return;
  }

  do
  {
    IoPortOutB(0x3C9,0x00);
    i++;
  }while(i<768);

  IoPortOutB(0x3C2,0x23);
  sub_51A();
//  IoPortOutB(0x3D4,0x1F);
//  IoPortOutB(0x3D5,0x81);

//  IoPortOutB(0x3D4,0x25);
//  IoPortOutB(0x3D5,0xFF);


 // if(((sub_292()&0x0E)==0x0C)&& IoPortInB(0x3CC)==0x67));

}
//*********************************************************************

#define SEGMENT_OFFSET(seg, off) ((((uint32_t)(seg) << 8)) + (uint32_t)off)

         uint32_t VidMemBase = SEGMENT_OFFSET(0xB800, 0);

         uint16_t   CursorScreen = 0;
         uint8_t   CurrentAttrib = 0x0F;
void  SetHwCursor(uint16_t pos)
{
    VgaIoWriteIx(0x3D4,((CursorScreen/256)<<8)+0x0E);
    VgaIoWriteIx(0x3D4,((CursorScreen%256)<<8)+0x0F);
}


void VgaMemoryWriteW(uint32_t addr, uint16_t data) {
  writeMemory(addr, data);
}

void TextClear(uint8_t attrib)
{
  uint16_t     i,ilim = Mode.width_uint8_ts ;
  uint32_t  address = VidMemBase;

//   asm {
//   MOV AX,0B800H
//   MOV ES,AX
//   XOR DI,DI

//   MOV AH,attrib
//   MOV AL,' '
//   MOV CX,bytes
//   CLD
//   REP STOSW
//   }
  for(i=0;i<ilim;i++)
  {
    VgaMemoryWriteW(address,(attrib<<8)+' ');
    address++;
//    VgaMemoryWriteB(address,attrib);
    address++;
  }
   CurrentAttrib = attrib;
  CursorScreen = 0;
  SetHwCursor(CursorScreen);
}

void pixel12H(uint16_t x, uint16_t y, uint8_t color)
{
     VgaIoWriteIx(0x3CE,((1<<(((x%256)&7)^7))<<8)+0x08);
     VgaIoWriteIx(0x3C4,(color<<8)+0x02);
     readMemoryByte(0xA00000+(y*80)+(x>>3));
     writeMemoryByte(0xA00000+(y*80)+(x>>3),0xFF);
}

void Pixel13H(uint16_t x, uint16_t y, uint8_t color);

void pixel(uint16_t x, uint16_t y, uint8_t color)
{
   int width = Mode.width;

   if (Mode.mode == MODE13H)
      Pixel13H(x,y,color);
   else if (Mode.attrib & TVU_UNCHAINED) {
//   asm {
//   MOV AX,0A000H   //    video memory segment number
//   MOV ES,AX       //    place it in es

//   MOV DX,03C4H
//   MOV AL,2
//   OUT DX,AL
//   INC DX
//     IoPortOutB(0x3C4,0x02);
//   MOV AL,1
//   MOV CX,x
//   AND CX,3
//   SHL AL,CL
//   OUT DX,AL
//     IoPortOutB(0x3C5,1<<((x%256)&3));
     VgaIoWriteIx(0x3C4,((1<<((x%256)&3))<<8)+0x02);
//   XOR DI,DI

// Calculate the Offset
//   mov ax,width  // width / 4
//   SHR AX,2
//   mul y         // (Y * (width / 4))
//   mov bx,x      // (X / 4) + (Y * (width / 4))
//   shr bx,2
//   add ax,bx
// Done!

//   ADD DI,AX
//   mov ah,color   //    move the Color into ah
//   mov es:[di],ah //    move the value to the screen
//   }
   writeMemoryByte(0xA00000+(x/4)+(y*(width/4)),color);
   }
   else if (Mode.attrib & TVU_PLANAR)
   {
//   asm {
//   MOV AX,0A000H   //    video memory segment number
//   MOV ES,AX       //    place it in es

//   MOV BX,x        //    X Value
//   MOV CX,BX
//   MOV AX,y        //    Y Value
//   MOV SI,80
//   MUL SI
//   SHR BX,3        //    /8
//   ADD AX,BX
//   MOV DI,AX


//   AND CL,7
//   XOR CL,7
//   MOV AH,1



//   SHL AH,CL

//   MOV DX,03CEH
//   MOV AL,8
//   OUT DX,AX
//     IoPortOutB(0x3CE,0x08);
//     IoPortOutB(0x3CF,1<<(((x%256)&7)^7));
//     IoPortOutB(0x3CF,rv);
     VgaIoWriteIx(0x3CE,((1<<(((x%256)&7)^7))<<8)+0x08);
//     IoPortOutB(0x3CF,0x0F);
//   MOV DX,03C4H
//   MOV AH,color
//   MOV AL,2
//   OUT DX,AX
//     IoPortOutB(0x3C4,0x02);
//     IoPortOutB(0x3C5,color);
     VgaIoWriteIx(0x3C4,(color<<8)+0x02);
//   MOV AL,0
//   XCHG ES:[DI],AL
     readMemoryByte(0xA00000+(y*80)+(x>>3));

//   MOV BYTE PTR ES:[DI],0FFh
     writeMemoryByte(0xA00000+(y*80)+(x>>3),0xFF);
//   MOV AX,0F02H
//   OUT DX,AX
//     IoPortOutB(0x3C4,0x02);
//     IoPortOutB(0x3C5,0x0F);
//     VgaIoWriteIx(0x3C4,(0x0F<<8)+0x02);

//   MOV DX,03CEH
//   MOV AX,0FF08h
//   OUT DX,AX
//     IoPortOutB(0x3CE,0x08);
//     IoPortOutB(0x3CF,0xFF);
//     VgaIoWriteIx(0x3CE,(0xFF<<8)+0x08);
   //}
   }
}

void  SetScrPage(uint8_t page)     // Set desired screen page
{
  uint32_t  addr;

  addr = 2000*(page&0x07);
  VgaIoWriteIx(0x3D4,((addr/256)<<8)+0x0C);
  VgaIoWriteIx(0x3D4,((addr%256)<<8)+0x0D);
  VidMemBase = 0xB08000 + (addr<<1);
}
//*********************************************************************
void  CursorOn(void)
{
  VgaIoWriteIx(0x3D4,((VgaIoReadIx(0x3D4,0x0A)&(0xFF-0x20))<<8)+0x0A);
}


void rect(int x, int y, int w, int h, uint8_t c) {
  for (int i = x; i < x + w; i++) 
    for (int j = y; j < y + h; j++) pixel(i, j, c);
}


void  DrawCircle(uint16_t x, uint16_t y, uint16_t rad,uint8_t color)
{
  long  Hiba;   //hiba valtozo
  long  X;
  long  Y;
  long  DU;     // hiba modosito, ha csak X lepett
  long  DD;      //hiba modosito, ha X es Y is lepett

  Hiba = 1L - rad;
  X = 0;
  Y = (long)rad;
  DU = 3L;
  DD = 5L - (2L * rad);

  // kezdopont kirajzol
  pixel(X+x,Y+y,color);
  //pixel12H(X+x,Y+y,color);

//  KorCikkInterpolal:

//  DoEvents

//  'Vege a 90 foktol 45 fokigtarto resznek?
  while(!(X > Y))
  {
    //'x mindig lep
    X = X + 1;
    if(Hiba < 0)
    {
      //az x-heztartozo felso y a jobb
      //azaz az aktualison marad
      Hiba = Hiba + DU;
      DU = DU + 2;
      DD = DD + 2;
    }
    else
    {
      //az x-heztartozo also y a jobb
      //azzaz y mar lephet egyet lefele
      Y = Y - 1;
      Hiba = Hiba + DD;
      DU = DU + 2;
      DD = DD + 4;
    }
    //aktualis kirajzol
    pixel(X+x, Y+y,color);
    //pixel12H(X+x, Y+y,color);
  }
}

void Clear13H(uint8_t Color)
{

   uint16_t     i;
   uint32_t  address = 0xA0000;

//   asm {
//   MOV AX,0A000H
//   MOV ES,AX
//   XOR DI,DI

//   MOV AL,Color
//   MOV AH,AL
//   CLD
//   MOV CX,8000H
//   REP STOSW
//   }
   i=0;
   do
   {
      writeMemoryByte(address,Color);
      address++;
      i++;
   }while(i);
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



//Clear13H(0x00);
  //for (int i = 0; i < 300; i++) for (int j = 0; i < 300; i++) pixel(i, i, 0x01);

  Serial.println("dumped");

}



void Pixel13H(uint16_t x, uint16_t y, uint8_t color)
{
//   data  DBLWORD  address;

//   data  BYTE     width = Mode.width;
//   asm {
//   MOV AX,0A000H   //    video memory segment number
//   MOV ES,AX       //    place it in es

//   MOV DX,03C4H
//   MOV AL,2
//   OUT DX,AL
//   INC DX
//     IoPortOutB(0x3C4,0x02);

//   MOV AL,1
//   MOV CX,x
//   AND CX,3
//   SHL AL,CL
//   OUT DX,AL
//     IoPortOutB(0x3C5,1<<((x%256)&3));
     VgaIoWriteIx(0x3C4,((1<<((x%256)&3))<<8)+0x02);
//   XOR DI,DI

// Calculate the Offset
//   mov ax,width  // width
//   mul y         // (Y * width))
//   mov bx,x      // (X + (Y * width))
//   add ax,bx
// Done!

//   ADD DI,AX
//   mov ah,color   //    move the Color into ah
//   mov es:[di],ah //    move the value to the screen
//   }
    writeMemoryByte(0xA0000+(x+(y*Mode.width)),color);
}

#define outb writeIO
#define inb readIO

void update_cursor(uint16_t pos)
{
	// uint16_t pos = y * mode.width + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}


uint32_t abc = 0xB8000, ii = 0, attr = 0x17;
bool done = false;

void loop() {
  if (! done) {
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
    const char* str = "Hello world!";
    while(*str) {
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
