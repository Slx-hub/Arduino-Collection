# Smart Picture Frame WiFi Controller

Arduino sketch for the Seeed XIAO ESP32-C6 that drives a 7.3" 7-colour e-Paper
display, fed by a smarthome over HTTP.

## Hardware Requirements

- **Microcontroller**: Seeed Studio XIAO ESP32-C6
- **Display**: Waveshare 7.3" e-Paper (F), 800x480, 7 colours
- **Power**: Stable 3.3V supply (recommended: USB-C or quality battery pack)
- **Buttons**: 4 buttons to GND, wired `INPUT_PULLUP` (active low)

## Wiring

Display pins are set in `epdif.h`, button pins in `buttonhandler.cpp`.

| Signal | GPIO | XIAO pin | Wire colour |
| --- | --- | --- | --- |
| BUSY | 0 | D0 | purple |
| RST | 1 | D1 | white |
| DC | 2 | D2 | green |
| CS | 20 | D9 | orange |
| SCK | 19 | D8 | yellow |
| MOSI | 18 | D10 | blue |
| Button 0 | 21 | D3 | |
| Button 1 | 22 | D4 | |
| Button 2 | 23 | D5 | |
| Button 3 | 16 | D6 | |

MISO is unused: the panel is write-only, so `SPI.begin()` is called with `-1` for it.
That frees GPIO20 to serve as CS. **GPIO17 (D7) is the only unused pin on the header.**

> **GPIO16 is the primary ESP-IDF console UART TX.** The C6 sdkconfig sets
> `CONFIG_ESP_CONSOLE_UART_NUM=0` with USB Serial/JTAG only as the *secondary*
> console, so IDF log output is transmitted on the same pad button 3 sits on. A
> UART line idles high and drives low for start and zero bits, which reads as a
> button press. `Serial` itself is unaffected (it is the USB peripheral), but
> the pin is a poor choice for a button and **moving button 3 to GPIO17 (D7) is
> the proper fix.** Until then, see the init ordering note below.

## Buttons

| Button | Pin | Action |
| --- | --- | --- |
| 0 | 21 | Ask smarthome for a new image (`picture_frame_display_image`) |
| 1 | 22 | Ask smarthome for the info screen (`picture_frame_display_info_screen`) |
| 2 | 23 | Ask smarthome for the special action (`picture_frame_special_action`) |
| 3 | 16 | Clear display to white -- handled locally, no network needed |

Buttons 0-2 send an HTTP GET to the smarthome at `http://192.168.178.30:5123/` and
are ignored while the display is not ready. Button 3 never touches the network so a
stuck image can always be cleared, even with wifi or the smarthome down. A clear
asked for while the panel is refreshing or resting is queued, not dropped.

A press must read low across three consecutive samples (~300ms) before it
counts, and the local clear is rate limited: more than 3 clears in 15 minutes
latches it off until reboot and sets `clear_lockout` in `/status`. Both exist
because a noisy GPIO16 once cycled the panel 34 times in two hours -- ready ->
phantom press -> clear -> refresh -> rest -> ready -> phantom press, a loop that
sustains itself. All buttons additionally share a 10s cooldown.

> **`buttons.Init()` must stay last in `setup()`.** GPIO16 carries the IDF
> console (see above), so the pin has to be claimed as an input *after* wifi has
> finished its noisy bringup. `41ebdbf` moved it ahead of `server.Init()` so the
> buttons would survive a wifi failure, and phantom presses started. Moving it
> back stopped the pin reading low at all: 176 samples over 15 minutes, zero
> lows, where previously a low showed up within seconds.

> **Do not call `buttons.Loop()` unconditionally.** It is gated on the display
> being ready in `wifi.ino`, because sampling buttons outside that state kills
> the WiFi stack -- the board keeps running but vanishes from the network, no
> ping and no ARP entry, until it is power cycled. The gate was added in
> `1e12519` right after the buttons landed, held for eleven months, was removed
> in `41ebdbf` on the assumption it was redundant, and the failure returned
> within a day. The full history is in the comment above the gate.

## Panel Timing

A refresh takes ~30s and the panel then rests for 2 minutes. `DisplayHandler`
enforces that rest: `IsReady()` stays false for the whole period and every refresh
path is refused until it elapses, so nothing can drive the panel back to back.
`/status` reports `resting` during that window.

