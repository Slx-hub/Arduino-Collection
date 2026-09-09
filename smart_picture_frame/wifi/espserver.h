#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <ElegantOTA.h>
#include <ArduinoJson.h>
#include "displayhandler.h"
#include "buttonhandler.h"

#ifdef HTTP_UPLOAD_BUFLEN //if the macro MEDIAN_MAX_SIZE is defined
#undef HTTP_UPLOAD_BUFLEN //un-define it
#define HTTP_UPLOAD_BUFLEN 200000 //redefine it with the new value
#endif

// netFailed means the config portal expired without a connection, i.e. settled
// failure worth spending a display refresh on. Anything else is still in flight.
enum NetState { netPortal, netConnected, netFailed };

class EspServer {
public:
  EspServer(void);
  ~EspServer(void);

  int  Init(void);
  void SetDisplay(DisplayHandler* ptr);
  void SetButtons(ButtonHandler* ptr);
  void Loop(void);

  NetState GetNetState(void) { return netState; }
  const char* GetFailureReason(void);

  static const char* AP_NAME;
  static const char* AP_PASSWORD;
  static const char* AP_PORTAL_IP;
private:
  DisplayHandler* dspPtr;
  ButtonHandler* btnPtr = NULL;
  // built in Init(), not here: EspServer is a global, and a WiFiManager member
  // would run its constructor during static init, before nvs_flash_init() and
  // before USB CDC is up. It still has to outlive Init() for the portal to be
  // driven from Loop(), hence the pointer.
  WiFiManager* wm = nullptr;
  NetState netState = netPortal;
  bool started = false;
  // the raw upload handler and the main handler can both try to answer one
  // request; only the first reply is valid
  bool uploadResponded = false;

  // link supervision
  static const unsigned long disconnectGraceMs = 30000;   // let short blips self heal
  static const unsigned long reconnectIntervalMs = 15000;
  static const int maxReconnectAttempts = 8;              // ~2 min, then reboot
  unsigned long lastConnectedMs = 0;
  unsigned long lastReconnectMs = 0;
  int reconnectAttempts = 0;
  unsigned long wifiDrops = 0;
  WebServer server;
  StaticJsonDocument<512> jsonDocument;
  char buffer[512];

  void CreateJson(char *tag, char *value);
  void SendJsonResponse(int code, char *tag, char *value);

  void StartServer(void);

  // Nothing watched the link after the initial connect, so a dropped STA left
  // the frame invisible until someone power cycled it.
  void SuperviseWifi(void);
  void SendBusyResponse(void);
  void GetStatus(void);
  void ClearDisplay(void);
  void FinalizeImageUpload(void);
  void UploadImageChunk(void);

  void Callback(char *topic, byte *payload, unsigned int length);
};
