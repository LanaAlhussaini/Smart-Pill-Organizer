#pragma once
#include "config.h"

// ─── Task 3: برمجة المخرجات على الشاشة ───────────────────────

// ── Home Screen ───────────────────────────────────────────────
void drawHome();
void drawTopBar(uint8_t h, uint8_t m, uint8_t s);
// forceRedraw=true redraws entire card; false only refreshes countdown
void drawMedCard(int idx, bool forceRedraw);

// ── Setup Screen ──────────────────────────────────────────────
void openSetup(int medIdx);   // initializes state then calls drawSetupScreen
void drawSetupScreen();

// ── Icon Picker Screen ────────────────────────────────────────
void drawIconPickScreen();

// ── Alarm Screen ──────────────────────────────────────────────
void drawAlarmScreen(int medIdx, int doseIdx);

// ── Shared Drawing Helpers ────────────────────────────────────
void drawRoundBtn(int x, int y, int w, int h, const char* lbl,
                  uint16_t bg, uint16_t fg, uint8_t textSize = 2);
void drawBmpFromSD(const char* filename, int16_t x, int16_t y,
                   int16_t w, int16_t h);