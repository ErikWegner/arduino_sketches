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
 * Libraries:
 * https://github.com/duinoWitchery/hd44780 version 1.3.2
 * 
 */

#include "comm.h"
#include "sensors.h"

volatile bool lueftung = true;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  setupDisplay();
  setupWifi();
}

void loop() {
  // put your main code here, to run repeatedly:
  updateDisplay();
}
