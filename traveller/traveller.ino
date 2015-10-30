#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_24bargraph bar = Adafruit_24bargraph();


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  bar.begin(0x71);  // pass in the address

  for (uint8_t b = 0; b < 24; b++ ) {
    if ((b % 3) == 0)  bar.setBar(b, LED_RED);
    if ((b % 3) == 1)  bar.setBar(b, LED_YELLOW);
    if ((b % 3) == 2)  bar.setBar(b, LED_GREEN);
  }
  bar.writeDisplay();
}

bool ledBlink = HIGH;
byte timer = 0;
byte numLed = 24;

// the loop function runs over and over again forever
void loop() {
  timer = (timer + 1) % 50;
  checkSerial();
  if (timer == 0) {
    ledBlink = !ledBlink;
    digitalWrite(13, ledBlink);
  }
  updateBarGraph(numLed);
  delay(20);
}

void checkSerial() {
  if (Serial.available() == false) {
    return;
  }

  char buffer[1];
  byte len = Serial.readBytes(buffer, 1);
  if (len != 1) {
    return;
  }

  numLed = ((byte)buffer[0]) - 65;
  if (numLed >= 0 && numLed <= 24) {
    updateBarGraph(numLed);
  }
}

void updateBarGraph(byte numLed) {
  for (uint8_t b = 1; b < 25; b++ ) {
    if (b < 15 && numLed > (b - 1)) {
      // 14 Minuten Restanzeige
      if (b < 6) {
        bar.setBar(b-1, LED_GREEN);
      } else {
        bar.setBar(b-1, LED_YELLOW);
      }
      continue;
    } else {
      if (b > 14 && (b - 1) < numLed) {
        bar.setBar(b-1, LED_RED);
      } else {
        bar.setBar(b-1, LED_OFF);
      }
    }
  }
  //bar.setBar(25-(timer % 25), LED_GREEN);
  //blink letzte LED
  if (ledBlink == HIGH) {
    bar.setBar(numLed - 1, LED_OFF);
  }
  bar.writeDisplay();
}
