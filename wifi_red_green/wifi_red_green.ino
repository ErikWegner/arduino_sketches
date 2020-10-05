
// https://www.adlerweb.info/blog/2018/07/01/bitbastelei-290-mqtt-protokoll-mosquitto-esp8266-homeassistant-tls/
#include "secret.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include "ota.h"
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
const char* mqtt_fprint = "B1:9D:1D:47:C6:01:06:71:7F:51:63:0B:61:19:67:E9:70:07:CF:7F";

#define TASK_DELAY 50
#define DEFAULT_TIMEOUT (5 * 1000 / TASK_DELAY)
int8_t mqtt_reconnect_timeout = 0;

bool startOTA = true;

extern "C" {
  #include "user_interface.h"
}

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("TestMQTT");
  bar.begin(0x71);
  bar.clear();
  barColor(LED_GREEN);
  wifi_set_sleep_type(LIGHT_SLEEP_T);

  pinMode(D4, OUTPUT);
  digitalWrite(D4, HIGH);
  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg_wifi_ssid, cfg_wifi_pwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  Serial.println(F(""));
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());
  digitalWrite(D4, LOW);

  espClient.setFingerprint(mqtt_fprint);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
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
  if (startOTA) {
    startOTA = false;
    setupOTA();
  }
  handleOTA();

  if (client.connected()) {
    client.loop();
    if (counter == 0) {
      client.publish("/d/r2/alive", "on", true);
    }
    counter = (counter + 1) % 600;

    delay(100);
  } else {
    if (mqtt_reconnect_timeout > 0)
    {
      Serial.print(F("MQTT reconnect timeout = "));
      Serial.println(mqtt_reconnect_timeout);
      mqtt_reconnect_timeout--;
    } else {
      mqtt_reconnect_timeout = DEFAULT_TIMEOUT;
      connectToMqtt();
    }
  }
}

void barColor(uint8_t bc) {
  for (uint8_t i = 0; i < 24; i++) {
    bar.setBar(i, bc);
  }
  bar.writeDisplay();

}


void connectToMqtt() {
  Serial.println(F("Connecting to MQTT broker..."));
  verifyFingerprint();
  if (client.connect(MQTT_CLIENTNAME, MQTT_USERNAME, MQTT_PASSWORD, "/d/r2/alive", 0, 1, "off", true)) {
    Serial.println(F("connected"));
    client.publish("/d/r2/alive", "on", true);
    client.subscribe("/d/r2/color");
    digitalWrite(D0, LOW);
  } else {
    Serial.print(F("failed, rc="));
    Serial.print(client.state());
  }
}
