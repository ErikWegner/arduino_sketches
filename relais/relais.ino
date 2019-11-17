#define RELAISDELAY 250
#define LEFTPOWER 3
#define LEFTDIR 4
#define RIGHTPOWER 5
#define RIGHTDIR 6

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(F("Start"));
  pinMode(LEFTPOWER, OUTPUT);
  pinMode(LEFTDIR, OUTPUT);
  pinMode(RIGHTPOWER, OUTPUT);
  pinMode(RIGHTDIR, OUTPUT);
}

void loop() {
  loop2();
}

void loop2() {
  // put your main code here, to run repeatedly:
  Serial.println(F("OFF"));
  stopAll();
  delay(1000);

  Serial.println(F("LEFT UP"));
  leftUp();
  delay(1000);

  Serial.println(F("LEFT DOWN"));
  leftDown();
  delay(1000);

  Serial.println(F("RIGHT UP"));
  rightUp();
  delay(1000);

  Serial.println(F("RIGHT DOWN"));
  rightDown();
  delay(1000);
}

void stopAll() {
  digitalWrite(LEFTPOWER, HIGH);
  digitalWrite(RIGHTPOWER, HIGH);
  digitalWrite(LEFTDIR, HIGH);
  digitalWrite(RIGHTDIR, HIGH);
}

void leftOn() {
  digitalWrite(LEFTPOWER, LOW);
}

void rightOn() {
  digitalWrite(RIGHTPOWER, LOW);
}

void leftUp() {
  stopAll();
  delay(RELAISDELAY);
  digitalWrite(LEFTDIR, HIGH);
  delay(RELAISDELAY);
  leftOn();
}

void leftDown() {
  stopAll();
  delay(RELAISDELAY);
  digitalWrite(LEFTDIR, LOW);
  delay(RELAISDELAY);
  leftOn();
}

void rightUp() {
  stopAll();
  delay(RELAISDELAY);
  digitalWrite(RIGHTDIR, HIGH);
  delay(RELAISDELAY);
  rightOn();
}

void rightDown() {
  stopAll();
  delay(RELAISDELAY);
  digitalWrite(RIGHTDIR, LOW);
  delay(RELAISDELAY);
  rightOn();
}
