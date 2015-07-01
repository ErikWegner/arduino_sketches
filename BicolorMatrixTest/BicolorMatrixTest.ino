#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

void setup() {
  // put your setup code here, to run once:
  Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();
  matrix.begin(0x70);  // pass in the address
  
  matrix.clear();
  matrix.setRotation(3);
  matrix.drawLine(0, 0, 7, 0, LED_RED);
  matrix.drawLine(0, 3, 7, 3, LED_YELLOW);
  matrix.drawLine(0, 6, 7, 6, LED_GREEN);
  
  matrix.writeDisplay();
}

void loop() {
}
