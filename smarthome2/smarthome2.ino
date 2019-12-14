// pip install --user esptool

#include "pinlayout.h"
#include "motorsteuerung.h"
#include "comm.h"

volatile bool readButtons = false;
volatile bool ledState = false;
volatile bool motorTick = false;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

Motorsteuerung motorLinks(MOTORLINKSPOWER, MOTORLINKSRICHTUNG);
Motorsteuerung motorRechts(MOTORRECHTSPOWER, MOTORRECHTSRICHTUNG);

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  ledState = !ledState;
  motorTick = true;
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
  setupWifi();
}

void loop() {
  checkButtons(&motorLinks, &motorRechts);
  if (motorTick) {
    motorLinks.tick();
    motorRechts.tick();
    motorTick = false;
    ensureWifiAndMqtt();
  }
}
