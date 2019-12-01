#include <SmingCore.h>
#include <Libraries/BME280/BME280.h>
#include <Libraries/Adafruit_SSD1306/Adafruit_SSD1306.h>

#define LED_PIN 16
#define BUFFERLENGTH_STATUS 20
#define BUFFERLENGTH_SENSORLINE 20
#include "secrets.h"
#include <ssl/private_key.h>
#include <ssl/cert.h>

Timer procTimer, reconnectMqtt;
bool state = true;
int blinkCounter = 0;
float temperature = 0.0;
float pressure = 0.0;
const Url url(MQTT_URL);
char statusText[BUFFERLENGTH_STATUS];
char sensorLine[BUFFERLENGTH_SENSORLINE];

// Forward declarations
void startMqttClient();

MqttClient mqtt;
BME280 bme;
Adafruit_SSD1306 display(-1);

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
	System.queueCallback(updateDisplay);
}

void publishFloat(const char* topic, float v) 
{
	Serial.println(_F("Publishing data..."));

	if(mqtt.getConnectionState() != eTCS_Connected) {
		Serial.println(_F("Disconnected, not sending data."));
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

void readSensors()
{
	Serial.println(_F("Reading sensors"));
	temperature = bme.GetTemperature();
	pressure = bme.GetPressure() / 100.0;
	Serial.print("T: ");
	Serial.print(temperature);
	Serial.print(" P: ");
	Serial.println(pressure);
	m_snprintf(sensorLine, BUFFERLENGTH_SENSORLINE, "%.1f hPA  %.1f 'C", pressure, temperature);
	System.queueCallback(updateDisplay);
	System.queueCallback(publishTemperature);
	System.queueCallback(publishPressure);
}

void blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
	blinkCounter++;
	if (blinkCounter >= 60) {
		blinkCounter = 0;
		System.queueCallback(readSensors);
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
	if(flag == true) {
		displayStatus("MQTT disconnected");
		Serial.println(_F("MQTT Broker Disconnected!!"));
	} else {
		displayStatus("MQTT unreach");
		Serial.println(_F("MQTT Broker Unreachable!!"));
	}

	// Restart connection attempt after few seconds
	reconnectMqtt.initializeMs(5 * 1000, startMqttClient).start(); // every 2 seconds
}

// Callback for messages, arrived from MQTT server
void onMessageReceived(String topic, String message)
{
	Serial.print(topic);
	Serial.print(":\r\n\t"); // Pretify alignment for printing
	Serial.println(message);
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
