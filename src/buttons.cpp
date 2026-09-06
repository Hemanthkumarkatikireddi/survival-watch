#include "buttons.h"
#include "config.h"
#include "state.h"
#include "display.h"
#include "menu.h"
#include "time_page.h"
#include "alarm_page.h"
#include "stopwatch_page.h"
#include "home.h"

// =====================================================
// BACKLIGHT TIMER
// =====================================================

void updateBacklightTimer() {

  // Backlight OFF
  if (!backlightState) {
    return;
  }

  // GPS or Compass ON
  // Keep backlight ON
  if (
    gpsActive ||
    compassActive
  ) {
    return;
  }

  // Auto OFF only on HOME
  if (currentScreen != HOME) {
    return;
  }

  // 10 seconds completed
  if (
    millis() -
    backlightStart >=
    BACKLIGHT_TIMEOUT
  ) {

    setBacklight(false);

    Serial.println(
      "BACKLIGHT AUTO OFF"
    );
  }
}

// =====================================================
// RESET BACKLIGHT TIMER
// =====================================================

void resetBacklightTimer() {

  if (!backlightState) {
    return;
  }

  backlightStart = millis();
}

// =====================================================
// UP SHORT PRESS
// =====================================================

void handleUpShortPress() {

  // -----------------------------
  // HOME
  // -----------------------------

  if (currentScreen == HOME) {

    // Backlight ON -> OFF
    if (backlightState) {

      setBacklight(false);

      Serial.println(
        "BACKLIGHT OFF - UP"
      );

      return;
    }

    // Backlight OFF -> ON
    setBacklight(true);

    backlightStart = millis();

    Serial.println(
      "BACKLIGHT ON - UP"
    );

    return;
  }

  // -----------------------------
  // MENU
  // -----------------------------

  if (currentScreen == MENU) {

    resetBacklightTimer();

    menuUp();

    return;
  }

  // -----------------------------
  // STOPWATCH
  // -----------------------------

  if (
    currentScreen ==
    STOPWATCH_PAGE
  ) {

    resetBacklightTimer();

    stopwatchReset();

    return;
  }

  // -----------------------------
  // TIME
  // -----------------------------

  if (
    currentScreen == TIME_PAGE &&
    timeSetting
  ) {

    resetBacklightTimer();

    timeValueUp();

    return;
  }

  // -----------------------------
  // ALARM
  // -----------------------------

  if (
    currentScreen == ALARM_PAGE &&
    alarmSetting
  ) {

    resetBacklightTimer();

    alarmValueUp();

    return;
  }
}

// =====================================================
// UP LONG PRESS
// HOME ONLY
// =====================================================

void handleUpLongPress() {

  if (currentScreen != HOME) {
    return;
  }

  use24Hour = !use24Hour;

  resetBacklightTimer();

  Serial.print(
    "TIME FORMAT: "
  );

  Serial.println(
    use24Hour ?
    "24H" :
    "12H"
  );
}

// =====================================================
// UP BUTTON PRESS
// =====================================================

void handleUpPress() {

  if (alarmRinging) {

    snoozeAlarm();

    return;
  }
}

// =====================================================
// DOWN BUTTON
// =====================================================

void handleDownPress() {

  if (alarmRinging) {

    snoozeAlarm();

    return;
  }

  resetBacklightTimer();

  // -----------------------------
  // MENU
  // -----------------------------

  if (currentScreen == MENU) {

    menuDown();

    return;
  }

  // -----------------------------
  // TIME
  // -----------------------------

  if (
    currentScreen == TIME_PAGE &&
    timeSetting
  ) {

    timeValueDown();

    return;
  }

  // -----------------------------
  // ALARM
  // -----------------------------

  if (
    currentScreen == ALARM_PAGE &&
    alarmSetting
  ) {

    alarmValueDown();

    return;
  }

  // -----------------------------
  // HOME
  // DOWN = CHANGE HOME VIEW
  // -----------------------------

  if (currentScreen == HOME) {

    // Do nothing while Moon View is active.
    if (isMoonView()) {
      return;
    }

    toggleHomeView();

    return;
  }
}

// =====================================================
// MENU BUTTON
// =====================================================

