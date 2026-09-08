#include <SPI.h>
#include <WebServer.h>
#include "espserver.h"
#include "displayhandler.h"
#include "buttonhandler.h"

DisplayHandler handler;
EspServer server;
ButtonHandler buttons;

// a refresh costs ~30s plus a 2 minute rest, so the failure screen is drawn once
// and only once the network state has actually settled
bool diagnosticShown = false;

void setup() {
  Serial.begin(115200);
  // native USB CDC: the host needs a moment to enumerate, anything printed
  // before that is dropped
  unsigned long serialStart = millis();
  while (!Serial && millis() - serialStart < 2000) {
    delay(10);
  }

  // display and buttons come up before the network, and nothing below is allowed
  // to abort setup, so the clear button stays usable even with no wifi at all
  Serial.println("Initializing e-Paper");
  if (!handler.Init()) {
    Serial.println("e-Paper init failed");
  }

  Serial.println("Initializing buttons");
  buttons.Init();
  buttons.SetDisplay(&handler);

  Serial.println("Initializing server");
  if (server.Init() != 0) {
    Serial.println("WiFi not up yet, carrying on so the buttons still work");
  }
  server.SetDisplay(&handler);
  server.SetButtons(&buttons);

  Serial.println("All done!");
}

// Returns false while the panel cannot be refreshed yet, so the caller simply
// tries again next loop instead of dropping the message.
bool ShowNetworkDiagnostic(void) {
  static char ssidLine[TEXT_COLS + 1];
  static char causeLine[TEXT_COLS + 1];
  static char apLine[TEXT_COLS + 1];
  static char portalLine[TEXT_COLS + 1];

  String ssid = WiFi.SSID();
  if (ssid.length() == 0) {
    ssid = "NONE SAVED";
  }
  snprintf(ssidLine, sizeof(ssidLine), "SSID: %s", ssid.c_str());
  snprintf(causeLine, sizeof(causeLine), "CAUSE: %s", server.GetFailureReason());
  snprintf(apLine, sizeof(apLine), "JOIN AP %s", EspServer::AP_NAME);
  snprintf(portalLine, sizeof(portalLine), "OPEN %s", EspServer::AP_PORTAL_IP);

  const char* lines[] = {
    "PICTURE FRAME OFFLINE",
    "",
    "WIFI: NOT CONNECTED",
    ssidLine,
    causeLine,
    "",
    "TO SET UP: POWER CYCLE",
    apLine,
    portalLine,
    "",
    "BUTTON 4 = CLEAR",
  };

  return handler.ShowMessage(lines, sizeof(lines) / sizeof(lines[0]),
                             EPD_7IN3F_BLACK, EPD_7IN3F_WHITE);
}

void loop() {
  handler.Loop();
  server.Loop();
  buttons.Loop();

  if (!diagnosticShown && server.GetNetState() == netFailed) {
    diagnosticShown = ShowNetworkDiagnostic();
  }

  delay(100);
}
