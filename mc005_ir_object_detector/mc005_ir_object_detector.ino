/**
 * Test of MC005 infrared object detector
 * 
 * Connections:
 * Sensor ⇒ Arduino UNI
 * Black  ⇒ Ground
 * Brown  ⇒ 5V
 * Blue   ⇒ Analog 5
 */

int pin = 5;
int val = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Ready");
}

void loop() {
  // put your main code here, to run repeatedly:
  val = analogRead(pin);    // read the input pin

  Serial.println(val);
}
