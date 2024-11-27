
// References:
// https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
// https://chromium.googlesource.com/chromiumos/third_party/seabios/+/refs/heads/0.14.811.B/vgasrc/vgaio.c
// https://android.googlesource.com/kernel/msm.git/+/android-9.0.0_r0.1/include/video/trident.h
// https://cdn.netbsd.org/pub/NetBSD/NetBSD-current/src/sys/arch/amiga/dev/grf_cv3dreg.h
// https://www.vgamuseum.info/index.php/component/k2/item/443-trident-tvga9000i-1


#include "isa.h"
#include "vga.h"
#include "font.h"
#include "trident.h"
#include "palette.h"
#include "graphics.h"

void Clear13H(uint8_t Color) {

  uint16_t i;
  uint32_t address = 0xA0000;

  i = 0;
  do {
    isa_write_byte(address, Color);
    address++;
    i++;
  } while (i);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  isa_arduino_setup();

  delay(1000);

  TR9000i_init();
  vga_set_mode(MODE12H);
  // Clear13H(0x00);

/*
  TextClear(0xAA);

  SetScrPage(0);
  SetHwCursor(8);
  CursorOn();
*/
  // Clear13H(0x00);
  // for (int i = 0; i < 300; i++) for (int j = 0; i < 300; i++) pixel(i, i,
  // 0x01);

  Serial.println("dumped");
}


uint32_t abc = 0xB8000, ii = 0, attr = 0x0f;
bool done = false;

uint8_t color = 0x11;

void loop() {
  /*
  int rx = random(50, 250);
  int ry = random(50, 150);
  int rr = random(10, 50);

  drawCircle(rx, ry, rr, color++);*/

  
  for (int y = 0; y < 480; y++) {
    for (int x = 0; x < 640; x++) {

      vga_set_pixel(x, y, (x >> 4) & 0xff);
    }
  }

  delay(1000);

  /*
  if (!done) {
    // clear screen
    abc = 0xB8000;
    for (int i = 0; i < 25; i++) {
      for (int j = 0; j < 80; j++) {
        isa_write_word(abc, 0x0000);
        abc += 2;
      }
    }

    abc = 0xB8000;
    update_cursor(0);
    const char *str = "Hello world!";
    while (*str) {
      isa_write_word(abc, (attr << 8) + (*str));

      str++;
      abc += 2;
    }
  }
  done = true;
  */
  /*
  isa_write_word(abc, (attr << 8) + (ii & 0xFF));
  abc += 2;
  update_cursor(ii);  ii +=1;

  if (ii > (80 * 25)) {
    ii = 0; attr++; abc = 0xB8000;
  }*/
}
