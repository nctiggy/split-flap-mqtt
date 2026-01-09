void mqttSetup() {
 client.setServer(mqtt_server, mqtt_port);
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

  Serial.print("Attempting MQTT connection...");
  /*
   YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
   To change the ESP device ID, you will have to give a new name to the ESP8266.
   Here's how it looks:
     if (client.connect("ESP8266Client")) {
   You can do it like this:
     if (client.connect("ESP1_Office")) {
   Then, for the other ESP:
     if (client.connect("ESP2_Garage")) {
    That should solve your MQTT multiple connections problem
  */
  if (client.connect(mdns_hostname)) {
    Serial.println("connected");
    client.subscribe(mqtt_listen);
    return true;
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    return false;
  }
}

void callback(String topic, byte* message, unsigned int length) {
  char flap_msg[number_units+1];
  int spaces = 0;
  
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  if (length < 9) {
    spaces = (int)(number_units - length)/2;
  }
  for (int i = 0; i < number_units + 1; i++) {
    char single_char;
    if (i < spaces) {
      single_char = ' ';
    } else if (i - spaces < length) {
      single_char = (char)message[i - spaces];
    } else if (i == number_units) {
      single_char = '\0';
    } else {
      single_char = ' ';
    }
    flap_msg[i] = single_char;
  }
  Serial.println(flap_msg);
  showNewData(flap_msg);
  Serial.println();
}

void mqttLoopOps() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void build_registration() {
  const size_t CAPACITY = JSON_OBJECT_SIZE(5);  // 5 fields: name, ip_address, state, uptime, last_written
  StaticJsonDocument<CAPACITY> doc;
  doc["name"] = mdns_hostname;
  doc["ip_address"] = ip_char;
  doc["state"] = "connected";
  doc["uptime"] = millis();
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
    serializeJson(doc, registration);
    lastMillis = uptime;
    client.publish(mqtt_registration, registration);
  }
}
