#include "Wire.h"
#include "I2C_4DLED.h"
#include "utility/SAA1064.h"
#include "MCP9801.h"

const uint8_t i2cAddressSAA1064 = 0x70;

#include <stdio.h>
#include "DateTime.h"
#include "RealTimeClock_DCF.h"
#include "PinChangeInt.h"

#include <Adafruit_NeoPixel.h>

/******************************************************************************
globale Variablen
******************************************************************************/
/* aktuelle Uhrzeit */
DateTime dateTime;

/* Flag zur Anzeige eines periodischen Interrupts */
volatile uint8_t periodicInterruptFlag = 0; 

#define SOUND_PIN    9

#define PIXEL_PIN    8    // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 12
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
#define PIXEL_MAX   32
uint8_t subsecondStripColor[][3] = {
  {PIXEL_MAX, 0, 0},
  {PIXEL_MAX, PIXEL_MAX, 0},
  {0, PIXEL_MAX, 0},
  {0, 0, PIXEL_MAX},
  {PIXEL_MAX, PIXEL_MAX, PIXEL_MAX}
};

/* Soll der Punkt angezeigt werden: */
volatile int8_t showSeparator = 1;

/* onboard led */
int ledPin = 13; 
/* binary leds for seconds */
//int secondPins[] ={ 0,1,2,3,10,11 };

/* 7 segment mode */
#define segmentModeOff      0
#define segmentModeClock    1
#define segmentModeSeconds  2
#define segmentModeTemp     3
#define segmentModeCalendar 4
byte segmentMode = segmentModeClock;
byte backToClockModeCounter = 0;

// 5 Seconds with 2Hz until we go back to clock
#define BACKTOCLOCKMODEDELAY 10

const uint8_t pinSecButton        = 7;
const uint8_t pinCalendarButton   = 6;
const uint8_t pinTempButton       = 5;
const uint8_t pinOffButton        = 4;
const uint8_t i2cAddressMCP9801   = 0x90;
int32_t temperature = 0;

// Helligkeitsstufe
uint8_t brightness = 0;
// Ausgangsstrom des SAA1064
I2C_4DLED::OutputCurrent outputCurrent = I2C_4DLED::OUTPUT_CURRENT_3_MA;

byte rainbowState = 0;
/******************************************************************************
Funktionen
******************************************************************************/


unsigned char ledsegments[] =
{
    0x3f,     // 0
    0x06,     // 1
    0x5b,     // 2
    0x4f,     // 3
    0x66,     // 4
    0x6d,     // 5
    0x7d,     // 6
    0x07,     // 7
    0x7f,     // 8
    0x6f,     // 9
    0x77,     // A
    0x7c,     // B
    0x58,     // C
    0x5e,     // D
    0x79,     // E
    0x71      // F
};
uint8_t dcf[4]= {0x0, ledsegments[13], ledsegments[12], ledsegments[15]};


