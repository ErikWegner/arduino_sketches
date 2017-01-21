#define DEBUG 0
#define BENCHMARK 1

#define BCM_RESOLUTION 5
#define CLK 19
#define LAT 18
#define OE  17
#define A   23
#define B   22
#define C   15
#define D   16
#define DATAPORT PORTD
#define DATADIR  DDRD
#define WIDTH 64
#define HEIGHT 32

uint8_t buffer[BCM_RESOLUTION][WIDTH * HEIGHT / 2]; // first dimension: time, second dimension pixel

uint8_t intensities[8] = {1, 2, 3, 4, 5, 6, 15, 31}; // max: (1 << BCM_RESOLUTION) - 1

#if BENCHMARK == 1
#define BENCHMARKSAMPLES 1000
volatile uint16_t benchmark_results[BENCHMARKSAMPLES] = { 0 };
volatile uint16_t benchmark_sampleindex = 0;
volatile uint8_t  benchmark_output = 0;
#endif

/* Match Adafruit function signature */
void drawPixel(uint16_t x, uint16_t y, uint16_t c) {
  uint8_t r, g, b;


  if ((x < 0) || (x >= WIDTH) || (y < 0) || (y >= HEIGHT)) return;
  /* TODO: support rotation */

  // calculate bcm values for each color
  r = c >> 11;                           // RRRRRggggggbbbbb
  g = (c >> 6) & ((1 << BCM_RESOLUTION) - 1); // RRRRRggggggbbbbb
  b = c        & ((1 << BCM_RESOLUTION) - 1); // RRRRRggggggbbbbb
  drawPixel555(x, y, r, g, b);
}

void drawPixel555(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
  if ((x < 0) || (x >= WIDTH) || (y < 0) || (y >= HEIGHT)) return;
  uint8_t addressmask, bcmbit, colorvalue;
  uint16_t c_pixel = (y >= HEIGHT ? y - HEIGHT/2 : y) * WIDTH + x;
  addressmask = (y >= HEIGHT / 2) ? B11100000 : B00011100;

  for (uint8_t c_time = 0; c_time < BCM_RESOLUTION; c_time++) {
    bcmbit = (1 << c_time);

    colorvalue = (
                   // combine all colors for all rows
                   ((r & bcmbit) > 0 ? B00100100 : B00000000)
                   | ((g & bcmbit) > 0 ? B01001000 : B00000000)
                   | ((b & bcmbit) > 0 ? B10010000 : B00000000)
                 ) & addressmask; // select only relevant bits for the row

    buffer[c_time][c_pixel] =
      (buffer[c_time][c_pixel] & ~addressmask) // set all relevant bits to zero
      | colorvalue; // add new color bits;
  }
}

void setupBuffer(uint8_t highpos) {
  for (uint8_t c_pixel = 0; c_pixel < 8; c_pixel++) {
    uint8_t intensity = intensities[(highpos + c_pixel) % 8];

    for (uint8_t c_time = 0; c_time < BCM_RESOLUTION; c_time++) {
      // is bit set for this pixel's brightness
      if ((intensity & (1 << c_time)) > 0) {
        buffer[c_time][c_pixel] = B11100000;
      } else {
        buffer[c_time][c_pixel] = 0;
      }
    }
  }
}

void setupPanelPins() {
  pinMode(CLK, OUTPUT); digitalWriteFast(CLK, LOW);
  pinMode(LAT, OUTPUT); digitalWriteFast(LAT, LOW);
  pinMode(OE, OUTPUT); digitalWriteFast(OE, HIGH);
  pinMode(A, OUTPUT); digitalWriteFast(A, LOW);
  pinMode(B, OUTPUT); digitalWriteFast(B, LOW);
  pinMode(C, OUTPUT); digitalWriteFast(C, LOW);
  pinMode(D, OUTPUT); digitalWriteFast(D, LOW);

  DATADIR = B11111100;
  DATAPORT = 0;

  setupBuffer(0);
}

void updatePanel(uint8_t c_time) {
#if DEBUG == 1
  Serial.print(F("Updating panel... "));
  Serial.print(c_time);
#endif
#if BENCHMARK == 1
  unsigned long starttime = micros();
#endif
  uint16_t i;

  digitalWriteFast(OE, LOW);
  for (uint8_t y = 0; y < HEIGHT / 2; y++) {
    digitalWriteFast(A, y & 0x1);
    digitalWriteFast(B, y & 0x2);
    digitalWriteFast(C, y & 0x4);
    digitalWriteFast(D, y & 0x8);
    for (i = 0; i < WIDTH ; i++) {
      DATAPORT = buffer[c_time][y * WIDTH + i];
      digitalWriteFast(CLK, HIGH);
      __asm__("nop\n\t");
      __asm__("nop\n\t");
      digitalWriteFast(CLK, LOW);
    }
    digitalWriteFast(LAT, HIGH);
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    digitalWriteFast(LAT, LOW);
  }

  digitalWriteFast(OE, HIGH);
#if BENCHMARK == 1
  benchmark_results[benchmark_sampleindex] = micros() - starttime;
  benchmark_sampleindex++;
  if (benchmark_sampleindex >= BENCHMARKSAMPLES) {
    benchmark_sampleindex = 0;
    benchmark_output = 1;
  }
#endif
#if DEBUG == 1
  Serial.println(F(" done"));
#endif
}

void bcmtimer() {
  g_tock--;
  if (g_tock <= 0) {
    g_tick++;
    if (g_tick > BCM_RESOLUTION - 1) {
      g_tick = 0;
    }
    g_tock = 1 << g_tick;
    updatePanel(g_tick);
  }
}

void benchmark() {
#if BENCHMARK == 1
  if (benchmark_output > 0) {
    benchmark_output = 0;
    float timeperoutput = 0;
    for (uint16_t benchmark_c = 0; benchmark_c < BENCHMARKSAMPLES; benchmark_c++) {
      timeperoutput += benchmark_results[benchmark_c] * 1.0 / BENCHMARKSAMPLES;
    }
    Serial.print(timeperoutput);
    Serial.println(F(" time per panel update "));
  }
#endif
}

