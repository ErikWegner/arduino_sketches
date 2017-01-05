#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library

#define DEBUG_off 1

#define PANELWIDTH 64
#define PANELHEIGHT 32
RGBmatrixPanel matrix(true, PANELWIDTH);

#define BUFFERSIZE (8 + PANELWIDTH*32*3/4) // width * height * 3  bytes per pixel / 4 colors per byte + command
char buffer[BUFFERSIZE + 1];
String parserString;

#define MODE_NONE 0
#define MODE_FIRE 1
#define MODE_STARFIELD 2

#define SINUS_OFF 0
#define SINUS_RAISING 1
#define SINUS_RUNNING 2
#define SINUS_FALLING 3

byte sinusStatus = SINUS_OFF;
byte sinusDelta[PANELWIDTH];
byte sinusAngle = 0; // rotation angle
byte sinusAmpl = 0;  // amplitude

byte displayMode = MODE_NONE;
void updateFire();
void setupStarfield();
void starfield();

void setup()
{
  displayMode = MODE_STARFIELD;
  setupStarfield();
  randomSeed(analogRead(0));

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
  if ((displayMode & MODE_FIRE) > 0) {
    updateFire();
  }
  if ((displayMode & MODE_STARFIELD) > 0) {
    starfield();
  }
  if (sinusStatus != SINUS_OFF) {
    processSinus();
  }
}

void processSinus() {
  if (sinusStatus == SINUS_RAISING) {
    sinusAmpl++;
    if (sinusAmpl >= 16) {
      sinusAmpl = 16;
      sinusStatus = SINUS_RUNNING;
    }
  }
  if (sinusStatus == SINUS_FALLING) {
    if (sinusAmpl >= 1) {
      sinusAmpl--;
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
    return;
  }

  if (s.substring(0, 3).equalsIgnoreCase(F("SWP"))) {
    matrix.swapBuffers(s.substring(3, 4) == "1");
    return;
  }

  if (s.substring(0, 5).equalsIgnoreCase(F("SINUS"))) {
    processSinus(s.substring(5));
    return;
  }
}

void processSinus(String s) {
  if (s.charAt(0) == 1) {
    if (sinusStatus == SINUS_OFF) {
      sinusStatus = SINUS_RAISING;
    } else {
      sinusStatus = SINUS_FALLING;
    }
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

