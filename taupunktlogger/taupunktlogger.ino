/**
 * This project was introduced in the German Make: magazine 1/22:
 * https://www.heise.de/select/make/2022/1/2135511212557842576
 * 
 */

#include "comm.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  setupWifi();
}

void loop() {
  // put your main code here, to run repeatedly:

}
