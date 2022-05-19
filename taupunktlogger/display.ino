#include <Wire.h>
#define I2C_SDA 21
#define I2C_SCL 22

#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

#define lcdWidth 16
hd44780_I2Cexp lcd(0x27); // declare lcd object: auto locate & auto config expander chip

// Made with: https://kakedev.github.io/GlyphGenerator/
const PROGMEM uint8_t customchar[2][8] = {
  // Umlaut ü
  {0x0a, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0d, 0x00},
  // °C
  {0x18, 0x18, 0x03, 0x04, 0x04, 0x04, 0x03, 0x00},
};


long lastDisplayDraw = 0;
long lastDisplaySwitch = 0;
byte sensorOutput = 0;

void setupDisplay() {
  // initialize the LCD
  lcd.begin(lcdWidth, 2);

  // Turn on the blacklight and print a message.
  lcd.setCursor(0, 0);
  lcd.print("Boot");
  lcd.createChar(1, customchar[0]);
  lcd.createChar(2, customchar[1]);
}

#define temperatureOutputLength 4

void updateDisplay() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastDisplayDraw < 250) {
    return; // Drawing is not needed now
  }
  lastDisplayDraw = currentMillis;
  if (currentMillis - lastDisplaySwitch >= 1500) {
    sensorOutput = (sensorOutput + 1) % 2;
    lastDisplaySwitch = currentMillis;
//    Serial.print("sensor Output = ");
//    Serial.println(sensorOutput);
  }

  char buffer[16];
  char numFormattedStr[temperatureOutputLength + 1];

  if (sensorOutput == 0) {
    memcpy(buffer, "I __._\002  A __._\002", lcdWidth);
    dtostrf(tempInside, temperatureOutputLength, 1, numFormattedStr);
    strncpy(&buffer[2], numFormattedStr, temperatureOutputLength);
    dtostrf(tempOutside, temperatureOutputLength, 1, numFormattedStr);
    strncpy(&buffer[11], numFormattedStr, temperatureOutputLength);
    buffer[4] = ',';
    buffer[13] = ',';
  }
  if (sensorOutput == 1) {
    memcpy(buffer, "I __._%  A __._%", lcdWidth);
    dtostrf(humInside, temperatureOutputLength, 1, numFormattedStr);
    strncpy(&buffer[2], numFormattedStr, temperatureOutputLength);
    dtostrf(humOutside, temperatureOutputLength, 1, numFormattedStr);
    strncpy(&buffer[11], numFormattedStr, temperatureOutputLength);
    buffer[4] = ',';
    buffer[13] = ',';
  }

  // First line
  lcd.setCursor(0, 0);
  if (lueftung) {
    lcd.print("L\001ftung     ");
  } else {
    lcd.print("Fenster zu  ");
  }
  lcd.print(commstateStr[comm]);

  // Second line
  lcd.setCursor(0, 1);

  if (fehler > 0) {
    lcd.print(sensorError);
  } else {
    for (byte col = 0; col < lcdWidth; col++) {
      lcd.write(buffer[col]);
    }
  }
}
