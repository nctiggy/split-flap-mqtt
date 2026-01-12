#include "declarations.h"

const uint8_t MSG_QUEUE_SIZE = 10;
QueueEntry messageQueue[MSG_QUEUE_SIZE];
uint8_t queueHead = 0;
uint8_t queueTail = 0;
uint8_t queueCount = 0;

// Current animation state
AnimationType currentAnimation = ANIM_INSTANT;
uint8_t currentSpeed = 10;

// Health metrics
uint32_t messagesProcessed = 0;
uint32_t mqttDisconnects = 0;

void mqttSetup() {
  // Use config values from WiFiManager
  int port = atoi(cfg_mqtt_port);
  if (port == 0) port = 1883;

  Serial.print("Setting up MQTT: ");
  Serial.print(cfg_mqtt_server);
  Serial.print(":");
  Serial.println(port);

  client.setServer(cfg_mqtt_server, port);
  client.setCallback(callback);
}

// Non-blocking reconnect - returns true if connected, false if still trying
bool reconnect() {
  static unsigned long lastReconnectAttempt = 0;

  if (client.connected()) {
    return true;
  }

  unsigned long now = millis();
  // Only attempt reconnect every 5 seconds
  if (now - lastReconnectAttempt < 5000) {
    return false;
  }
  lastReconnectAttempt = now;
  mqttDisconnects++;

  Serial.print("Attempting MQTT connection...");

  // Connect with LWT (Last Will and Testament) for offline detection
  // Use config values from WiFiManager
  bool connected = false;
  if (strlen(cfg_mqtt_user) > 0) {
    connected = client.connect(cfg_device_name, cfg_mqtt_user, cfg_mqtt_pass,
                               mqtt_status_topic, 0, true, "offline");
  } else {
    connected = client.connect(cfg_device_name, NULL, NULL,
                               mqtt_status_topic, 0, true, "offline");
  }

  if (connected) {
    Serial.println("connected");

    // Subscribe to all command topics
    client.subscribe(mqtt_command);     // Device-specific: /flaps/{device}/command
    client.subscribe(mqtt_broadcast);   // Fleet-wide: /flaps/all/command
    client.subscribe(mqtt_listen);      // Legacy: /game

    // Publish online status
    client.publish(mqtt_status_topic, "online", true);

    Serial.print("Subscribed to: ");
    Serial.println(mqtt_command);
    return true;
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    return false;
  }
}

// Parse animation type from string
AnimationType parseAnimation(const char* anim) {
  if (strcmp(anim, "wave") == 0) return ANIM_WAVE;
  if (strcmp(anim, "scroll") == 0) return ANIM_SCROLL;
  if (strcmp(anim, "random") == 0) return ANIM_RANDOM;
  return ANIM_INSTANT;
}

// Add message to queue with animation
bool enqueueMessage(const char* msg, AnimationType anim, uint8_t speed) {
  if (queueCount >= MSG_QUEUE_SIZE) {
    Serial.println("Queue full, dropping message");
    return false;
  }
  strncpy(messageQueue[queueTail].text, msg, number_units);
  messageQueue[queueTail].text[number_units] = '\0';
  messageQueue[queueTail].animation = anim;
  messageQueue[queueTail].speed = speed;
  queueTail = (queueTail + 1) % MSG_QUEUE_SIZE;
  queueCount++;
  return true;
}

// Get next message from queue
bool dequeueMessage(QueueEntry* entry) {
  if (queueCount == 0) {
    return false;
  }
  *entry = messageQueue[queueHead];
  queueHead = (queueHead + 1) % MSG_QUEUE_SIZE;
  queueCount--;
  return true;
}

// Center-pad a message
void centerPadMessage(const char* input, int inputLen, char* output) {
  int spaces = 0;
  if (inputLen < number_units) {
    spaces = (number_units - inputLen) / 2;
  }
  for (int i = 0; i < number_units + 1; i++) {
    if (i < spaces) {
      output[i] = ' ';
    } else if (i - spaces < inputLen) {
      output[i] = input[i - spaces];
    } else if (i == number_units) {
      output[i] = '\0';
    } else {
      output[i] = ' ';
    }
  }
}

