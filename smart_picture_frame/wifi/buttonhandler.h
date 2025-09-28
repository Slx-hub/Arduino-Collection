// buttonhandler.h
#pragma once
#include <Arduino.h>
#include <HTTPClient.h>

class ButtonHandler {
public:
    void Init();
    void Loop();

private:
    static const int BUTTON_PINS[4];
    unsigned long lastPressTime = 0;
    const unsigned long debounceDelay = 10000; // 10 seconds

    void HandleButton(int index);
    void SendRequest(const char* path);
};
