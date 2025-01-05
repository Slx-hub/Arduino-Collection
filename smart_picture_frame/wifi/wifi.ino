#include <SPI.h>
#include <WebServer.h>
#include "espserver.h"
#include "displayhandler.h"

DisplayHandler handler;
EspServer server;

void setup() {
  Serial.begin(115200);

  Serial.println("Initializing server");
  if (server.Init() != 0) {
    Serial.println("server init failed");
    return;
  }

  Serial.println("Initializing e-Paper");
  if (handler.Init() != 0) {
    Serial.println("e-Paper init failed");
    return;
  }
  server.SetDisplay(&handler);

  Serial.println("All done!");
}

void loop() {
  handler.Loop();
  server.Loop();
  delay(50);
}