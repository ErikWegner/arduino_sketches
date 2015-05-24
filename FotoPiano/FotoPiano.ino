int sensorPin = 0;
int speakerPin = 9;
 
void setup() {
  Serial.begin(9600);
}
 
void loop() {
  Serial.println(analogRead(sensorPin));
  tone(speakerPin,analogRead(sensorPin));
  delay(20);
}
