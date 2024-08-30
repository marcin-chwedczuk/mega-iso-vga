#define ATTRCON_ADDR			0x03C0
#define MISC_ADDR         0x03C2
#define VGAENABLE_ADDR    0x03C3
#define SEQ_ADDR          0x03C4
#define GRACON_ADDR       0x03CE
#define CRTC_ADDR         0x03D4
#define STATUS_ADDR       0x03DA
#include "font.h"

uint8_t   *FontPtr = Font8x16;


void SetFont8x16(void)
{
  FontPtr = Font8x16;
}

void SetFont8x8(void)
{
  FontPtr = Font8x8;
}

#define SEGMENT_OFFSET(seg, off) (( ((uint32_t)(seg)) << 8 ) + off)


//*********************************************************************
void FontsRead( uint8_t  *biosfont, uint8_t uint8_tsperchar)
{
  uint32_t   vidmem;
  uint16_t      oldmode,oldmisc,oldmem,oldmask;
  uint16_t      newmode,newmisc,newmem;
  uint16_t      i,j;

  vidmem = SEGMENT_OFFSET(0xA000, 0);

//  Store the OLD 'Mode Register' value
//  outportb(GRACON_ADDR,5);
//  oldmode = inportb(GRACON_ADDR+1);
//  IoPortOutB(GRACON_ADDR,5);
//  oldmode = IoPortInB(GRACON_ADDR+1);
  oldmode = VgaIoReadIx(GRACON_ADDR,5);

//  Store the OLD 'Miscellaneous Register' value
//  outportb(GRACON_ADDR,6);
//  oldmisc = inportb(GRACON_ADDR+1);
//  IoPortOutB(GRACON_ADDR,6);
//  oldmisc = IoPortInB(GRACON_ADDR+1);
  oldmisc = VgaIoReadIx(GRACON_ADDR,6);

//  Store the OLD 'Mask Map' value
//  outportb(SEQ_ADDR,2);
//  oldmask = inportb(SEQ_ADDR+1);
//  IoPortOutB(SEQ_ADDR,2);
//  oldmask = IoPortInB(SEQ_ADDR+1);
  oldmask = VgaIoReadIx(SEQ_ADDR,2);

//  Store the OLD 'Memory Mode' value
//  outportb(SEQ_ADDR,4);
//  oldmem = inportb(SEQ_ADDR+1);
//  IoPortOutB(SEQ_ADDR,4);
//  oldmem = IoPortInB(SEQ_ADDR+1);
  oldmem = VgaIoReadIx(SEQ_ADDR,4);

//  Write the NEW 'Mode Register' value
  newmode = (oldmode & 0xFC);

//  outport(GRACON_ADDR, (newmode << 8) | 0x05);
//  IoPortOutB(GRACON_ADDR,0x05);
//  IoPortOutB(GRACON_ADDR+1,newmode);
  VgaIoWriteIx(GRACON_ADDR, (newmode << 8) | 0x05);

//  Write the NEW 'Miscellaneous Register' value
  newmisc = ((oldmisc & 0xF1)|0x04);

//  outport(GRACON_ADDR, (newmisc << 8) | 0x06);
//  IoPortOutB(GRACON_ADDR,0x06);
//  IoPortOutB(GRACON_ADDR+1,newmisc);
  VgaIoWriteIx(GRACON_ADDR, (newmisc << 8) | 0x06);

//  Write the NEW 'Mask Map' value
//  outport(SEQ_ADDR, 0x0402);
//  IoPortOutB(SEQ_ADDR,2);
//  IoPortOutB(SEQ_ADDR+1,0x04);
  VgaIoWriteIx(SEQ_ADDR, 0x0402);

//  Write the NEW 'Memory Mode' value
  newmem = (oldmem | 4);

//  outport(SEQ_ADDR, (newmem << 8) | 0x04);
//  IoPortOutB(SEQ_ADDR,4);
//  IoPortOutB(SEQ_ADDR+1,newmem);
  VgaIoWriteIx(SEQ_ADDR, (newmem << 8) | 0x04);

//  Copy the font from BIOS
  i = 0;
  biosfont++ ;  // Skip fontsize descriptor
  do
  {
    for (j = 0; j < uint8_tsperchar; j++)
    {
      writeMemoryByte(vidmem, *biosfont);
      vidmem++;
      biosfont++;
    }
    for (j = 0; j < 32-uint8_tsperchar; j++)
    {
      writeMemoryByte(vidmem,0x00);
      vidmem++;
    }
    //i++;
  }while(i++ != 255);
//  Write the OLD 'Mode Register' value
//  outport(GRACON_ADDR, (oldmode << 8) | 0x05);
//  IoPortOutB(GRACON_ADDR,0x05);
//  IoPortOutB(GRACON_ADDR+1,oldmode);
  VgaIoWriteIx(GRACON_ADDR, (oldmode << 8) | 0x05);

//  Write the OLD 'Miscellaneous Register' value
//  outport(GRACON_ADDR, (oldmisc << 8) | 0x06);
//  IoPortOutB(GRACON_ADDR,0x06);
//  IoPortOutB(GRACON_ADDR+1,oldmisc);
  VgaIoWriteIx(GRACON_ADDR, (oldmisc << 8) | 0x06);

//  Write the OLD 'Mask Map' value
//  outport(SEQ_ADDR,(oldmask << 8) | 0x02);
//  IoPortOutB(SEQ_ADDR,2);
//  IoPortOutB(SEQ_ADDR+1,oldmask);
  VgaIoWriteIx(SEQ_ADDR,(oldmask << 8) | 0x02);

//  Write the OLD 'Memory Mode' value
//  outport(SEQ_ADDR, (oldmem << 8) | 0x04);
//  IoPortOutB(SEQ_ADDR,4);
//  IoPortOutB(SEQ_ADDR+1,oldmem);
  VgaIoWriteIx(SEQ_ADDR, (oldmem << 8) | 0x04);
}

