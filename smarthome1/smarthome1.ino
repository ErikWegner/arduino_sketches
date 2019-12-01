  // Mar 2019 by Jesse Bedard
// Released to the public domain

// Arduino IDE 1.8.9
#include <Wire.h>

// ESP8266 Version 2.6.1
#include <ESP8266WiFi.h>
// Version 2.7.0
#include <PubSubClient.h>
// Version 1.0.5
#include <Adafruit_BMP280.h>

#include "settings.h"
#include "display.h"

//Simple boolean to indicate first startup loop
bool startup = false;

// timer and interrupt variables
bool readSensor = false;
float temperature = 0.0;
float pressure = 0.0;
bool sendMqttData;

WiFiClientSecure client;
PubSubClient mqttclient(client);
DisplayHelper display;
Adafruit_BMP280 bmp;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    char receivedChar = (char)payload[i];
    Serial.println(receivedChar);
    //****
    //Do some action based on message received
    //***
  }
}


void reconnect() {
  Serial.print("Attempting MQTT connection...");
  display.status("MQTT connect... ");
  //Attempt to connect
  if (mqttclient.connect(mqtt_clientname, mqtt_username, mqtt_password)) {
    Serial.println("connected");
    display.status("OK");
    //Subscribe to topic
    mqttclient.subscribe(mqtt_topic);
  } else {
    Serial.print("failed, rc=");
    Serial.print(mqttclient.state());
    display.status("mqtt failed (5 sec)");
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(5000);
  } //if
}

void publishFloat(char* topic, float v) {
  Serial.println("publishFloat");
  char floatBuffer[8];
  Serial.println("dtostrf");
  dtostrf(v, 5, 2, floatBuffer);
  Serial.println("publish");
  mqttclient.publish(topic, floatBuffer);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Boot");
  Wire.begin();
  display.init();
  WiFi.enableAP(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //****
  //Important to set fingerprint to verify server
  //setInsecure() will allow the ssl connection without verification
  //****
  //client.setInsecure(); //Do NOT verify server!!!
  client.setFingerprint(fingerprint);

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
  }

  isr_init();

  Serial.println("Connecting to WiFi.");
  display.status("Waiting for WiFi");
  int _try = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    _try++;
    if (_try > 40) {
      Serial.println("Reset..");
      display.status("Reset");
      delay(800);
      ESP.restart();
    }
  }
  Serial.println("Connected to the WiFi network");
  display.status("WiFi ok");

  //Connect to your MQTT Server and set callback
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(callback);
}

void loop()
{
  if (!mqttclient.connected()) {
    reconnect();
  }

  //Publish a startup message
  if (startup == false) {
    Serial.print("Publish startup message 1");
    mqttclient.publish(mqtt_topic, "1", true);
    startup = true;
  };

  mqttclient.loop();

  if (readSensor) {
    updateSensorValues();
  }

  if (sendMqttData) {
    Serial.println("sendMqttt");
    display.isTransmitting(true);
    sendMqttData = false;
    publishFloat("/d/r1/sensors/temp", temperature);
    publishFloat("/d/r1/sensors/pressure", pressure);
    display.isTransmitting(false);
  }

  yield();
}
