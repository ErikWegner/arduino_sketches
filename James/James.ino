#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

#define LCD_PORT_TX 7
#define LCD_PORT_RX 6 // not really used 
#include <SoftwareSerial.h>
#include <inttypes.h>
class mySerial : public SoftwareSerial {

  public:
    mySerial(uint8_t receivePin, uint8_t transmitPin,
      bool inverse_logic = false) :
      SoftwareSerial(receivePin, transmitPin,inverse_logic) {}

    virtual size_t write(uint8_t byte) {
      return SoftwareSerial::write(byte);
    }

    int printf(char* fmt, ...) {
      char buff[256];
      va_list args;
      va_start(args, fmt);
      int return_status = vsnprintf(buff, sizeof(buff), fmt, args);
      va_end(args);
      uint8_t *s = (uint8_t *)&buff;
      while (*s) write(*s++);
      return return_status;
    }

};

mySerial lcdserial(LCD_PORT_RX, LCD_PORT_TX);
Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

static const uint8_t PROGMEM
  e8_bmp[] =
  { B10010010,
    B01001001,
    B00000000,
    B00111100,
    B01000110,
    B01011000,
    B01000011,
    B00111100 };

byte paddlePosition = 0;
#define paddlePin 0

#include <stdarg.h>
void p(char *fmt, ... ){
        char buf[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 128, fmt, args);
        va_end (args);
        Serial.print(buf);
}

void setup() {
  Serial.begin(115200);
  
  lcdserial.begin(9600);
  lcdserial.write(254); // move cursor to beginning of first line
  lcdserial.write(128);

  lcdserial.print("                "); // clear display
  lcdserial.print("                ");

  lcdserial.write(254); // move cursor to beginning of first line
  lcdserial.write(128);
  
  Serial.println(F("Ready"));
  // put your setup code here, to run once:
  matrix.begin(0x70);  // pass in the address
  
  matrix.clear();
  matrix.setRotation(3);

  intro();
}

void loop() {
  // put your main code here, to run repeatedly:
  tennis();
}

void intro() {
  matrix.drawLine(0,0,1,1, LED_GREEN);
  matrix.drawLine(3,0,4,1, LED_GREEN);
  matrix.drawLine(6,0,7,1, LED_GREEN);
  matrix.writeDisplay();
  delay(500);
  
  matrix.drawLine(2,7,5,7, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(2,3,5,3, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(3,5,4,5, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(1,4,1,6, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  matrix.drawLine(5,4,6,4, LED_GREEN);
  matrix.drawLine(6,6,7,6, LED_GREEN);
  matrix.writeDisplay();
  delay(250);

  for (byte b=0; b < 9; b++) {
    matrix.drawBitmap(0,0, e8_bmp, 8, 8, 1 + b % 3);
    matrix.writeDisplay();
    delay(250);
  }
  
  matrix.clear();
}


void tennis() {
  matrix.clear();
  paddlePosition = map(analogRead(paddlePin), 0, 1023, 0, 6);
  
  // paint paddle
  matrix.drawLine(0, paddlePosition, 0, paddlePosition + 1, LED_YELLOW);
  matrix.writeDisplay();
}
