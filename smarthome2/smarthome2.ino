#include "pinlayout.h"

volatile bool readButtons = false;
volatile bool ledState = false;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  ledState = !ledState;
  portEXIT_CRITICAL_ISR(&timerMux);
  digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
}

void setupTimer() {
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Booting"));
  setupGPIO();
  delay(1500);
  Serial.println(F("Starting timer"));
  setupTimer();
}

void loop() {
  checkButtons();
}
