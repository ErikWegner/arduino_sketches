IntervalTimer myTimer;

volatile uint8_t g_tick = 1;
volatile uint16_t g_tock = 1;

uint8_t lightpos = 0;

void benchmark();
void bcmtimer();
void updatePanel();
void setupPanelPins();
void debugBuffer();

void setup() {
  // put your setup code here, to run once:
  delay(750);
  setupPanelPins();
  //myTimer.begin(bcmtimer, 100000); // 1/1.000.000 seconds
  myTimer.begin(bcmtimer, 25); // 1/1.000.000 seconds
  drawPixel(1, 1, 65535);
  drawPixel555(0, 1, 0, 0, 255);
  drawPixel555(0, 30, 255, 255, 255);
  drawPixel555(4, 4, 0, 255, 255);
  drawPixel555(6, 6, 255, 0, 255);
  drawPixel555(8, 15, 255, 255, 0);
  drawPixel555(10, 16, 1, 1, 0);
  drawImage();
  //debugBuffer();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(500);
  benchmark();
  lightpos = lightpos + 1;
  if (lightpos > 7) {
    lightpos = 0;
    drawImage();
  }
  //setupBuffer(7 - lightpos);
}

