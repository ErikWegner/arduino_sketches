/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  This example code is in the public domain.
 */

// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 13;

// the setup routine runs once when you press reset:
void setup() {
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
}

// the loop routine runs over and over again forever:
void loop() {
  /*digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
*/

  kurz(); kurz(); kurz();
  zeichenPause();
  lang(); lang(); lang();
  zeichenPause();
  kurz(); kurz(); kurz();
  zeichenPause();
}




void an() { digitalWrite(led, HIGH); }
void aus() { digitalWrite(led, LOW); }
void pause() { delay(300); }
void zeichenPause() { delay(500); }
void anZeit(int d) { an(); delay(d); aus(); }
void lang() { anZeit(600); pause(); }
void kurz() { anZeit(200); pause(); }
