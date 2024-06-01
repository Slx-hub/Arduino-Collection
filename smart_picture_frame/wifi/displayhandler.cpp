#include "displayhandler.h"

DisplayHandler::~DisplayHandler() {
};

DisplayHandler::DisplayHandler() {
};

int DisplayHandler::Init(void) {
  int code = epd.Init();
  if (code == 0) {
    dspState = sleeping;
  }
  return code;
}

void DisplayHandler::Wake(void) {
  Serial.println("Waking display");
  epd.Wake();
  dspState = idle;
}

void DisplayHandler::Sleep(void) {
  Serial.println("Napping display");
  epd.Sleep();
  dspState = sleeping;
}

void DisplayHandler::Loop(void) {
  if (dspState == busy && !epd.EPD_7IN3F_IsBusy())
  {
    epd.TurnOffDisplay();
    Sleep();
  }
}

bool DisplayHandler::IsReady(void) {
  return (dspState != busy && dspState != uninitialized);
}

bool DisplayHandler::Clear(String colorIndex) {
  if (!PrepareForTask()) {
    return false;
  }
  unsigned char color = (unsigned char)colorIndex.toInt();
  epd.Clear(color);
  dspState = busy;

  return true;
}

bool DisplayHandler::PrepareForTask() {
  if (dspState == busy || dspState == uninitialized)
  {
    return false;
  }

  if (dspState == sleeping)
  {
    Wake();
  }

  return true;
}