// buttonhandler.h
#pragma once
#include <Arduino.h>
#include <HTTPClient.h>
#include "displayhandler.h"

class ButtonHandler {
public:
    void Init();
    void SetDisplay(DisplayHandler* ptr);
    void Loop();

private:
    static const int BUTTON_PINS[4];
    DisplayHandler* dspPtr = nullptr;
    unsigned long lastPressTime = 0;
    const unsigned long debounceDelay = 10000; // 10 seconds

    void HandleButton(int index);
    void ClearDisplay(void);
    void SendRequest(const char* path);
};
