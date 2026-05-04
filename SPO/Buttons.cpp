#include "buttons.h"
#include "gui.h"
#include "timers.h"

// ══════════════════════════════════════════════════════════════
//  Task 2 – كتابة منطق الأزرار
// ══════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────
//  HOME SCREEN
//  Tapping any card opens its setup screen.
// ─────────────────────────────────────────────────────────────
void handleHomeTouch(int tx, int ty) {
  for (int i = 0; i < MAX_MEDS; i++) {
    int cx = CARD_START_X + i * (CARD_W + CARD_GAP);
    int cy = CARD_Y;
    if (touchInRect(tx, ty, cx, cy, CARD_W, CARD_H)) {
      openSetup(i);
      return;
    }
  }
}

// ─────────────────────────────────────────────────────────────
//  SETUP SCREEN
//  Handles: back arrow, icon box, frequency buttons,
//           custom count arrows, time field selection,
//           time up/down arrows, "Set timer" button.
// ─────────────────────────────────────────────────────────────
void handleSetupTouch(int tx, int ty) {

  // ── Back arrow ──
  if (touchInRect(tx, ty, 0, 0, 30, 24)) {
    currentScreen = SCR_HOME;
    drawHome();
    return;
  }

  // ── Icon preview box → open icon picker ──
  if (touchInRect(tx, ty, 8, 28, 74, 74)) {
    currentScreen = SCR_ICON_PICK;
    drawIconPickScreen();
    return;
  }

  // ── Frequency buttons: 1X 2X 3X cust ──
  for (int i = 0; i < 4; i++) {
    int bx = 95 + i * 54;
    if (touchInRect(tx, ty, bx, 45, 48, 22)) {
      if      (i == 0) setupTimes = 1;
      else if (i == 1) setupTimes = 2;
      else if (i == 2) setupTimes = 3;
      else if (i == 3 && setupTimes <= 3) setupTimes = 4; // enter custom mode
      drawSetupScreen();
      return;
    }
  }

  // ── Custom count: + / - ──
  if (setupTimes > 3) {
    if (touchInRect(tx, ty, 160, 68, 24, 16) && setupTimes < 9) {
      setupTimes++;
      drawSetupScreen();
      return;
    }
    if (touchInRect(tx, ty, 186, 68, 24, 16) && setupTimes > 4) {
      setupTimes--;
      drawSetupScreen();
      return;
    }
  }

  // ── Time fields: tap to select hour or minute ──
  int timeY = 108;
  int hourX = 95;
  int minX  = 148;

  if (touchInRect(tx, ty, hourX, timeY + 14, 36, 22)) {
    setupHourActive = true;
    drawSetupScreen();
    return;
  }
  if (touchInRect(tx, ty, minX, timeY + 14, 36, 22)) {
    setupHourActive = false;
    drawSetupScreen();
    return;
  }

  // ── Up arrows (above the time fields) ──
  if (touchInRect(tx, ty, hourX, timeY - 4, 36, 16)) {
    setupHour   = (setupHour + 1) % 24;
    setupHourActive = true;
    drawSetupScreen();
    return;
  }
  if (touchInRect(tx, ty, minX, timeY - 4, 36, 16)) {
    setupMinute = (setupMinute + 1) % 60;
    setupHourActive = false;
    drawSetupScreen();
    return;
  }

  // ── Down arrows (below the time fields) ──
  if (touchInRect(tx, ty, hourX, timeY + 38, 36, 16)) {
    setupHour   = (setupHour + 23) % 24;
    setupHourActive = true;
    drawSetupScreen();
    return;
  }
  if (touchInRect(tx, ty, minX, timeY + 38, 36, 16)) {
    setupMinute = (setupMinute + 59) % 60;
    setupHourActive = false;
    drawSetupScreen();
    return;
  }

  // ── "Set timer" button ──
  if (touchInRect(tx, ty, 220, 160, 90, 68)) {
    Med& m          = meds[setupMedIdx];
    m.active        = true;
    m.iconIndex     = setupIconIdx;
    m.timesPerDay   = setupTimes;
    m.doses[0].hour   = setupHour;
    m.doses[0].minute = setupMinute;
    computeDoseTimes(setupMedIdx);  // fill in remaining dose times
    for (int d = 0; d < 9; d++) m.doseTaken[d] = false;
    m.alarmFiring = false;

    Serial.printf("[BTN] Med %d saved: icon=%d times=%d start=%02d:%02d\n",
      setupMedIdx, setupIconIdx, setupTimes, setupHour, setupMinute);

    currentScreen = SCR_HOME;
    drawHome();
  }
}

// ─────────────────────────────────────────────────────────────
//  ICON PICKER SCREEN
//  Left/right arrows scroll icons, OK confirms selection.
// ─────────────────────────────────────────────────────────────
void handleIconPickTouch(int tx, int ty) {

  // ── Back arrow ──
  if (touchInRect(tx, ty, 0, 0, 30, 24)) {
    currentScreen = SCR_SETUP;
    drawSetupScreen();
    return;
  }

  // ── Left arrow ──
  if (touchInRect(tx, ty, 6, 70, 44, 90)) {
    setupIconIdx = (setupIconIdx + MAX_ICONS - 1) % MAX_ICONS;
    drawIconPickScreen();
    return;
  }

  // ── Right arrow ──
  if (touchInRect(tx, ty, SCREEN_W - 50, 70, 44, 90)) {
    setupIconIdx = (setupIconIdx + 1) % MAX_ICONS;
    drawIconPickScreen();
    return;
  }

  // ── OK button ──
  if (touchInRect(tx, ty, (SCREEN_W - 80) / 2, 190, 80, 36)) {
    currentScreen = SCR_SETUP;
    drawSetupScreen();
    return;
  }
}

// ─────────────────────────────────────────────────────────────
//  ALARM SCREEN
//  Manual dismiss button (backup for when reed switch fails).
//  Primary dismissal is via reed switch in timers.cpp.
// ─────────────────────────────────────────────────────────────
void handleAlarmTouch(int tx, int ty) {
  if (touchInRect(tx, ty, (SCREEN_W - 100) / 2, 205, 100, 28)) {
    if (alarmMedIdx >= 0) {
      meds[alarmMedIdx].doseTaken[alarmDoseIdx] = true;
      meds[alarmMedIdx].alarmFiring             = false;
    }
    noTone(BUZZER_PIN);
    digitalWrite(LED_PIN, LOW);
    currentScreen = SCR_HOME;
    drawHome();
  }
}

// ─────────────────────────────────────────────────────────────
//  SHARED TOUCH UTILITIES
// ─────────────────────────────────────────────────────────────

bool touchInRect(int tx, int ty, int x, int y, int w, int h) {
  return (tx >= x && tx <= x + w && ty >= y && ty <= y + h);
}

int mapTouch(int raw, int inMin, int inMax, int outMin, int outMax) {
  raw = constrain(raw, inMin, inMax);
  return map(raw, inMin, inMax, outMin, outMax);
}