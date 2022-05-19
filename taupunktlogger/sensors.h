
// *******  Korrekturwerte der einzelnen Sensorwerte  *******
#define Korrektur_t_1  0 // Korrekturwert Innensensor Temperatur
#define Korrektur_t_2  0 // Korrekturwert Außensensor Temperatur
#define Korrektur_h_1  2  // Korrekturwert Innensensor Luftfeuchtigkeit
#define Korrektur_h_2  -1  // Korrekturwert Außensensor Luftfeuchtigkeit
//***********************************************************
#define SCHALTmin   5.0 // minimaler Taupunktunterschied, bei dem das Relais schaltet
#define HYSTERESE   1.0 // Abstand von Ein- und Ausschaltpunkt
#define TEMP1_min  10.0 // Minimale Innentemperatur, bei der die Lüftung aktiviert wird
#define TEMP2_min -10.0 // Minimale Außentemperatur, bei der die Lüftung aktiviert wird

#define DHTPIN_1 17 // Datenleitung für den DHT-Sensor 1 (innen)
#define DHTPIN_2 16 // Datenleitung für den DHT-Sensor 2 (außen)
#define DHTTYPE_1 DHT22 // DHT 22
#define DHTTYPE_2 DHT22 // DHT 22

float tempInside = 22.4;
float tempOutside = 12.3;
float humInside = 98.1;
float humOutside = 12.1;
bool lueftung = true;

byte fehler = 0;
String sensorError = "";

#include "DHT.h"

DHT dht1(DHTPIN_1, DHTTYPE_1); //Der Innensensor wird ab jetzt mit dht1 angesprochen
DHT dht2(DHTPIN_2, DHTTYPE_2); //Der Außensensor wird ab jetzt mit dht2 angesprochen


void setupSensors() {
  Serial.println("Initialize sensors");
  dht1.begin();
  dht2.begin();
}

long lastSensorRead = 0;

float taupunkt(float t, float r) {

  float a, b;

  if (t >= 0) {
    a = 7.5;
    b = 237.3;
  } else if (t < 0) {
    a = 7.6;
    b = 240.7;
  }

  // Sättigungsdampfdruck in hPa
  float sdd = 6.1078 * pow(10, (a * t) / (b + t));

  // Dampfdruck in hPa
  float dd = sdd * (r / 100);

  // v-Parameter
  float v = log10(dd / 6.1078);

  // Taupunkttemperatur (°C)
  float tt = (b * v) / (a - v);
  return { tt };
}

void readSensors() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorRead < 10000) {
    return; // Reading
  }
  lastSensorRead = currentMillis;

  Serial.println("Read sensors");
  float h1 = dht1.readHumidity() + Korrektur_h_1;     // Innenluftfeuchtigkeit auslesen und unter „h1“ speichern
  float t1 = dht1.readTemperature() + Korrektur_t_1;  // Innentemperatur auslesen und unter „t1“ speichern
  float h2 = dht2.readHumidity() + Korrektur_h_2;     // Außenluftfeuchtigkeit auslesen und unter „h2“ speichern
  float t2 = dht2.readTemperature() + Korrektur_t_2;  // Außentemperatur auslesen und unter „t2“ speichern

  if (fehler > 0) {
    fehler--;
  }
  if (isnan(h1) || isnan(t1) || isnan(h2) || isnan(t2)) {
    fehler = 8; // number of ticks to show error
    if (isnan(h1) || isnan(t1) || h1 > 100 || h1 < 1 || t1 < -40 || t1 > 80 )  {
      Serial.println(F("Fehler beim Auslesen vom 1. Sensor!"));
      sensorError = F("Fehler Sensor 1");
    }
    if (isnan(h2) || isnan(t2) || h2 > 100 || h2 < 1 || t2 < -40 || t2  > 80)  {
      Serial.println(F("Fehler beim Auslesen vom 2. Sensor!"));
      sensorError = F("Fehler Sensor 2");
    }
  }

  if (fehler > 0) {
    return;
  }
  //**** Taupunkte errechnen********
  float Taupunkt_1 = taupunkt(t1, h1);
  float Taupunkt_2 = taupunkt(t2, h2);

  tempInside = t1;
  tempOutside = t2;
  humInside = min(99.9f, h1);
  humOutside = min(99.9f, h2);

  // Werteausgabe auf Serial Monitor
  Serial.print(F("Sensor-1: " ));
  Serial.print(F("Luftfeuchtigkeit: "));
  Serial.print(h1);
  Serial.print(F("%  Temperatur: "));
  Serial.print(t1);
  Serial.print(F("°C  "));
  Serial.print(F("  Taupunkt: "));
  Serial.print(Taupunkt_1);
  Serial.println(F("°C  "));

  Serial.print("Sensor-2: " );
  Serial.print(F("Luftfeuchtigkeit: "));
  Serial.print(h2);
  Serial.print(F("%  Temperatur: "));
  Serial.print(t2);
  Serial.print(F("°C "));
  Serial.print(F("   Taupunkt: "));
  Serial.print(Taupunkt_2);
  Serial.println(F("°C  "));

  Serial.println();


  float DeltaTP = Taupunkt_1 - Taupunkt_2;

  if (DeltaTP > (SCHALTmin + HYSTERESE)) lueftung = true;
  if (DeltaTP < (SCHALTmin)) lueftung = false;
  if (t1 < TEMP1_min) lueftung = false;
  if (t2 < TEMP2_min) lueftung = false;
}
