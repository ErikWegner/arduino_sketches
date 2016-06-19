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
  B00111100
};

#define BUFFERSIZE 80
char buffer[BUFFERSIZE+1];
String parserString;

void setup() {
  Serial.begin(115200);
  Serial.println(F("Intro"));

  // put your setup code here, to run once:
  matrix.begin(0x70);  // pass in the address

  matrix.clear();
  matrix.setRotation(3);

  intro();

  Serial.println(F("Ready"));
  Serial.println(F("Waiting for commands. Terminate with ';'."));
}

void loop() {
  // put your main code here, to run repeatedly:
  processSerial();
}

void processSerial() {
  if (Serial.available()) {
    byte len = Serial.readBytesUntil(';', buffer, BUFFERSIZE);
    if (len > 0) {
      buffer[len] = '\0';
      parserString = String(buffer);
      if (parserString.startsWith("M")) {
        processMatrix(parserString.substring(1));
      }
    }
  } 
  else {
    delay(2);
  }
}

void processMatrix(String s) {
  if (s.equalsIgnoreCase(F("CLEAR"))) {
    matrix.clear();
    matrix.writeDisplay();
    return;
  }

  if (s.equalsIgnoreCase(F("SHOW"))) {
    matrix.writeDisplay();
  }

  if (s.substring(0, 4).equalsIgnoreCase(F("DATA"))) {
    byte lastSep = 4, nextSep, line = 0;
    while (line < 8) {
      nextSep = s.indexOf(",", lastSep);
      if (nextSep == -1) nextSep = s.length();
      matrix.displaybuffer[line] = s.substring(lastSep, nextSep).toInt();
      lastSep = nextSep + 1;
      line++;
    }
    matrix.writeDisplay();
  }
}

void intro() {
  matrix.drawLine(0, 0, 1, 1, LED_GREEN);
  matrix.drawLine(3, 0, 4, 1, LED_GREEN);
  matrix.drawLine(6, 0, 7, 1, LED_GREEN);
  matrix.writeDisplay();
  delay(500);

  matrix.drawLine(2, 7, 5, 7, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(2, 3, 5, 3, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(3, 5, 4, 5, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(1, 4, 1, 6, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(5, 4, 6, 4, LED_GREEN);
  matrix.drawLine(6, 6, 7, 6, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  for (byte b = 0; b < 9; b++) {
    matrix.drawBitmap(0, 0, e8_bmp, 8, 8, 1 + b % 3);
    matrix.writeDisplay();
    delay(250);
  }

  matrix.clear();
}


