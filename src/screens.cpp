#include "screens.h"
#include "state.h"

#include "home.h"
#include "menu.h"
#include "time_page.h"
#include "alarm_page.h"
#include "stopwatch_page.h"
#include "compass_page.h"
#include "gps_page.h"
#include "sensors_page.h"
#include "settings_page.h"

void drawCurrentScreen() {

  switch (currentScreen) {

    case HOME:
      drawHomeScreen();
      break;

    case MENU:
      drawMenuScreen();
      break;

    case TIME_PAGE:
      drawTimePage();
      break;

    case ALARM_PAGE:
      drawAlarmPage();
      break;

    case STOPWATCH_PAGE:
      drawStopwatchPage();
      break;

    case COMPASS_PAGE:
      drawCompassPage();
      break;

    case GPS_PAGE:
      drawGPSPage();
      break;

    case SENSOR_PAGE:
      drawSensorPage();
      break;

    case SETTINGS_PAGE:
      drawSettingsPage();
      break;
  }
}
