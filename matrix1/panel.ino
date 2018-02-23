const int defaultBrightness = 100 * (255 / 100); // full brightness
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

void panel_showBootText() {
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

void panel_showImage(uint8_t buf[], uint8_t bytes_per_pixel) {
  rgb24 color;
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      color.red = buf[0];
      color.green = buf[1];
      color.blue = buf[2];
      backgroundLayer.drawPixel(x, y, color);
      buf += bytes_per_pixel;
    }
  }
  backgroundLayer.swapBuffers();
}

