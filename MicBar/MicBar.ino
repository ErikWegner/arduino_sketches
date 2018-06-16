#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_24bargraph bar = Adafruit_24bargraph();

const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
double val;
uint8_t i;

void setup() {
  // put your setup code here, to run once:
  bar.begin(0x71);
  analogReference(EXTERNAL);
  pinMode(A0, INPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned long startMillis = millis(); // Start of sample window
  signed int peakToPeak = 0;   // peak-to-peak level

  unsigned int signalMax = 0;
  unsigned int signalMin = 1024;
  
  bar.clear();
  
  // collect data for 50 mS
  while (millis() - startMillis < sampleWindow)
  {
    sample = analogRead(A0);
    if (sample < 1024)  // toss out spurious readings
    {
      if (sample > signalMax)
      {
        signalMax = sample;  // save just the max levels
      }
      else if (sample < signalMin)
      {
        signalMin = sample;  // save just the min levels
      }
    }
  }
  peakToPeak = signalMax - signalMin - 0; // max - min = peak-peak amplitude
  if (peakToPeak < 0) {
    peakToPeak = 0;
  }
  Serial.print(peakToPeak);
  val = (peakToPeak / 46.0);
  Serial.print(' ');
  Serial.print(val);
  Serial.print('\n');
  for (i = 0; i <= val && i < 24; i++) {
    bar.setBar(i, LED_GREEN);
  }
  bar.writeDisplay();
}
