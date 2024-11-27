
// References:
// https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
// https://chromium.googlesource.com/chromiumos/third_party/seabios/+/refs/heads/0.14.811.B/vgasrc/vgaio.c
// https://android.googlesource.com/kernel/msm.git/+/android-9.0.0_r0.1/include/video/trident.h
// https://cdn.netbsd.org/pub/NetBSD/NetBSD-current/src/sys/arch/amiga/dev/grf_cv3dreg.h
// https://www.vgamuseum.info/index.php/component/k2/item/443-trident-tvga9000i-1


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
  vga_set_mode(MODE03H);

  Serial.println("initialized");
}

void loop() {
  vga_cursor_visible(true);

  console_clear();
  // console_set_cursor(0, 0);
  for (int fc = 0; fc < 16; fc++) {
    vga_set_coursor_pos(fc, 0);
    console_puts("Hello, world!\n", fc, VGA_WHITE);
    delay(1000);
  }

  delay(60000);
}
