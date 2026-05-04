
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h> 
#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include <Adafruit_SHT4x.h>

// --- Pin Definitions ---
#define TFT_CS   15
#define TFT_DC    4
#define TFT_RST  16
#define TS_CS    5

#define RTC_RST   17   
#define RTC_DAT   27   
#define RTC_CLK   14  

#define LED_PIN    2
#define BUZZER_PIN 13 
#define REED_1    32 
#define REED_2    34
#define REED_3    25

// --- Objects ---
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen touch(TS_CS);
Adafruit_SHT4x sht4 = Adafruit_SHT4x(); 
ThreeWire myWire(RTC_DAT, RTC_CLK, RTC_RST); 
RtcDS1302<ThreeWire> Rtc(myWire);

// --- Variables ---
bool ledOn = false, buzOn = false, senOn = true;
int lastSec = -1;
float lastT = -1.0;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Initializing System...");

  // Pin Modes
  pinMode(TFT_RST, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(REED_1, INPUT_PULLUP);
  pinMode(REED_2, INPUT_PULLUP);
  pinMode(REED_3, INPUT_PULLUP);

  // TFT Reset
  digitalWrite(TFT_RST, LOW); delay(100);
  digitalWrite(TFT_RST, HIGH); delay(100);

  // Start Hardware
  tft.begin();
  tft.setRotation(1);
  
  if (!touch.begin()) {
    Serial.println("Touch controller NOT found. Check T_CS (Pin 5) and SPI wiring!");
  }
  touch.setRotation(1);

  Rtc.Begin();
  Rtc.SetIsWriteProtected(false);
  Rtc.SetIsRunning(true);
  
  Wire.begin(21, 22); 
  sht4.begin();

  tft.fillScreen(ILI9341_BLACK);
  tft.drawFastHLine(0, 165, 320, ILI9341_WHITE);
  refreshButtons();
}

void refreshButtons() {
  drawBtn(10, 180, 90, 50, "LED", ledOn);
  drawBtn(115, 180, 90, 50, "BUZZ", buzOn);
  drawBtn(220, 180, 90, 50, "SENS", senOn);
}

void drawBtn(int x, int y, int w, int h, const char* lbl, bool act) {
  tft.fillRoundRect(x, y, w, h, 8, act ? ILI9341_GREEN : ILI9341_RED);
  tft.setCursor(x + 15, y + 18);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.print(lbl);
}

void loop() {
  // 1. Touch Processing
  if (touch.touched()) {
    TS_Point p = touch.getPoint();
    
    // Serial debugging to verify touch hardware is alive
    Serial.printf("Touch Raw: X=%d, Y=%d\n", p.x, p.y);

    // Coordinate Mapping
    int tx = map(p.x, 200, 3800, 0, 320); 
    int ty = map(p.y, 200, 3800, 0, 240);

    if (ty > 170) {
      if (tx < 105) ledOn = !ledOn;
      else if (tx < 215) buzOn = !buzOn;
      else if (tx < 320) senOn = !senOn;
      
      refreshButtons();
      delay(250); // Debounce
    }
  }

  // 2. Output Control
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  if (buzOn) tone(BUZZER_PIN, 1000); else noTone(BUZZER_PIN);

  // 3. UI Redraw (Once per second)
  RtcDateTime n = Rtc.GetDateTime();
  if (n.Second() != lastSec) {
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    
    // Time
    tft.setCursor(10, 10);
    tft.printf("TIME: %02u:%02u:%02u  ", n.Hour(), n.Minute(), n.Second());
    
    // Reeds
    tft.setCursor(10, 80);
    tft.print("REEDS: ");
    showReed(REED_1, "1"); tft.print(" ");
    showReed(REED_2, "2"); tft.print(" ");
    showReed(REED_3, "3");

    // Sensor Update
    if (senOn) {
      sensors_event_t h, t;
      sht4.getEvent(&h, &t);
      tft.setCursor(10, 45);
      tft.setTextColor(ILI9341_CYAN, ILI9341_BLACK);
      tft.printf("T: %.1fC  H: %.1f%%  ", t.temperature, h.relative_humidity);
    } else {
      tft.setCursor(10, 45);
      tft.setTextColor(ILI9341_DARKGREY, ILI9341_BLACK);
      tft.print("SENSORS DISABLED   ");
    }

    lastSec = n.Second();
  }
}

void showReed(int p, const char* n) {
  if (digitalRead(p) == HIGH) {
    tft.setTextColor(ILI9341_RED, ILI9341_BLACK); tft.printf("R%s:O", n);
  } else {
    tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK); tft.printf("R%s:C", n);
  }
}