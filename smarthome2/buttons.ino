#include <Bounce2.h>

#include "pinlayout.h"

Bounce debouncerLinksHoch = Bounce();
Bounce debouncerLinksRunter = Bounce();
Bounce debouncerRechtsHoch = Bounce();
Bounce debouncerRechtsRunter = Bounce();

void setupGPIO() {
  // ok: braun  schwarz rot    1000 Ohm
  // ok: orange orange  orange 2200 Ohm
  // ok: braun  schwarz ?
  // ok: gelb   lila    rot    4600 Ohm
  debouncerLinksHoch.attach(BUTTONLINKSHOCH, INPUT);
  debouncerRechtsHoch.attach(BUTTONRECHTSHOCH, INPUT);
  debouncerLinksRunter.attach(BUTTONLINKSRUNTER, INPUT);
  debouncerRechtsRunter.attach(BUTTONRECHTSRUNTER, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
}

void checkButtons() {
  // Links hoch
  debouncerLinksHoch.update();
  if (debouncerLinksHoch.fell()) {
    Serial.println("LH close");
  }
  if (debouncerLinksHoch.rose()) {
    Serial.println("LH open");
  }


  // Rechts hoch
  debouncerRechtsHoch.update();
  if (debouncerRechtsHoch.fell()) {
    Serial.println("RH close");
  }
  if (debouncerRechtsHoch.rose()) {
    Serial.println("RH open");
  }

  // Links runter
  debouncerLinksRunter.update();
  if (debouncerLinksRunter.fell()) {
    Serial.println("LR close");
  }
  if (debouncerLinksRunter.rose()) {
    Serial.println("LR open");
  }


  // Rechts runter
  debouncerRechtsRunter.update();
  if (debouncerRechtsRunter.fell()) {
    Serial.println("RR close");
  }
  if (debouncerRechtsRunter.rose()) {
    Serial.println("RR open");
  }
}
