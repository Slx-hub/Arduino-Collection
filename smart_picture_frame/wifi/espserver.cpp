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
  bool status = dspPtr->IsReady();
  if (status) {
    CreateJson("status", "ready");
  } else {
    DisplayState state = dspPtr->GetState();
    if (state == uninitialized) {
      CreateJson("status", "uninitialized");
    } else if (state == busy) {
      CreateJson("status", "busy");
    } else if (state == error) {
      CreateJson("status", "error");
    } else if (dspPtr->IsResting()) {
      CreateJson("status", "resting");
    } else {
      CreateJson("status", "unknown");
    }
  }
  server.send(200, "application/json", buffer);
}

void EspServer::ClearDisplay(void) {
  Serial.println("Request: Clear display");

  if (!server.hasArg("color")) {
    SendJsonResponse(400, "error", "Missing color query parameter");
    return;
  }
  if (!dspPtr->Clear((unsigned char)server.arg("color").toInt())) {
    SendJsonResponse(500, "error", "Display is busy");
    return;
  }

  SendJsonResponse(200, "action", "OK");
}

void EspServer::UploadImageChunk(void) {
  if (server.raw().status == RAW_START) {
    Serial.println("Request: Display image");

    if (!dspPtr->PrepareImageUpload()) {
      server.raw().status = RAW_ABORTED;
      SendJsonResponse(500, "error", "Display is busy");
    }
    return;
  }
  
  if (server.raw().status == UPLOAD_FILE_END) {
    return;
  }

  if (!dspPtr->UploadImageChunk(server.raw().buf, server.raw().currentSize)) {
      server.raw().status = RAW_ABORTED;
      SendJsonResponse(500, "error", "An error occured during buffering");
      return;
  }
}

void EspServer::FinalizeImageUpload(void) {
  Serial.println("Displaying image");

  if (!dspPtr->FinalizeImageUpload()) {
    SendJsonResponse(500, "error", "An error occured during image display");
    return;
  }
  SendJsonResponse(200, "action", "OK");
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
  server.handleClient();
  ElegantOTA.loop();
}