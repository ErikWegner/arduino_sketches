#include <SmingCore.h>

#define LED_PIN 16
#define MOTORLINKSPOWER 5 // D1, GPIO5
#define MOTORLINKSRICHTUNG 4 // D2, GPIO4
#define MOTORRECHTSPOWER 14 // D5, GPIO14
#define MOTORRECHTSRICHTUNG 12 // D6, GPIO12
#define BUTTONLINKSHOCH 0 // D3, GPIO0
#define BUTTONLINKSRUNTER 2 // D4, GPIO2
#define BUTTONRECHTSHOCH 13 // D7, GPIO13
#define BUTTONRECHTSRUNTER 15 // D8, GPIO15

#include "secrets.h"
#include <ssl/private_key.h>
#include <ssl/cert.h>
#include "motor.h"

Timer procTimer, reconnectMqtt;
bool state = true;
bool startingMqtt = false;
int blinkCounter = 0;
const Url url(MQTT_URL);

bool leftUp = false;
bool leftDown = false;
bool rightUp = false;
bool rightDown = false;
volatile bool buttonInterruptTriggered = false;

// Forward declarations
void startMqttClient();
void reconnectMQTT(bool flag);
void clearInterruptStates();

MqttClient mqtt;
Motorsteuerung motorLinks(MOTORLINKSPOWER, MOTORLINKSRICHTUNG);
Motorsteuerung motorRechts(MOTORRECHTSPOWER, MOTORRECHTSRICHTUNG);

void displayStatus(const char* text) {
	Serial.println(text);
}

void publishInt(const char* topic, int v)
{
	Serial.println(_F("Publishing data..."));

	if(mqtt.getConnectionState() != eTCS_Connected) {
		Serial.println(_F("Disconnected, not sending data."));
		reconnectMQTT(true);
		return;
	}

	#define PUBLISH_INT_BUFFER_LENGTH 4
	char svalue[PUBLISH_INT_BUFFER_LENGTH];
	m_snprintf(svalue, PUBLISH_INT_BUFFER_LENGTH, "%i", v);
	Serial.print(_F("Value is "));
	Serial.println(svalue);
	mqtt.publish(topic, svalue, MQTT_FLAG_RETAINED);
	Serial.println(_F("Data send"));
}

void readButtons() {
	clearInterruptStates();

	bool btnLinksHochActive = digitalRead(BUTTONLINKSHOCH) == HIGH;
	bool btnLinksRunterActive = digitalRead(BUTTONLINKSRUNTER) == HIGH;
	bool btnRechtsHochActive = digitalRead(BUTTONRECHTSHOCH) == HIGH;
	bool btnRechtsRunterActive = digitalRead(BUTTONRECHTSRUNTER) == HIGH;

	if (btnLinksHochActive && leftUp == false) {
		Serial.println("Taster: Links hoch");
		motorLinks.setCommand(MotorCommands::MOVE_UP);
	}
	if (btnLinksRunterActive && leftDown == false) {
		Serial.println("Taster: Links runter");
		motorLinks.setCommand(MotorCommands::MOVE_DOWN);
	}
	leftUp = btnLinksHochActive;
	leftDown = btnLinksRunterActive;
	if (leftUp == false && leftDown == false) {
		Serial.println("Taster: Links stops");
		motorLinks.setCommand(MotorCommands::STOP);
	}

	if (btnRechtsHochActive && rightUp == false) {
		Serial.println("Taster: Rechts hoch");
		motorRechts.setCommand(MotorCommands::MOVE_UP);
	}
	if (btnRechtsRunterActive && rightDown == false) {
		Serial.println("Taster: Rechts runter");
		motorRechts.setCommand(MotorCommands::MOVE_DOWN);
	}
	rightUp = btnRechtsHochActive;
	rightDown = btnRechtsRunterActive;
	if (rightUp == false && rightDown == false) {
		Serial.println("Taster: Rechts stops");
		motorRechts.setCommand(MotorCommands::STOP);
	}
}

void blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
	blinkCounter++;
	//Serial.println(SystemClock.getSystemTimeString());
	if (blinkCounter >= 60) {
		blinkCounter = 0;
	}
	if (buttonInterruptTriggered) {
		readButtons();
	}
	motorLinks.tick();
	if (motorLinks.publishPosition()) {
		publishInt("/d/r1/position/left", motorLinks.estimatedPosition());
	}
	motorRechts.tick();
	if (motorRechts.publishPosition()) {
		publishInt("/d/r1/position/right", motorRechts.estimatedPosition());
	}
}

// Check for MQTT Disconnection
void checkMQTTDisconnect(TcpClient& client, bool flag)
{
	Serial.println(_F("MQcheckMQTTDisconnect"));
	reconnectMQTT(true);
}

void reconnectMQTT(bool flag)
{
	if(flag == true) {
		displayStatus("MQTT disconnected");
		Serial.println(_F("MQTT Broker Disconnected!!"));
	} else {
		displayStatus("MQTT unreach");
		Serial.println(_F("MQTT Broker Unreachable!!"));
	}

	if (reconnectMqtt.isStarted()) {
		Serial.println(_F("reconnectMqtt.isStarted"));
		return;
	}

	// Restart connection attempt after few seconds
	reconnectMqtt.initializeMs(5 * 1000, startMqttClient).start(); // every 5 seconds
}

// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Pretify alignment for printing
	Serial.println(message);
	if (message == _F("left-up")) {
		motorLinks.setCommand(MotorCommands::MOVE_UP);
	}
	if (message == _F("left-down")) {
		motorLinks.setCommand(MotorCommands::MOVE_DOWN);
	}
	if (message == _F("left-stop")) {
		motorLinks.setCommand(MotorCommands::STOP);
	}
	if (message == _F("right-up")) {
		motorRechts.setCommand(MotorCommands::MOVE_UP);
	}
	if (message == _F("right-down")) {
		motorRechts.setCommand(MotorCommands::MOVE_DOWN);
	}
	if (message == _F("right-stop")) {
		motorRechts.setCommand(MotorCommands::STOP);
	}
}

void clearInterruptStates()
{
	buttonInterruptTriggered = false;
}

void buttonInterruptHandler() { buttonInterruptTriggered = true; }

void attachInterrupts()
{
	pinMode(BUTTONLINKSHOCH, INPUT_PULLUP);
	pinMode(BUTTONLINKSRUNTER, INPUT_PULLUP);
	pinMode(BUTTONRECHTSHOCH, INPUT_PULLUP);
	pinMode(BUTTONRECHTSRUNTER, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(BUTTONLINKSRUNTER), buttonInterruptHandler, CHANGE);
	attachInterrupt(digitalPinToInterrupt(BUTTONLINKSHOCH), buttonInterruptHandler, CHANGE);
	attachInterrupt(digitalPinToInterrupt(BUTTONRECHTSRUNTER), buttonInterruptHandler, CHANGE);
	attachInterrupt(digitalPinToInterrupt(BUTTONRECHTSHOCH), buttonInterruptHandler, CHANGE);
}

void startMqttClient()
{
	reconnectMqtt.stop();
	mqtt.setCompleteDelegate(checkMQTTDisconnect);
	mqtt.setCallback(onMessageReceived);

	mqtt.addSslOptions(SSL_SERVER_VERIFY_LATER);

	mqtt.setSslKeyCert(default_private_key, default_private_key_len, default_certificate, default_certificate_len,
					   nullptr,
					   /*freeAfterHandshake*/ false);

	displayStatus("MQTT connect...");
	Serial.print("Connecting to \t");
	Serial.println(url);
	mqtt.connect(url, MQTT_CLIENTNAME);
	displayStatus("OK");
	mqtt.subscribe("/d/r1/cmd/#");
}

// Will be called when WiFi station was connected to AP
void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	Serial.print(_F("WiFi "));
	Serial.println(ip);
	displayStatus(("WiFi"));
	startMqttClient();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println(_F("Sming. Let's do smart things!"));

	attachInterrupts();

	Wire.begin();

	pinMode(LED_PIN, OUTPUT);
	procTimer.initializeMs(1000, blink).start();

	// Station - WiFi client
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));
	WifiStation.enable(true);
	// Set callback that should be triggered when we have assigned IP
	WifiEvents.onStationGotIP(connectOk);
	WifiAccessPoint.enable(false);
}
