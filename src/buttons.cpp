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

void updateBacklightTimer()
{
  // Backlight OFF
  if (!backlightState)
  {
    return;
  }

  // GPS or Compass ON
  // Keep backlight ON
  if (
    gpsActive ||
    compassActive
  )
  {
    return;
  }

  // Auto OFF only on HOME
  if (currentScreen != HOME)
  {
    return;
  }

  // 10 seconds completed
  if (
    millis() -
    backlightStart >=
    BACKLIGHT_TIMEOUT
  )
  {
    setBacklight(false);

    Serial.println(
      "BACKLIGHT AUTO OFF"
    );
  }
}


// =====================================================
// RESET BACKLIGHT TIMER
// =====================================================

void resetBacklightTimer()
{
  if (!backlightState)
  {
    return;
  }

  backlightStart = millis();
}


// =====================================================
// UP SHORT PRESS
// =====================================================

void handleUpShortPress()
{
  // ===================================================
  // HOME
  // ===================================================

  if (currentScreen == HOME)
  {
    // Backlight ON -> OFF
    if (backlightState)
    {
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


  // ===================================================
  // TIMEZONE MENU
  // ===================================================

  if (
    currentScreen ==
    TIMEZONE_PAGE
  )
  {
    resetBacklightTimer();

    timezoneMenuUp();

    return;
  }


  // ===================================================
  // CLOCK MENU
  // ===================================================

  if (
    currentScreen ==
    CLOCK_MENU_PAGE
  )
  {
    resetBacklightTimer();

    clockMenuUp();

    return;
  }

  if (currentScreen == SNOOZE_PAGE)
{
    snoozeEnabled = !snoozeEnabled;
    resetBacklightTimer();
    return;
}

  // ===================================================
  // MAIN MENU
  // ===================================================

  if (currentScreen == MENU)
  {
    resetBacklightTimer();

    menuUp();

    return;
  }


  // ===================================================
  // STOPWATCH
  // UP = RESET
  // ===================================================

  if (
    currentScreen ==
    STOPWATCH_PAGE
  )
  {
    resetBacklightTimer();

    stopwatchReset();

    return;
  }


  // ===================================================
  // TIME SETTING
  // ===================================================

  if (
    currentScreen == TIME_PAGE &&
    timeSetting
  )
  {
    resetBacklightTimer();

    timeValueUp();

    return;
  }


  // ===================================================
  // ALARM MENU
  // ===================================================

  if (
    currentScreen ==
    ALARM_MENU_PAGE
  )
  {
    resetBacklightTimer();

    alarmMenuUp();

    return;
  }


  // ===================================================
  // ALARM SETTING
  // ===================================================

  if (
    currentScreen == ALARM_PAGE &&
    alarmSetting
  )
  {
    resetBacklightTimer();

    alarmValueUp();

    return;
  }
}


// =====================================================
// UP LONG PRESS
// HOME ONLY
// =====================================================

void handleUpLongPress()
{
  if (currentScreen != HOME)
  {
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

void handleUpPress()
{
  if (alarmRinging)
  {
    snoozeAlarm();

    return;
  }
}


// =====================================================
// DOWN BUTTON
// =====================================================

void handleDownPress()
{
  // ===================================================
  // ALARM RINGING
  // ===================================================

  if (alarmRinging)
  {
    snoozeAlarm();

    return;
  }


  resetBacklightTimer();


  // ===================================================
  // TIMEZONE MENU
  // ===================================================

  if (
    currentScreen ==
    TIMEZONE_PAGE
  )
  {
    timezoneMenuDown();

    return;
  }

    if (currentScreen == SNOOZE_PAGE)
  {
      snoozeEnabled = !snoozeEnabled;
      resetBacklightTimer();
      return;
  }

  // ===================================================
  // CLOCK MENU
  // ===================================================

  if (
    currentScreen ==
    CLOCK_MENU_PAGE
  )
  {
    clockMenuDown();

    return;
  }


  // ===================================================
  // MAIN MENU
  // ===================================================

  if (currentScreen == MENU)
  {
    menuDown();

    return;
  }


  // ===================================================
  // TIME SETTING
  // ===================================================

  if (
    currentScreen == TIME_PAGE &&
    timeSetting
  )
  {
    timeValueDown();

    return;
  }


  // ===================================================
  // ALARM MENU
  // ===================================================

  if (
    currentScreen ==
    ALARM_MENU_PAGE
  )
  {
    alarmMenuDown();

    return;
  }


  // ===================================================
  // ALARM SETTING
  // ===================================================

  if (
    currentScreen == ALARM_PAGE &&
    alarmSetting
  )
  {
    alarmValueDown();

    return;
  }


  // ===================================================
  // HOME
  // ===================================================

  if (currentScreen == HOME)
  {
    // =================================================
    // BACK BLANK VIEW
    // DOWN -> NORMAL HOME
    // =================================================

    if (isBackBlankView())
    {
      setBackBlankView(false);

      setHomeTithiView(false);

      Serial.println(
        "BACK BLANK -> HOME"
      );

      return;
    }


    // =================================================
    // MOON VIEW
    // DOWN -> NORMAL HOME
    // =================================================

    if (isMoonView())
    {
      toggleMoonView();

      setHomeTithiView(false);

      Serial.println(
        "MOON -> HOME"
      );

      return;
    }


    // =================================================
    // NORMAL HOME
    // DOWN #1 -> PANCHANG
    // =================================================

    if (!isHomeTithiView())
    {
      setHomeTithiView(true);

      Serial.println(
        "HOME -> PANCHANG"
      );

      return;
    }


    // =================================================
    // PANCHANG
    // DOWN #2 -> MOON
    // =================================================

    if (isHomeTithiView())
    {
      toggleMoonView();

      Serial.println(
        "PANCHANG -> MOON"
      );

      return;
    }
  }
}


// =====================================================
// MENU BUTTON
// =====================================================

void handleMenuPress()
{
  // ===================================================
  // ALARM RINGING
  // ===================================================

  if (alarmRinging)
  {
    return;
  }


  // ===================================================
  // TIMEZONE MENU -> SELECT
  // ===================================================

  if (
    currentScreen ==
    TIMEZONE_PAGE
  )
  {
    resetBacklightTimer();

    openTimezoneItem();

    return;
  }


  // ===================================================
  // HOME -> MENU
  // ===================================================

  if (currentScreen == HOME)
  {
    resetBacklightTimer();

    currentScreen = MENU;

    menuIndex = 0;
    menuTop = 0;

    Serial.println(
      "MENU OPEN"
    );

    return;
  }


  // ===================================================
  // CLOCK MENU -> SELECT ITEM
  // ===================================================

  if (
    currentScreen ==
    CLOCK_MENU_PAGE
  )
  {
    resetBacklightTimer();

    openClockMenuItem();

    return;
  }


  // ===================================================
  // MAIN MENU -> OPEN ITEM
  // ===================================================

  if (currentScreen == MENU)
  {
    resetBacklightTimer();

    openMenuItem();

    return;
  }

  // ===================================================
  // TIME PAGE
  // MENU = NEXT SELECTION
  // ===================================================

  if (
    currentScreen ==
    TIME_PAGE
  )
  {
    resetBacklightTimer();

    timeNextStep();

    return;
  }


  // ===================================================
  // ALARM MENU -> SELECT ITEM
  // ===================================================

  if (
    currentScreen ==
    ALARM_MENU_PAGE
  )
  {
    resetBacklightTimer();

    openAlarmMenuItem();

    return;
  }


  // ===================================================
  // ALARM PAGE
  // ===================================================

  if (currentScreen == ALARM_PAGE)
  {
    resetBacklightTimer();

    // --------------------------------------------------------
    // NEXT SETTING
    //
    // 0 = HOUR
    // 1 = MINUTE
    // 2 = AM / PM
    // 3 = ON / OFF
    // --------------------------------------------------------

    alarmSetStep++;

    // --------------------------------------------------------
    // AFTER ON/OFF
    // GO BACK TO HOUR
    // --------------------------------------------------------


    if (alarmSetStep > 3)
    {
        alarmSetStep = 0;
    }

    alarmSetting = true;

    Serial.print("ALARM SET STEP: ");
    Serial.println(alarmSetStep);

    return;
  }
  


  // ===================================================
  // STOPWATCH
  // MENU = START / STOP / RESUME
  // ===================================================

  if (
    currentScreen ==
    STOPWATCH_PAGE
  )
  {
    resetBacklightTimer();

    stopwatchToggle();

    return;
  }


  // ===================================================
  // COMPASS
  // ===================================================

  if (
    currentScreen ==
    COMPASS_PAGE
  )
  {
    resetBacklightTimer();

    compassActive =
      !compassActive;

    if (compassActive)
    {
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


  // ===================================================
  // GPS
  // ===================================================

  if (
    currentScreen ==
    GPS_PAGE
  )
  {
    resetBacklightTimer();

    gpsActive =
      !gpsActive;

    if (gpsActive)
    {
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

void handleBackPress()
{
  // ===================================================
  // ALARM RINGING
  // ===================================================

  if (alarmRinging)
  {
    stopAlarm();

    return;
  }


  // ===================================================
  // HOME
  // ===================================================

  if (currentScreen == HOME)
  {
    resetBacklightTimer();


    // -----------------------------------------------
    // ALREADY IN BACK BLANK VIEW
    // SECOND BACK -> NORMAL HOME
    // -----------------------------------------------

    if (isBackBlankView())
    {
      setBackBlankView(false);

      setHomeTithiView(false);

      Serial.println(
        "BACK BLANK -> HOME"
      );

      return;
    }


    // -----------------------------------------------
    // NORMAL HOME / PANCHANG / MOON
    // BACK -> BLANK VIEW
    // -----------------------------------------------

    setBackBlankView(true);

    Serial.println(
      "HOME -> BACK BLANK VIEW"
    );

    return;
  }


  // ===================================================
  // ALARM MENU -> CLOCK MENU
  // ===================================================

  if (
    currentScreen ==
    ALARM_MENU_PAGE
  )
  {
    resetBacklightTimer();

    currentScreen =
      CLOCK_MENU_PAGE;

    Serial.println(
      "ALARMS -> CLOCK MENU"
    );

    return;
  }


  // ===================================================
  // CLOCK CHILD PAGES -> CLOCK MENU
  // ===================================================

    if (currentScreen == ALARM_PAGE)
      {
        resetBacklightTimer();

        alarmSetting = false;
        alarmSetStep = 0;

        currentScreen = ALARM_MENU_PAGE;

        Serial.println("ALARM PAGE -> ALARMS");

        return;
      }

    // ===================================================
    // TIME PAGE
    // BACK = CANCEL TIME EDIT + CLOCK MENU
    // ===================================================

    if (
      currentScreen ==
      TIME_PAGE
    )
    {
      resetBacklightTimer();

      // -----------------------------------------------
      // BACK = CANCEL
      // Do NOT save edited time
      // -----------------------------------------------

      timeSetting = false;
      timeSetStep = 0;

      currentScreen =
        CLOCK_MENU_PAGE;

      Serial.println(
        "TIME EDIT CANCELLED -> CLOCK MENU"
      );

      return;
    }


    // ===================================================
    // OTHER CLOCK CHILD PAGES
    // ===================================================

    if (
      currentScreen == TIMEZONE_PAGE ||
      currentScreen == STOPWATCH_PAGE
    )
    {
      resetBacklightTimer();

      currentScreen =
        CLOCK_MENU_PAGE;

      Serial.println(
        "CLOCK PAGE -> CLOCK MENU"
      );

      return;
    }

      if (currentScreen == SNOOZE_PAGE)
      {
          resetBacklightTimer();

          currentScreen = ALARM_MENU_PAGE;

          Serial.println("SNOOZE PAGE -> ALARMS");

          return;
      }

  // ===================================================
  // CLOCK MENU -> MAIN MENU
  // ===================================================

  if (
    currentScreen ==
    CLOCK_MENU_PAGE
  )
  {
    resetBacklightTimer();

    currentScreen =
      MENU;

    Serial.println(
      "CLOCK -> MENU"
    );

    return;
  }


  // ===================================================
  // MENU -> HOME
  // ===================================================

  if (currentScreen == MENU)
  {
    resetBacklightTimer();

    currentScreen = HOME;

    Serial.println(
      "HOME"
    );

    return;
  }


  // ===================================================
  // ANY OTHER PAGE -> MENU
  // ===================================================

  resetBacklightTimer();

  currentScreen = MENU;

  Serial.println(
    "BACK TO MENU"
  );
}


// =====================================================
// BACK LONG PRESS
// =====================================================

void handleBackLongPress()
{
  // No special action currently.
}


// =====================================================
// UPDATE DOUBLE CLICK / LONG PRESS
// =====================================================

void updateDoubleClick()
{
  static bool upLongHandled = false;


  // ===================================================
  // UP HELD
  // ===================================================

  if (
    upLast == LOW &&
    !upLongHandled
  )
  {
    if (
      millis() -
      upPressTime >=
      LONG_PRESS_TIME
    )
    {
      if (currentScreen == HOME)
      {
        handleUpLongPress();

        upLongHandled = true;
      }
    }
  }


  // ===================================================
  // BACKLIGHT AUTO OFF
  // ===================================================

  updateBacklightTimer();


  // ===================================================
  // UP RELEASE
  // ===================================================

  if (upLast == HIGH)
  {
    upLongHandled = false;
  }
}


// =====================================================
// READ BUTTONS
// =====================================================

void handleButtons()
{
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
  )
  {
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
  )
  {
    unsigned long duration =
      millis() -
      upPressTime;

    if (
      duration <
      LONG_PRESS_TIME
    )
    {
      handleUpShortPress();
    }
  }


  // ===================================================
  // DOWN
  // ===================================================

  if (
    down == LOW &&
    downLast == HIGH
  )
  {
    handleDownPress();
  }


  // ===================================================
  // MENU
  // ===================================================

  if (
    menu == LOW &&
    menuLast == HIGH
  )
  {
    handleMenuPress();
  }


  // ===================================================
  // BACK PRESS
  // ===================================================

  if (
    back == LOW &&
    selectLast == HIGH
  )
  {
    selectPressTime =
      millis();
  }


  // ===================================================
  // BACK RELEASE
  // ===================================================

  if (
    back == HIGH &&
    selectLast == LOW
  )
  {
    unsigned long duration =
      millis() -
      selectPressTime;

    if (
      duration >=
      LONG_PRESS_TIME
    )
    {
      handleBackLongPress();
    }
    else
    {
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