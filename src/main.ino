#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <PubSubClient.h>
#include "declarations.h"

//#define SERIAL // Only define for debug, will break the I2C interface if enabled

#define FW_VERSION "1.1.0"

const int number_units = 10;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  // Initialize serial for debug output
  Serial.begin(115200);
  Serial.println("\n\nSplit-Flap Display Starting...");
  Serial.print("Firmware version: ");
  Serial.println(FW_VERSION);

  // Enable hardware watchdog (8 seconds)
  ESP.wdtEnable(WDTO_8S);

  Wire.begin(1, 3);
  connectToWifi();

  // Build device ID and MQTT topics after WiFi is up (needs MAC)
  buildDeviceId();
  buildMqttTopics();

  build_registration();
  otaSetup();
  mqttSetup();
}

void loop() {
  // Feed watchdog
  ESP.wdtFeed();

  checkWifiConnection();
  mqttLoopOps();
  ArduinoOTA.handle();
  processMessageQueue();  // Process queued messages when display is idle
  publishHealth();        // Publish health metrics periodically
  delay(10);
  checkIn();
}
