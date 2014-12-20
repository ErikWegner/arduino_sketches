int tasterPin = 8;
int ledPin = 9;

void setup(){
  pinMode(ledPin,OUTPUT);
  pinMode(tasterPin,INPUT);
}
 
void loop(){
  if (digitalRead(tasterPin)==HIGH){
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}
