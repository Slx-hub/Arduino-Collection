// buttonhandler.cpp
#include "buttonhandler.h"

const int ButtonHandler::BUTTON_PINS[4] = {21, 22, 23, 16};

void ButtonHandler::Init() {
    for (int i = 0; i < 4; i++) {
        pinMode(BUTTON_PINS[i], INPUT_PULLUP); // active low
    }
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
            SendRequest("picture_frame_clear_display");
            break;
        case 3:
            SendRequest("picture_frame_clear_display");
            break;
    }
}

void ButtonHandler::SendRequest(const char* path) {
    HTTPClient http;
    String url = String("http://192.168.178.30:5123/") + path;
    http.begin(url);
    int httpCode = http.GET();
    http.end();
}
