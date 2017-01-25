/* The source image is in this buffer */
uint16_t waverBuffer[PANELWIDTH][PANELHEIGHT / 2];

/* New image data is written to this column */
uint8_t waverWritePos = 0;

uint8_t waverAmplitude = PANELHEIGHT / 4;
uint8_t waverWavelength = 92;
/* PANELWIDTH euqals one turn, one turn equals 256 */
uint8_t angle_per_column = 256 / waverWavelength;
#define WAVERANGLEINCREASE 254
uint8_t waverAngle = 0;

/* Increate the start angle */
void waverStep() {
  waverAngle = waverAngle + WAVERANGLEINCREASE;
  /* because one turn equals 256 and waverAngle is just one byte of size, just overflow the the variable */
  /*while (waverAngle > 256) {
    waverAngle = waverAngle - 256;
    }*/
}

/* Draw the image, move each column up or down */
void waverDraw() {
  /* Reader is set to writing position and increased before usage */
  uint8_t waverReadPos = waverWritePos;

  for (uint8_t column = 0; column < PANELWIDTH; column++) {
    waverReadPos = waverReadPos + 1;
    while (waverReadPos >= PANELWIDTH) {
      waverReadPos = waverReadPos - PANELWIDTH;
    }
    /* wave uses y-axis, calculate displacement value */

    int8_t relocate_y = PANELHEIGHT / 4 +
                        (sinetab[(byte)(waverAngle + column * angle_per_column)] * waverAmplitude / 128.0);
    /* go through all pixels of the column */
    for (uint8_t y = 0; y < PANELHEIGHT / 2; y++) {
      /* read the pixel value */
      uint16_t pixel = waverBuffer[waverReadPos][y];
      /* draw the pixel if it is not black */
      if (pixel > 0) {
        matrix.drawPixel(column, relocate_y + y, pixel);
      }
    }
  }
  matrix.swapBuffers(true);
}

void waverAddColumn(String s) {
  /* next */
  waverWritePos++;
  if (waverWritePos >= PANELWIDTH) {
    waverWritePos = 0;
  }
  /* write data to buffer */
  if (stringsplit(s) != PANELHEIGHT / 2 * 2) {
#if DEBUG == 1
    Serial.println(F("waverAddColumn mismatch"));
#endif
    return;
  }
  for (uint8_t row = 0; row < PANELHEIGHT / 2; row++) {
    waverBuffer[waverWritePos][row] = serialdatabuffer[row * 2] * 256 + serialdatabuffer[row * 2 + 1];
#if DEBUG == 1
    Serial.print(waverBuffer[waverWritePos][row], HEX);
#endif
  }
#if DEBUG == 1
  Serial.println(F(" written"));
#endif
#if DEBUG == 1
  Serial.println(F("waverAddColumn done"));
#endif
}

