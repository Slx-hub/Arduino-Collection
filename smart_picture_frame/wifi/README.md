# Smart Picture Frame WiFi Controller

Arduino sketch for ESP32-C6 that controls a 7.3" e-Paper display with WiFi connectivity.

## Hardware Requirements

- **Microcontroller**: ESP32-C6
- **Display**: 7.3" e-Paper (F) display
- **Power**: Stable 3.3V supply (recommended: USB-C or quality battery pack)
- **Buttons**: 4 GPIO buttons (pins 21, 22, 23, 16)

## Project Setup

### 1. Install Arduino IDE & ESP32 Support
- Download [Arduino IDE 2.0+](https://www.arduino.cc/en/software)
- Add ESP32 board support:
  - Go to **Preferences** → **Additional Boards Manager URLs**
  - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
  - Open **Boards Manager**, search "ESP32", install latest version

### 2. Install Required Libraries
Open **Library Manager** (Sketch → Include Library → Manage Libraries) and install:
- `WiFiManager` by tzapu
- `ArduinoJson` by bblanchon
- `ElegantOTA` by ayushsharma82
- `WebServer` (built-in, may need to verify)

### 3. Select Board & Port
- **Tools** → **Board** → **ESP32** → **ESP32-C6**
- **Tools** → **Port** → Select your device's COM port

### 4. Configure Pins (if needed)
Edit `epdif.h` to match your wiring if pins differ from:
- CS: GPIO 17
- MOSI: GPIO 11
- SCK: GPIO 12
- RST: GPIO 8
- DC: GPIO 9
- BUSY: GPIO 10

## Building & Uploading

```
1. Open wifi.ino in Arduino IDE
2. Click Upload button (or Sketch → Upload)
3. Watch Serial Monitor for initialization messages
```

## WiFi Connection

On first boot, the ESP32 creates an access point:
- **SSID**: `Esp32AP`
- **Password**: `password`

Connect to this network, then it will prompt you to select your home WiFi network.

## Web Server Endpoints

The device runs a local web server with these endpoints:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/status` | GET | Check if display is ready |
| `/clear` | GET | Clear display (query param: `color=0-7`) |
| `/image` | POST | Upload image data to display |

## OTA Updates (Over-The-Air)

Once connected to WiFi, you can update the firmware wirelessly:

1. In Arduino IDE: **Tools** → **ElegantOTA**
2. Enter the ESP32's IP address (shown in Serial Monitor on boot)
3. Select your sketch and click **Upload**

Alternatively, navigate to `http://<ESP32_IP>:8080/update` in a web browser and upload the compiled `.bin` file.

## Troubleshooting

**Device becomes unresponsive:**
- Check power supply stability (use USB-C or quality power bank)
- Watchdog timer will auto-reset after 60 seconds if no response
- Check Serial Monitor for debug messages

**WiFi won't connect:**
- Reset WiFi settings by holding button for 10+ seconds
- Check SSID and password are correct
- Try moving closer to router

**Display not updating:**
- Check display BUSY pin (GPIO 10) is not stuck LOW
- Verify SPI pins are correctly wired
- Try power cycling the device

## Serial Monitor Output

Open **Tools** → **Serial Monitor** (115200 baud) to see debug information and status messages.

## File Structure

```
wifi.ino              - Main sketch with setup() and loop()
espserver.cpp/h       - WiFi server and HTTP endpoints
displayhandler.cpp/h  - E-Paper display control
buttonhandler.cpp/h   - Button input handling
epd7in3f.cpp/h        - E-Paper driver
epdif.cpp/h           - SPI interface
```

## License

Part of Arduino-Collection by Slx-hub
