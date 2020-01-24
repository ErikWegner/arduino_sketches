// pip install --user esptool
#include <Adafruit_BMP280.h>

#include "pinlayout.h"
#include "motorsteuerung.h"
#include "comm.h"

volatile bool readButtons = false;
volatile bool ledState = false;
volatile bool motorTick = false;

Adafruit_BMP280 bmp;
bool bmpInitialized = false;
#define DEFAULT_SENSOR_WAIT 60
int readSensorWait = DEFAULT_SENSOR_WAIT;
uint16_t sensorReset = 0;
float temperature = 0.0;
float pressure = 0.0;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

Motorsteuerung motorLinks(MOTORLINKSPOWER, MOTORLINKSRICHTUNG);
Motorsteuerung motorRechts(MOTORRECHTSPOWER, MOTORRECHTSRICHTUNG);

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  ledState = !ledState;
  motorTick = true;
  portEXIT_CRITICAL_ISR(&timerMux);
  digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
}

void setupTimer() {
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Booting"));
  setupGPIO();
  delay(500);
  bmpInitialized = bmp.begin();
  if (!bmpInitialized) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
  } else {
    /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }
  setupDisplay();

  Serial.println(F("Starting timer"));
  setupTimer();
  setupWifi();
}

void loop() {
  if (mqttCmdRechts != nullptr) {
    motorRechts.setCommand(*mqttCmdRechts);
    mqttCmdRechts = nullptr;
    motorTick = true;
  }
  if (mqttCmdLinks != nullptr) {
    motorLinks.setCommand(*mqttCmdLinks);
    mqttCmdLinks = nullptr;
    motorTick = true;
  }

  checkButtons(&motorLinks, &motorRechts);
  updateDisplay(&motorLinks, &motorRechts);
  if (motorTick) {
    motorLinks.tick();
    motorRechts.tick();
    motorTick = false;
    bool publishLeft = motorLinks.publishPosition();
    bool publishRight = motorRechts.publishPosition();
    if (publishLeft || publishRight) {
      positionL = motorLinks.estimatedPosition();
      positionR = motorRechts.estimatedPosition();
      portENTER_CRITICAL_ISR(&publishPositionMutex);
      publishPositionL = publishLeft;
      publishPositionR = publishRight;
      portEXIT_CRITICAL_ISR(&publishPositionMutex);
    }

    sensorReset += 1;
    readSensorWait--;
    if (readSensorWait < 1) {
      readSensorWait = DEFAULT_SENSOR_WAIT;
      // https://github.com/adafruit/Adafruit_BMP280_Library/pull/33
      /*
       * https://github.com/adafruit/Adafruit_BMP280_Library/pull/33
       * If sensor.getStatus() returns 0xFF -> call reset() and begin()
       * If not initialized and 5 minutes past -> reset()+begin()
       */
      if ((!bmpInitialized && sensorReset > 300) || (bmp.getStatus() == 0xFF)) {
        // Reset
        bmp.reset();
        delay(150);
        sensorReset = 0;
        bmpInitialized = bmp.begin();
      }
      if (bmpInitialized) {
        temperature = bmp.readTemperature();
        pressure = bmp.readPressure() / 100.0;
        dtostrf(temperature, 3, 2, temperatureMqtt);
        dtostrf(pressure, 4, 2, pressureMqtt);
        sendMqttData = true;
      }
    }
  }
}
