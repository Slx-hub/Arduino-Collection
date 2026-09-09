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
    // true once the runaway guard has tripped; surfaced by /status
    bool IsClearLockedOut(void) { return localClearLockedOut; }

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

    // GPIO16 doubles as the primary ESP-IDF console UART TX
    // (CONFIG_ESP_CONSOLE_UART_NUM=0), so console traffic on that pin reads as
    // button presses. buttons.Init() now runs last in setup() again, after wifi
    // has finished its noisy init, which is how it was before 41ebdbf.
    //
    // Re-enabled on trial. If the theory is wrong the panel would cycle itself
    // to death again, so the runaway guard below latches the local clear off
    // after a few clears in one window. /clear over HTTP is never affected.
    static const bool localClearEnabled = true;

    static const int maxClearsPerWindow = 3;
    static const unsigned long clearWindowMs = 900000; // 15 minutes
    unsigned long clearWindowStart = 0;
    int clearsInWindow = 0;
    bool localClearLockedOut = false;
    int lowStreak[4] = {0, 0, 0, 0};

    void HandleButton(int index);
    void ClearDisplay(void);
    void SendRequest(const char* path);
};
