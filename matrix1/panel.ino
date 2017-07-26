#include <SmartMatrix3.h>

#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 64;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint8_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);
const uint8_t kIndexedLayerOptions = (SM_INDEXED_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);

const int defaultBrightness = 100*(255/100);    // full brightness
//const int defaultBrightness = 15*(255/100);    // dim: 15% brightness

// Teensy 3.0 has the LED on pin 13
const int ledPin = 13;

void panel_setup() {
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);

  matrix.addLayer(&backgroundLayer);
  matrix.begin();

  matrix.setBrightness(defaultBrightness);
}

void panel_showBootimg() {
  prog_char btext[] PROGMEM = "Booting";
  backgroundLayer.fillScreen({0, 0, 0});
  backgroundLayer.setFont(font5x7);
  backgroundLayer.drawString(0, 0, {0, 0xff, 0}, btext);
  backgroundLayer.swapBuffers();
}

void panel_showError(const char text[]) {
  backgroundLayer.fillScreen({0, 0, 0});
  backgroundLayer.setFont(font5x7);
  backgroundLayer.drawString(0, 0, {0xff, 0, 0}, text);
  backgroundLayer.swapBuffers();
  
}

