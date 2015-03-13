#include <SoftwareSerial.h>

#include <Adafruit_NeoPixel.h>
#define PIXEL_PIN 9 // Digital IO pin connected to the NeoPixels.
#define TONE_PIN 3
#define PIXEL_COUNT 12
#define BUFFERSIZE 80
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

int ledPin_R1 = 11;
int ledPin_R2 = 5;
int ledPin_Y1 = 12;
int ledPin_Y2 = 6;
int ledPin_G1 = 13;
int ledPin_G2 = 7;

char buffer[BUFFERSIZE+1];

void setup() {
  pinMode(ledPin_R1,OUTPUT);
  pinMode(ledPin_R2,OUTPUT);
  pinMode(ledPin_Y1,OUTPUT);
  pinMode(ledPin_Y2,OUTPUT);
  pinMode(ledPin_G1,OUTPUT);
  pinMode(ledPin_G2,OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  Serial.println("Waiting for commands. Terminate with ';'.");
}

String parserString;

void loop() {
  if (Serial.available()) {
    byte len = Serial.readBytesUntil(';', buffer, BUFFERSIZE);
    if (len > 0) {
      buffer[len] = '\0';
      parserString = String(buffer);
/*      Serial.print("parserString: ");
      Serial.println(parserString);
      Serial.println(parserString.length());*/
      if (parserString.startsWith("LED")) {
        processLED(parserString.substring(4));
      }

      if (parserString.substring(0, 4).equalsIgnoreCase("RING")) {
        processRING(parserString.substring(5));
      }
    }
  } 
  else {
    delay(2);
  }
}

void processRING(String s) {
  Serial.print("processRING: ");
  Serial.println(s);

  if (s.equalsIgnoreCase("SHOW")) {
    Serial.println("show ring");
    strip.show();
  } 
  else {
    int sep1, sep2, sep3;
    sep1 = s.indexOf(',');
    sep2 = s.indexOf(',', sep1 + 1);
    sep3 = s.indexOf(',', sep2 + 2);
    /*Serial.print("sep1: ");
    Serial.print(sep1);
    Serial.print(" sep2: ");
    Serial.print(sep2);
    Serial.print(" sep3: ");
    Serial.println(sep3);*/
   
    int led = s.substring(0, sep1).toInt();
    int red = s.substring(sep1+1, sep2).toInt();
    /*Serial.print("R ");
    Serial.println(red);*/
    int green = s.substring(sep2+1, sep3).toInt();
    /*Serial.print("G ");
    Serial.println(green);*/
    int blue = s.substring(sep3+1).toInt();
    /*Serial.print("B ");
    Serial.println(blue);*/
    
    strip.setPixelColor(led, red, green, blue);
  }
}

void processLED(String s) {
  int seppos = s.indexOf(',');
  Serial.println("PIN: .." + s.substring(0, seppos) + "..");
  int pin = s.substring(0, seppos).toInt();
  bool state = s.substring(seppos + 1) == "1";
  //  Serial.println("State: .." +  "..");
  Serial.print("State: ");
  Serial.println(state ? "HIGH" : "LOW ");
  digitalWrite(pin, state);
}



