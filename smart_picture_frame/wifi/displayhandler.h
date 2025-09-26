#ifndef __DSP_HANDLER_H__
#define __DSP_HANDLER_H__

#include "epd7in3f.h"

enum DisplayState { uninitialized, idle, busy, sleeping };

class DisplayHandler {
public:
  DisplayHandler(void);
  ~DisplayHandler(void);

  int  Init(void);
  void Wake(void);
  void Sleep(void);
  void Loop(void);
  bool IsReady(void);
  bool Clear(String colorIndex);
  bool PrepareImageUpload(void);
  bool UploadImageChunk(uint8_t *buffer, size_t buffer_size);
  bool FinalizeImageUpload(void);
private:
  Epd epd;
  DisplayState dspState = uninitialized;
  DisplayState lastState = uninitialized;

  bool PrepareForTask();
};

#endif /* __DSP_HANDLER_H__ */