#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

static const uint8_t PROGMEM
  e8_bmp[] =
  { B10010010,
    B01001001,
    B00000000,
    B00111100,
    B01000110,
    B01011000,
    B01000011,
    B00111100 };


void setup() {
  // put your setup code here, to run once:
  matrix.begin(0x70);  // pass in the address
  
  matrix.clear();
  matrix.setRotation(3);

  intro();
}

void loop() {
  // put your main code here, to run repeatedly:

}


void intro() {
  matrix.drawLine(0,0,1,1, LED_GREEN);
  matrix.drawLine(3,0,4,1, LED_GREEN);
  matrix.drawLine(6,0,7,1, LED_GREEN);
  matrix.writeDisplay();
  delay(500);
  
  matrix.drawLine(2,7,5,7, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(2,3,5,3, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(3,5,4,5, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(1,4,1,6, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(5,4,6,4, LED_GREEN);
  matrix.drawLine(6,6,7,6, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  for (byte b=0; b < 9; b++) {
    matrix.drawBitmap(0,0, e8_bmp, 8, 8, 1 + b % 3);
    matrix.writeDisplay();
    delay(250);
  }
}
