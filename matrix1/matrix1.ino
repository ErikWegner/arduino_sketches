/**
 * Arduino IDE 1.8.2
 * SmartMatrix3 3.0.1
 * SdFat 2016.7.24
 * SPI 1.0
 */

#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32


prog_char ERROR_SD_Init[] PROGMEM = "01 SD fail";

void error(const char text[]) {
  panel_showError(text);
}

void setup() {
  panel_setup();
  panel_showBootText();
  if (sd_init() < 0) {
    error(ERROR_SD_Init);
  } else {
    loadBootImageFromSd();
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

uint8_t imagedata[MATRIX_WIDTH * MATRIX_HEIGHT * 4];

void loadBootImageFromSd() {
  if (sd_loadBootImage((uint8_t*)&imagedata, MATRIX_WIDTH * MATRIX_HEIGHT * 4) == 0) {
    panel_showImage((uint8_t*)&imagedata, 4);
  }
}

