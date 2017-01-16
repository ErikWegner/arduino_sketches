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
}

void updatePanel() {
  Serial.print("Updating panel...");
  digitalWriteFast(OE, HIGH); // Re-enable output
  digitalWriteFast(LAT, LOW); // Latch down

  __asm__("nop\n\t");
  __asm__("nop\n\t");

  uint16_t  i;
  for (i = 0; i < WIDTH ; i++) {
    if (i == 0) {
      DATAPORT = B11000000;
    } else {
      DATAPORT = B00100000;
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
  Serial.println(" done");
}

