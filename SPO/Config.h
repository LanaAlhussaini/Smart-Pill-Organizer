#pragma once

// ─── Libraries ────────────────────────────────────────────────
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <Adafruit_SHT4x.h>

// ─── Pin Definitions ──────────────────────────────────────────
#define TFT_CS    15
#define TFT_DC     4
#define TFT_RST   16
#define TS_CS      5
#define SD_CS     33

#define RTC_RST   17
#define RTC_DAT   27
#define RTC_CLK   14

#define LED_PIN    2
#define BUZZER_PIN 13

#define REED_1    32
#define REED_2    34
#define REED_3    25

// ─── Colors (ILI9341 RGB565) ──────────────────────────────────
#define C_BG        0x0841
#define C_CARD      0x1082
#define C_CARD_SEL  0x2124
#define C_GREEN     0x07E0
#define C_RED       0xF800
#define C_YELLOW    0xFFE0
#define C_WHITE     0xFFFF
#define C_LGREY     0xC618
#define C_DGREY     0x4208
#define C_ACCENT    0x07E0
#define C_ALARM     0xF800

// ─── Screen / Layout Constants ────────────────────────────────
#define SCREEN_W      320
#define SCREEN_H      240
#define TOPBAR_H       22
#define CARD_W         90
#define CARD_H        170
#define CARD_GAP        8
#define CARD_START_X  ((SCREEN_W - (3*CARD_W + 2*CARD_GAP)) / 2)
#define CARD_Y        (TOPBAR_H + 4)

// ─── App Constants ────────────────────────────────────────────
#define MAX_MEDS          3
#define MAX_ICONS        10
#define TOUCH_DEBOUNCE_MS 220

// ─── Icon filenames (stored on SD card root) ──────────────────
extern const char* ICON_FILES[MAX_ICONS];

// ─── Data Structures ──────────────────────────────────────────
struct DoseTime {
  uint8_t hour;
  uint8_t minute;
};

struct Med {
  bool     active;
  char     name[10];
  uint8_t  iconIndex;
  uint8_t  timesPerDay;
  DoseTime doses[9];
  bool     doseTaken[9];
  bool     alarmFiring;
  uint8_t  alarmDoseIdx;
  uint8_t  reedPin;
};

// ─── Screen Enum ──────────────────────────────────────────────
enum Screen { SCR_HOME, SCR_SETUP, SCR_ICON_PICK, SCR_ALARM };

// ─── Shared Hardware Objects (defined in main .ino) ───────────
extern Adafruit_ILI9341     tft;
extern XPT2046_Touchscreen  touch;
extern RtcDS1302<ThreeWire> Rtc;
extern Adafruit_SHT4x       sht4;

// ─── Shared Global State (defined in main .ino) ───────────────
extern Med    meds[MAX_MEDS];
extern Screen currentScreen;

extern int     setupMedIdx;
extern uint8_t setupIconIdx;
extern uint8_t setupTimes;
extern uint8_t setupHour;
extern uint8_t setupMinute;
extern bool    setupHourActive;

extern int alarmMedIdx;
extern int alarmDoseIdx;

extern int     lastDrawnSecond;
extern uint8_t lastDrawnMin;
extern unsigned long lastTouchMs;