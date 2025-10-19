#ifndef __DSP_HANDLER_H__
#define __DSP_HANDLER_H__

#include "epd7in3f.h"

enum DisplayState { uninitialized, idle, busy, sleeping, error };

class DisplayHandler {
public:
  DisplayHandler(void);
  ~DisplayHandler(void);

  bool  Init(void);
  void Wake(void);
  void Sleep(void);
  void Loop(void);
  bool IsReady(void);
  bool Clear(String colorIndex);
  bool PrepareImageUpload(void);
  bool UploadImageChunk(uint8_t *buffer, size_t buffer_size);
  bool FinalizeImageUpload(void);
  DisplayState GetState(void) { return dspState; }
private:
  Epd epd;
  DisplayState dspState = uninitialized;

  bool PrepareForTask();
};

#endif /* __DSP_HANDLER_H__ */