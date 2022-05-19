/**
 * This project was introduced in the German Make: magazine 1/22:
 * https://www.heise.de/select/make/2022/1/2135511212557842576
 * 
 * Connections:
 * 
 * ESP32 to ...
 * G21: LCD/SDA
 * G22: LCD/SCL
 * 
 * G17: DHT22 inside (sensor 1)
 * G16: DHT22 outside (sensor 2)
 * 
 * Libraries:
 * https://github.com/duinoWitchery/hd44780 version 1.3.2
 * 
 */

#include "comm.h"
#include "sensors.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  setupDisplay();
  setupSensors();
  setupWifi();
}

void loop() {
  updateDisplay();
  readSensors();
  delay(100);
}
