#ifndef COMMUNICATION
#define COMMUNICATION

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "secret.h"

#define DEFAULT_TIMEOUT 15
bool isWifiConnected = false;
signed char wifi_reconnect_timeout = 0;

WiFiClientSecure espClient;
PubSubClient client(espClient);

void connectToWifi();
void connectToMqtt();

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(topic);

  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void ensureWifiAndMqtt() {
  if (!isWifiConnected) {
    if (wifi_reconnect_timeout > 0) {
      wifi_reconnect_timeout = wifi_reconnect_timeout - 1;
      return;
    }

    wifi_reconnect_timeout = DEFAULT_TIMEOUT;
    WiFi.disconnect();
    connectToWifi();
    return;
  }

  if (client.connected()) {
    client.loop();
    return;
  }

  connectToMqtt();
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT broker...");

  if (client.connect(MQTT_CLIENTNAME, MQTT_USERNAME, MQTT_PASSWORD)) {
    Serial.println("mqtt connected");
    client.subscribe("/d/r1/cmd/move");
  } else {
    Serial.print("failed, status code =");
    Serial.print(client.state());
  }
}

void connectToWifi() {
  if (isWifiConnected) {
    return;
  }

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PWD);
}

void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      isWifiConnected = true;
      /*
          Calling connectToMqtt() here gives a
          Guru Meditation Error: Core  1 panic'ed (Unhandled debug exception)
          Debug exception reason: Stack canary watchpoint triggered (network_event)
      */
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      isWifiConnected = false;
      wifi_reconnect_timeout = 0;
      break;
  }
}

void setupWifi() {
  espClient.setCACert(ca_cert);
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(receivedCallback);
  WiFi.onEvent(WiFiEvent);
  connectToWifi();
}
#endif
