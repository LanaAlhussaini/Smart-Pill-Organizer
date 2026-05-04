#include "gui.h"

// ══════════════════════════════════════════════════════════════
//  Task 3 – برمجة المخرجات على الشاشة
// ══════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────
//  HOME SCREEN
// ─────────────────────────────────────────────────────────────

void drawHome() {
  tft.fillScreen(C_BG);
  RtcDateTime now = Rtc.GetDateTime();
  drawTopBar(now.Hour(), now.Minute(), now.Second());
  for (int i = 0; i < MAX_MEDS; i++) drawMedCard(i, true);
}

void drawTopBar(uint8_t h, uint8_t m, uint8_t s) {
  tft.fillRect(0, 0, SCREEN_W, TOPBAR_H, C_BG);
  char buf[6];
  snprintf(buf, sizeof(buf), "%02u:%02u", h, m);
  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(SCREEN_W - 52, 3);
  tft.print(buf);
}

/*
 * Draws one medication card.
 *
 * Card layout (CARD_W × CARD_H):
 *   ┌──────────────┐
 *   │  [icon 64px] │  ← BMP from SD
 *   │   med name   │
 *   │  ● ● ●       │  ← dose dots (green=taken, red=missed, grey=future)
 *   │  time left:  │
 *   │   HH:MM      │  ← countdown to next dose
 *   │              *│  ← settings indicator (bottom-right)
 *   └──────────────┘
 *
 * Empty slot shows a "+" to open setup.
 */
void drawMedCard(int idx, bool forceRedraw) {
  int x = CARD_START_X + idx * (CARD_W + CARD_GAP);
  int y = CARD_Y;

  uint16_t cardBg = meds[idx].alarmFiring ? 0x8000 : C_CARD;
  tft.fillRoundRect(x, y, CARD_W, CARD_H, 10, cardBg);
  tft.drawRoundRect(x, y, CARD_W, CARD_H, 10, C_DGREY);

  // ── Empty slot ──
  if (!meds[idx].active) {
    tft.setTextColor(C_LGREY, cardBg);
    tft.setTextSize(3);
    tft.setCursor(x + 33, y + 70);
    tft.print("+");
    tft.setTextSize(1);
    tft.setCursor(x + 18, y + 110);
    tft.setTextColor(C_DGREY, cardBg);
    tft.print("Add med");
    return;
  }

  // ── Icon ──
  int iconX = x + (CARD_W - 64) / 2;
  int iconY = y + 6;
  drawBmpFromSD(ICON_FILES[meds[idx].iconIndex], iconX, iconY, 64, 64);

  // ── Med name ──
  tft.setTextColor(C_WHITE, cardBg);
  tft.setTextSize(1);
  tft.setCursor(x + 4, y + 73);
  tft.print(meds[idx].name);

  // ── Dose dots ──
  RtcDateTime now = Rtc.GetDateTime();
  int dotY       = y + 88;
  int totalDots  = meds[idx].timesPerDay;
  int dotSpacing = (totalDots > 1) ? (CARD_W - 8) / totalDots : 0;

  for (int d = 0; d < totalDots; d++) {
    int dotX = x + 4 + d * dotSpacing + dotSpacing / 2 - 3;
    uint16_t dotCol;
    if (meds[idx].doseTaken[d]) {
      dotCol = C_GREEN;
    } else {
      int doseMins = meds[idx].doses[d].hour * 60 + meds[idx].doses[d].minute;
      int nowMins  = now.Hour() * 60 + now.Minute();
      dotCol = (doseMins <= nowMins) ? C_RED : C_DGREY;
    }
    tft.fillCircle(dotX, dotY, 4, dotCol);
  }

  // ── "time left:" label ──
  tft.setTextColor(C_LGREY, cardBg);
  tft.setTextSize(1);
  tft.setCursor(x + 4, y + 100);
  tft.print("time left:");

  // ── Countdown to next pending dose ──
  int nextDose = -1;
  for (int d = 0; d < meds[idx].timesPerDay; d++) {
    if (!meds[idx].doseTaken[d]) { nextDose = d; break; }
  }

  tft.setCursor(x + 10, y + 112);
  if (meds[idx].alarmFiring) {
    tft.setTextColor(C_RED, cardBg);
    tft.print("TAKE NOW");
  } else if (nextDose < 0) {
    tft.setTextColor(C_GREEN, cardBg);
    tft.print("ALL DONE");
  } else {
    int nowMins  = now.Hour() * 60 + now.Minute();
    int doseMins = meds[idx].doses[nextDose].hour * 60
                 + meds[idx].doses[nextDose].minute;
    int diff = doseMins - nowMins;
    if (diff < 0) diff += 24 * 60;
    char tbuf[6];
    snprintf(tbuf, sizeof(tbuf), "%02d:%02d", diff / 60, diff % 60);
    tft.setTextColor(C_WHITE, cardBg);
    tft.print(tbuf);
  }

  // ── Settings indicator ──
  tft.setTextColor(C_DGREY, cardBg);
  tft.setTextSize(1);
  tft.setCursor(x + CARD_W - 14, y + CARD_H - 14);
  tft.print("*");
}

