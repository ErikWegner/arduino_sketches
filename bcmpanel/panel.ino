#define DEBUG 0

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

uint8_t buffer[4][8]; // first dimension: time, second dimension pixel

uint8_t intensities[8] = {1, 1, 1, 2, 3, 4, 8, 15};

void setupBuffer(uint8_t highpos) {
  for (uint8_t c_pixel = 0; c_pixel < 8; c_pixel++) {
    uint8_t intensity = intensities[(highpos + c_pixel) % 8];

    for (uint8_t c_time = 0; c_time < 4; c_time++) {
      // is bit set for this pixel's brightness
      if ((intensity & (1 << c_time)) > 0) {
        buffer[c_time][c_pixel] = B00100000;
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

  digitalWriteFast(OE, HIGH); // Re-enable output
  digitalWriteFast(LAT, LOW); // Latch down

  __asm__("nop\n\t");
  __asm__("nop\n\t");

  uint16_t  i;
  for (i = 0; i < WIDTH ; i++) {
    if (i < 8) {
      DATAPORT = buffer[c_time][i];
    } else {
      DATAPORT = B00000000;
    }
    // Clock lo
    digitalWriteFast(CLK, HIGH);
    __asm__("nop\n\t");
    __asm__("nop\n\t");
    // Clock high
    digitalWriteFast(CLK, LOW);
  }
  __asm__("nop\n\t");
  __asm__("nop\n\t");

  digitalWriteFast(OE, LOW); // Disable LED output during row/plane switchover
  digitalWriteFast(LAT, HIGH); // Latch data loaded during *prior* interrupt
#if DEBUG == 1
  Serial.println(F(" done"));
#endif
}

