IntervalTimer myTimer;

volatile uint8_t g_tick = 1;
volatile uint8_t g_tock = 1;

void bcmtimer();
void updatePanel();
void setupPanelPins();

void setup() {
  // put your setup code here, to run once:
  delay(750);
  setupPanelPins();
  myTimer.begin(bcmtimer, 100); // 1/1.000.000 seconds
}

void loop() {
  // put your main code here, to run repeatedly:

}

void bcmtimer() {
  g_tock--;
  if (g_tock <= 0) {
    g_tick++;
    if (g_tick > 3) {
      g_tick = 0;
    }
    g_tock = 1 << g_tick;
    updatePanel(g_tick);
  }
}