// ─────────────────────────────────────────────────────────────
//  SETUP SCREEN
// ─────────────────────────────────────────────────────────────

void openSetup(int medIdx) {
  setupMedIdx  = medIdx;
  setupIconIdx = meds[medIdx].active ? meds[medIdx].iconIndex    : 0;
  setupTimes   = meds[medIdx].active ? meds[medIdx].timesPerDay  : 1;
  setupHour    = meds[medIdx].active ? meds[medIdx].doses[0].hour   : 8;
  setupMinute  = meds[medIdx].active ? meds[medIdx].doses[0].minute : 0;
  setupHourActive = true;
  currentScreen   = SCR_SETUP;
  drawSetupScreen();
}

void drawSetupScreen() {
  tft.fillScreen(C_BG);

  // Top bar + back arrow + med name
  RtcDateTime now = Rtc.GetDateTime();
  drawTopBar(now.Hour(), now.Minute(), now.Second());

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(4, 4);
  tft.print("<");

  tft.setTextSize(1);
  tft.setCursor(SCREEN_W - 52, 4);
  tft.print(meds[setupMedIdx].name);

  // ── Icon preview box (top-left, 74×74) ──
  tft.drawRoundRect(8, 28, 74, 74, 6, C_DGREY);
  drawBmpFromSD(ICON_FILES[setupIconIdx], 16, 34, 60, 60);
  tft.setTextColor(C_LGREY, C_BG);
  tft.setTextSize(1);
  tft.setCursor(12, 105);
  tft.print("Choose icon");

  // ── Frequency buttons ──
  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(1);
  tft.setCursor(95, 30);
  tft.print("How many times per day?");

  const char* freqLabels[] = {"1X", "2X", "3X", "cust"};
  int freqVals[]           = {1, 2, 3, 0};
  for (int i = 0; i < 4; i++) {
    int bx  = 95 + i * 54;
    bool sel = (i < 3) ? (setupTimes == freqVals[i]) : (setupTimes > 3);
    drawRoundBtn(bx, 45, 48, 22, freqLabels[i], sel ? C_ACCENT : C_DGREY, C_BG, 1);
  }

  // Custom count up/down (shown only when custom selected)
  if (setupTimes > 3) {
    char buf[12];
    snprintf(buf, sizeof(buf), "Count: %d", setupTimes);
    tft.setTextColor(C_WHITE, C_BG);
    tft.setTextSize(1);
    tft.setCursor(95, 72);
    tft.print(buf);
    drawRoundBtn(160, 68, 24, 16, "+", C_ACCENT, C_BG, 1);
    drawRoundBtn(186, 68, 24, 16, "-", C_DGREY,  C_BG, 1);
  }

  // ── Start time picker ──
  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(1);
  tft.setCursor(95, 95);
  tft.print("Choose Start time:");

  int timeY = 108;
  int hourX = 95;
  int minX  = 148;

  // Up arrows
  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(hourX + 6, timeY - 2);  tft.print("^");
  tft.setCursor(minX  + 6, timeY - 2);  tft.print("^");

  // Hour field
  tft.drawRoundRect(hourX, timeY + 14, 36, 22, 4,
                    setupHourActive ? C_ACCENT : C_DGREY);
  tft.fillRoundRect(hourX + 1, timeY + 15, 34, 20, 4, C_CARD);
  char hbuf[4]; snprintf(hbuf, sizeof(hbuf), "%02d", setupHour);
  tft.setTextColor(C_WHITE, C_CARD);
  tft.setTextSize(2);
  tft.setCursor(hourX + 5, timeY + 18);
  tft.print(hbuf);

  // Colon
  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(137, timeY + 18);
  tft.print(":");

  // Minute field
  tft.drawRoundRect(minX, timeY + 14, 36, 22, 4,
                    !setupHourActive ? C_ACCENT : C_DGREY);
  tft.fillRoundRect(minX + 1, timeY + 15, 34, 20, 4, C_CARD);
  char mbuf[4]; snprintf(mbuf, sizeof(mbuf), "%02d", setupMinute);
  tft.setTextColor(C_WHITE, C_CARD);
  tft.setTextSize(2);
  tft.setCursor(minX + 5, timeY + 18);
  tft.print(mbuf);

  // Down arrows
  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(hourX + 6, timeY + 38);  tft.print("v");
  tft.setCursor(minX  + 6, timeY + 38);  tft.print("v");

  // ── Set Timer button (right side) ──
  tft.fillRoundRect(220, 160, 90, 68, 8, C_ACCENT);
  tft.setTextColor(C_BG, C_ACCENT);
  tft.setTextSize(2);
  tft.setCursor(228, 172);
  tft.print("Set");
  tft.setCursor(222, 192);
  tft.print("timer");
}

