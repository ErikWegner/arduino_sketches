IntervalTimer myTimer;

volatile uint8_t g_tick = 1;
volatile uint8_t g_tock = 1;

void bcmtimer();
void updatePanel();
void setupPanelPins();

void setup() {
  // put your setup code here, to run once:
  setupPanelPins();
  myTimer.begin(bcmtimer, 500000);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void bcmtimer() {
  g_tock--;
  if (g_tock <= 0) {
    g_tick = (g_tick * 2) & 0xF;
    if (g_tick < 1) {
      g_tick = 1;
    }
    g_tock = g_tick;
    updatePanel(g_tick);
  }
}
