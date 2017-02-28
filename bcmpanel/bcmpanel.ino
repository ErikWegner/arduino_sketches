IntervalTimer myTimer;
#define PANELWIDTH 64
#define BUFFERSIZE (8 + PANELWIDTH*32*3/4) // width * height * 3  bytes per pixel / 4 colors per byte + command
char serialbuffer[BUFFERSIZE + 1];
String parserString;


void benchmark();
void bcmtimer();
void updatePanel();
void setupPanelPins();
void debugBuffer();
void frdecode(uint8_t* in_data, uint8_t* out_data);

void setup() {
  // put your setup code here, to run once:
  delay(750);
  setupPanelPins();
  //myTimer.begin(bcmtimer, 100000); // 1/1.000.000 seconds
  myTimer.begin(bcmtimer, 27); // 1/1.000.000 seconds
}

void loop() {
  processSerial();
}

void processSerial() {
  if (Serial.available())
  {
    byte len = Serial.readBytesUntil(0, serialbuffer, BUFFERSIZE);
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
