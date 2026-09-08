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
    // Raw pin states, bit per button, 1 = reading pressed. Reported by /status
    // so a stuck or noisy line can be seen without a scope.
    uint8_t GetRawMask(void);

private:
    static const int BUTTON_PINS[4];
    DisplayHandler* dspPtr = nullptr;
    unsigned long lastPressTime = 0;
    const unsigned long debounceDelay = 10000; // 10 seconds

    // A single LOW read used to be enough to fire an action. The panel pulls
    // hard while refreshing and couples noise into the button wiring, which
    // produced phantom presses -- and on button 3 that means wiping the screen.
    // Require the line to stay low across several consecutive samples instead.
    static const int stableSamples = 3; // ~300ms at the 100ms loop interval
    int lowStreak[4] = {0, 0, 0, 0};

    void HandleButton(int index);
    void ClearDisplay(void);
    void SendRequest(const char* path);
};
