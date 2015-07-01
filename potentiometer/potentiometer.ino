#define LCD_PORT_TX 7
#define LCD_PORT_RX 6 // not really used 

// http://neaptide.org/journal/2014/07/arduino-softwareserial-with-printf/
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

int sensorPin = A0;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

int updateMonitor = 0;

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
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  updateMonitor++;
  
  if (updateMonitor > 10) {
    Serial.println(sensorValue);
    updateMonitor = 0;
  }
  delay(20);
  

  lcdserial.write(254); // move cursor to beginning of first line
  lcdserial.write(128);
  lcdserial.printf("Value: %4d", sensorValue);

}
