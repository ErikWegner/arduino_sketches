#ifndef COMMUNICATION
#define COMMUNICATION

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "secret.h"
#include "motorsteuerung.h"

TaskHandle_t WifiMqttTask;

#define TASK_DELAY 50
#define DEFAULT_TIMEOUT (5 * 1000 / TASK_DELAY)
bool isWifiConnected = false;
signed char wifi_reconnect_timeout = 0;
signed char mqtt_reconnect_timeout = 0;
long lastRun = 0;

// Set this pointers to a command to tell the other task what to do
MotorCommand *mqttCmdLinks = nullptr;
MotorCommand *mqttCmdRechts = nullptr;
MotorCommand mqttCmdStop = MotorCommands::STOP;
MotorCommand mqttCmdUp = MotorCommands::MOVE_UP;
MotorCommand mqttCmdDown = MotorCommands::MOVE_DOWN;

WiFiClientSecure espClient;
PubSubClient client(espClient);

void connectToWifi();
void connectToMqtt();

const char leftup[]    = "left-up";
const char leftdown[]  = "left-down";
const char leftstop[]  = "left-stop";
const char rightup[]   = "right-up";
const char rightdown[] = "right-down";
const char rightstop[] = "right-stop";

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received: ");
  Serial.println(topic);

  Serial.print("payload: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (length == 10) {
    if (0 == memcmp(rightdown, payload, length)) {
      Serial.println("MQTT-Command: right down");
      mqttCmdRechts = &mqttCmdDown;
    } else if (0 == memcmp(rightstop, payload, length)) {
      Serial.println("MQTT-Command: right stop");
      mqttCmdRechts = &mqttCmdStop;
    }
  } else {
    if (length == 8) {
      if (0 == memcmp(rightup, payload, length)) {
        Serial.println("MQTT-Command: right up");
        mqttCmdRechts = &mqttCmdUp;
      }
    } else {
      if (length == 9) {
        if (0 == memcmp(leftdown, payload, length)) {
          Serial.println("MQTT-Command: left down");
          mqttCmdLinks = &mqttCmdDown;
        } else if (0 == memcmp(leftstop, payload, length)) {
          Serial.println("MQTT-Command: left stop");
          mqttCmdLinks = &mqttCmdStop;
        }
      } else {
        if (length == 7) {
          if (0 == memcmp(leftup, payload, length)) {
            Serial.println("MQTT-Command: left up");
            mqttCmdLinks = &mqttCmdUp;
          }
        }
      } // length == 9
    }// length == 8
  } // length == 10
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

  if (mqtt_reconnect_timeout > 0)
  {
    Serial.print("MQTT reconnect timeout = ");
    Serial.println(mqtt_reconnect_timeout);
    mqtt_reconnect_timeout--;
    return;
  }

  mqtt_reconnect_timeout = DEFAULT_TIMEOUT;
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

// https://randomnerdtutorials.com/esp32-dual-core-arduino-ide/
void ensureWifiAndMqttTask(void * pvParameters) {
  Serial.print("ensureWifiAndMqtt running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    ensureWifiAndMqtt();
    delay(TASK_DELAY);
  }
}

void setupWifi() {
  espClient.setCACert(ca_cert);
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(receivedCallback);
  WiFi.onEvent(WiFiEvent);
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

#endif
