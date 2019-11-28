#ifndef SMARTHOME_DISPLAY
#define SMARTHOME_DISPLAY

// Version 1.6.1
#include <Adafruit_GFX.h>
// Version 2.0.1
#include <Adafruit_SSD1306.h>

#include <SPI.h>
#include <Wire.h>

#define BUFFERLENGTH_STATUS 20

class DisplayHelper
{
  public:
    DisplayHelper();
    void init();
    void status(char* text);
    void sensorData(float temperature, float pressure);
    void isTransmitting(bool transmitStatus);
  private:
    Adafruit_SSD1306 _display;
    char _text[BUFFERLENGTH_STATUS];
    bool _isTransmitting;
    char _sensorData[BUFFERLENGTH_STATUS];
    void refresh();
};

#endif
