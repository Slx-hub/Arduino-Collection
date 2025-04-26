#include "WebServer.h"
#include "espserver.h"
#include "base64.hpp"

// MQTT Broker
const char *mqtt_broker = "192.168.178.30";
const int mqtt_port = 1883;
const char *topic = "hermes/intent/PictureFrame";

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

  mqttClient = PubSubClient(server.client());

  mqttClient.setServer(mqtt_broker, mqtt_port);
  while (!mqttClient.connected()) {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the MQTT broker\n", client_id.c_str());
      if (mqttClient.connect(client_id.c_str())) {
          Serial.println("MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.println(mqttClient.state());
          delay(2000);
      }
  }
  mqttClient.publish(topic, "{\"input\":\"display image\"}");

  return 0;
}

void EspServer::Callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

void EspServer::SetDisplay(DisplayHandler* ptr) {
  dspPtr = ptr;
}

void EspServer::Loop(void) {
  server.handleClient();
}