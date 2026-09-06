#ifndef __DSP_HANDLER_H__
#define __DSP_HANDLER_H__

#include "epd7in3f.h"
#include "font5x7.h"

enum DisplayState { uninitialized, idle, busy, sleeping, error };

// Text layout for ShowMessage. A glyph cell is one font pixel wider and taller
// than the glyph itself, so characters and lines do not touch.
#define TEXT_SCALE    5
#define TEXT_CELL_W   ((FONT_WIDTH + 1) * TEXT_SCALE)
#define TEXT_CELL_H   ((FONT_HEIGHT + 1) * TEXT_SCALE)
#define TEXT_MARGIN_X 20
#define TEXT_MARGIN_Y 20
#define TEXT_COLS     25
#define TEXT_ROWS     11

class DisplayHandler {
public:
  DisplayHandler(void);
  ~DisplayHandler(void);

  bool  Init(void);
  void Wake(void);
  void Sleep(void);
  void Loop(void);
  bool IsReady(void);
  bool IsResting(void);
  unsigned long RestRemaining(void);
  bool Clear(unsigned char color);
  bool ShowMessage(const char* const* lines, int lineCount, unsigned char fg, unsigned char bg);
  bool PrepareImageUpload(void);
  bool UploadImageChunk(uint8_t *buffer, size_t buffer_size);
  bool FinalizeImageUpload(void);
  DisplayState GetState(void) { return dspState; }
private:
  Epd epd;
  DisplayState dspState = uninitialized;

  // the panel wants ~2 minutes of rest after a refresh, so every refresh path
  // goes through IsResting() rather than firing back to back
  static const unsigned long restPeriod = 120000;
  unsigned long lastRefreshEnd = 0;
  bool hasRefreshed = false;

  // a clear asked for during the rest period is remembered, not dropped, so the
  // clear button always does something even if the panel cannot obey right away
  bool clearPending = false;
  unsigned char pendingClearColor = EPD_7IN3F_WHITE;

  bool PrepareForTask();
  bool StartClear(unsigned char color);
  void BuildTextRow(uint8_t *row, int fontRow, const char* const* lines,
                    int lineCount, unsigned char fg, unsigned char bg);
  uint8_t GlyphRow(char c, int glyphY);
};

#endif /* __DSP_HANDLER_H__ */
