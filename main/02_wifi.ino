void connectToWifi() {
  Serial.print("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  ip_address = WiFi.localIP();
  sprintf(ip_char, "%d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
}
