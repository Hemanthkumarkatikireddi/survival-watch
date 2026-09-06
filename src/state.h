#pragma once

#include <Arduino.h>
#include <RTClib.h>

// =====================================================
// SCREEN
// =====================================================

enum Screen {
  HOME,
  MENU,
  TIME_PAGE,
  ALARM_PAGE,
  STOPWATCH_PAGE,
  COMPASS_PAGE,
  GPS_PAGE,
  SENSOR_PAGE,
  SETTINGS_PAGE
};

// =====================================================
// GLOBAL HARDWARE / STATE
// =====================================================

extern RTC_DS1307 rtc;

extern bool backlightState;
extern unsigned long backlightStart;

extern bool gpsActive;
extern bool compassActive;
extern bool trackBackActive;

extern bool use24Hour;

extern int menuIndex;
extern int menuTop;

extern Screen currentScreen;

// =====================================================
// TIME SETTING
// =====================================================

extern bool timeSetting;
extern int timeSetStep;
extern int setHour;
extern int setMinute;
extern int setAMPM;

// =====================================================
// ALARM
// =====================================================

extern bool alarmEnabled;
extern bool alarmRinging;

extern int alarmHour;
extern int alarmMinute;
extern int alarmAMPM;

extern bool alarmSetting;
extern int alarmSetStep;

// =====================================================
// STOPWATCH
// =====================================================

extern bool stopwatchRunning;
extern unsigned long stopwatchStart;
extern unsigned long stopwatchElapsed;

// =====================================================
// BUTTON STATES
// =====================================================

extern bool upLast;
extern bool downLast;
extern bool menuLast;
extern bool selectLast;

extern unsigned long selectPressTime;
extern unsigned long upPressTime;

// =====================================================
// DOUBLE CLICK
// =====================================================

extern bool upWaitingDouble;
extern unsigned long firstUpClickTime;
