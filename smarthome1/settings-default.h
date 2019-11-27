// Define Your Settings
const char* ssid = "WiFi";
const char* password = "12340987";
const char* mqtt_server = "mqtt.example.org";
const char* mqtt_username = "client";
const char* mqtt_password = "passw";
const char* mqtt_clientname = "client1";
const int mqtt_port = 8883;
char* mqtt_topic = "/d/r1/sensors/temp";

// openssl x509 -noout -fingerprint -sha1 -inform pem -in server.crt
const char fingerprint[] PROGMEM = "FF 7F FE FF 59 83 CE E8 53 CB 30 D1 BF DB 08 2C F5 C9 AB 93";
