#ifndef __DSP_HANDLER_H__
#define __DSP_HANDLER_H__

#include "epd7in3f.h"
#include "font5x7.h"

enum DisplayState { uninitialized, idle, busy, sleeping, error };

// What triggered the most recent refresh. Reported by /status so a stray refresh
// can be traced without a serial cable.
enum RefreshSource { srcNone, srcHttpClear, srcButtonClear, srcDeferredClear,
                     srcImage, srcDiagnostic };

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
  // Refuses if the panel cannot refresh right now. Used by the HTTP endpoint so
  // the caller gets a truthful answer and can retry at the right time.
  bool Clear(unsigned char color, RefreshSource source);
  // Queues instead of refusing, so the physical button always does something.
  // Deliberately NOT used by the HTTP endpoint: a queued clear fires up to two
  // minutes later and would wipe whatever the smarthome drew in the meantime.
  bool ClearWhenPossible(unsigned char color, RefreshSource source);
  bool ShowMessage(const char* const* lines, int lineCount, unsigned char fg, unsigned char bg);
  bool PrepareImageUpload(void);
  bool UploadImageChunk(uint8_t *buffer, size_t buffer_size);
  // Refuses unless a whole frame was actually staged. A refresh with nothing
  // staged repaints the panel from its existing RAM -- white after a clear.
  bool FinalizeImageUpload(void);
  void AbortImageUpload(void);
  size_t GetUploadedBytes(void) { return uploadedBytes; }
  // 2 pixels per byte
  static const size_t imageBytes = ((size_t)EPD_WIDTH * (size_t)EPD_HEIGHT) / 2;
  DisplayState GetState(void) { return dspState; }
  unsigned long GetRefreshCount(void) { return refreshCount; }
  RefreshSource GetLastSource(void) { return lastSource; }
  const char* GetLastSourceName(void);
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

  unsigned long refreshCount = 0;
  RefreshSource lastSource = srcNone;

  // an upload only counts once every byte of the frame has arrived; a truncated
  // or rejected transfer must never reach the panel
  bool uploadActive = false;
  size_t uploadedBytes = 0;

  bool PrepareForTask();
  bool StartClear(unsigned char color, RefreshSource source);
  void NoteRefresh(RefreshSource source);
  void BuildTextRow(uint8_t *row, int fontRow, const char* const* lines,
                    int lineCount, unsigned char fg, unsigned char bg);
  uint8_t GlyphRow(char c, int glyphY);
};

#endif /* __DSP_HANDLER_H__ */
