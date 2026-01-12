const uint8_t WIFI_TIMEOUT_SEC = 20;

void connectToWifi() {
  Serial.print("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.begin(wifi_ssid, wifi_password);

  // Wait for connection with timeout
  uint8_t timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < WIFI_TIMEOUT_SEC) {
    Serial.print(".");
    delay(1000);
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    ip_address = WiFi.localIP();
    sprintf(ip_char, "%d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    Serial.println(" connected!");
    Serial.print("IP: ");
    Serial.println(ip_char);
  } else {
    Serial.println(" timeout! Will retry in background.");
  }
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
