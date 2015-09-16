int potPin = 5;    // select the input pin for the potentiometer
int val = 0;       // variable to store the value coming from the sensor

void setup() {
  Serial.begin(115200);
}

void loop() {
  val = analogRead(potPin);    // read the value from the sensor
  Serial.println(val);
  delay(100);
}
