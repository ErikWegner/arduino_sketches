IntervalTimer myTimer;

volatile uint8_t g_tick = 1;
volatile uint8_t g_tock = 1;

uint8_t lightpos = 0;

void benchmark();
void bcmtimer();
void updatePanel();
void setupPanelPins();

void setup() {
  // put your setup code here, to run once:
  delay(750);
  setupPanelPins();
  myTimer.begin(bcmtimer, 160); // 1/1.000.000 seconds
  //drawPixel(1, 1, 65535);
  drawPixel555(0, 0, 0, 0, 31);
  //drawPixel555(4, 4, 0, 31, 31);
  //drawPixel555(6, 6, 31, 0, 31);
  //drawPixel555(8, 15, 31, 31, 0);
  //drawPixel555(10, 16, 1, 1, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
  benchmark();
  lightpos = lightpos + 1;
  if (lightpos > 7) {
    lightpos = 0;
  }
  //setupBuffer(7 - lightpos);
}

