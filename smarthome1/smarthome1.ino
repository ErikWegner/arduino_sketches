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
  // Loop until we're reconnected to the MQTT server
  while (!mqttclient.connected()) {
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
  }//while
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Boot");
  Wire.begin();
  display.init();
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

  Serial.println("Connecting to WiFi.");
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
}
