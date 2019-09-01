/**
   Arduino IDE 1.8.9 https://www.arduino.cc/en/Main/Software
   Teensyduino 1.47 https://www.pjrc.com/teensy/td_download.html
   SmartMatrix3 3.2.0 https://github.com/pixelmatix/SmartMatrix
   SdFat 2016.7.24 https://github.com/greiman/SdFat-beta/tree/master/SdFat
   FlexiTimer2 1.1.0 https://github.com/PaulStoffregen/FlexiTimer2
   SPI 1.0
*/

#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32
#define DEBUG 1

#include <SmartMatrix3.h>

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = MATRIX_WIDTH;   // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = MATRIX_HEIGHT; // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 2;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);


prog_char ERROR_SD_Init[] PROGMEM = "01 SD fail";

void error(const char text[]) {
  panel_showError(text);
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));

  panel_setup();
  panel_showBootText();
  if (sd_init() < 0) {
    error(ERROR_SD_Init);
  }

#if DEBUG == 1
  Serial.println("Setup");
#endif

  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
  scriptprocessor_executeNextStatement();
#if DEBUG == 1
  Serial.println("loop");
#endif
//delay(20);
//jump_and_run_tick();
}

uint8_t volatile imagedata[MATRIX_WIDTH * MATRIX_HEIGHT * 4];

void loadAndShowImageFromSd(String filename) {
#if DEBUG == 1
  Serial.print("loadAndShowImageFromSd(");
  Serial.print(filename);
  Serial.println(")");
#endif

  if (sd_loadImage((uint8_t*)&imagedata, MATRIX_WIDTH * MATRIX_HEIGHT * 4, filename) == 0) {
    panel_showImage((uint8_t*)&imagedata, 4);
  }
}
