#include "pinlayout.h"

volatile bool readButtons = false;
volatile bool ledState = false;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  ledState = !ledState;
  portEXIT_CRITICAL_ISR(&timerMux);
  digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
  Serial.print(ledState);
  Serial.println("onTimer");
}

void setupTimer() {
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
}

void IRAM_ATTR handleInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
  readButtons = true;
  portEXIT_CRITICAL_ISR(&mux);
}

void readButtonsHandler() {
  portENTER_CRITICAL(&mux);
  readButtons = false;
  portEXIT_CRITICAL(&mux);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Booting"));
  setupGPIO();
  attachInterrupt(digitalPinToInterrupt(BUTTONLINKSHOCH), readButtonsHandler, CHANGE);
  delay(1500);
  Serial.println(F("Starting timer"));
  setupTimer();
}

void loop() {
  // put your main code here, to run repeatedly:
}
