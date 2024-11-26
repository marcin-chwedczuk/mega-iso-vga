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