## Offline Diagnostic Screen

The display, buttons and the network come up in that order, and nothing aborts
`setup()`, so the panel is usable even with no wifi. The config portal runs
non-blocking so `loop()` keeps servicing buttons while it is up.

If the portal expires without a connection, the frame draws a failure screen once,
naming the SSID, the reason from `WiFi.status()`, and how to get back into setup.
It is drawn a single time, only after the network state has settled, and it waits
out the rest period rather than forcing a refresh. Text is rendered locally from
`font5x7.h` -- one 400-byte scanline at a time, since a full 800x480 frame is 192KB
and will not allocate next to the 200KB upload buffer.

`font5x7.h` is generated; the glyphs are authored as ASCII art in
`genfont.py` (run it from this directory to regenerate).

> `WiFiManager` is built inside `EspServer::Init()` rather than being a member.
> `EspServer` is a global, so a member would run its constructor during static
> init -- before `nvs_flash_init()` and before USB CDC comes up -- which crashes
> the board before it can print anything at all.

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
- `base64` by Densaugeo (used by `espserver.cpp`)

`WebServer` ships with the ESP32 core, no install needed.

### 3. Select Board & Port
- **Tools** → **Board** → **ESP32** → **XIAO_ESP32C6**
- **Tools** → **Port** → Select your device's COM port
- Leave **USB CDC On Boot** at *Enabled* (the default); `Serial` is the USB
  Serial/JTAG peripheral, not UART0.

## Building & Uploading

From the IDE: open `wifi.ino`, click Upload, then open the Serial Monitor.

From the command line -- `arduino-cli` is not on PATH but ships inside the IDE:

```sh
CLI="$LOCALAPPDATA/Programs/Arduino IDE/resources/app/lib/backend/resources/arduino-cli.exe"

"$CLI" compile --fqbn esp32:esp32:XIAO_ESP32C6 .
"$CLI" upload -p COM3 --fqbn esp32:esp32:XIAO_ESP32C6 .
"$CLI" monitor -p COM3 -c baudrate=115200
```

> **Close the Arduino IDE first.** It holds the serial port, and both upload and
> monitor fail with `PermissionError(13, 'Access is denied')` while it is running.

The sketch uses ~94% of the 1.2MB app partition. There is little headroom left,
and an OTA update has to fit the second partition, so watch the size when adding
code.

## WiFi Supervision

Nothing watched the link after the initial connect, so a dropped STA left the
frame running but invisible -- no ping, no ARP entry -- until it was power
cycled. `EspServer::SuperviseWifi()` now runs every loop once the server is up:
a 30s grace period lets brief blips self heal, then `WiFi.reconnect()` every
15s, and after 8 failed attempts (~2 minutes) `ESP.restart()`, which re-runs
`autoConnect` and falls back to the setup portal. `WiFi.setAutoReconnect(true)`
is set in `StartServer()` since the core does not reliably keep it on across a
WiFiManager connect.

## WiFi Connection

On first boot, or when the saved network cannot be reached, the ESP32 creates an
access point:
- **SSID**: `Esp32AP`
- **Password**: `password`
- **Portal**: `http://192.168.4.1`

Connect to it and pick your home WiFi. The portal is non-blocking and gives up
after 4 minutes, after which the frame draws the offline diagnostic screen and
needs a power cycle to offer setup again.

## Web Server Endpoints

The device runs a web server on **port 80**:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/status` | GET | Panel state plus diagnostics, see below |
| `/clear` | GET | Clear display (query param: `color=0-6`) |
| `/image` | POST | Upload one full 192000 byte frame |
| `/update` | GET | ElegantOTA firmware upload page |

`/status` returns:

```json
{"status":"ready","uptime_s":41,"refreshes":3,"last_refresh":"image",
 "rest_left_s":0,"buttons_down":0,"rssi":-48,"wifi_drops":0}
