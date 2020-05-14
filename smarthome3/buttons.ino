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

void checkButtons(Motorsteuerung *motorLinks, Motorsteuerung *motorRechts) {
  // Links hoch
  debouncerLinksHoch.update();
  if (debouncerLinksHoch.fell()) {
    Serial.println("LH close");
    motorLinks->setCommand(MotorCommands::MOVE_UP);
  }
  if (debouncerLinksHoch.rose()) {
    Serial.println("LH open");
    motorLinks->setCommand(MotorCommands::STOP);
  }


  // Rechts hoch
  debouncerRechtsHoch.update();
  if (debouncerRechtsHoch.fell()) {
    Serial.println("RH close");
    motorRechts->setCommand(MotorCommands::MOVE_UP);
  }
  if (debouncerRechtsHoch.rose()) {
    Serial.println("RH open");
    motorRechts->setCommand(MotorCommands::STOP);
  }

  // Links runter
  debouncerLinksRunter.update();
  if (debouncerLinksRunter.fell()) {
    Serial.println("LR close");
    motorLinks->setCommand(MotorCommands::MOVE_DOWN);
  }
  if (debouncerLinksRunter.rose()) {
    Serial.println("LR open");
    motorLinks->setCommand(MotorCommands::STOP);
  }


  // Rechts runter
  debouncerRechtsRunter.update();
  if (debouncerRechtsRunter.fell()) {
    Serial.println("RR close");
    motorRechts->setCommand(MotorCommands::MOVE_DOWN);
  }
  if (debouncerRechtsRunter.rose()) {
    Serial.println("RR open");
    motorRechts->setCommand(MotorCommands::STOP);
  }
}
