#ifndef VGA_H
#define VGA_H

#include <stdint.h>

// VGA Stuff 

#define	VGA_AC_INDEX		    0x3C0
#define	VGA_AC_WRITE		    0x3C0
#define	VGA_AC_READ		      0x3C1
#define	VGA_MISC_WRITE	    0x3C2
#define VGA_SEQ_INDEX		    0x3C4
#define VGA_SEQ_DATA		    0x3C5
#define VGA_PEL_MASK        0x3C6
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
#define	VGA_INSTAT_READ		  0x3DA // Input Status register


#define	VGA_NUM_SEQ_REGS	  5
#define	VGA_NUM_CRTC_REGS	  25
#define	VGA_NUM_GC_REGS		  9
#define	VGA_NUM_AC_REGS		  21
#define	VGA_NUM_REGS		    (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)

// Video modes
#define MODE03H 0x03
#define MODE12H 0x12
#define MODE13H 0x13

// Start addresses of video memory
#define VIDEO_MEMORY_GRAPH 0xA0000
#define VIDEO_MEMORY_CTEXT 0xB8000

#define TVU_TEXT 0x0001
#define TVU_GRAPHICS 0x0002
#define TVU_MONOCHROME 0x0004
#define TVU_PLANAR 0x0008
#define TVU_UNCHAINED 0x0010

// Text Color Constants
#define VGA_BLACK         0x0
#define VGA_BLUE          0x1
#define VGA_GREEN         0x2
#define VGA_CYAN          0x3
#define VGA_RED           0x4
#define VGA_MAGENTA       0x5
#define VGA_BROWN         0x6
#define VGA_LIGHT_GRAY    0x7
#define VGA_DARK_GRAY     0x8
#define VGA_LIGHT_BLUE    0x9
#define VGA_LIGHT_GREEN   0xA
#define VGA_LIGHT_CYAN    0xB
#define VGA_LIGHT_RED     0xC
#define VGA_LIGHT_MAGENTA 0xD
#define VGA_YELLOW        0xE
#define VGA_WHITE         0xF

typedef struct _VMODE_ST {
  uint8_t mode;            // Videomode Number
  uint16_t width;          // Width in pixels
  uint16_t height;         // Height in pixels
  uint16_t width_bytes;    // Number of bytes per screen
  uint16_t colors;         // Number of colors
  uint16_t attrib;         // Videomode attributes
} VMODE_ST;


void VgaIoWriteIx(uint32_t addr, uint16_t valIx);
uint8_t VgaIoReadIx(uint32_t addr, uint8_t ix);

void vga_set_mode(uint8_t mode);
const VMODE_ST* vga_get_current_mode();

void vga_screen_clear(uint8_t color);
void vga_set_pixel(uint16_t x, uint16_t y, uint8_t color);

void vga_set_coursor_pos(uint8_t row, uint8_t col);
void vga_cursor_visible(bool visible);
void vga_set_character(uint8_t row, uint8_t col, char character, uint8_t attribs);
void vga_set_blinking(bool enabled);

#endif