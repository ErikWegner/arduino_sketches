#define DEBUG 0
#define BENCHMARK 0

#define BCM_RESOLUTION 4
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
volatile uint8_t is_drawing = 0;
uint8_t intensities[8] = {1, 3, 5, 7, 9, 11, 13, 15}; // max: (1 << BCM_RESOLUTION) - 1
//uint8_t rgbgamma[32] = {
//  0, 0, 1, 1, 1, 2, 2, 2,
//  3, 3, 4, 4, 4, 5, 5, 5,
//  6, 6, 7, 7, 8, 8, 9, 9,
//  10, 12, 14, 16, 18, 20, 31, 63
//};
/* (x / 11.3) ^ 4 https://rechneronline.de/function-graphs/
  input values: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
  output values: 0-63
*/
/*uint8_t rgbgamma[32] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 1, 1, 2, 2, 3, 4,
  5, 6, 8, 10, 12, 14, 17, 20,
  24, 28, 33, 38, 43, 50, 57, 63
  };*/

/* (x / 13.3) ^ 4 https://rechneronline.de/function-graphs/
  input values: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
  output values: 0-31
*/
/*uint8_t rgbgamma[32] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 2, 2,
  3, 3, 3, 4, 5, 6, 7, 9,
  11, 12, 15, 17, 20, 23, 26, 31
  };*/

/* (x / 10.45) ^ 2.5 https://rechneronline.de/function-graphs/
  input values: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32
  output values: 0-15
*/
uint8_t rgbgamma[32] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 2, 2, 2,
  3, 3, 4, 4, 5, 6, 6, 7,
  8, 9, 10, 11, 12, 13, 14, 15
};

#if BENCHMARK == 1
#define BENCHMARKSAMPLES 2000
volatile unsigned long benchmark_results[BENCHMARKSAMPLES] = { 0 };
volatile uint16_t benchmark_sampleindex = 0;
volatile uint8_t  benchmark_output = 0;
#endif

/* Match Adafruit function signature */
void drawPixel(uint16_t x, uint16_t y, uint16_t c) {
  uint8_t r, g, b;


  if ((x < 0) || (x >= WIDTH) || (y < 0) || (y >= HEIGHT)) return;
  /* TODO: support rotation */

  r = c >> 11;       // RRRRRggggggbbbbb
  g = (c >> 6) & 31; // rrrrrGGGGGgbbbbb
  b = c & 31;        // rrrrrggggggBBBBB
  drawPixel555(x, y, rgbgamma[r], rgbgamma[g], rgbgamma[b]);
}

void drawPixel555(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
  if ((x < 0) || (x >= WIDTH) || (y < 0) || (y >= HEIGHT)) return;
  uint8_t addressmask, bcmbit, colorvalue;
  uint16_t c_pixel = (y >= HEIGHT / 2 ? y - HEIGHT / 2 : y) * WIDTH + x;
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
        buffer[c_time][c_pixel] = B00101000;
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

/*
   see https://cdn-learn.adafruit.com/downloads/pdf/connecting-a-16x32-rgb-led-matrix-panel-to-a-raspberry-pi.pdf
*/
void updatePanel(uint8_t c_time, uint8_t y) {
  is_drawing = 1;
#if DEBUG == 1
  Serial.print(F("Updating panel... "));
  Serial.print(c_time);
#endif
#if BENCHMARK == 1
  unsigned long starttime = micros();
#endif
  uint16_t i;

  digitalWriteFast(OE, HIGH);
  digitalWriteFast(LAT, HIGH);
  __asm__("nop\n\t");
  __asm__("nop\n\t");

  for (i = 0; i < WIDTH ; i++) {
    DATAPORT = buffer[c_time][y * WIDTH + i];
    digitalWriteFast(CLK, HIGH);
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    digitalWriteFast(CLK, LOW);
  }
  digitalWriteFast(A, (y & 0x1) > 0 ? HIGH : LOW);
  digitalWriteFast(B, (y & 0x2) > 0 ? HIGH : LOW);
  digitalWriteFast(C, (y & 0x4) > 0 ? HIGH : LOW);
  digitalWriteFast(D, (y & 0x8) > 0 ? HIGH : LOW);
  __asm__("nop\n\t");
  __asm__("nop\n\t");
  digitalWriteFast(OE, LOW);
  digitalWriteFast(LAT, LOW);

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
  is_drawing = 0;
}


volatile uint8_t row = 0;
void bcmtimer() {
  if (is_drawing > 0) {
    return;
  }

  // g_tock: decreased on each interrupt
  // g_tick: bcm duration doubled each time g_tock is zero

  g_tock--;
  if (g_tock <= 0) {
    row++;
    if (row >= HEIGHT / 2) {
      row = 0;
      g_tick++;
      if (g_tick > BCM_RESOLUTION - 1) {
        g_tick = 0;
      }
    }
    g_tock = 1 << g_tick;
    updatePanel(g_tick, row);
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

uint8_t image_drawImage = 0;
void drawImage() {
  image_drawImage++;
  if (image_drawImage > 1) {
    image_drawImage = 0;
  }

  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++)
    {
      if (image_drawImage == 0) {
        drawPixel(x, y, pgm_read_word_near(color_image + 2 * (y * WIDTH + x)));
      } else {
        drawPixel(x, y, pgm_read_word_near(gimp_image + 2 * (y * WIDTH + x)));
      }
    }
  }
}

void debugBuffer() {
  for (uint8_t c_time = 0; c_time < BCM_RESOLUTION; c_time++) {
    Serial.println(buffer[c_time][3 + 16 * WIDTH], BIN);
  }
}