// ─────────────────────────────────────────────────────────────
//  ICON PICKER SCREEN
// ─────────────────────────────────────────────────────────────

void drawIconPickScreen() {
  tft.fillScreen(C_BG);

  // Top bar + back arrow + med name
  RtcDateTime now = Rtc.GetDateTime();
  drawTopBar(now.Hour(), now.Minute(), now.Second());

  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(2);
  tft.setCursor(4, 4);
  tft.print("<");

  tft.setTextSize(1);
  tft.setCursor(SCREEN_W - 52, 4);
  tft.print(meds[setupMedIdx].name);

  // Large icon preview (120×120, centered)
  int cx = (SCREEN_W - 120) / 2;
  int cy = 30;
  tft.drawRoundRect(cx - 2, cy - 2, 124, 124, 8, C_DGREY);
  drawBmpFromSD(ICON_FILES[setupIconIdx], cx, cy, 120, 120);

  // Icon label (filename without .bmp)
  char label[20];
  strncpy(label, ICON_FILES[setupIconIdx], sizeof(label));
  for (int i = strlen(label) - 1; i >= 0; i--) {
    if (label[i] == '.') { label[i] = 0; break; }
  }
  tft.setTextColor(C_LGREY, C_BG);
  tft.setTextSize(1);
  int lx = (SCREEN_W - strlen(label) * 6) / 2;
  tft.setCursor(lx, cy + 128);
  tft.print(label);

  // Nav arrows + OK button
  drawRoundBtn(6,              90, 44, 44, "<",  C_CARD,   C_WHITE, 2);
  drawRoundBtn(SCREEN_W - 50, 90, 44, 44, ">",  C_CARD,   C_WHITE, 2);
  drawRoundBtn((SCREEN_W - 80) / 2, 190, 80, 36, "ok", C_ACCENT, C_BG,   2);
}

// ─────────────────────────────────────────────────────────────
//  ALARM SCREEN
// ─────────────────────────────────────────────────────────────

