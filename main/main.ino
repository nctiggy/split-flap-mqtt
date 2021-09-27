#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <PubSubClient.h>

//#define SERIAL // Only define for debug, will break the I2C interface if enabled

const int number_units = 10;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Wire.begin(1, 3);
  connectToWifi();
  build_registration();
  otaSetup();
  mqttSetup();
}

void loop() {
  mqttLoopOps();
  ArduinoOTA.handle();
  delay(10);
  checkIn();
}
