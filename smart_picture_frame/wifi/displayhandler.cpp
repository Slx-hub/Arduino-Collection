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
    lastRefreshEnd = millis();
    hasRefreshed = true;
    Sleep();
  }

  // a clear that arrived mid-refresh or mid-rest runs as soon as the panel allows
  if (clearPending && dspState != busy && !IsResting()) {
    clearPending = false;
    Serial.println("Running deferred clear");
    StartClear(pendingClearColor);
  }
}

bool DisplayHandler::IsReady(void) {
  return (dspState == idle || dspState == sleeping) && !IsResting();
}

bool DisplayHandler::IsResting(void) {
  return hasRefreshed && (millis() - lastRefreshEnd) < restPeriod;
}

unsigned long DisplayHandler::RestRemaining(void) {
  if (!IsResting()) {
    return 0;
  }
  return restPeriod - (millis() - lastRefreshEnd);
}

bool DisplayHandler::Clear(unsigned char color) {
  if (dspState == uninitialized) {
    return false;
  }

  // never drop a clear: if the panel is mid-refresh or resting, remember it and
  // let Loop() run it once the panel is allowed to refresh again
  if (dspState == busy || IsResting()) {
    pendingClearColor = color;
    clearPending = true;
    Serial.printf("Clear deferred, %lus of rest left\n", RestRemaining() / 1000);
    return true;
  }

  return StartClear(color);
}

bool DisplayHandler::StartClear(unsigned char color) {
  if (!PrepareForTask()) {
    return false;
  }
  dspState = busy;
  if (!epd.Clear(color)){dspState=error;}

  return true;
}

bool DisplayHandler::ShowMessage(const char* const* lines, int lineCount, unsigned char fg, unsigned char bg) {
  if (!PrepareForTask()) {
    return false;
  }

  // streamed one scanline at a time: a full 800x480 frame is 192KB, which will
  // not allocate next to the 200KB upload buffer, but a single row is only 400B
  uint8_t row[EPD_WIDTH / 2];
  int lastFontRow = -2;

  epd.PrepareImageUpload();
  for (int y = 0; y < EPD_HEIGHT; y++) {
    int textY = y - TEXT_MARGIN_Y;
    int fontRow = -1; // -1 means margin, i.e. a blank row
    if (textY >= 0 && textY < TEXT_ROWS * TEXT_CELL_H) {
      fontRow = textY / TEXT_SCALE;
    }

    // consecutive display rows repeat the same font row, so only rebuild on change
    if (fontRow != lastFontRow) {
      BuildTextRow(row, fontRow, lines, lineCount, fg, bg);
      lastFontRow = fontRow;
    }
    epd.UploadImageChunk(row, sizeof(row));
  }

  dspState = busy;
  if (!epd.FinalizeImageUpload()) {
    dspState = error;
    return false;
  }
  return true;
}

void DisplayHandler::BuildTextRow(uint8_t *row, int fontRow, const char* const* lines,
                                  int lineCount, unsigned char fg, unsigned char bg) {
  unsigned char bgByte = (unsigned char)((bg << 4) | bg);
  for (int i = 0; i < EPD_WIDTH / 2; i++) {
    row[i] = bgByte;
  }

  if (fontRow < 0) {
    return;
  }

  int textRow = fontRow / (FONT_HEIGHT + 1);
  int glyphY = fontRow % (FONT_HEIGHT + 1);
  if (glyphY >= FONT_HEIGHT || textRow >= lineCount) {
    return; // the spacing row between lines, or past the last line
  }

  const char *line = lines[textRow];
  if (line == NULL) {
    return;
  }

  for (int col = 0; col < TEXT_COLS && line[col] != '\0'; col++) {
    uint8_t bits = GlyphRow(line[col], glyphY);
    if (bits == 0) {
      continue;
    }
    for (int gx = 0; gx < FONT_WIDTH; gx++) {
      if (!(bits & (1 << (FONT_WIDTH - 1 - gx)))) {
        continue;
      }
      int x0 = TEXT_MARGIN_X + col * TEXT_CELL_W + gx * TEXT_SCALE;
      for (int px = x0; px < x0 + TEXT_SCALE && px < EPD_WIDTH; px++) {
        int idx = px / 2;
        if (px % 2) {
          row[idx] = (uint8_t)((row[idx] & 0xF0) | (fg & 0x0F)); // right pixel
        } else {
          row[idx] = (uint8_t)((row[idx] & 0x0F) | ((fg & 0x0F) << 4)); // left pixel
        }
      }
    }
  }
}

uint8_t DisplayHandler::GlyphRow(char c, int glyphY) {
  unsigned char uc = (unsigned char)c;
  if (uc >= 'a' && uc <= 'z') {
    uc -= 32; // the font is uppercase only
  }
  if (uc < FONT_FIRST_CHAR || uc > FONT_LAST_CHAR) {
    uc = '?';
  }
  return FONT5X7[uc - FONT_FIRST_CHAR][glyphY];
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

  // refusing here is what keeps the panel from being refreshed back to back
  if (IsResting())
  {
    return false;
  }

  if (dspState == sleeping)
  {
    Wake();
  }

  return true;
}
