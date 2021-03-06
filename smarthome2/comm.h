#ifndef COMMUNICATION
#define COMMUNICATION

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "secret.h"
#include "motorsteuerung.h"
#include "ota.h"

portMUX_TYPE publishPositionMutex = portMUX_INITIALIZER_UNLOCKED;
bool publishPositionL = false;
bool publishPositionR = false;
int positionL = 0;
int positionR = 0;
bool sendMqttData = false;
bool syncRequest = false;
char temperatureMqtt[8];
char pressureMqtt[8];

TaskHandle_t WifiMqttTask;

#define TASK_DELAY 50
#define DEFAULT_TIMEOUT (5 * 1000 / TASK_DELAY)
bool isWifiConnected = false;
bool startOTA = true;
char wifiText[5] = {0};
char mqttText[5] = {0};
signed char wifi_reconnect_timeout = 0;
signed char mqtt_reconnect_timeout = 0;
char mqtt_payload[20];
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
const char syncCmd[]   = "sync";

void receivedCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print(F("Message received: "));
  Serial.println(topic);

  Serial.print(F("payload: "));
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (length == 10) {
    if (0 == memcmp(rightdown, payload, length)) {
      Serial.println(F("MQTT-Command: right down"));
      mqttCmdRechts = &mqttCmdDown;
    } else if (0 == memcmp(rightstop, payload, length)) {
      Serial.println(F("MQTT-Command: right stop"));
      mqttCmdRechts = &mqttCmdStop;
    }
  } else {
    if (length == 8) {
      if (0 == memcmp(rightup, payload, length)) {
        Serial.println(F("MQTT-Command: right up"));
        mqttCmdRechts = &mqttCmdUp;
      }
    } else {
      if (length == 9) {
        if (0 == memcmp(leftdown, payload, length)) {
          Serial.println(F("MQTT-Command: left down"));
          mqttCmdLinks = &mqttCmdDown;
        } else if (0 == memcmp(leftstop, payload, length)) {
          Serial.println(F("MQTT-Command: left stop"));
          mqttCmdLinks = &mqttCmdStop;
        }
      } else {
        if (length == 7) {
          if (0 == memcmp(leftup, payload, length)) {
            Serial.println(F("MQTT-Command: left up"));
            mqttCmdLinks = &mqttCmdUp;
          }
        } else {
          if (length == 4) {
            if (0 == memcmp(syncCmd, payload, length)) {
              Serial.println(F("MQTT-Command: sync"));
              syncRequest = true;
            }
          }
        } // length == 7
      } // length == 9
    }// length == 8
  } // length == 10
}

void publishInt(const char* topic, int v)
{
#define PUBLISH_INT_BUFFER_LENGTH 4
  char svalue[PUBLISH_INT_BUFFER_LENGTH];
  snprintf(svalue, PUBLISH_INT_BUFFER_LENGTH, "%i", v);
  client.publish(topic, svalue, true);
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
    return;
  }

  if (mqtt_reconnect_timeout > 0)
  {
    Serial.print(F("MQTT reconnect timeout = "));
    Serial.println(mqtt_reconnect_timeout);
    mqtt_reconnect_timeout--;
    return;
  }

  mqtt_reconnect_timeout = DEFAULT_TIMEOUT;
  connectToMqtt();
}

void connectToMqtt() {
  Serial.println(F("Connecting to MQTT broker..."));
  //strcpy_P(mqttText, (char *)pgm_read_word(&mqttOffline));
  strcpy(mqttText, "OFFL");

  if (client.connect(MQTT_CLIENTNAME, MQTT_USERNAME, MQTT_PASSWORD, "/d/r1/alive", 0, 1, "off", true)) {
    Serial.println(F("mqtt connected"));
    client.publish("/d/r1/alive", "on", true);
    client.subscribe((const char*)F("/d/r1/cmd/move"));
    client.subscribe((const char*)F("/d/r1/alive"));
    //strcpy_P(mqttText, (char *)pgm_read_word(&mqttOnline));
    strcpy(mqttText, "MQTT");
  } else {
    Serial.print(F("failed, status code ="));
    Serial.print(client.state());
    //strcpy_P(mqttText, (char *)pgm_read_word(&mqttFail));
    strcpy(mqttText, "FAIL");
  }
}

void connectToWifi() {
  if (isWifiConnected) {
    return;
  }

  Serial.println("Connecting to Wi-Fi...");
  //strcpy_P(wifiText, (char *)pgm_read_word(&wifiOffline));
  strcpy(wifiText, "OFFL");
  //strcpy_P(mqttText, (char *)pgm_read_word(&mqttOffline));
  strcpy(mqttText, "OFFL");
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
      strcpy(wifiText, "Wifi");
      /*
          Calling connectToMqtt() here gives a
          Guru Meditation Error: Core  1 panic'ed (Unhandled debug exception)
          Debug exception reason: Stack canary watchpoint triggered (network_event)
      */
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      //strcpy_P(wifiText, (char *)pgm_read_word(&wifiDisconnected));
      strcpy(wifiText, "Disc");
      isWifiConnected = false;
      wifi_reconnect_timeout = DEFAULT_TIMEOUT;
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
    if (publishPositionL || publishPositionR || syncRequest) {
      if (publishPositionL || syncRequest) {
        publishInt("/d/r1/position/left", positionL);
      }
      if (publishPositionR || syncRequest) {
        publishInt("/d/r1/position/right", positionR);
      }
      portENTER_CRITICAL_ISR(&publishPositionMutex);
      publishPositionL = false;
      publishPositionR = false;
      portEXIT_CRITICAL_ISR(&publishPositionMutex);
    }
    if (sendMqttData || syncRequest) {
      sendMqttData = false;
      client.publish((const char*)F("/d/r1/sensors/temp"), temperatureMqtt);
      client.publish((const char*)F("/d/r1/sensors/pressure"), pressureMqtt);
    }
    if (syncRequest) {
      client.publish("/d/r1/alive", "on", true);
      syncRequest = false;
    }
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
