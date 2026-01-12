# Split-Flap MQTT Display

A WiFi-enabled split-flap display controller using ESP8266 (ESP-01) with MQTT control, web configuration, and OTA updates.

Based on [Dave19171's split-flap project](https://github.com/Dave19171/split-flap) ([Printables](https://www.prusaprinters.org/prints/69464-split-flap-display)).

## Features

- **WiFi Manager**: Captive portal for easy WiFi setup (no hardcoded credentials)
- **Web Configuration**: Configure MQTT settings via web browser at device IP
- **MQTT Control**: Send messages to the display via MQTT
- **OTA Updates**: Update firmware over WiFi (no USB needed after initial flash)
- **Message Queue**: Queue up to 10 messages
- **Animations**: Instant, Wave, Scroll, and Random reveal effects
- **Fleet Support**: Unique device IDs, broadcast topics for multiple displays
- **Health Metrics**: Monitor device status via MQTT

## Quick Start

### 1. Start MQTT Broker

```bash
docker-compose up -d
```

This starts Mosquitto MQTT broker on port 1883.

**Find your broker IP:**
```bash
# Linux
hostname -I | awk '{print $1}'

# macOS
ipconfig getifaddr en0
```

### 2. Flash the ESP8266

**Requirements:**
- [PlatformIO](https://platformio.org/install)
- USB-to-Serial programmer for ESP-01

**Build and flash:**
```bash
# Build firmware
pio run

# Flash via USB (adjust port as needed)
pio run -t upload
```

### 3. Configure the Device

1. Power on the ESP8266
2. Connect to WiFi network: **Flaps-Setup-XXXXXX**
3. Password: **flapsetup**
4. Configure:
   - Your WiFi network and password
   - MQTT Server IP (e.g., `192.168.1.100`)
   - MQTT Port: `1883`
   - Device Name (optional)
5. Click **Save**

The device will connect to your network.

### 4. Send Messages

**Using mosquitto_pub:**
```bash
# Send to specific device
mosquitto_pub -h YOUR_BROKER_IP -t "/flaps/68c63af65362/command" -m "HELLO"

# Broadcast to all devices
mosquitto_pub -h YOUR_BROKER_IP -t "/flaps/all/command" -m "HELLO"
```

**JSON payload with animation:**
```bash
mosquitto_pub -h YOUR_BROKER_IP -t "/flaps/all/command" -m '{"text":"HELLO","animation":"wave"}'
```

**Animation options:** `instant`, `wave`, `scroll`, `random`

## Web Interface

Access the device configuration at: `http://DEVICE_IP/`

Features:
- View device info (IP, firmware version, MQTT status)
- Configure MQTT settings
- Test display with messages
- Reboot or factory reset

## MQTT Topics

| Topic | Description |
|-------|-------------|
| `/flaps/{device_id}/command` | Send message to specific device |
| `/flaps/{device_id}/status` | Device status (online/offline) |
| `/flaps/{device_id}/health` | Health metrics (uptime, errors, etc.) |
| `/flaps/all/command` | Broadcast to all devices |
| `/flaps/register` | Device registration announcements |

## OTA Updates

After initial USB flash, update over WiFi:

```bash
# Update platformio.ini with device IP, then:
pio run -e ota -t upload
```

**OTA Password:** `flaps-ota-secret` (change in `src/03_ota.ino`)

## Hardware

- **Controller:** ESP8266 ESP-01 (1MB flash)
- **Display:** Split-flap units connected via I2C
- **I2C Pins:** GPIO1 (SDA), GPIO3 (SCL) - uses TX/RX pins

## Configuration

### platformio.ini

```ini
[env:esp8266]
upload_port = /dev/cu.usbserial-XXXX  # Your serial port

[env:ota]
upload_port = 192.168.1.XXX           # Device IP for OTA
```

### Display Settings

Edit `src/01_config.ino`:
- `number_units`: Number of split-flap units (default: 10)
- `flap_speed`: Motor speed in RPM

## Troubleshooting

### WiFi Setup Not Appearing
- Ensure ESP-01 is in UART mode (not PROG)
- Check if already connected to a network
- Factory reset: Access web UI at device IP and click "Reset WiFi Config"

### MQTT Not Connecting
- Verify broker IP and port
- Check broker allows anonymous connections
- View device logs at `http://DEVICE_IP/status`

### Flash Failing
- Use USB-A port directly (not through hub/adapter)
- ESP-01 programmer in PROG mode
- Try lower baud rate: `upload_speed = 115200`

### OTA Update Failing
- Ensure device is on same network
- Check OTA password matches
- Device must be running and connected

## Project Structure

```
src/
├── main.ino           # Entry point, setup/loop
├── declarations.h     # Type definitions, function declarations
├── 01_config.ino      # Constants, topic building
├── 02_wifi.ino        # WiFiManager, LittleFS config
├── 03_ota.ino         # OTA update handler
├── 04_mqtt.ino        # MQTT client, message handling
├── 05_flaps.ino       # Display control, animations
└── 06_webconfig.ino   # Web configuration server
```

## Node-RED Integration

Example flow to send time to display:

```json
[{"id":"inject","type":"inject","repeat":"60","payload":"","topic":""},
 {"id":"function","type":"function","func":"var d = new Date();\nvar h = d.getHours().toString().padStart(2,'0');\nvar m = d.getMinutes().toString().padStart(2,'0');\nmsg.payload = h + ':' + m;\nreturn msg;","wires":[["mqtt"]]},
 {"id":"mqtt","type":"mqtt out","topic":"/flaps/all/command","broker":"your-broker"}]
```

## License

MIT License - See original project for hardware designs.

## Credits

- Original hardware design: [Dave19171](https://github.com/Dave19171/split-flap)
- ESP8266 MQTT implementation: This project
