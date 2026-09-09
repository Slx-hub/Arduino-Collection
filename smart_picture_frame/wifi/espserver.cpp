#include "espserver.h"
#include "base64.hpp"

const char* EspServer::AP_NAME = "Esp32AP";
const char* EspServer::AP_PASSWORD = "password";
const char* EspServer::AP_PORTAL_IP = "192.168.4.1";

EspServer::~EspServer() {
};

EspServer::EspServer() {
};

void EspServer::SendJsonResponse(int code, char *tag, char *value) {
    CreateJson(tag, value);
    server.send(code, "application/json", buffer);
}

void EspServer::CreateJson(char *tag, char *value) {  
  jsonDocument.clear();  
  jsonDocument[tag] = value;
  serializeJson(jsonDocument, buffer);
}

void EspServer::GetStatus(void) {
  Serial.println("Request: Check status");

  const char *status = "unknown";
  if (dspPtr->IsReady()) {
    status = "ready";
  } else {
    switch (dspPtr->GetState()) {
      case uninitialized: status = "uninitialized"; break;
      case busy:          status = "busy"; break;
      case error:         status = "error"; break;
      default:            status = dspPtr->IsResting() ? "resting" : "unknown"; break;
    }
  }

  // uptime exposes reboots and refresh/last_refresh trace a stray refresh back to
  // its trigger, both of which otherwise need a serial cable to see
  jsonDocument.clear();
  jsonDocument["status"] = status;
  jsonDocument["uptime_s"] = millis() / 1000;
  jsonDocument["refreshes"] = dspPtr->GetRefreshCount();
  jsonDocument["last_refresh"] = dspPtr->GetLastSourceName();
  jsonDocument["rest_left_s"] = dspPtr->RestRemaining() / 1000;
  // a button reading pressed here with nobody at the frame means a stuck or
  // noisy line, which is what phantom clears look like
  jsonDocument["buttons_down"] = btnPtr != NULL ? btnPtr->GetRawMask() : 0;
  // rssi separates "weak signal" from "something else killed the link"
  jsonDocument["rssi"] = WiFi.RSSI();
  jsonDocument["wifi_drops"] = wifiDrops;
  serializeJson(jsonDocument, buffer);

  server.send(200, "application/json", buffer);
}

// Tells the caller how long to wait, so the smarthome can retry at the right
// time instead of hammering a panel that is mid-refresh or resting.
void EspServer::SendBusyResponse(void) {
  char msg[96];
  if (dspPtr->IsResting()) {
    snprintf(msg, sizeof(msg), "Panel resting, retry in %lus",
             dspPtr->RestRemaining() / 1000);
    SendJsonResponse(503, "error", msg);
    return;
  }
  snprintf(msg, sizeof(msg), "Display is busy");
  SendJsonResponse(503, "error", msg);
}

void EspServer::ClearDisplay(void) {
  Serial.println("Request: Clear display");

  if (!server.hasArg("color")) {
    SendJsonResponse(400, "error", "Missing color query parameter");
    return;
  }
  // deliberately the non-queueing call: a deferred clear would fire up to two
  // minutes later and wipe whatever was drawn in the meantime
  if (!dspPtr->Clear((unsigned char)server.arg("color").toInt(), srcHttpClear)) {
    SendBusyResponse();
    return;
  }

  SendJsonResponse(200, "action", "OK");
}

void EspServer::UploadImageChunk(void) {
  if (server.raw().status == RAW_START) {
    Serial.println("Request: Display image");
    uploadResponded = false;

    if (!dspPtr->PrepareImageUpload()) {
      server.raw().status = RAW_ABORTED;
      SendBusyResponse();
      uploadResponded = true;
    }
    return;
  }

  if (server.raw().status == UPLOAD_FILE_END) {
    return;
  }

  if (!dspPtr->UploadImageChunk(server.raw().buf, server.raw().currentSize)) {
      server.raw().status = RAW_ABORTED;
      dspPtr->AbortImageUpload();
      if (!uploadResponded) {
        SendJsonResponse(500, "error", "An error occured during buffering");
        uploadResponded = true;
      }
      return;
  }
}

