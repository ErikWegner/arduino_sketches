#include <Ticker.h>

Ticker ticker;

void triggerReadSensor() {
  readSensor = true  ;
}

void updateSensorValues() {
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0;
  display.sensorData(temperature, pressure);
  readSensor = false;
  sendMqttData = true;
}

void isr_init() {
  Serial.println("ISR init");
  ticker.attach(60, triggerReadSensor);
  triggerReadSensor();
}
