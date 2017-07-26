prog_char ERROR_SD_Init[] PROGMEM = "01 SD fail";

void error(const char text[]) {
  panel_showError(text);
}

void setup() {
  panel_setup();
  panel_showBootimg();
  if (sd_init() < 0) {
    error(ERROR_SD_Init);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
