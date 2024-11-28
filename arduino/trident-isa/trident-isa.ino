
// References:
// https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
// https://chromium.googlesource.com/chromiumos/third_party/seabios/+/refs/heads/0.14.811.B/vgasrc/vgaio.c
// https://android.googlesource.com/kernel/msm.git/+/android-9.0.0_r0.1/include/video/trident.h
// https://cdn.netbsd.org/pub/NetBSD/NetBSD-current/src/sys/arch/amiga/dev/grf_cv3dreg.h
// https://www.vgamuseum.info/index.php/component/k2/item/443-trident-tvga9000i-1
//
// https://github.com/qemu/qemu-palcode/blob/master/vgaio.c
// https://chromium.googlesource.com/chromiumos/third_party/seabios/+/refs/heads/release-1011.B/vgasrc/vgaio.c


#include "vga.h"
#include "isa.h"
#include "trident.h"
#include "graphics.h"
#include "console.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();

  isa_arduino_setup();

  delay(1000);

  TR9000i_init();
  vga_set_mode(MODE13H);

  gfx_draw_rect(0, 0, 320, 200, VGA_BLACK);
}

void loop() {
  /*
  vga_cursor_visible(true);

  console_clear();
  // console_set_cursor(0, 0);
  for (int fc = 0; fc < 16; fc++) {
    // vga_set_coursor_pos(fc, 0);
    console_puts("Hello, world!\n", fc, VGA_WHITE);
    delay(1000);
  }
  */

  
  // Serial.println("initialized");



  for (int i = 0; i < 16; i++) {
    gfx_draw_circle(80, 80, 50+i, i);
    gfx_draw_circle(81, 81, 50+i, i);
  }

  delay(1000);
}
