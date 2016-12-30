#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library

#define DEBUG_off 1

RGBmatrixPanel matrix(true, 64);
#define BUFFERSIZE (8 + 64*32*3/4) // width * height * 3  bytes per pixel / 4 colors per byte + command
char buffer[BUFFERSIZE + 1];
String parserString;

void setup()
{
  matrix.begin();
  matrix.setRotation(0);

  Serial.begin(9600);
  delay(1000);
  Serial.println(F("Ready"));
  Serial.println(F("Waiting for commands. Terminate with ';'."));
}

void loop()
{
  processSerial();
  delay(1);
}

void processSerial() {
  if (Serial.available())
  {
    byte len = Serial.readBytesUntil(';', buffer, BUFFERSIZE);
    if (len > 0) {
      buffer[len] = '\0';
      parserString = String(buffer);
      if (parserString.startsWith("M")) {
        #if defined (DEBUG)
        Serial.println(F("M"));
        #endif
        processMatrix(parserString.substring(1));
      }
    }
  }
}

void processMatrix(String s) {
  if (s.substring(0, 3).equalsIgnoreCase(F("ROT"))) {
    byte rotation = byte(s.charAt(3));
    #if defined (DEBUG)
    Serial.println(rotation);
    #endif
    if (rotation >= 48 && rotation <= 51) {
      matrix.setRotation(rotation - 48);
    }
    return;
  }

  if (s.substring(0, 3).equalsIgnoreCase(F("PXL"))) {
    processPixelCommand(s.substring(3));
  }

  if (s.substring(0,3).equalsIgnoreCase(F("SWP"))) {
    matrix.swapBuffers(s.substring(3, 4) == "1");
  }
}

/** s = x,y,r,g,b */
void processPixelCommand(String s) {
  uint8_t position, lastposition;
  position = s.indexOf(",");
  byte x = s.substring(0, position).toInt();
  lastposition = 1 + position;
  position = s.indexOf(",", lastposition);
  byte y = s.substring(lastposition, position).toInt();
  lastposition = 1 + position;
  position = s.indexOf(",", lastposition);
  byte r = s.substring(lastposition, position).toInt();
  lastposition = 1 + position;
  position = s.indexOf(",", lastposition);
  byte g = s.substring(lastposition, position).toInt();
  lastposition = 1 + position;
  position = s.indexOf(",", lastposition);
  byte b = s.substring(lastposition, position).toInt();
  matrix.drawPixel(x, y, matrix.Color888(r, g, b));  
}

