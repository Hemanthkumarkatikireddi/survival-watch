#include "display.h"
#include "config.h"
#include "state.h"

Adafruit_PCD8544 display(
  LCD_CLK,
  LCD_DIN,
  LCD_DC,
  LCD_CE,
  LCD_RST
);

// =====================================================
// DISPLAY INITIALIZATION
// =====================================================

void initDisplay() {

  display.begin();

  display.setContrast(50);

  display.clearDisplay();
  display.setTextColor(BLACK);
  display.setTextSize(1);

  display.setCursor(0, 10);
  display.println("ESP32 WATCH");

  display.setCursor(0, 25);
  display.println("STARTING...");

  display.display();

  delay(1000);
}

// =====================================================
// BACKLIGHT
// =====================================================

void setBacklight(bool state) {

  backlightState = state;

  digitalWrite(
    LCD_BL,
    state ? HIGH : LOW
  );

  if (state) {
    backlightStart = millis();
  }

  Serial.print("BACKLIGHT: ");
  Serial.println(state ? "ON" : "OFF");
}

// =====================================================
// BACKLIGHT AUTO OFF
// =====================================================

void updateBacklight() {

  if (!backlightState) {
    return;
  }

  // Keep ON when these systems are active.
  if (
    gpsActive ||
    compassActive ||
    trackBackActive
  ) {
    return;
  }

  if (
    millis() - backlightStart >=
    BACKLIGHT_TIMEOUT
  ) {
    setBacklight(false);
  }
}

// =====================================================
// 12 / 24 HOUR
// =====================================================

void getDisplayHour(
  int hour24,
  int &hourDisplay,
  bool &pm
) {

  if (use24Hour) {

    hourDisplay = hour24;
    pm = false;

    return;
  }

  pm = hour24 >= 12;

  hourDisplay = hour24 % 12;

  if (hourDisplay == 0) {
    hourDisplay = 12;
  }
}
