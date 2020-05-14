
// https://www.adlerweb.info/blog/2018/07/01/bitbastelei-290-mqtt-protokoll-mosquitto-esp8266-homeassistant-tls/
#include "secret.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_24bargraph bar = Adafruit_24bargraph();

const char* cfg_wifi_ssid = WIFI_SSID;
const char* cfg_wifi_pwd = WIFI_PWD;

const char* mqtt_server = MQTT_SERVER;
const unsigned int mqtt_port = 8883;
const char* mqtt_user =   MQTT_USERNAME;
const char* mqtt_pass =   MQTT_PASSWORD;

//echo | openssl s_client -connect server:8883 | openssl x509 -fingerprint -noout
const char* mqtt_fprint = "33:B5:1D:6A:C3:7D:88:04:FC:16:63:4D:2E:75:25:DF:80:39:AD:F5";

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("TestMQTT");
  bar.begin(0x71);
  bar.clear();
  barColor(LED_GREEN);

  pinMode(D4, OUTPUT);
  digitalWrite(D4, HIGH);
  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg_wifi_ssid, cfg_wifi_pwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(D4, LOW);

  espClient.setFingerprint(mqtt_fprint);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    verifyFingerprint();
    if (client.connect(WiFi.macAddress().c_str(), mqtt_user, mqtt_pass, "/d/r2/alive", 0, 1, "off", true)) {
      Serial.println("connected");
      client.publish("/d/r1/alive", "on", true);
      client.subscribe("/d/r2/color");
      digitalWrite(D0, LOW);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);
  if (message[0] == 'r') {
    barColor(LED_RED);
  }
  if (message[0] == 'g') {
    barColor(LED_GREEN);
  }
}

void verifyFingerprint() {
  if (client.connected() || espClient.connected()) return; //Already connected

  Serial.print("Checking TLS @ ");
  Serial.print(mqtt_server);
  Serial.print("...");

  if (!espClient.connect(mqtt_server, mqtt_port)) {
    Serial.println("Connection failed. Rebooting.");
    Serial.flush();
    ESP.restart();
  }
  if (espClient.verify(mqtt_fprint, mqtt_server)) {
    Serial.print("Connection secure -> .");
  } else {
    Serial.println("Connection insecure! Rebooting.");
    Serial.flush();
    ESP.restart();
  }

  espClient.stop();

  delay(100);
}

uint16_t counter = 0;

void loop() {
  client.loop();

  if (counter == 0) {
    client.publish("/d/r2/alive", "on", true);
  }
  counter = (counter + 1) % 600;

  delay(100);
}

void barColor(uint8_t bc) {
  for (uint8_t i = 0; i < 24; i++) {
    bar.setBar(i, bc);
  }
  bar.writeDisplay();

}
