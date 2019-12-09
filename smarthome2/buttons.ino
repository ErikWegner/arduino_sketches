#include "pinlayout.h"

void setupGPIO() {
  pinMode(BUTTONLINKSHOCH, INPUT);
  pinMode(BUTTONRECHTSHOCH, INPUT);
  pinMode(BUTTONLINKSRUNTER, INPUT);
  pinMode(BUTTONRECHTSRUNTER, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
}
