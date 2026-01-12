#include "declarations.h"
#include <WiFiManager.h>
#include <FS.h>
#include <LittleFS.h>

// Config file path
const char* CONFIG_FILE = "/config.json";

// Config values (will be loaded from flash or set via portal)
char cfg_mqtt_server[64] = "";
char cfg_mqtt_port[6] = "1883";
char cfg_mqtt_user[32] = "";
char cfg_mqtt_pass[32] = "";
char cfg_device_name[32] = "flaps-01";

// Flag for saving config
bool shouldSaveConfig = false;

// Callback for WiFiManager when config needs saving
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Load config from LittleFS
bool loadConfig() {
  Serial.println("Mounting LittleFS...");
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
    return false;
  }

  if (!LittleFS.exists(CONFIG_FILE)) {
    Serial.println("Config file not found");
    return false;
  }

  File configFile = LittleFS.open(CONFIG_FILE, "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  configFile.close();

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }

  strlcpy(cfg_mqtt_server, doc["mqtt_server"] | "", sizeof(cfg_mqtt_server));
  strlcpy(cfg_mqtt_port, doc["mqtt_port"] | "1883", sizeof(cfg_mqtt_port));
  strlcpy(cfg_mqtt_user, doc["mqtt_user"] | "", sizeof(cfg_mqtt_user));
  strlcpy(cfg_mqtt_pass, doc["mqtt_pass"] | "", sizeof(cfg_mqtt_pass));
  strlcpy(cfg_device_name, doc["device_name"] | "flaps-01", sizeof(cfg_device_name));

  Serial.println("Config loaded successfully");
  Serial.print("MQTT Server: ");
  Serial.println(cfg_mqtt_server);
  return true;
}

// Save config to LittleFS
bool saveConfig() {
  Serial.println("Saving config...");

  StaticJsonDocument<512> doc;
  doc["mqtt_server"] = cfg_mqtt_server;
  doc["mqtt_port"] = cfg_mqtt_port;
  doc["mqtt_user"] = cfg_mqtt_user;
  doc["mqtt_pass"] = cfg_mqtt_pass;
  doc["device_name"] = cfg_device_name;

  File configFile = LittleFS.open(CONFIG_FILE, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  configFile.close();
  Serial.println("Config saved");
  return true;
}

void connectToWifi() {
  Serial.println("\n\nStarting WiFi setup...");

  // Initialize LittleFS and load config
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed, formatting...");
    LittleFS.format();
    LittleFS.begin();
  }
  loadConfig();

  // Create WiFiManager instance
  WiFiManager wifiManager;

  // Set config save callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Set timeout for portal (3 minutes)
  wifiManager.setConfigPortalTimeout(180);

  // Custom parameters for MQTT config
  WiFiManagerParameter custom_mqtt_server("mqtt_server", "MQTT Server", cfg_mqtt_server, 64);
  WiFiManagerParameter custom_mqtt_port("mqtt_port", "MQTT Port", cfg_mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_user("mqtt_user", "MQTT User (optional)", cfg_mqtt_user, 32);
  WiFiManagerParameter custom_mqtt_pass("mqtt_pass", "MQTT Password (optional)", cfg_mqtt_pass, 32);
  WiFiManagerParameter custom_device_name("device_name", "Device Name", cfg_device_name, 32);

  // Add custom parameters
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_user);
  wifiManager.addParameter(&custom_mqtt_pass);
  wifiManager.addParameter(&custom_device_name);

  // Build AP name with chip ID for uniqueness
  String apName = "Flaps-Setup-" + String(ESP.getChipId(), HEX);

  // Try to connect, if fails start config portal
  Serial.println("Attempting WiFi connection...");
  if (!wifiManager.autoConnect(apName.c_str(), "flapsetup")) {
    Serial.println("Failed to connect and hit timeout");
    Serial.println("Restarting...");
    delay(3000);
    ESP.restart();
  }

  // Connected!
  Serial.println("WiFi connected!");

  // Read updated parameters
  strlcpy(cfg_mqtt_server, custom_mqtt_server.getValue(), sizeof(cfg_mqtt_server));
  strlcpy(cfg_mqtt_port, custom_mqtt_port.getValue(), sizeof(cfg_mqtt_port));
  strlcpy(cfg_mqtt_user, custom_mqtt_user.getValue(), sizeof(cfg_mqtt_user));
  strlcpy(cfg_mqtt_pass, custom_mqtt_pass.getValue(), sizeof(cfg_mqtt_pass));
  strlcpy(cfg_device_name, custom_device_name.getValue(), sizeof(cfg_device_name));

  // Save config if needed
  if (shouldSaveConfig) {
    saveConfig();
  }

  // Update global variables
  ip_address = WiFi.localIP();
  sprintf(ip_char, "%d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);

  Serial.print("IP Address: ");
  Serial.println(ip_char);
  Serial.print("MQTT Server: ");
  Serial.println(cfg_mqtt_server);
}

// Call this in main loop to check WiFi status
void checkWifiConnection() {
  static unsigned long lastWifiCheck = 0;
  unsigned long now = millis();

  // Check every 10 seconds
  if (now - lastWifiCheck < 10000) {
    return;
  }
  lastWifiCheck = now;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, reconnecting...");
    WiFi.reconnect();
  }
}

// Reset WiFi settings and config (call to force reconfiguration)
void resetWifiConfig() {
  Serial.println("Resetting WiFi config...");
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  LittleFS.remove(CONFIG_FILE);
  Serial.println("Config reset. Restarting...");
  delay(1000);
  ESP.restart();
}
