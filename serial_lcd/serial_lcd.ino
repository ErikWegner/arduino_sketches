#include <SoftwareSerial.h>

SoftwareSerial mySerial(10,11); // RX, TX

void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(38400);
  mySerial.println("~C~P103http://ewus.de");
}

int b1 = 1;
boolean forward = true;

void loop() // run over and over
{
  while (mySerial.available())
    Serial.write(mySerial.read());
  while (Serial.available())
    mySerial.write(Serial.read());
//    mySerial.println("Welt"); delay(200);
  mySerial.write("~L");
  mySerial.write(b1 % 256);
  mySerial.write((b1 / 256) % 4);
  if (forward) {
    b1 *= 2;
    forward = b1 < 512;
  }
  else {
    b1 /= 2;
    forward = (b1 == 0);
    if (b1 == 0) b1++;
  }
  delay(33);
}
