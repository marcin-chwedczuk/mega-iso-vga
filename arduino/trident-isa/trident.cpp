
#include "trident.h"
#include "isa.h"
#include "vga.h"

// TODO: Cleanup later
#include <HardwareSerial.h>


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
