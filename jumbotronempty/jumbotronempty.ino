#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library

RGBmatrixPanel matrix(true, 64);

void setup() {
  // put your setup code here, to run once:
  matrix.begin();
  matrix.drawPixel(0, 0, matrix.Color333(7, 7, 7)); 
}

void loop() {
  // put your main code here, to run repeatedly:

}
