#define DEBUG 0
#define BENCHMARK 1

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
#define NROWS (HEIGHT / 2)

IntervalTimer drawTimer;

volatile uint8_t row = 0, BcmBrightnessBit = BCM_RESOLUTION - 1, duration = 0;

#define BUFFERSIZE WIDTH * HEIGHT / 2
uint8_t buffer[2][BCM_RESOLUTION][BUFFERSIZE]; // first dimension: time, second dimension pixel
volatile uint8_t backbuffer = 1;
volatile boolean swapflag = false;

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

/* (x / 5.1) ^ 1.5 https://rechneronline.de/function-graphs/
  input values: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
  output values: 0-15
*/
uint8_t rgbgamma[32] = {0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10, 10, 11, 12, 12, 13, 14, 14, 15};

/*
  n = document.createElement("script")
  n.setAttribute("src", "https://code.jquery.com/jquery-3.1.1.min.js")
  document.head.appendChild(n)

  a = []; jQuery("tr").eq(1).find("td").each(function() {a.push(Math.round(parseFloat($(this).text().trim())));}); console.log(a.join(","));
*/

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

    buffer[backbuffer][c_time][c_pixel] =
      (buffer[backbuffer][c_time][c_pixel] & ~addressmask) // set all relevant bits to zero
      | colorvalue; // add new color bits;
  }
}

void drawPixel444(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
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

    buffer[backbuffer][c_time][c_pixel] =
      (buffer[backbuffer][c_time][c_pixel] & ~addressmask) // set all relevant bits to zero
      | colorvalue; // add new color bits;
  }
}

void swapBuffers(boolean copy) {
  swapflag = true;                    // Set flag here, then...
  while (swapflag == true) delayMicroseconds(75); // wait for interrupt to clear it
  if (copy == true) {
    memcpy(
      /* destination */
      buffer[backbuffer],
      /* source */
      buffer[1 - backbuffer],
      /* size */
      BUFFERSIZE);
  }
}