void callback(String topic, byte* message, unsigned int length) {
  char flap_msg[number_units + 1];
  AnimationType anim = ANIM_INSTANT;
  uint8_t speed = flap_speed;

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Raw: ");

  // Null-terminate message for parsing
  char msgBuffer[256];
  int msgLen = (length < 255) ? length : 255;
  memcpy(msgBuffer, message, msgLen);
  msgBuffer[msgLen] = '\0';
  Serial.println(msgBuffer);

  // Check if message is JSON (starts with '{')
  if (msgBuffer[0] == '{') {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, msgBuffer);

    if (!error) {
      // Parse JSON payload: {"text":"HELLO","animation":"wave","speed":5}
      const char* text = doc["text"] | "";
      const char* animStr = doc["animation"] | "instant";
      speed = doc["speed"] | flap_speed;

      anim = parseAnimation(animStr);
      msgLen = strlen(text);
      if (msgLen > number_units) msgLen = number_units;
      centerPadMessage(text, msgLen, flap_msg);

      Serial.print("JSON parsed - text: ");
      Serial.print(text);
      Serial.print(", animation: ");
      Serial.print(animStr);
      Serial.print(", speed: ");
      Serial.println(speed);
    } else {
      // JSON parse failed, treat as plain text
      Serial.println("JSON parse failed, treating as plain text");
      if (msgLen > number_units) msgLen = number_units;
      centerPadMessage(msgBuffer, msgLen, flap_msg);
    }
  } else {
    // Plain text message (backward compatible)
    if (msgLen > number_units) msgLen = number_units;
    centerPadMessage(msgBuffer, msgLen, flap_msg);
  }

  // Queue the message with animation info
  if (enqueueMessage(flap_msg, anim, speed)) {
    messagesProcessed++;
  }
}

void mqttLoopOps() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

// Process queued messages (call from main loop)
void processMessageQueue() {
  static QueueEntry nextEntry;

  // Only process if display is idle and queue has messages
  if (queueCount > 0 && !isDisplayMoving()) {
    if (dequeueMessage(&nextEntry)) {
      currentAnimation = nextEntry.animation;
      currentSpeed = nextEntry.speed;
      showNewDataAnimated(nextEntry.text, nextEntry.animation, nextEntry.speed);
    }
  }
}

void build_registration() {
  const size_t CAPACITY = JSON_OBJECT_SIZE(7);
  StaticJsonDocument<CAPACITY> doc;
  doc["name"] = cfg_device_name;
  doc["device_id"] = device_id;
  doc["ip_address"] = ip_char;
  doc["state"] = "connected";
  doc["uptime"] = millis();
  doc["fw_version"] = FW_VERSION;
  serializeJson(doc, reg);
}

void checkIn() {
  unsigned long uptime = millis();
  char registration[1024];
  if (uptime - lastMillis > check_in_freq) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, reg);
    doc["uptime"] = uptime;
    doc["last_written"] = writtenLast;
    doc["queue_depth"] = queueCount;
    serializeJson(doc, registration);
    lastMillis = uptime;
    client.publish(mqtt_registration_topic, registration);
  }
}

// Publish health metrics
void publishHealth() {
  static unsigned long lastHealthPublish = 0;
  unsigned long now = millis();

  // Publish every 30 seconds
  if (now - lastHealthPublish < 30000) {
    return;
  }
  lastHealthPublish = now;

  DynamicJsonDocument doc(512);
  doc["uptime"] = now;
  doc["messages_processed"] = messagesProcessed;
  doc["mqtt_disconnects"] = mqttDisconnects;
  doc["i2c_errors"] = getTotalI2CErrors();
  doc["queue_depth"] = queueCount;
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["free_heap"] = ESP.getFreeHeap();

  char healthJson[512];
  serializeJson(doc, healthJson);
  client.publish(mqtt_health, healthJson);
}
