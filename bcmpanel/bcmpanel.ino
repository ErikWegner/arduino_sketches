#define PANELWIDTH 64
#define PANELHEIGHT 32
#define SERIALBUFFERSIZE (8 + PANELWIDTH*32*3/4) // width * height * 3  bytes per pixel / 4 colors per byte + command
char serialbuffer[SERIALBUFFERSIZE + 1];
String parserString;

#define MODE_NONE 0
#define MODE_IDLE 1
#define MODE_WAVER 4

uint8_t displayMode = MODE_NONE;
unsigned long lasttime; /* Keep time for waver */

void benchmark();
void bcmtimer();
void updatePanel();
void setupPanelPins();
void debugBuffer();
void clearBuffer();
void step_idle();
void frdecode(uint8_t* in_data, uint8_t* out_data);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(750);
  setupPanelPins();
  drawImage(); swapBuffers(false);
  lasttime = millis();
  Serial.println("Start");
}

void loop() {
  unsigned long time = millis();
  processSerial();
  if ((displayMode & MODE_WAVER) > 0) {
    if (time - lasttime > 15) {
      lasttime = millis();
      waverStep();
      clearBuffer();
      waverDraw();
      swapBuffers(false);
    }
  }
  if (displayMode == MODE_IDLE) {
    if (time - lasttime > 33) {
      lasttime = millis();
      drawImage();
      step_idle();
      swapBuffers(false);
    }
  }
  //benchmark();
}

void processSerial() {
  if (Serial.available())
  {
    byte len = Serial.readBytesUntil(0, serialbuffer, SERIALBUFFERSIZE);
    if (len > 0) {
      serialbuffer[len] = '\0';
      parserString = String(serialbuffer);
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
  if (s.substring(0, 2).equalsIgnoreCase(F("FR"))) {
    processFastRow(s.substring(2));
  }
  if (s.substring(0, 3).equalsIgnoreCase(F("SWP"))) {
    swapBuffers(s.substring(3, 4) == "1");
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
}

void processFastRow(String s) {
  uint8_t position = s.indexOf(":");
  uint8_t cmdRow = s.substring(0, position).toInt();
  s = s.substring(position + 1);

  uint16_t l = s.length();
  if (l != (PANELWIDTH + PANELWIDTH / 2) + 1) {
    Serial.println(F("Fast row length mismatch"));
    return;
  }

  uint8_t packedpixels[(PANELWIDTH + PANELWIDTH / 2) + 2];

  size_t decoded_size = frdecode(s, packedpixels, (PANELWIDTH + PANELWIDTH / 2) + 2);
#if DEBUG == 1
  Serial.print(F("decoded_size "));
  Serial.println(decoded_size, DEC);
#endif

  // 2 pixels are packed 4/4/4 to 3 bytes
  for (uint8_t lc = 0; lc < PANELWIDTH / 2; lc++) {
    // left pixel
    drawPixel444(2 * lc, cmdRow,
                 packedpixels[3 * lc] >> 4,
                 packedpixels[3 * lc] & 0xF,
                 packedpixels[3 * lc + 1] >> 4
                );
    // right pixel
    drawPixel444(2 * lc + 1, cmdRow,
                 packedpixels[3 * lc + 1] & 0xF,
                 packedpixels[3 * lc + 2] >> 4,
                 packedpixels[3 * lc + 2] & 0xF
                );

  }
}

void switchMode(String s) {
  if (s.equalsIgnoreCase(F("IDLE"))) {
    displayMode = MODE_IDLE;
    return;
  }
  if (s.equalsIgnoreCase(F("NONE"))) {
    displayMode = MODE_NONE;
    return;
  }
  //  if (s.equalsIgnoreCase(F("FIRE"))) {
  //    displayMode = displayMode ^ MODE_FIRE;
  //    return;
  //  }
  //  if (s.equalsIgnoreCase(F("STAR"))) {
  //    displayMode = displayMode ^ MODE_STARFIELD;
  //    return;
  //  }
  if (s.equalsIgnoreCase(F("WAVE"))) {
    displayMode = displayMode ^ MODE_WAVER;
    return;
  }

  if (s.equalsIgnoreCase(F("WAVE1"))) {
    displayMode |= MODE_WAVER;
    return;
  }
}

