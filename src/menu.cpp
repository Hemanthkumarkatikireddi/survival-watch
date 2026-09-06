#include "menu.h"
#include "display.h"
#include "config.h"
#include "state.h"

void drawMenuScreen() {

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(BLACK);

  // -----------------------------
  // TITLE
  // -----------------------------

  display.setCursor(25, 0);
  display.print("MENU");

  display.drawLine(
    0,
    8,
    83,
    8,
    BLACK
  );

  // -----------------------------
  // MENU ITEMS
  // -----------------------------

  for (
    int row = 0;
    row < MENU_VISIBLE;
    row++
  ) {

    int index =
      menuTop + row;

    if (index >= MENU_COUNT) {
      break;
    }

    int y =
      11 + (row * 9);

    display.setCursor(1, y);

    if (index == menuIndex) {
      display.print(">");
    } else {
      display.print(" ");
    }

    display.print(
      menuItems[index]
    );
  }

  // -----------------------------
  // SCROLL UP
  // -----------------------------

  if (menuTop > 0) {

    display.setCursor(76, 10);
    display.print("^");
  }

  // -----------------------------
  // SCROLL DOWN
  // -----------------------------

  if (
    menuTop + MENU_VISIBLE <
    MENU_COUNT
  ) {

    display.setCursor(76, 37);
    display.print("v");
  }

  display.display();
}

// =====================================================
// OPEN SELECTED MENU ITEM
// =====================================================

void openMenuItem() {

  switch (menuIndex) {

    case 0:
      currentScreen = TIME_PAGE;
      break;

    case 1:
      currentScreen = ALARM_PAGE;
      break;

    case 2:
      currentScreen = STOPWATCH_PAGE;
      break;

    case 3:
      currentScreen = COMPASS_PAGE;
      break;

    case 4:
      currentScreen = GPS_PAGE;
      break;

    case 5:
      currentScreen = SENSOR_PAGE;
      break;

    case 6:
      currentScreen = SETTINGS_PAGE;
      break;
  }

  Serial.print("OPEN: ");
  Serial.println(
    menuItems[menuIndex]
  );
}

// =====================================================
// MENU UP
// =====================================================

void menuUp() {

  menuIndex--;

  if (menuIndex < 0) {
    menuIndex = MENU_COUNT - 1;
  }

  if (menuIndex < menuTop) {
    menuTop = menuIndex;
  }

  if (menuIndex == MENU_COUNT - 1) {
    menuTop = MENU_COUNT - MENU_VISIBLE;
  }

  if (menuTop < 0) {
    menuTop = 0;
  }
}

// =====================================================
// MENU DOWN
// =====================================================

void menuDown() {

  menuIndex++;

  if (menuIndex >= MENU_COUNT) {

    menuIndex = 0;
    menuTop = 0;

    return;
  }

  if (
    menuIndex >=
    menuTop + MENU_VISIBLE
  ) {

    menuTop =
      menuIndex -
      MENU_VISIBLE +
      1;
  }
}
