int ledPin_R1 = 11;
int ledPin_R2 = 5;
int ledPin_Y1 = 12;
int ledPin_Y2 = 6;
int ledPin_G1 = 13;
int ledPin_G2 = 7;

int buttonPin = 8;

int tonePin = 3;

boolean strasse1offen = true;

boolean buttonPressed = false;

void setup() {
  pinMode(ledPin_R1,OUTPUT);
  pinMode(ledPin_R2,OUTPUT);
  pinMode(ledPin_Y1,OUTPUT);
  pinMode(ledPin_Y2,OUTPUT);
  pinMode(ledPin_G1,OUTPUT);
  pinMode(ledPin_G2,OUTPUT);
  pinMode(buttonPin,INPUT);
  
  tone(tonePin, 262);
  delay(45),
  tone(tonePin, 294);
  delay(45),
  tone(tonePin, 330);
  delay(45),
  noTone(tonePin);
  
  gruen(strasse1offen);  
}

void loop() {
  if ((digitalRead(buttonPin)==HIGH)&&(buttonPressed==false)){
    buttonPressed=true;
  } 
  if (digitalRead(buttonPin)==LOW&&(buttonPressed==true)){
    buttonPressed=false;
    gelb(strasse1offen);
    rot();
    strasse1offen = !strasse1offen;
    rotgelb(strasse1offen);
    gruen(strasse1offen);
  }
}

void led(boolean r1, boolean y1, boolean g1, boolean r2, boolean y2, boolean g2, int d) {
  digitalWrite(ledPin_R1, r1);
  digitalWrite(ledPin_R2, r2);
  digitalWrite(ledPin_Y1, y1);
  digitalWrite(ledPin_Y2, y2);
  digitalWrite(ledPin_G1, g1);
  digitalWrite(ledPin_G2, g2);
  delay(d);
}

void rot() {
  led(
  HIGH, LOW, LOW,
  HIGH, LOW, LOW,
  1000);
}

void gelb(boolean strasse) {
  tone(tonePin, 4400);
  delay(25),
  noTone(tonePin);
  led(
  strasse ? LOW : HIGH, strasse ? HIGH : LOW, LOW,
  strasse ? HIGH : LOW, strasse ? LOW : HIGH, LOW,
  1000);
}

void rotgelb(boolean strasse) {
  led(
  HIGH, strasse ? HIGH : LOW, LOW,
  HIGH, strasse ? LOW : HIGH, LOW,
  1000);
}

void gruen(boolean strasse) {
    
  tone(tonePin, 440);
  delay(50),
  noTone(tonePin);

  led(
  strasse ? LOW : HIGH, LOW, strasse ? HIGH : LOW,
  strasse ? HIGH : LOW, LOW, strasse ? LOW : HIGH,
  0);
}


