#define BARWIDTH 6

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "motorsteuerung.h"
#include "comm.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define FONT_HEIGHT 8

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

long lastDisplayDraw = 0;

void drawGaugeBar(uint8_t x, uint8_t absoluteFill) {
  display.drawRect(x, 0, BARWIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  uint8_t y = (POSITION_UP - absoluteFill) * SCREEN_HEIGHT / POSITION_UP;
  display.fillRect(x, 0, BARWIDTH, y, SSD1306_WHITE);
}

void drawBars(unsigned int leftFill, unsigned int rightFill) {
  // Left bar
  drawGaugeBar(0, leftFill);

  // Right bar
  drawGaugeBar(SCREEN_WIDTH - 1 - BARWIDTH, rightFill);
}

void drawConnectionStatus() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(BARWIDTH + 2, 0);
  display.print(wifiText);
  display.print(" ");
  display.print(mqttText);
  display.setCursor(BARWIDTH + 2, FONT_HEIGHT);
  display.print(temperatureMqtt);
  display.print(" *C");
  display.setCursor(BARWIDTH + 2, FONT_HEIGHT * 2);
  display.print(pressureMqtt);
  display.print(" hPa");
}

void setupDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Adressierung beachten, hier 0x3C!
  display.display();
}

void updateDisplay(Motorsteuerung *motorLinks, Motorsteuerung *motorRechts) {
  unsigned long currentMillis = millis();
  if (currentMillis - lastDisplayDraw < 250) {
    return; // Drawing is not needed now
  }
  lastDisplayDraw = currentMillis;
  display.clearDisplay();

  drawBars(motorLinks->estimatedPosition(), motorRechts->estimatedPosition());
  drawConnectionStatus();
  display.display();
}
