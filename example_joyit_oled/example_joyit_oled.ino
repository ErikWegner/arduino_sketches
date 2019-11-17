#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

void setup()   {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Adressierung beachten, hier 0x3C!
}

int i;

void loop() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(23, 0);
  display.println("OLED -Display");
  display.setCursor(23, 12);
  display.println("www.joy-it.net");
  display.setCursor(36, 24);
  display.println("SBC-OLED01");
  display.display();
  delay(8000);
  display.clearDisplay();
  display.invertDisplay(true);
  delay(8000);
  display.invertDisplay(false);
  delay(1000);
}
