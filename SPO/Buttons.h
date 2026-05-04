#pragma once
#include "config.h"

// ─── Task 2: كتابة منطق الأزرار ──────────────────────────────
// One handler per screen. Called from the main loop
// after touch coordinates are mapped and debounced.

void handleHomeTouch(int tx, int ty);
void handleSetupTouch(int tx, int ty);
void handleIconPickTouch(int tx, int ty);
void handleAlarmTouch(int tx, int ty);

// ─── Shared Touch Utilities ───────────────────────────────────
bool touchInRect(int tx, int ty, int x, int y, int w, int h);
int  mapTouch(int raw, int inMin, int inMax, int outMin, int outMax);