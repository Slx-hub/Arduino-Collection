// buttonhandler.cpp
//
// WARNING: ButtonHandler::Loop() must only be called while the display is ready
// (or in the error state). See the gate in wifi.ino's loop() and the comment
// above it -- calling it unconditionally kills the WiFi stack and the board
// silently leaves the network until it is power cycled. This cost two separate
// debugging sessions, eleven months apart. Do not call it from anywhere else.
#include "buttonhandler.h"

// Pin 16 is UART0 TX on the ESP32-C6, so Serial0 is unavailable while it is wired
// as a button. Harmless here: this board builds with USB CDC on boot, so Serial
// is the USB Serial/JTAG peripheral and logging is unaffected.
const int ButtonHandler::BUTTON_PINS[4] = {21, 22, 23, 16};

void ButtonHandler::Init() {
    for (int i = 0; i < 4; i++) {
        pinMode(BUTTON_PINS[i], INPUT_PULLUP); // active low
    }
}

void ButtonHandler::SetDisplay(DisplayHandler* ptr) {
    dspPtr = ptr;
}

uint8_t ButtonHandler::GetRawMask(void) {
    uint8_t mask = 0;
    for (int i = 0; i < 4; i++) {
        if (digitalRead(BUTTON_PINS[i]) == LOW) {
            mask |= (uint8_t)(1 << i);
        }
    }
    return mask;
}

void ButtonHandler::Loop() {
    unsigned long now = millis();

    // Never sample while the panel is refreshing: that is the high current
    // window where the phantom presses came from, and a press is useless then
    // anyway since every action would be refused.
    bool quiet = (dspPtr == NULL) || (dspPtr->GetState() != busy);

    for (int i = 0; i < 4; i++) {
        if (quiet && digitalRead(BUTTON_PINS[i]) == LOW) {
            lowStreak[i]++;
        } else {
            lowStreak[i] = 0; // a single high read means it was not a real press
        }
    }

    // only act if cooldown has passed
    if (now - lastPressTime < debounceDelay) return;

    for (int i = 0; i < 4; i++) {
        if (lowStreak[i] >= stableSamples) {
            lastPressTime = now;
            lowStreak[i] = 0;
            HandleButton(i);
            break; // only handle one button at a time
        }
    }
}

void ButtonHandler::HandleButton(int index) {
    Serial.print("pressed Button ");
    Serial.println(index);
    switch (index) {
        case 0:
            SendRequest("picture_frame_display_image");
            break;
        case 1:
            SendRequest("picture_frame_display_info_screen");
            break;
        case 2:
            SendRequest("picture_frame_special_action");
            break;
        case 3:
            ClearDisplay();
            break;
    }
}

// driven locally instead of via the smarthome, so a stuck image can be cleared
// with wifi down. Clear() also accepts the error state, which the image path does
// not, making this the way out of a failed refresh.
void ButtonHandler::ClearDisplay(void) {
    Serial.println("Clearing display locally");

    if (dspPtr == nullptr) {
        Serial.println("No display attached");
        return;
    }

    if (!dspPtr->ClearWhenPossible(EPD_7IN3F_WHITE, srcButtonClear)) {
        Serial.println("Clear rejected, display is busy or uninitialized");
        return;
    }

    Serial.println("Clear started");
}

// no point waking the smarthome for new content the display cannot take
void ButtonHandler::SendRequest(const char* path) {
    if (dspPtr == nullptr || !dspPtr->IsReady()) {
        Serial.println("Display not ready, skipping request");
        return;
    }

    HTTPClient http;
    String url = String("http://192.168.178.30:5123/") + path;
    
    http.setTimeout(2000); // 2 second timeout
    http.setConnectTimeout(1000); // 1 second connection timeout
    
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
        Serial.printf("HTTP GET succeeded, code: %d\n", httpCode);
    } else {
        Serial.printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
}
