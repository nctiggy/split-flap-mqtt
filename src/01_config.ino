#include "declarations.h"

// MQTT topic base - device ID will be appended
const char* mqtt_base = "/flaps";

// Dynamic topics (built at runtime with device ID)
char mqtt_command[64];      // /flaps/{device}/command
char mqtt_status_topic[64]; // /flaps/{device}/status
char mqtt_health[64];       // /flaps/{device}/health
char mqtt_registration_topic[64]; // /flaps/registration

// Broadcast topic for fleet-wide commands
const char* mqtt_broadcast = "/flaps/all/command";

// Legacy topic for backward compatibility
const char* mqtt_listen = "/game";

const int check_in_freq = 60000;
IPAddress ip_address;
char ip_char[16];
char device_id[18];  // MAC address as device ID
unsigned long lastMillis = 0;
String reg;
int displayState[number_units];
const uint8_t answersize = 1; //Size of units request answer
const int flapamount = 45; //Amount of Flaps in each unit
const int minspeed = 1; //min Speed
const int maxspeed = 12; //max Speed
const char letters[] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '$', '&', '#', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', '.', '-', '?', '!'};
char writtenLast[number_units+1] = "";
const int flap_speed = 10;

// Build device ID from MAC address
void buildDeviceId() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(device_id, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("Device ID: ");
  Serial.println(device_id);
}

// Build dynamic MQTT topics with device ID
void buildMqttTopics() {
  sprintf(mqtt_command, "%s/%s/command", mqtt_base, device_id);
  sprintf(mqtt_status_topic, "%s/%s/status", mqtt_base, device_id);
  sprintf(mqtt_health, "%s/%s/health", mqtt_base, device_id);
  sprintf(mqtt_registration_topic, "%s/registration", mqtt_base);

  Serial.print("MQTT command topic: ");
  Serial.println(mqtt_command);
}
