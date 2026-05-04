#pragma once
#include "config.h"

// ─── Task 1: كتابة المنطق الأساسي للمؤقت ─────────────────────
// Checks if any dose alarm should fire based on current RTC time.
void checkTimers(RtcDateTime& now);

// Checks reed switches; opening a compartment = dose taken, stops alarm.
void checkReedSwitches();

// Distributes dose times evenly across the day starting from doses[0].
void computeDoseTimes(int medIdx);

// Resets all taken flags for a med (called at midnight).
void resetDailyFlags(int medIdx);