void setup() {
  //prepareSecondPins();
  strip.begin();
  strip.show();
  
// I2C-Modul initialisieren
  Wire.begin();
  
  // 4-Digit-LED-Anzeige mit I2C-Adresse des SAA1064 initialisieren
  FourDigitLedDisplay.begin(i2cAddressSAA1064);
  FourDigitLedDisplay.setDisplayOutputCurrent(outputCurrent);
  
  // alle im SAA1064 gespeicherten Digits löschen
  FourDigitLedDisplay.clearDisplay();
  
  // Segmenttest des Displays starten
  FourDigitLedDisplay.testDisplaySegments(1);
  
  // nach einer Sekunde den Segmenttest beenden
  delay(500);
  FourDigitLedDisplay.testDisplaySegments(0);
  LedDriver.writeDigits(SAA1064::SUBADDRESS_DIGIT_1, dcf, 4);
  
  // Temperatursensor mit I2C-Adresse des MCP9801 initialisieren
  TemperatureSensor.begin(i2cAddressMCP9801);
  
  // Temperatursensor auf eine Auflösung von 12 Bit umschalten
  TemperatureSensor.setADCResolution(TemperatureSensor.RESOLUTION_12BIT);
  

  
  pinMode(pinSecButton, INPUT);
  pinMode(pinCalendarButton, INPUT);
  pinMode(pinTempButton, INPUT);
  pinMode(pinOffButton, INPUT);
  
  digitalWrite(pinSecButton, HIGH);
  digitalWrite(pinCalendarButton, HIGH);
  digitalWrite(pinTempButton, HIGH);
  digitalWrite(pinOffButton, HIGH);
  
    /* der RTC-DCF benötigt ca. 1,5 Sekunden bis er Daten empfangen kann */
  //delay(1000);
  for (int s = 0; s < 256; s++) {
    rainbowCycle(s);
    delay(12);

  }

  /* den Pin für den periodischen Interrupt als Eingang und als externe
     Interrupt-Quelle definieren */
  pinMode(RTC_DCF_PER_INT_PIN, INPUT);
  PCintPort::attachInterrupt(RTC_DCF_PER_INT_PIN, &periodicInterrupt, FALLING);
  
  /* RTC-DCF initialisieren */  
  RTC_DCF.begin();

  RTC_DCF.enableDCF77Reception();
  RTC_DCF.enableDCF77LED();

    /* der RTC-DCF benötigt ca. 1,5 Sekunden bis er Daten empfangen kann */
  //delay(1500);  

  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);

  /* den periodischen Interrupt auf 1 Hz einstellen */
  RTC_DCF.setPeriodicInterruptMode(RTC_PERIODIC_INT_PULSE_2_HZ);
  
  /* den periodischen Interrupt des RTC-DCF aktivieren */
  RTC_DCF.enablePeriodicInterrupt();
  
  /* die serielle Ausgabe initialisieren */
  Serial.begin(115200);

  /* Interrupts einschalten */  
  interrupts();
  Serial.println("Start");
}

void loop() {
  // put your main code here, to run repeatedly:
  if(periodicInterruptFlag == 1)
  {
    RTC_DCF.getDateTime(&dateTime);
    
    updateLED();
    if (showSeparator > 0) {
      printClock();
      //updateSecondLED();
      //updateStrip();
      checkResetDCF();
    }
    if (backToClockModeCounter > 0) backToClockModeCounter--;
    periodicInterruptFlag = 0;
  }
  
  if (backToClockModeCounter == 1) {
    // the last counter resets the segment mode.
    segmentMode = segmentModeClock;
    // when we turn off the segments, we set backToClockModeCounter directly to 0 to avoid the switch back
  }
  
  if(digitalRead(pinSecButton) == 0) {
    segmentMode = segmentModeSeconds;
    backToClockModeCounter = BACKTOCLOCKMODEDELAY;
  }
  
  if(digitalRead(pinCalendarButton) == 0) {
    segmentMode = segmentModeCalendar;
    backToClockModeCounter = BACKTOCLOCKMODEDELAY;
  }
  
  if(digitalRead(pinTempButton) == 0) {
    delay(200);
    if (segmentMode == segmentModeTemp) {
      segmentMode = segmentModeClock;
    } else {
      segmentMode = segmentModeTemp;
      backToClockModeCounter = 0;
    }
  }
  
  if(digitalRead(pinOffButton) == 0) {
    adjustBrightness();
  }
  
  delay(50);
}

void checkResetDCF() {
  if (dateTime.getHour() > 23 || (dateTime.getMinute() % 15 == 4 && dateTime.getSecond() == 58)) {
    RTC_DCF.disableDCF77Reception();
    LedDriver.writeDigits(SAA1064::SUBADDRESS_DIGIT_1, dcf, 4);
    delay(2500);
    RTC_DCF.enableDCF77Reception();
  }
}

void periodicInterrupt(void)
{
  periodicInterruptFlag = 1;
  showSeparator *= -1;
}

void printClock(void)
{
  char clockString[30];
  
  sprintf(clockString, "Es ist %02u:%02u:%02u am %02u.%02u.%02u", dateTime.getHour(), dateTime.getMinute(), dateTime.getSecond(), dateTime.getDay(), dateTime.getMonth(), dateTime.getYear());
  
  Serial.println(clockString);  
}