void EspServer::FinalizeImageUpload(void) {
  Serial.println("Displaying image");

  if (!dspPtr->FinalizeImageUpload()) {
    // the raw handler already answered when it rejected the upload; a second
    // response on the same request corrupts the reply the client reads
    if (!uploadResponded) {
      char msg[96];
      snprintf(msg, sizeof(msg), "Incomplete image: %u of %u bytes",
               (unsigned)dspPtr->GetUploadedBytes(), (unsigned)DisplayHandler::imageBytes);
      SendJsonResponse(422, "error", msg);
      uploadResponded = true;
    }
    return;
  }
  SendJsonResponse(200, "action", "OK");
  uploadResponded = true;
}

int EspServer::Init(void) {
  Serial.println("---------- INIT ----------");

  wm = new WiFiManager();

  // 5s was not enough for the STA to finish associating, so each retry hit
  // esp_wifi_set_config() mid-connect and was rejected outright
  wm->setConnectTimeout(20);
  wm->setConfigPortalTimeout(240);
  // each retry blocks setup(), so cap the worst case at ~40s rather than ~60s
  wm->setConnectRetries(2);
  // setup() must not stall here: loop() has to run for the buttons to work
  wm->setConfigPortalBlocking(false);

  if (wm->autoConnect(AP_NAME, AP_PASSWORD)) {
    StartServer();
    return 0;
  }

  Serial.println("WiFi not up, config portal running in the background");
  netState = netPortal;
  return -1;
}

void EspServer::StartServer(void) {
  server.on("/status", [&](){GetStatus();});
  server.on("/clear", [&](){ClearDisplay();});
  server.on("/image", HTTP_POST, [&](){FinalizeImageUpload();}, [&](){UploadImageChunk();});

  ElegantOTA.begin(&server);

  server.begin();

  Serial.print("Server up and running on ");
  Serial.println(WiFi.localIP());

  netState = netConnected;
  started = true;
  lastConnectedMs = millis();
  // the core does not always keep this on across a WiFiManager connect
  WiFi.setAutoReconnect(true);
}

void EspServer::SuperviseWifi(void) {
  unsigned long now = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (reconnectAttempts > 0) {
      Serial.printf("WiFi back after %d attempts\n", reconnectAttempts);
    }
    lastConnectedMs = now;
    reconnectAttempts = 0;
    return;
  }

  // a brief drop usually recovers on its own; only step in once it persists
  if (now - lastConnectedMs < disconnectGraceMs) {
    return;
  }
  if (reconnectAttempts > 0 && now - lastReconnectMs < reconnectIntervalMs) {
    return;
  }

  if (reconnectAttempts == 0) {
    wifiDrops++;
  }
  lastReconnectMs = now;
  reconnectAttempts++;
  Serial.printf("WiFi down %lus, reconnect attempt %d of %d\n",
                (now - lastConnectedMs) / 1000, reconnectAttempts, maxReconnectAttempts);
  WiFi.reconnect();

  if (reconnectAttempts >= maxReconnectAttempts) {
    // out of options: a reboot re-runs autoConnect and, failing that, the portal
    Serial.println("WiFi unrecoverable, restarting");
    Serial.flush();
    ESP.restart();
  }
}

const char* EspServer::GetFailureReason(void) {
  switch (WiFi.status()) {
    case WL_NO_SSID_AVAIL:   return "AP NOT FOUND";
    case WL_CONNECT_FAILED:  return "WRONG PASSWORD";
    case WL_CONNECTION_LOST: return "CONNECTION LOST";
    case WL_DISCONNECTED:    return "DISCONNECTED";
    case WL_IDLE_STATUS:     return "RADIO IDLE";
    case WL_NO_SHIELD:       return "NO RADIO";
    default:                 return "UNKNOWN";
  }
}

void EspServer::SetDisplay(DisplayHandler* ptr) {
  dspPtr = ptr;
}

void EspServer::SetButtons(ButtonHandler* ptr) {
  btnPtr = ptr;
}

void EspServer::Loop(void) {
  if (!started) {
    if (wm == nullptr) {
      return; // Init() never ran
    }
    wm->process();

    if (WiFi.status() == WL_CONNECTED) {
      StartServer();
    } else if (!wm->getConfigPortalActive() && netState != netFailed) {
      // portal expired and still nothing: this is as settled as failure gets
      Serial.println("WiFi setup ended without a connection");
      netState = netFailed;
    }
    return;
  }
  SuperviseWifi();

  server.handleClient();
  ElegantOTA.loop();
}