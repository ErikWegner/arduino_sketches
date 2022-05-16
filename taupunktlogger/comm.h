#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "secret.h"
#include "ota.h"

bool sendMqttData = false;
bool syncRequest = false;

TaskHandle_t WifiMqttTask;

#define TASK_DELAY 50
#define DEFAULT_TIMEOUT (5 * 1000 / TASK_DELAY)
bool isWifiConnected = false;
bool startOTA = true;
signed char wifi_reconnect_timeout = 0;
signed char mqtt_reconnect_timeout = 0;

WiFiClientSecure espClient;
PubSubClient client(espClient);

void WiFiEventHandler(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      isWifiConnected = true;
      // TODO: LCD output
      /*
          Calling connectToMqtt() here gives a
          Guru Meditation Error: Core  1 panic'ed (Unhandled debug exception)
          Debug exception reason: Stack canary watchpoint triggered (network_event)
      */
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      // TODO: LCD output
      isWifiConnected = false;
      wifi_reconnect_timeout = DEFAULT_TIMEOUT;
      break;
  }
}

void connectToWifi() {
  if (isWifiConnected) {
    return;
  }

  Serial.println("Connecting to Wi-Fi...");
  //TODO: output LCD
  WiFi.begin(WIFI_SSID, WIFI_PWD);
}

void connectToMqtt() {
  Serial.println(F("Connecting to MQTT broker..."));
  // TODO: LCD output

  if (client.connect(MQTT_CLIENTNAME, MQTT_USERNAME, MQTT_PASSWORD, "/d/r4/alive", 0, 1, "off", true)) {
    Serial.println(F("mqtt connected"));
    client.publish("/d/r4/alive", "on", true);
    // TODO: LCD output
  } else {
    Serial.print(F("failed, status code ="));
    Serial.print(client.state());
    // TODO: LCD output
  }
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

  if (startOTA) {
    startOTA = false;
    setupOTA();
  }
  handleOTA();

  if (client.connected()) {
    client.loop();
    
  } else if (mqtt_reconnect_timeout > 0)
  {
//    Serial.print(F("MQTT reconnect timeout = "));
//    Serial.println(mqtt_reconnect_timeout);
    mqtt_reconnect_timeout--;
  } else {
    mqtt_reconnect_timeout = DEFAULT_TIMEOUT;
    connectToMqtt();
  }
}

// https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
void ensureWifiAndMqttTask(void * pvParameters) {
  Serial.print("ensureWifiAndMqtt running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    ensureWifiAndMqtt();
    delay(TASK_DELAY);
    if (sendMqttData || syncRequest) {
      sendMqttData = false;
      client.publish((const char*)F("/d/r4/sensors/inside"), ""); // TODO: values
      client.publish((const char*)F("/d/r4/sensors/outside"), ""); // TODO: values
    }
    if (syncRequest) {
      client.publish("/d/r4/alive", "on", true);
      syncRequest = false;
    }
  }
}

void setupWifi() {
  espClient.setCACert(ca_cert);
  client.setServer(MQTT_SERVER, MQTT_PORT);
  WiFi.onEvent(WiFiEventHandler);
  connectToWifi();
  xTaskCreatePinnedToCore(
    ensureWifiAndMqttTask,    /* Task function. */
    "WiFiMqtt",               /* name of task. */
    10000,                    /* Stack size of task */
    NULL,                     /* parameter of the task */
    1,                        /* priority of the task */
    &WifiMqttTask,            /* Task handle to keep track of created task */
    0);                       /* pin task to core 0 */
}
