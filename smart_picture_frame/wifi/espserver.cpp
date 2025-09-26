#include "WebServer.h"
#include "espserver.h"
#include "base64.hpp"

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
    CreateJson("status", "unable");
  }
  server.send(200, "application/json", buffer);
}

void EspServer::ClearDisplay(void) {
  Serial.println("Request: Clear display");

  if (!server.hasArg("color")) {
    SendJsonResponse(400, "error", "Missing color query parameter");
    return;
  }
  if (!dspPtr->Clear(server.arg("color"))) {
    SendJsonResponse(500, "error", "Display is busy");
    return;
  }

  SendJsonResponse(200, "action", "OK");
}

void EspServer::UploadImageChunk(void) {
  if (server.raw().status == RAW_START) {
    Serial.println("Request: Display image");

    if (!dspPtr->PrepareImageUpload()) {
      SendJsonResponse(500, "error", "Display is busy");
      server.raw().status = RAW_ABORTED;
    }
    return;
  }
  
  if (server.raw().status == UPLOAD_FILE_END) {
    return;
  }

  if (!dspPtr->UploadImageChunk(server.raw().buf, server.raw().currentSize)) {
      SendJsonResponse(500, "error", "An error occured during buffering");
      server.raw().status = RAW_ABORTED;
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
  WiFiManager wm;

  if(!wm.autoConnect("Esp32AP","password")) {
    Serial.println("Failed to connect to WiFi");
    return -1;
  }
  Serial.println("Connected to WiFi!");

  server.on("/status", [&](){GetStatus();});
  server.on("/clear", [&](){ClearDisplay();});
  server.on("/image", HTTP_POST, [&](){FinalizeImageUpload();}, [&](){UploadImageChunk();});

  server.begin();

  Serial.println("Server up and running!");

  return 0;
}

void EspServer::SetDisplay(DisplayHandler* ptr) {
  dspPtr = ptr;
}

void EspServer::Loop(void) {
  server.handleClient();
}