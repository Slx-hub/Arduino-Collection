// buttonhandler.cpp
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

void ButtonHandler::Loop() {
    unsigned long now = millis();

    // only check if cooldown has passed
    if (now - lastPressTime < debounceDelay) return;

    for (int i = 0; i < 4; i++) {
        if (digitalRead(BUTTON_PINS[i]) == LOW) { // button pressed
            lastPressTime = now;
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

    if (!dspPtr->Clear(EPD_7IN3F_WHITE)) {
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