void drawAlarmScreen(int medIdx, int doseIdx) {
  tft.fillScreen(C_BG);

  // Double red border
  tft.drawRoundRect(4, 4, SCREEN_W - 8,  SCREEN_H - 8,  12, C_ALARM);
  tft.drawRoundRect(6, 6, SCREEN_W - 12, SCREEN_H - 12, 10, C_ALARM);

  // "TIME TO TAKE"
  tft.setTextColor(C_ALARM, C_BG);
  tft.setTextSize(2);
  tft.setCursor((SCREEN_W - 13 * 12) / 2, 20);
  tft.print("TIME TO TAKE");

  // Med name
  tft.setTextColor(C_WHITE, C_BG);
  tft.setTextSize(3);
  int nameLen = strlen(meds[medIdx].name);
  tft.setCursor((SCREEN_W - nameLen * 18) / 2, 50);
  tft.print(meds[medIdx].name);

  // Icon
  int iconX = (SCREEN_W - 80) / 2;
  drawBmpFromSD(ICON_FILES[meds[medIdx].iconIndex], iconX, 88, 80, 80);

  // Dose time
  tft.setTextColor(C_LGREY, C_BG);
  tft.setTextSize(1);
  char tbuf[20];
  snprintf(tbuf, sizeof(tbuf), "Dose time: %02d:%02d",
    meds[medIdx].doses[doseIdx].hour,
    meds[medIdx].doses[doseIdx].minute);
  tft.setCursor((SCREEN_W - strlen(tbuf) * 6) / 2, 174);
  tft.print(tbuf);

  // Hint
  tft.setTextColor(C_DGREY, C_BG);
  const char* hint = "Open compartment to dismiss";
  tft.setCursor((SCREEN_W - strlen(hint) * 6) / 2, 188);
  tft.print(hint);

  // Manual dismiss button
  drawRoundBtn((SCREEN_W - 100) / 2, 205, 100, 28, "DISMISS", C_CARD, C_WHITE, 1);
}

// ─────────────────────────────────────────────────────────────
//  SHARED DRAWING HELPERS
// ─────────────────────────────────────────────────────────────

void drawRoundBtn(int x, int y, int w, int h, const char* lbl,
                  uint16_t bg, uint16_t fg, uint8_t textSize) {
  tft.fillRoundRect(x, y, w, h, 6, bg);
  tft.setTextColor(fg, bg);
  tft.setTextSize(textSize);
  int lx = x + (w - (int)strlen(lbl) * 6 * textSize) / 2;
  int ly = y + (h - 8 * textSize) / 2;
  tft.setCursor(lx, ly);
  tft.print(lbl);
}

/*
 * Draws an uncompressed 24-bit BMP from SD card.
 * Falls back to a grey "?" placeholder if the file is missing.
 * Images are scaled to fit the requested w×h area.
 * Tip: use 60×60 or 64×64 BMP files for best performance.
 */
void drawBmpFromSD(const char* filename, int16_t x, int16_t y,
                   int16_t w, int16_t h) {
  File f = SD.open(filename);
  if (!f) {
    tft.fillRect(x, y, w, h, C_DGREY);
    tft.setTextColor(C_LGREY, C_DGREY);
    tft.setTextSize(1);
    tft.setCursor(x + w / 2 - 3, y + h / 2 - 4);
    tft.print("?");
    return;
  }

  // BMP signature
  if (f.read() != 'B' || f.read() != 'M') { f.close(); return; }

  // Skip file size (4 bytes) + reserved (4 bytes)
  for (int i = 0; i < 8; i++) f.read();

  // Pixel data offset (4 bytes, little-endian)
  uint32_t dataOffset = 0;
  for (int i = 0; i < 4; i++) dataOffset |= ((uint32_t)f.read() << (8 * i));

  // Skip DIB header size (4 bytes)
  for (int i = 0; i < 4; i++) f.read();

  // Image width (4 bytes)
  int32_t imgW = 0;
  for (int i = 0; i < 4; i++) imgW |= ((int32_t)f.read() << (8 * i));

  // Image height (4 bytes)
  int32_t imgH = 0;
  for (int i = 0; i < 4; i++) imgH |= ((int32_t)f.read() << (8 * i));

  // Row size padded to 4-byte boundary
  int rowBytes = ((imgW * 3 + 3) / 4) * 4;

  // Draw rows bottom-up (standard BMP order)
  for (int row = imgH - 1; row >= 0; row--) {
    f.seek(dataOffset + row * rowBytes);
    int drawY = y + (imgH - 1 - row) * h / imgH;
    for (int col = 0; col < imgW; col++) {
      uint8_t  b     = f.read();
      uint8_t  g     = f.read();
      uint8_t  r     = f.read();
      uint16_t color = tft.color565(r, g, b);
      int drawX = x + col * w / imgW;
      tft.drawPixel(drawX, drawY, color);
    }
  }
  f.close();
}