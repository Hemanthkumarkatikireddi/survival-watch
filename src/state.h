#pragma once

#include <Arduino.h>
#include <RTClib.h>

// =====================================================
// SCREEN
// =====================================================

enum Screen
{
    HOME,
    MENU,
    CLOCK_MENU_PAGE,
    TIMEZONE_PAGE,
    ALARM_MENU_PAGE,
    TIME_PAGE,
    ALARM_PAGE,
    SNOOZE_PAGE,
    STOPWATCH_PAGE,
    COMPASS_PAGE,
    GPS_PAGE,
    SENSOR_PAGE,
    SETTINGS_PAGE
};

// =====================================================
// CLOCK MENU
// =====================================================

extern const char* clockMenuItems[];

extern int clockMenuIndex;

extern int clockMenuTop;

extern const int CLOCK_MENU_COUNT;

extern const int CLOCK_MENU_VISIBLE;

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

struct AlarmData
{
    uint8_t hour;
    uint8_t minute;
    bool enabled;
};

extern AlarmData alarms[4];

extern bool snoozeEnabled;

extern int selectedAlarmIndex;

extern int alarmMenuIndex;
extern int alarmMenuTop;

extern int alarmSetStep;
extern bool alarmSetting;
extern int selectedAlarmIndex;