void handleMenuPress() {

  if (alarmRinging) {
    return;
  }

  // -----------------------------
  // HOME -> MENU
  // -----------------------------

  if (currentScreen == HOME) {

    resetBacklightTimer();

    currentScreen = MENU;

    menuIndex = 0;
    menuTop = 0;

    Serial.println(
      "MENU OPEN"
    );

    return;
  }

  // -----------------------------
  // MENU -> OPEN ITEM
  // -----------------------------

  if (currentScreen == MENU) {

    resetBacklightTimer();

    openMenuItem();

    return;
  }

  // -----------------------------
  // TIME
  // -----------------------------

  if (currentScreen == TIME_PAGE) {

    resetBacklightTimer();

    if (!timeSetting) {

      startTimeSetting();

      return;
    }

    timeSetStep++;

    if (timeSetStep > 2) {

      saveTime();

      timeSetting = false;
      timeSetStep = 0;

      Serial.println(
        "TIME SET COMPLETE"
      );

      return;
    }

    return;
  }

  // -----------------------------
  // ALARM
  // -----------------------------

  if (currentScreen == ALARM_PAGE) {

    resetBacklightTimer();

    if (!alarmSetting) {

      alarmSetting = true;
      alarmSetStep = 0;

      Serial.println(
        "ALARM SETTING START"
      );

      return;
    }

    alarmSetStep++;

    if (alarmSetStep > 2) {

      alarmSetting = false;
      alarmSetStep = 0;
      alarmEnabled = true;

      Serial.println(
        "ALARM ENABLED"
      );

      return;
    }

    return;
  }

  // -----------------------------
  // STOPWATCH
  // -----------------------------

  if (
    currentScreen ==
    STOPWATCH_PAGE
  ) {

    resetBacklightTimer();

    stopwatchSelect();

    return;
  }

  // -----------------------------
  // COMPASS
  // -----------------------------

  if (
    currentScreen ==
    COMPASS_PAGE
  ) {

    resetBacklightTimer();

    compassActive =
      !compassActive;

    if (compassActive) {

      setBacklight(true);

      backlightStart =
        millis();
    }

    Serial.print(
      "COMPASS: "
    );

    Serial.println(
      compassActive ?
      "ON" :
      "OFF"
    );

    return;
  }

  // -----------------------------
  // GPS
  // -----------------------------

  if (
    currentScreen ==
    GPS_PAGE
  ) {

    resetBacklightTimer();

    gpsActive =
      !gpsActive;

    if (gpsActive) {

      setBacklight(true);

      backlightStart =
        millis();
    }

    Serial.print(
      "GPS: "
    );

    Serial.println(
      gpsActive ?
      "ON" :
      "OFF"
    );

    return;
  }
}

// =====================================================
// BACK BUTTON
//
// Physical SELECT = BACK
// =====================================================

void handleBackPress() {

  // -----------------------------
  // ALARM RINGING
  // -----------------------------

  if (alarmRinging) {

    stopAlarm();

    return;
  }

  // -----------------------------
  // HOME
  // -----------------------------

  if (currentScreen == HOME) {

    resetBacklightTimer();

    // -----------------------------------------------
    // NORMAL HOME -> MOON VIEW
    // -----------------------------------------------

    toggleMoonView();

    return;
  }

  // -----------------------------
  // MENU -> HOME
  // -----------------------------

  if (currentScreen == MENU) {

    resetBacklightTimer();

    currentScreen = HOME;

    Serial.println(
      "HOME"
    );

    return;
  }

  // -----------------------------
  // ANY PAGE -> MENU
  // -----------------------------

  resetBacklightTimer();

  currentScreen = MENU;

  Serial.println(
    "BACK TO MENU"
  );
}

// =====================================================
// BACK LONG PRESS
// =====================================================

void handleBackLongPress() {

  // No special action currently.
}

// =====================================================
// UPDATE BACKLIGHT / UP LONG PRESS
//
// main.cpp currently calls:
// updateDoubleClick();
//
// Double-click is no longer used.
// =====================================================

void updateDoubleClick() {

  static bool upLongHandled = false;

  // -----------------------------
  // UP HELD
  // -----------------------------

  if (
    upLast == LOW &&
    !upLongHandled
  ) {

    if (
      millis() -
      upPressTime >=
      LONG_PRESS_TIME
    ) {

      if (currentScreen == HOME) {

        handleUpLongPress();

        upLongHandled = true;
      }
    }
  }

  // -----------------------------
  // BACKLIGHT AUTO OFF
  // -----------------------------

  updateBacklightTimer();

  // -----------------------------
  // UP RELEASE
  // -----------------------------

  if (upLast == HIGH) {

    upLongHandled = false;
  }
}

// =====================================================
// READ BUTTONS
// =====================================================

void handleButtons() {

  bool up =
    digitalRead(BTN_UP);

  bool down =
    digitalRead(BTN_DOWN);

  bool menu =
    digitalRead(BTN_MENU);

  // Physical SELECT button
  // is now used as BACK
  bool back =
    digitalRead(BTN_SELECT);

  // ===================================================
  // UP PRESS
  // ===================================================

  if (
    up == LOW &&
    upLast == HIGH
  ) {

    upPressTime =
      millis();

    handleUpPress();
  }

  // ===================================================
  // UP RELEASE
  // ===================================================

  if (
    up == HIGH &&
    upLast == LOW
  ) {

    unsigned long duration =
      millis() -
      upPressTime;

    if (
      duration <
      LONG_PRESS_TIME
    ) {

      handleUpShortPress();
    }
  }

  // ===================================================
  // DOWN
  // ===================================================

  if (
    down == LOW &&
    downLast == HIGH
  ) {

    handleDownPress();
  }

  // ===================================================
  // MENU
  // ===================================================

  if (
    menu == LOW &&
    menuLast == HIGH
  ) {

    handleMenuPress();
  }

  // ===================================================
  // BACK PRESS
  // ===================================================

  if (
    back == LOW &&
    selectLast == HIGH
  ) {

    selectPressTime =
      millis();
  }

  // ===================================================
  // BACK RELEASE
  // ===================================================

  if (
    back == HIGH &&
    selectLast == LOW
  ) {

    unsigned long duration =
      millis() -
      selectPressTime;

    if (
      duration >=
      LONG_PRESS_TIME
    ) {

      handleBackLongPress();

    } else {

      handleBackPress();
    }
  }

  // ===================================================
  // SAVE BUTTON STATES
  // ===================================================

  upLast = up;
  downLast = down;
  menuLast = menu;
  selectLast = back;
}