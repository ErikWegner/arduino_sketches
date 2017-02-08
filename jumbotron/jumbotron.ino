#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library

#define DEBUG 1

#define PANELWIDTH 64
#define PANELHEIGHT 32
RGBmatrixPanel matrix(true, PANELWIDTH);

#define BUFFERSIZE (8 + PANELWIDTH*32*3/4) // width * height * 3  bytes per pixel / 4 colors per byte + command
char buffer[BUFFERSIZE + 1];
String parserString;

/*
   a buffer for string split results
   important for its size:
   5 bytes: processPixelCommand
   32 bytes: waver data
*/
uint8_t serialdatabuffer[32];

#define MODE_NONE 0
#define MODE_FIRE 1
#define MODE_STARFIELD 2
#define MODE_WAVER 4

uint8_t displayMode = MODE_NONE;
void updateFire();
void setupStarfield();
void starfield();

unsigned long lasttime;

void setup()
{
  //displayMode = MODE_WAVER;
  setupStarfield();
  randomSeed(analogRead(0));

  matrix.begin();
  matrix.setRotation(0);

  Serial.begin(9600);
  delay(1000);
  Serial.println(F("Ready"));
  Serial.println(F("Waiting for commands. Terminate with ';'."));

  lasttime = millis();
}

void loop()
{
  unsigned long time = millis();
  processSerial();
  if ((displayMode & MODE_FIRE) > 0) {
    updateFire();
  }
  if ((displayMode & MODE_STARFIELD) > 0) {
    starfield();
  }
  if ((displayMode & MODE_WAVER) > 0) {
    if (time - lasttime > 333) {
      waverStep();
      matrix.fillScreen(0);
      waverDraw();
    }
  }

}

void processSerial() {
  if (Serial.available())
  {
    byte len = Serial.readBytesUntil(';', buffer, BUFFERSIZE);
    if (len > 0) {
      buffer[len] = '\0';
      parserString = String(buffer);
      if (parserString.startsWith("M")) {
#if DEBUG == 1
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
#if DEBUG == 1
    Serial.println(rotation);
#endif
    if (rotation >= 48 && rotation <= 51) {
      matrix.setRotation(rotation - 48);
    }
    return;
  }

  if (s.substring(0, 3).equalsIgnoreCase(F("PXL"))) {
    processPixelCommand(s.substring(3));
    return;
  }

  if (s.substring(0, 3).equalsIgnoreCase(F("SWP"))) {
    matrix.swapBuffers(s.substring(3, 4) == "1");
    return;
  }

  if (s.substring(0, 4).equalsIgnoreCase(F("MODE"))) {
#if DEBUG == 1
    Serial.println(F("Switching to ") + s.substring(4, 8));
#endif
    switchMode(s.substring(4));
#if DEBUG == 1
    Serial.println(F("Mode is now ") + String(displayMode, 16));
#endif
  }

  if (s.substring(0, 6).equalsIgnoreCase(F("WAVROW"))) {
    waverAddColumn(s.substring(6));
  }

  if (s.substring(0, 4).equalsIgnoreCase(F("SDLS"))) {
#if DEBUG == 1
    Serial.println(F("Accessing SD to list files ") + s.substring(4, 8));
#endif
    sdListDirectory();
  }
}

void switchMode(String s) {
  if (s.equalsIgnoreCase(F("NONE"))) {
    displayMode = MODE_NONE;
    return;
  }
  if (s.equalsIgnoreCase(F("FIRE"))) {
    displayMode = displayMode ^ MODE_FIRE;
    return;
  }
  if (s.equalsIgnoreCase(F("STAR"))) {
    displayMode = displayMode ^ MODE_STARFIELD;
    return;
  }
  if (s.equalsIgnoreCase(F("WAVE"))) {
    displayMode = displayMode ^ MODE_WAVER;
    return;
  }

  if (s.equalsIgnoreCase(F("WAVE1"))) {
    displayMode |= MODE_WAVER;
    return;
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

