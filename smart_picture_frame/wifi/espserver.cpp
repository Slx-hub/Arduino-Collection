#include "espserver.h"

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
  }
  if (!dspPtr->Clear(server.arg("color"))) {
    SendJsonResponse(500, "error", "Display is busy");
  }

  SendJsonResponse(200, "action", "OK");
}

int EspServer::Init(void) {
  WiFiManager wm;

  if(!wm.autoConnect("Esp32AP","password")) {
    Serial.println("Failed to connect to WiFi");
    return -1;
  }
  Serial.println("Connected to WiFi!");

  server.on("/status", [&](){GetStatus();});  
  server.on("/clear", [&](){ClearDisplay();});  
          
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