```

- `status` -- `ready`, `busy`, `resting`, `uninitialized` or `error`
- `uptime_s` -- resets on reboot, so a crash loop is visible remotely
- `refreshes` / `last_refresh` -- how many panel refreshes since boot and what
  caused the last one: `image`, `http-clear`, `button-clear`, `deferred-clear`
  or `diagnostic`. This is how a stray refresh gets traced without a cable.
- `rest_left_s` -- seconds until the panel will accept another refresh
- `buttons_down` -- raw pin bitmask, bit per button, 1 = reading pressed. A bit
  set with nobody at the frame means a stuck or noisy line.
- `rssi` -- signal strength in dBm. Better than -60 is comfortable, so a healthy
  value here rules out range as the cause of a dropout.
- `wifi_drops` -- how many times the link has been lost and recovered since
  boot. Climbing while `uptime_s` keeps rising means supervision is healing
  drops; `uptime_s` resetting instead means the restart fallback fired.
- `clear_lockout` -- true once the runaway guard has disabled the local clear
  button (more than 3 clears in 15 minutes). HTTP `/clear` is unaffected.

Error responses:

- `503` from `/clear` or `/image` -- panel busy or resting, body says how long
  to wait. Retry then rather than immediately.
- `422` from `/image` -- fewer than 192000 bytes arrived. **The panel is left
  untouched.** Triggering a refresh paints whatever is already in panel RAM, so
  a truncated upload used to wipe the picture to white and report `200 OK`.

Colour indices are in `epd7in3f.h`: 0 black, 1 white, 2 green, 3 blue, 4 red,
5 yellow, 6 orange. Index 7 is documented by Waveshare as unusable (afterimage).

A Bruno collection for these endpoints lives in `bruno/esp32/`.

## OTA Updates (Over-The-Air)

Once connected to WiFi, browse to `http://<ESP32_IP>/update` and upload the
compiled `.bin`. Export one with **Sketch** → **Export Compiled Binary**, or take
`build/esp32.esp32.XIAO_ESP32C6/wifi.ino.bin` after a command-line compile.

The IP is printed on boot (`Server up and running on ...`).

## Troubleshooting

**No serial output at all, not even the `ESP-ROM:` banner:**
- That banner comes from the mask ROM before any user code, so nothing in
  `setup()` can suppress it. Suspect a crashing global constructor instead --
  see the `WiFiManager` note above.
- Also check the Arduino IDE is not holding the port.

**Serial monitor shows nothing after a reset:**
- The USB Serial/JTAG device re-enumerates on every reset. A monitor that does
  not reconnect will silently show an empty window.

**WiFi won't connect:**
- Watch the boot log for the `*wm:` lines; they name the SSID and attempt count.
- `setConnectTimeout` is 20s. Shorter values cause
  `sta is connecting, cannot set config` -- the retry fires while the previous
  attempt is still associating and is rejected, wasting every retry.
- To re-run setup, power cycle and join `Esp32AP`.

**Display not updating:**
- Check the BUSY pin (GPIO0) is not stuck LOW.
- Check `/status`: `resting` means the 2 minute rest period has not elapsed.
- Verify SPI wiring against the table above.

**Panel goes white on its own:**
- Check `/status` `last_refresh`. `image` with a `422` in the client log means a
  truncated upload; `deferred-clear` or `button-clear` with nobody at the frame
  means a phantom press on a button line.
- `buttons_down` non-zero at idle means a stuck or noisy button line. Buttons
  are not sampled while the panel refreshes, and a press must read low across
  three consecutive samples, because the refresh couples noise into long button
  wires and a single spurious low on button 3 wipes the screen.

## Serial Monitor Output

115200 baud. `Serial` is the USB Serial/JTAG peripheral (the board enumerates as
`VID_303A&PID_1001`), so the log arrives over the same USB-C cable used for
flashing.

A healthy boot looks like:

```
Initializing e-Paper
Initializing buttons
Initializing server
---------- INIT ----------
*wm:AutoConnect
*wm:Connecting to SAVED AP: <your ssid>
*wm:AutoConnect: SUCCESS
*wm:STA IP Address: 192.168.178.42
Server up and running on 192.168.178.42
All done!
```

## File Structure

```
wifi.ino              - Main sketch with setup() and loop()
espserver.cpp/h       - WiFi, config portal and HTTP endpoints
displayhandler.cpp/h  - E-Paper state machine, rest period, text rendering
buttonhandler.cpp/h   - Button input handling
font5x7.h             - Generated 5x7 bitmap font for the diagnostic screen
genfont.py            - Authors and regenerates font5x7.h
epd7in3f.cpp/h        - E-Paper driver
epdif.cpp/h           - SPI interface
bruno/esp32/          - Bruno HTTP collection and test images
```

## License

Part of Arduino-Collection by Slx-hub
