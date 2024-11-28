
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
  vga_set_mode(MODE03H);
}

void text_demo();

void loop() {
  text_demo();
  delay(10000);

  // Low resolution graphics mode demo
  vga_set_mode(MODE13H);
  vga_screen_clear(0x00);
  mandelbrot(320, 200, 2);

  // High resolution graphics mode demo
  vga_set_mode(MODE12H);
  vga_screen_clear(0x00);
  mandelbrot(640, 480, 4);
}

void text_demo() {
  vga_set_mode(MODE03H);

  console_clear(TEXT_WHITE);  // Sets coursor color
  console_blinking_text(true);

  console_draw_frame(2, 3, 50, 15, TEXT_WHITE | BACKGROUND_BLUE);


  const char* poem[] = { "All that is gold does not glitter,",
                     "Not all those who wander are lost;",
                     "The old that is strong does not wither,",
                     "Deep roots are not reached by the frost.",
                     "",
                     "From the ashes a fire shall be woken,",
                     "A light from the shadows shall spring;",
                     "Renewed shall be blade that was broken,",
                     "The crownless again shall be king.",
                     NULL };
  
  int i = 0;
  while (poem[i]) {
      console_set_cursor(3 + i, 4);
      console_puts(poem[i], TEXT_YELLOW | BACKGROUND_BLUE);
      i++;
  }
}

// Mandelbrot Demo
// https://rosettacode.org/wiki/Mandelbrot_set#Fixed_point_16_bit_arithmetic

short s(short i);
short toPrec(double f, int bitsPrecision);

int mandelbrot(int screen_width, int screen_height, int zoom) {
  // chosen to match https://www.youtube.com/watch?v=DC5wi6iv9io
  // int zoom=4;  // bigger with finer detail ie a smaller step size - leave at 1 for 32x22
  int width = screen_width / zoom;    // basic width of a zx81
  int height = screen_height / zoom;  // basic width of a zx81


  // params
  short bitsPrecision = 6;
  printf("PRECISION=%d\n", bitsPrecision);

  short X1 = toPrec(3.5, bitsPrecision) / zoom;
  short X2 = toPrec(2.25, bitsPrecision);
  short Y1 = toPrec(3, bitsPrecision) / zoom;  // horiz pos
  short Y2 = toPrec(1.5, bitsPrecision);       // vert pos
  short LIMIT = toPrec(4, bitsPrecision);


  // fractal
  short maxIters = 255;

  short py = 0;
  while (py < height * zoom) {
    short px = 0;
    while (px < width * zoom) {
      vga_set_pixel(px, py, VGA_RED);

      short x0 = s(s(px * X1) / width) - X2;
      short y0 = s(s(py * Y1) / height) - Y2;

      short x = 0;
      short y = 0;

      short i = 0;

      short xSqr;
      short ySqr;
      while (i < maxIters) {
        xSqr = s(x * x) >> bitsPrecision;
        ySqr = s(y * y) >> bitsPrecision;

        // Breakout if sum is > the limit OR breakout also if sum is negative which indicates overflow of the addition has occurred
        // The overflow check is only needed for precisions of over 6 bits because for 7 and above the sums come out overflowed and negative therefore we always run to maxIters and we see nothing.
        // By including the overflow break out we can see the fractal again though with noise.
        if ((xSqr + ySqr) >= LIMIT || (xSqr + ySqr) < 0) {
          break;
        }

        short xt = xSqr - ySqr + x0;
        y = s(s(s(x * y) >> bitsPrecision) * 2) + y0;
        x = xt;

        i = i + 1;
      }
      i = i - 1;

      vga_set_pixel(px, py, (i >> 4));
      px = px + 1;
    }

    py = py + 1;
  }
}

// convert decimal value to a fixed point value in the given precision
short toPrec(double f, int bitsPrecision) {
  short whole = ((short)floor(f) << (bitsPrecision));
  short part = (f - floor(f)) * (pow(2, bitsPrecision));
  short ret = whole + part;
  return ret;
}

// convenient casting
short s(short i) {
  return i;
}