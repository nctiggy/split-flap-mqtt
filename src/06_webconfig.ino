#include "declarations.h"
#include <ESP8266WebServer.h>

ESP8266WebServer webServer(80);

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Flaps Config</title>";
  html += "<style>body{font-family:Arial;margin:40px;background:#f0f0f0}";
  html += ".card{background:white;padding:20px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);margin-bottom:20px}";
  html += "h1{color:#333}h2{color:#555;margin-top:0}";
  html += "input,select{width:100%;padding:8px;margin:8px 0;border:1px solid #ddd;border-radius:4px;box-sizing:border-box}";
  html += "label{display:block;margin-top:12px;font-weight:bold;color:#555}";
  html += "button{background:#007bff;color:white;border:none;padding:10px 20px;";
  html += "border-radius:4px;cursor:pointer;font-size:16px;margin:10px 5px 0 0}button:hover{background:#0056b3}";
  html += ".danger{background:#dc3545}.danger:hover{background:#c82333}";
  html += ".success{background:#28a745}.success:hover{background:#218838}</style></head><body>";

  html += "<div class='card'><h1>Split-Flap Display</h1>";
  html += "<p><strong>Device ID:</strong> " + String(device_id) + "</p>";
  html += "<p><strong>IP Address:</strong> " + String(ip_char) + "</p>";
  html += "<p><strong>MQTT Server:</strong> " + String(cfg_mqtt_server) + ":" + String(cfg_mqtt_port) + "</p>";
  html += "<p><strong>Device Name:</strong> " + String(cfg_device_name) + "</p>";
  html += "<p><strong>Firmware:</strong> " + String(FW_VERSION) + "</p>";
  html += "</div>";

  html += "<div class='card'><h2>MQTT Configuration</h2>";
  html += "<form method='POST' action='/saveconfig'>";
  html += "<label>MQTT Server:</label>";
  html += "<input type='text' name='mqtt_server' value='" + String(cfg_mqtt_server) + "' placeholder='broker.example.com'>";
  html += "<label>MQTT Port:</label>";
  html += "<input type='number' name='mqtt_port' value='" + String(cfg_mqtt_port) + "' placeholder='1883'>";
  html += "<label>MQTT User (optional):</label>";
  html += "<input type='text' name='mqtt_user' value='" + String(cfg_mqtt_user) + "' placeholder='username'>";
  html += "<label>MQTT Password (optional):</label>";
  html += "<input type='password' name='mqtt_pass' value='" + String(cfg_mqtt_pass) + "' placeholder='password'>";
  html += "<label>Device Name:</label>";
  html += "<input type='text' name='device_name' value='" + String(cfg_device_name) + "' placeholder='flaps-01'>";
  html += "<button type='submit' class='success'>Save & Reconnect</button>";
  html += "</form></div>";

  html += "<div class='card'><h2>Actions</h2>";
  html += "<button onclick=\"location.href='/reset'\">Reset WiFi Config</button>";
  html += "<button onclick=\"location.href='/restart'\">Restart Device</button>";
  html += "</div>";

  html += "<div class='card'><h2>MQTT Topics</h2>";
  html += "<p><strong>Command:</strong> " + String(mqtt_command) + "</p>";
  html += "<p><strong>Status:</strong> " + String(mqtt_status_topic) + "</p>";
  html += "<p><strong>Health:</strong> " + String(mqtt_health) + "</p>";
  html += "<p><strong>Broadcast:</strong> " + String(mqtt_broadcast) + "</p>";
  html += "</div>";

  html += "</body></html>";

  webServer.send(200, "text/html", html);
}

void handleReset() {
  String html = "<!DOCTYPE html><html><head><title>Resetting...</title>";
  html += "<meta http-equiv='refresh' content='10;url=http://192.168.4.1'>";
  html += "<style>body{font-family:Arial;margin:40px;text-align:center;background:#f0f0f0}</style></head><body>";
  html += "<h1>Resetting Configuration...</h1>";
  html += "<p>Device will restart in config mode.</p>";
  html += "<p>Connect to the WiFi network 'Flaps-Setup-...' (password: flapsetup)</p>";
  html += "<p>You will be redirected to the config portal in 10 seconds...</p>";
  html += "</body></html>";

  webServer.send(200, "text/html", html);
  delay(1000);
  resetWifiConfig();
}

void handleRestart() {
  String html = "<!DOCTYPE html><html><head><title>Restarting...</title>";
  html += "<style>body{font-family:Arial;margin:40px;text-align:center;background:#f0f0f0}</style></head><body>";
  html += "<h1>Restarting Device...</h1>";
  html += "<p>Please wait a few seconds and refresh this page.</p>";
  html += "</body></html>";

  webServer.send(200, "text/html", html);
  delay(1000);
  ESP.restart();
}

void handleSaveConfig() {
  // Get form values
  if (webServer.hasArg("mqtt_server")) {
    strlcpy(cfg_mqtt_server, webServer.arg("mqtt_server").c_str(), sizeof(cfg_mqtt_server));
  }
  if (webServer.hasArg("mqtt_port")) {
    strlcpy(cfg_mqtt_port, webServer.arg("mqtt_port").c_str(), sizeof(cfg_mqtt_port));
  }
  if (webServer.hasArg("mqtt_user")) {
    strlcpy(cfg_mqtt_user, webServer.arg("mqtt_user").c_str(), sizeof(cfg_mqtt_user));
  }
  if (webServer.hasArg("mqtt_pass")) {
    strlcpy(cfg_mqtt_pass, webServer.arg("mqtt_pass").c_str(), sizeof(cfg_mqtt_pass));
  }
  if (webServer.hasArg("device_name")) {
    strlcpy(cfg_device_name, webServer.arg("device_name").c_str(), sizeof(cfg_device_name));
  }

  // Save to LittleFS
  bool saved = saveConfig();

  String html = "<!DOCTYPE html><html><head><title>Configuration Saved</title>";
  html += "<meta http-equiv='refresh' content='3;url=/'>";
  html += "<style>body{font-family:Arial;margin:40px;text-align:center;background:#f0f0f0}</style></head><body>";

  if (saved) {
    html += "<h1>✓ Configuration Saved!</h1>";
    html += "<p>MQTT settings updated. Device will reconnect...</p>";
  } else {
    html += "<h1>✗ Save Failed</h1>";
    html += "<p>Could not save configuration. Check serial output.</p>";
  }

  html += "<p>Redirecting to home page in 3 seconds...</p>";
  html += "</body></html>";

  webServer.send(200, "text/html", html);

  if (saved) {
    delay(1000);
    // Force MQTT reconnect with new settings
    Serial.println("Restarting for new MQTT config...");
    delay(2000);
    ESP.restart();
  }
}

void webConfigSetup() {
  webServer.on("/", handleRoot);
  webServer.on("/reset", handleReset);
  webServer.on("/restart", handleRestart);
  webServer.on("/saveconfig", HTTP_POST, handleSaveConfig);
  webServer.begin();
  Serial.println("Web config server started on port 80");
  Serial.print("Access at: http://");
  Serial.println(ip_char);
}

void webConfigLoop() {
  webServer.handleClient();
}
