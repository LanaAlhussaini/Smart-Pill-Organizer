#include "timers.h"
#include "gui.h"  // for drawHome(), drawMedCard()

// ══════════════════════════════════════════════════════════════
//  Task 1 – كتابة المنطق الأساسي للمؤقت
// ══════════════════════════════════════════════════════════════

/*
 * Called every loop iteration.
 * Compares current RTC time against each med's dose schedule.
 * Fires alarm when hour:minute matches and dose not yet taken.
 * Triggers within the first 5 seconds of the matching minute.
 */
void checkTimers(RtcDateTime& now) {
  for (int i = 0; i < MAX_MEDS; i++) {
    if (!meds[i].active)      continue;
    if (meds[i].alarmFiring)  continue;  // already alarming this slot

    for (int d = 0; d < meds[i].timesPerDay; d++) {
      if (meds[i].doseTaken[d]) continue;

      if (meds[i].doses[d].hour   == now.Hour()   &&
          meds[i].doses[d].minute == now.Minute()  &&
          now.Second() < 5) {
        meds[i].alarmFiring  = true;
        meds[i].alarmDoseIdx = d;
        Serial.printf("[TIMER] Alarm fired: med %d, dose %d at %02d:%02d\n",
          i, d, meds[i].doses[d].hour, meds[i].doses[d].minute);
        break;
      }
    }
  }

  // Midnight reset – clear all taken flags for a new day
  if (now.Hour() == 0 && now.Minute() == 0 && now.Second() < 5) {
    for (int i = 0; i < MAX_MEDS; i++) resetDailyFlags(i);
    Serial.println("[TIMER] Daily flags reset at midnight.");
  }
}

/*
 * Checks all three reed switch pins each loop.
 * A HIGH reading means the magnet is gone = lid is OPEN = dose taken.
 * Stops the alarm and returns to the home screen.
 */
void checkReedSwitches() {
  for (int i = 0; i < MAX_MEDS; i++) {
    if (!meds[i].active)      continue;
    if (!meds[i].alarmFiring) continue;

    if (digitalRead(meds[i].reedPin) == HIGH) {
      int d = meds[i].alarmDoseIdx;
      meds[i].doseTaken[d] = true;
      meds[i].alarmFiring  = false;

      noTone(BUZZER_PIN);
      digitalWrite(LED_PIN, LOW);

      Serial.printf("[REED] Med %d dose %d taken (compartment opened).\n", i, d);

      // Return to home or refresh card
      if (currentScreen == SCR_ALARM && alarmMedIdx == i) {
        currentScreen = SCR_HOME;
        drawHome();
      } else if (currentScreen == SCR_HOME) {
        drawMedCard(i, true);
      }
    }
  }
}

/*
 * Evenly distributes timesPerDay doses across 24 hours,
 * starting from the user-configured start time in doses[0].
 *
 * Example: start=08:00, timesPerDay=3
 *   doses[0] = 08:00
 *   doses[1] = 16:00
 *   doses[2] = 00:00
 */
void computeDoseTimes(int medIdx) {
  Med& m = meds[medIdx];
  int startMins = m.doses[0].hour * 60 + m.doses[0].minute;
  int interval  = (m.timesPerDay > 1) ? (24 * 60 / m.timesPerDay) : 0;

  for (int d = 0; d < m.timesPerDay; d++) {
    int total        = (startMins + d * interval) % (24 * 60);
    m.doses[d].hour   = total / 60;
    m.doses[d].minute = total % 60;
    Serial.printf("[TIMER] Med %d dose %d: %02d:%02d\n",
      medIdx, d, m.doses[d].hour, m.doses[d].minute);
  }
}

void resetDailyFlags(int medIdx) {
  for (int d = 0; d < 9; d++) meds[medIdx].doseTaken[d] = false;
}