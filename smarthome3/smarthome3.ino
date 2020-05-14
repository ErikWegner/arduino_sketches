/**
 * 
 *  This project was supposed to run on a ESP8266, but it did
 *  not work  out as expected. The current  implementation is
 *  in smarthome2 running on a ESP32.
 * 
 */

// pip install --user esptool
#include "Ticker.h"

#include "pinlayout.h"
#include "motorsteuerung.h"
#include "comm.h"

volatile bool readButtons = false;
volatile bool ledState = false;
volatile bool motorTick = false;

Ticker timer;

Motorsteuerung motorLinks(MOTORLINKSPOWER, MOTORLINKSRICHTUNG);
Motorsteuerung motorRechts(MOTORRECHTSPOWER, MOTORRECHTSRICHTUNG);

void IRAM_ATTR onTimer() {
  ledState = !ledState;
  motorTick = true;
  digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
}

void setupTimer() {
  timer.attach_ms(1000, onTimer);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Booting"));
  setupGPIO();
  delay(500);
  Serial.println(F("Starting timer"));
  setupTimer();
  setupWifi();
}

void loop() {
  ensureWifiAndMqttTask();
  
  if (mqttCmdRechts != nullptr) {
    motorRechts.setCommand(*mqttCmdRechts);
    mqttCmdRechts = nullptr;
    motorTick = true;
  }
  if (mqttCmdLinks != nullptr) {
    motorLinks.setCommand(*mqttCmdLinks);
    mqttCmdLinks = nullptr;
    motorTick = true;
  }

  checkButtons(&motorLinks, &motorRechts);
  if (motorTick) {
    motorLinks.tick();
    motorRechts.tick();
    motorTick = false;
    bool publishLeft = motorLinks.publishPosition();
    bool publishRight = motorRechts.publishPosition();
    if (publishLeft || publishRight) {
      positionL = motorLinks.estimatedPosition();
      positionR = motorRechts.estimatedPosition();
      publishPositionL = publishLeft;
      publishPositionR = publishRight;
    }
  }
}
