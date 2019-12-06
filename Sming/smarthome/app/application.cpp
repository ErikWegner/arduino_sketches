#include <SmingCore.h>
#include <Libraries/BME280/BME280.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

// SDA <-> D4
// SCL <-> D3
#define LED_PIN 16
#define MOTORLINKSPOWER 5 // D1, GPIO5
#define MOTORLINKSRICHTUNG 4 // D2, GPIO4
#define MOTORRECHTSPOWER 14 // D5, GPIO14
#define MOTORRECHTSRICHTUNG 12 // D6, GPIO12
#define BUFFERLENGTH_STATUS 20
#define BUFFERLENGTH_SENSORLINE 20
#include "secrets.h"
#include <ssl/private_key.h>
#include <ssl/cert.h>
#include "motor.h"

Timer procTimer, reconnectMqtt;
bool state = true;
bool startingMqtt = false;
int blinkCounter = 0;
float temperature = 0.0;
float pressure = 0.0;
const Url url(MQTT_URL);
char statusText[BUFFERLENGTH_STATUS];
char sensorLine[BUFFERLENGTH_SENSORLINE];



// Forward declarations
void startMqttClient();
void reconnectMQTT(bool flag);

MqttClient mqtt;
NtpClient* ntpClient;
BME280 bme;
Adafruit_SSD1306 display(-1);
Motorsteuerung motorLinks(MOTORLINKSPOWER, MOTORLINKSRICHTUNG);
Motorsteuerung motorRechts(MOTORRECHTSPOWER, MOTORRECHTSRICHTUNG);

void updateDisplay() {
	display.clearDisplay();
	display.setCursor(0,0);
	display.println(statusText);
	display.println(sensorLine);
	display.display();
}

void displayStatus(const char* text) {
	Serial.println(text);
	strncpy(statusText, text, BUFFERLENGTH_STATUS);
	updateDisplay();
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
	mqtt.publish(topic, svalue);
	Serial.println(_F("Data send"));
}

void publishFloat(const char* topic, float v) 
{
	Serial.println(_F("Publishing data..."));

	if(mqtt.getConnectionState() != eTCS_Connected) {
		Serial.println(_F("Disconnected, not sending data."));
		reconnectMQTT(true);
		return;
	}

	#define PUBLISH_FLOAT_BUFFER_LENGTH 10
	char svalue[PUBLISH_FLOAT_BUFFER_LENGTH];
	m_snprintf(svalue, PUBLISH_FLOAT_BUFFER_LENGTH, "%.1f", v);
	Serial.print(_F("Value is "));
	Serial.println(svalue);
	mqtt.publish(topic, svalue);
	Serial.println(_F("Data send"));
}

void publishTemperature() 
{
	publishFloat("/d/r1/sensors/temp", temperature);
}

void publishPressure() 
{
    publishFloat("/d/r1/sensors/pressure", pressure);
}

void updateSensorLine()
{
	m_snprintf(sensorLine, BUFFERLENGTH_SENSORLINE, "%.1f hPA  %.1f 'C", pressure, temperature);
	updateDisplay();
}

void readTemperatur()
{
	temperature = bme.GetTemperature();
	Serial.print("T: ");
	Serial.println(temperature);
	System.queueCallback(publishTemperature);
}

void readPressure()
{
	pressure = bme.GetPressure() / 100.0;
	Serial.print(temperature);
	Serial.print(" P: ");
	Serial.println(pressure);
	System.queueCallback(publishPressure);
}

void readSensors()
{
	Serial.println(_F("Reading sensors"));
	readTemperatur();
	readPressure();
	updateSensorLine();
}

void blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
	blinkCounter++;
	//Serial.println(SystemClock.getSystemTimeString());
	if (blinkCounter >= 60) {
		blinkCounter = 0;
		System.queueCallback(readSensors);
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

void initSensor() {
	bme = BME280();
	if (bme.EnsureConnected()) {
		bme.SoftReset();
		bme.Initialize();
		System.queueCallback(readSensors);
	} else {
		displayStatus("Sensor not found");
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
	if (message == _F("right-up")) {
		motorRechts.setCommand(MotorCommands::MOVE_UP);
	}
	if (message == _F("right-down")) {
		motorRechts.setCommand(MotorCommands::MOVE_DOWN);
	}
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
	ntpClient = new NtpClient("pool.ntp.org", 30);
	startMqttClient();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println(_F("Sming. Let's do smart things!"));

	Wire.begin();

	pinMode(LED_PIN, OUTPUT);
	procTimer.initializeMs(1000, blink).start();

	display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
	display.clearDisplay();
	display.setTextColor(WHITE);
	display.setTextSize(1);
	display.setCursor(0, 0);
	display.println("Booting...");
	display.display();
	display.dim(true);

	initSensor();

	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));

	// Set callback that should be triggered when we have assigned IP
	WifiEvents.onStationGotIP(connectOk);
}
