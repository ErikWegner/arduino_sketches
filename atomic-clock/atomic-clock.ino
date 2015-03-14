#include "Wire.h"
#include "I2C_4DLED.h"
#include "utility/SAA1064.h"

const uint8_t i2cAddressSAA1064 = 0x70;

#include <stdio.h>
#include "DateTime.h"
#include "RealTimeClock_DCF.h"
#include "PinChangeInt.h"

/******************************************************************************
globale Variablen
******************************************************************************/
/* aktuelle Uhrzeit */
DateTime dateTime;

/* Flag zur Anzeige eines periodischen Interrupts */
volatile uint8_t periodicInterruptFlag = 0; 


/* Soll der Punkt angezeigt werden: */
volatile int8_t showSeparator = 1;

/* onboard led */
int ledPin = 13; 
/******************************************************************************
Funktionen
******************************************************************************/


void setup() {
    /* der RTC-DCF benötigt ca. 1,5 Sekunden bis er Daten empfangen kann */
  delay(1500);  

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

// I2C-Modul initialisieren
  Wire.begin();
  
  // 4-Digit-LED-Anzeige mit I2C-Adresse des SAA1064 initialisieren
  FourDigitLedDisplay.begin(i2cAddressSAA1064);
  FourDigitLedDisplay.setDisplayOutputCurrent(I2C_4DLED::OUTPUT_CURRENT_3_MA);
  
  // alle im SAA1064 gespeicherten Digits löschen
  FourDigitLedDisplay.clearDisplay();
  
  // Segmenttest des Displays starten
  FourDigitLedDisplay.testDisplaySegments(1);
  
  // nach einer Sekunde den Segmenttest beenden
  delay(500);
  FourDigitLedDisplay.testDisplaySegments(0);
  



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
    
    if (showSeparator > 0) printClock();
    updateLED();
 
    periodicInterruptFlag = 0;
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

void updateLED(void)
{
  uint8_t digits[4] = {0x00, 0x00, 0x00, 0x00};

  uint8_t d = dateTime.getHour();
  if (d > 9) digits[0] = ledsegments[d / 10];
  digits[1] = ledsegments[d % 10] | (showSeparator > 0 ? 128 : 0);
  
  d = dateTime.getMinute();
  digits[2] = ledsegments[d / 10];
  digits[3] = ledsegments[d % 10];
//  FourDigitLedDisplay.writeDecimal(dateTime.getHour() * 100 + dateTime.getMinute(), showSeparator > 0 ? 2 : 0);
  LedDriver.writeDigits(SAA1064::SUBADDRESS_DIGIT_1, digits, 4);
  
  digitalWrite(ledPin, showSeparator > 0 ? HIGH : LOW);
}
