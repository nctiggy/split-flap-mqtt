#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <Arduino.h>

// Constants
extern const int number_units;
extern const int flapamount;
extern const int flap_speed;
extern const char letters[];
extern char writtenLast[];
extern int displayState[];
extern const uint8_t answersize;

// WiFiManager config variables (defined in 02_wifi.ino)
extern char cfg_mqtt_server[64];
extern char cfg_mqtt_port[6];
extern char cfg_mqtt_user[32];
extern char cfg_mqtt_pass[32];
extern char cfg_device_name[32];

// Global variables for network (defined in 01_config.ino)
extern IPAddress ip_address;
extern char ip_char[16];
extern char device_id[18];
extern char mqtt_command[64];
extern char mqtt_status_topic[64];
extern char mqtt_health[64];
extern char mqtt_registration_topic[64];
extern const char* mqtt_broadcast;
extern const char* mqtt_listen;
extern unsigned long lastMillis;
extern String reg;

// Animation types
enum AnimationType {
  ANIM_INSTANT = 0,
  ANIM_WAVE,
  ANIM_SCROLL,
  ANIM_RANDOM
};

// Queue entry structure
struct QueueEntry {
  char text[11];  // number_units + 1
  AnimationType animation;
  uint8_t speed;
};

// Function declarations - Config
void buildDeviceId();
void buildMqttTopics();

// Function declarations - WiFi
void connectToWifi();
void checkWifiConnection();
void resetWifiConfig();

// Function declarations - OTA
void otaSetup();

// Function declarations - MQTT
void mqttSetup();
bool reconnect();
void callback(String topic, byte* message, unsigned int length);
void mqttLoopOps();
void processMessageQueue();
void build_registration();
void checkIn();
void publishHealth();
bool enqueueMessage(const char* msg, AnimationType anim, uint8_t speed);
bool dequeueMessage(QueueEntry* entry);

// Function declarations - Flaps
void showNewData(char message[]);
void showNewDataAnimated(char message[], AnimationType anim, uint8_t speed);
void showMessage(char message[], int flapSpeed);
void showMessageAnimated(char message[], int flapSpeed, AnimationType anim);
void waitForDisplayIdle();
void showWaveAnimation(char message[], int flapSpeed);
void showScrollAnimation(char message[], int flapSpeed);
void showRandomAnimation(char message[], int flapSpeed);
uint8_t translateLettertoInt(char letterchar);
void writeToUnit(uint8_t address, uint8_t letter, int flapSpeed);
bool isDisplayMoving();
int checkIfMoving(uint8_t address);
uint32_t getTotalI2CErrors();

#endif
