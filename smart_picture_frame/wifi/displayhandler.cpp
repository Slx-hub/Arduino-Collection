#include "displayhandler.h"

DisplayHandler::~DisplayHandler() {
};

DisplayHandler::DisplayHandler() {
};

bool DisplayHandler::Init(void) {
  if (epd.Init()) {
    dspState = sleeping;
    return true;
  }
  return false;
}

void DisplayHandler::Wake(void) {
  Serial.println("Waking display");
  if (!epd.Wake()) {
    Serial.println("Failed to wake display");
    dspState = error;
    return;
  }
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
    if (!epd.TurnOffDisplay()){
      dspState=error;
      return;
    }
    Sleep();
  }
}

bool DisplayHandler::IsReady(void) {
  return (dspState == idle || dspState == sleeping);
}

bool DisplayHandler::Clear(String colorIndex) {
  if (!PrepareForTask()) {
    return false;
  }
  unsigned char color = (unsigned char)colorIndex.toInt();
  dspState = busy;
  if (!epd.Clear(color)){dspState=error;}

  return true;
}

bool DisplayHandler::PrepareImageUpload(void) {
  if (!PrepareForTask()) {
    return false;
  }
  epd.PrepareImageUpload();
  return true;
}
bool DisplayHandler::UploadImageChunk(uint8_t *buffer, size_t buffer_size) {
  if (dspState != idle) {
    return false;
  }
  epd.UploadImageChunk(buffer, buffer_size);
  return true;
}
bool DisplayHandler::FinalizeImageUpload(void) {
  if (!PrepareForTask()) {
    return false;
  }
  dspState = busy;
  if (!epd.FinalizeImageUpload()) {
    dspState = error;
    return false;
  }
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