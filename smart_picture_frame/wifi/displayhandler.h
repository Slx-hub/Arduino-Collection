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

private:
  Epd epd;
  DisplayState dspState = uninitialized;

  bool PrepareForTask();
};

#endif /* __DSP_HANDLER_H__ */