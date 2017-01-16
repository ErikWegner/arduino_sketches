IntervalTimer myTimer;

volatile uint8_t g_tick = 1;
volatile uint8_t g_tock = 1;

void blinkLED();

void setup() {
  // put your setup code here, to run once:
  myTimer.begin(blinkLED, 500000);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void blinkLED() {
  Serial.print(F("tick="));
  Serial.print(g_tick);
  Serial.print(F(" tock="));
  Serial.print(g_tock);

  g_tock--;
  if (g_tock <= 0) {
    Serial.print(F(" switch"));
    g_tick = (g_tick * 2) & 07;
    if (g_tick < 1) {
      g_tick = 1;
    }
    g_tock = g_tick;
  }

  Serial.println();
}