void setupBuffer(uint8_t highpos) {
  for (uint8_t c_pixel = 0; c_pixel < 8; c_pixel++) {
    uint8_t intensity = intensities[(highpos + c_pixel) % 8];

    for (uint8_t c_time = 0; c_time < BCM_RESOLUTION; c_time++) {
      // is bit set for this pixel's brightness
      if ((intensity & (1 << c_time)) > 0) {
        buffer[backbuffer][c_time][c_pixel] = B00101000;
      } else {
        buffer[backbuffer][c_time][c_pixel] = 0;
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

  drawTimer.begin(updatePanel, 25);
}

/*
   see https://cdn-learn.adafruit.com/downloads/pdf/connecting-a-16x32-rgb-led-matrix-panel-to-a-raspberry-pi.pdf
*/
void updatePanel() {
#if BENCHMARK == 1
  unsigned long starttime = micros();
#endif

  uint16_t i;

  
//  // Early leave if panel needs no refresh
//  if (duration > 0) {
//    duration = duration - 1;
//    return;
//  }
  

  digitalWriteFast(OE, HIGH); // Disable output (all leds go off)
  digitalWriteFast(LAT, HIGH); // Sent data to output pins

  drawTimer.end();
  duration = (1 << BcmBrightnessBit) + 1; // How often skip this routine?

  #define CALLOVERHEAD 0   // Actual value measured = 56
  #define LOOPTIME     23  // Actual value measured = 188
  uint16_t t = (NROWS > 8) ? LOOPTIME : (LOOPTIME * 2);
  uint16_t timerDuration = ((t + CALLOVERHEAD * 2) << BcmBrightnessBit) - CALLOVERHEAD;

  drawTimer.begin(updatePanel, timerDuration);
  
  BcmBrightnessBit += 1;

  if (BcmBrightnessBit >= BCM_RESOLUTION) {     // Advance plane counter.  Maxed out?
    BcmBrightnessBit = 0;                  // Yes, reset to plane 0, and
    if (++row >= NROWS) {       // advance row counter.  Maxed out?
      row     = 0;              // Yes, reset row counter, then...
      if (swapflag == true) {   // Swap front/back buffers if requested
        backbuffer = 1 - backbuffer;
        swapflag  = false;
      }
    }
  } else if (BcmBrightnessBit == 1) {
    digitalWriteFast(A, row & 0x1);
    digitalWriteFast(B, row & 0x2);
    digitalWriteFast(C, row & 0x4);
    digitalWriteFast(D, row & 0x8);
  }

  digitalWriteFast(OE, LOW); // Enable output
  digitalWriteFast(LAT, LOW); // Prepare shift registers to receive data

  uint8_t (* buffptr);
  uint16_t pixelbase = row * WIDTH;
  buffptr = &(buffer[1 - backbuffer][BcmBrightnessBit][pixelbase]);
  uint8_t a = buffptr[0];
  for (i = 0; i < WIDTH ; i++) {
    // set the data pins
    digitalWriteFast(2, a & (1 << 2)); // R0
    digitalWriteFast(3, a & (1 << 3)); // G0
    digitalWriteFast(4, a & (1 << 4)); // B0
    digitalWriteFast(5, a & (1 << 5)); // R1
    digitalWriteFast(6, a & (1 << 6)); // G1
    digitalWriteFast(7, a & (1 << 7)); // B1

    digitalWriteFast(CLK, HIGH);
    // load data in between clock toggle to better fit the 25 MHz
    a = buffptr[i+1];
    digitalWriteFast(CLK, LOW);
  }

#if BENCHMARK == 1
  benchmark_results[benchmark_sampleindex] = micros() - starttime;
  benchmark_sampleindex++;
  if (benchmark_sampleindex >= BENCHMARKSAMPLES) {
    benchmark_sampleindex = 0;
    benchmark_output = 1;
  }
#endif
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
  /*  image_drawImage++;
    if (image_drawImage > 3) {
      image_drawImage = 0;
    }
  */
  uint8_t *imgptr = (uint8_t *)night_image;
  /*  switch (image_drawImage) {
      case 1:
        imgptr = (uint8_t *)gimp_image;
        break;
      case 2:
        imgptr = (uint8_t *)night_image;
        break;
      case 3:
        imgptr = (uint8_t *)emblem_image;
        break;
    }
  */
  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++)
    {
      drawPixel(x, y, pgm_read_word_near(imgptr + 2 * (y * WIDTH + x)));
    }
  }
}

void debugBuffer() {
  for (uint8_t c_time = 0; c_time < BCM_RESOLUTION; c_time++) {
    Serial.println(buffer[1 - backbuffer][c_time][3 + 16 * WIDTH], BIN);
  }
}

void clearBuffer() {
  memset(buffer[backbuffer], 0, BUFFERSIZE * BCM_RESOLUTION);
}


/* counter from 0 to 14 for each animation frame */
uint8_t pointstep = 0;
/* number of pixels to draw within animation frame */
uint8_t numPoints = 0;
/* index within `points´ array pointing at first pixel of current frame */
uint8_t pointIndex = 0;

void step_idle() {
  /* counter for pixels within current frame */
  uint8_t pointLoop;
  /* pointer to pixel data for current pixel */
  uint8_t* pointsStart;

  //  /* erase points drawn in previous loop */
  //  for (pointLoop = 0; pointLoop < numPoints; pointLoop++) {
  //    /* set pointer to pixel data */
  //    pointsStart = (uint8_t *)points[pointIndex + pointLoop];
  //    /* load coordinates */
  //    x = pointsStart[0];
  //    y = pointsStart[1];
  //    /* load color from poweron_image at (x,y), draw it at (x,y) */
  //    drawPixel(x, y, pgm_read_word_near(poweron_image + 2 * (y * WIDTH + x)));
  //  }

  /* proceed to next animation frame */
  pointstep += 1;

  /* forward index */
  pointIndex += numPoints;

  /* overflow detection */
  if (pointstep >= 45) {
    pointstep = 0;
    pointIndex = 0;
  }

  /* read the number of points of current frame */
  numPoints = pointslengths[pointstep];

  /* draw all points */
  for (pointLoop = 0; pointLoop < numPoints; pointLoop++) {
    /* set pointer to pixel data */
    pointsStart = (uint8_t *)points[pointIndex + pointLoop];

    drawPixel(pointsStart[0], pointsStart[1], (31 - pointsStart[2] / 2) << 11);
  }

}

