#include "display.h"

#define OLED_RESET LED_BUILTIN

#define DISPLAY_DEBUG

Adafruit_SSD1306 i2c_display(128, 64, &Wire, OLED_RESET);


DisplayHelper::DisplayHelper() {}
void DisplayHelper::init()
{
#ifdef DISPLAY_DEBUG
  Serial.println("Initialize display");
#endif
  _display = i2c_display;
  i2c_display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Adressierung beachten, hier 0x3C!
  i2c_display.clearDisplay();
  i2c_display.setTextColor(WHITE);
  i2c_display.setTextSize(1);
  i2c_display.setCursor(23, 0);
  i2c_display.println("Booting...");
  i2c_display.display();
#ifdef DISPLAY_DEBUG
  Serial.println("Display ready");
#endif
}

void DisplayHelper::status(char* text)
{
#ifdef DISPLAY_DEBUG
  Serial.print("Setting text ");
#endif
  strlcpy(_text, text, BUFFERLENGTH_STATUS);
#ifdef DISPLAY_DEBUG
  Serial.println(_text);
#endif
  i2c_display.clearDisplay();
  i2c_display.setCursor(23, 0);
  i2c_display.print(_text);
  i2c_display.display();
}
