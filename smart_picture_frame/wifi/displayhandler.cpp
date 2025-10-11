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
  dspState = busy;
  epd.Clear(color);

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
  epd.FinalizeImageUpload();
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