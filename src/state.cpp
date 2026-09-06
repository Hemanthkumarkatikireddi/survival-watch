#include "state.h"
#include "config.h"

// =====================================================
// GLOBAL HARDWARE / STATE
// =====================================================

RTC_DS1307 rtc;

bool backlightState = false;
unsigned long backlightStart = 0;

bool gpsActive = false;
bool compassActive = false;
bool trackBackActive = false;

bool use24Hour = false;

int menuIndex = 0;
int menuTop = 0;

Screen currentScreen = HOME;

// =====================================================
// TIME SETTING
// =====================================================

bool timeSetting = false;
int timeSetStep = 0;

int setHour = 12;
int setMinute = 0;
int setAMPM = 0;

// =====================================================
// ALARM
// =====================================================

bool alarmEnabled = false;
bool alarmRinging = false;

int alarmHour = 7;
int alarmMinute = 0;
int alarmAMPM = 0;

bool alarmSetting = false;
int alarmSetStep = 0;

// =====================================================
// STOPWATCH
// =====================================================

bool stopwatchRunning = false;

unsigned long stopwatchStart = 0;
unsigned long stopwatchElapsed = 0;

// =====================================================
// BUTTON STATES
// =====================================================

bool upLast = HIGH;
bool downLast = HIGH;
bool menuLast = HIGH;
bool selectLast = HIGH;

unsigned long selectPressTime = 0;
unsigned long upPressTime = 0;

// =====================================================
// DOUBLE CLICK
// =====================================================

bool upWaitingDouble = false;
unsigned long firstUpClickTime = 0;

// =====================================================
// MENU ITEMS
// =====================================================

const char* menuItems[MENU_COUNT] = {
  "TIME",
  "ALARM",
  "STOPWATCH",
  "COMPASS",
  "GPS",
  "SENSORS",
  "SETTINGS"
};

// =====================================================
// DAY / MONTH
// =====================================================

const char* days[7] = {
  "SUN",
  "MON",
  "TUE",
  "WED",
  "THU",
  "FRI",
  "SAT"
};

const char* months[12] = {
  "JAN",
  "FEB",
  "MAR",
  "APR",
  "MAY",
  "JUN",
  "JUL",
  "AUG",
  "SEP",
  "OCT",
  "NOV",
  "DEC"
};
