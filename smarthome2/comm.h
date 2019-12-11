#ifndef COMMUNICATION
#define COMMUNICATION

#include <WiFi.h>

#include "secret.h"

bool isWifiConnected = false;

void connectToWifi() {
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
      //connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      isWifiConnected = false;
      //stopReconnectMqttTimer
      //startReconnectWifiTimer
      break;
  }
}

void setupWifi() {
  WiFi.onEvent(WiFiEvent);
  connectToWifi();
}
#endif
