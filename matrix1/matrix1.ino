/**
   Arduino IDE 1.8.2
   Teensyduino 1.36
   SmartMatrix3 3.0.1
   SdFat 2016.7.24
   SPI 1.0
*/

#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32
#define DEBUG 1

prog_char ERROR_SD_Init[] PROGMEM = "01 SD fail";

void error(const char text[]) {
  panel_showError(text);
}

void setup() {
  Serial.begin(9600);
  panel_setup();
  panel_showBootText();
  if (sd_init() < 0) {
    error(ERROR_SD_Init);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  scriptprocessor_executeNextStatement();
}

uint8_t imagedata[MATRIX_WIDTH * MATRIX_HEIGHT * 4];

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

