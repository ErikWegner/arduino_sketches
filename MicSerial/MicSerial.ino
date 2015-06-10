unsigned int sample;

void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
   analogReference(EXTERNAL);
}

void loop() {
  // put your main code here, to run repeatedly:
  sample = analogRead(0); 
  Serial.println(sample);
  delay(10);
}
