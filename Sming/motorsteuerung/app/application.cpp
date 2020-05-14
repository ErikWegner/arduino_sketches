#include <SmingCore.h>

#define LED_PIN 16
#define MOTORLINKSPOWER 5 // D1, GPIO5
#define MOTORLINKSRICHTUNG 4 // D2, GPIO4
#define BUFFERLENGTH_STATUS 20
#define BUFFERLENGTH_SENSORLINE 20

#include "motor.h"

Timer procTimer, reconnectMqtt;
bool state = true;
bool startingMqtt = false;
int blinkCounter = 0;
float temperature = 0.0;
float pressure = 0.0;
char statusText[BUFFERLENGTH_STATUS];
char sensorLine[BUFFERLENGTH_SENSORLINE];



Motorsteuerung motorLinks(MOTORLINKSPOWER, MOTORLINKSRICHTUNG);

void displayStatus(const char* text) {
	Serial.println(text);
	strncpy(statusText, text, BUFFERLENGTH_STATUS);
}

void publishFloat(const char* topic, float v) 
{
	Serial.println(_F("Publishing data..."));

	#define PUBLISH_FLOAT_BUFFER_LENGTH 10
	char svalue[PUBLISH_FLOAT_BUFFER_LENGTH];
	m_snprintf(svalue, PUBLISH_FLOAT_BUFFER_LENGTH, "%.1f", v);
	Serial.print(_F("Value is "));
	Serial.println(svalue);
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
}

void blink()
{
	state = !state;
	blinkCounter++;
	Serial.print("Blink ");
	Serial.println(blinkCounter);
	if (blinkCounter == 3) {
		Serial.println("Command: MOVE UP");
		motorLinks.setCommand(MotorCommands::MOVE_UP);
	}
	if (blinkCounter == 8) {
		Serial.println("Command: MOVE DOWN");
		motorLinks.setCommand(MotorCommands::MOVE_DOWN);
	}
	motorLinks.tick();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println(_F("Sming. Let's do smart things!"));

	pinMode(LED_PIN, OUTPUT);
	procTimer.initializeMs(1000, blink).start();

	pinMode(MOTORLINKSPOWER, OUTPUT);
	pinMode(MOTORLINKSRICHTUNG, OUTPUT);

}