/*void prepareSecondPins() {
  for (int i = 0; i < 6; i++) {
    pinMode(secondPins[i], OUTPUT);
    digitalWrite(secondPins[i], HIGH);
    tone(SOUND_PIN, 140 + i*10);
    delay(50);
  }
  for (int i = 0; i < 6; i++) {
    digitalWrite(secondPins[i], LOW);
    tone(SOUND_PIN, 240 + i*10);
    delay(50);
  }
  noTone(SOUND_PIN);
}*/


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(byte state) {
  uint16_t i;
  for(i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + state) & 255));
  }
  strip.show();
}

void updateLED(void)
{
  
  // update 7 segment display
  uint8_t digits[4] = {0x00, 0x00, 0x00, 0x00};

  switch (segmentMode) {
    uint8_t d;
    case segmentModeClock:
      d = dateTime.getHour();
      if (d > 9) digits[0] = ledsegments[d / 10];
      digits[1] = ledsegments[d % 10] | (showSeparator > 0 ? 128 : 0);
      
      d = dateTime.getMinute();
      digits[2] = ledsegments[d / 10];
      digits[3] = ledsegments[d % 10];
      break;
    case segmentModeSeconds:
      d = dateTime.getSecond();
      digits[1] = (showSeparator > 0 ? 128 : 0);
      digits[2] = ledsegments[d / 10];
      digits[3] = ledsegments[d % 10];      
      break;
    case segmentModeCalendar:
      d = dateTime.getDay();
      if (d > 9) digits[0] = ledsegments[d / 10];
      digits[1] = ledsegments[d % 10] | 128;
      
      d = dateTime.getMonth();
      if (d > 9) digits[2] = ledsegments[d / 10];
      digits[3] = ledsegments[d % 10] | 128;
      break;
    case segmentModeTemp:
      temperature = TemperatureSensor.readTemperature();
      FourDigitLedDisplay.writeDecimal( (temperature + 500) / 1000, 1 );

      break;
  }
  if (segmentMode != segmentModeTemp) LedDriver.writeDigits(SAA1064::SUBADDRESS_DIGIT_1, digits, 4);
  
  // blink onboard led
  digitalWrite(ledPin, showSeparator > 0 ? HIGH : LOW);
}

/*void updateSecondLED() {
  // show seconds in binary form on additional led
  int seconds = dateTime.getSecond();
  for (int i = 5; i >= 0; i--) {
    digitalWrite(secondPins[i], seconds % 2 == 1);
    seconds /= 2;
  }
}*/

void updateStrip() {
  Serial.print("updateString ... ");
  int seconds = dateTime.getSecond();
  uint8_t* pixelcolor = subsecondStripColor[seconds % 5];
  strip.clear();
  strip.setPixelColor(2, 0, PIXEL_MAX/8, 0);
  strip.setPixelColor(5, 0, PIXEL_MAX/8, 0);
  strip.setPixelColor(8, 0, PIXEL_MAX/8, 0);
  strip.setPixelColor(11, PIXEL_MAX/8, 0, 0);
  strip.setPixelColor(seconds / 5, (uint8_t)pixelcolor[0], (uint8_t)pixelcolor[1], (uint8_t)pixelcolor[2]);
  strip.show();
  Serial.println("done");
}

void adjustBrightness() {
  brightness = (brightness + 1) % 5;
  // je nach aktueller Helligkeit den Ausgangstrom einstellen
  switch(brightness)
  {
  case 0:
    outputCurrent = I2C_4DLED::OUTPUT_CURRENT_0_MA;
    break;
    
  case 1:
    outputCurrent = I2C_4DLED::OUTPUT_CURRENT_3_MA;
    break;

  case 2:
    outputCurrent = I2C_4DLED::OUTPUT_CURRENT_6_MA;
    break;

  case 3:
    outputCurrent = I2C_4DLED::OUTPUT_CURRENT_9_MA;
    break;

  case 4:
    outputCurrent = I2C_4DLED::OUTPUT_CURRENT_12_MA;
    break;
  
  default:
    brightness = 0;
    break; 
  }
  
  // Ausgangsstrom dem SAA1064 übergeben
  FourDigitLedDisplay.setDisplayOutputCurrent(outputCurrent);

  tone(SOUND_PIN, 244 + 12 * brightness);
  delay(20);
  noTone(SOUND_PIN);
  delay(180);
}
