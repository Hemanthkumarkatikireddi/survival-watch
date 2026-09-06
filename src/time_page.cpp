#include "time_page.h"
#include "display.h"
#include "state.h"

void drawTimePage() {

  DateTime now = rtc.now();

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(28, 0);
  display.print("TIME");

  display.drawLine(
    0,
    9,
    83,
    9,
    BLACK
  );

  if (timeSetting) {

    display.setCursor(0, 12);
    display.print("SET TIME");

    display.setTextSize(2);

    display.setCursor(5, 21);

    if (setHour < 10) {
      display.print("0");
    }

    display.print(setHour);
    display.print(":");

    if (setMinute < 10) {
      display.print("0");
    }

    display.print(setMinute);

    display.setTextSize(1);

    display.setCursor(63, 23);

    display.print(
      setAMPM == 0 ? "AM" : "PM"
    );

    display.setCursor(0, 40);

    if (timeSetStep == 0) {
      display.print("UP/DN HOUR");
    }
    else if (timeSetStep == 1) {
      display.print("UP/DN MIN");
    }
    else {
      display.print("UP/DN AM/PM");
    }

    display.setCursor(0, 47);
    display.print("SEL=NEXT");

  } else {

    int hourDisplay;
    bool pm;

    getDisplayHour(
      now.hour(),
      hourDisplay,
      pm
    );

    display.setTextSize(2);

    display.setCursor(5, 17);

    if (hourDisplay < 10) {
      display.print("0");
    }

    display.print(hourDisplay);
    display.print(":");

    if (now.minute() < 10) {
      display.print("0");
    }

    display.print(now.minute());

    display.setTextSize(1);

    if (!use24Hour) {

      display.setCursor(64, 20);

      display.print(
        pm ? "PM" : "AM"
      );
    }

    display.setCursor(0, 42);
    display.print("SEL=SET");
  }

  display.display();
}

// =====================================================
// START TIME SETTING
// =====================================================

void startTimeSetting() {

  DateTime now = rtc.now();

  setAMPM =
    now.hour() >= 12 ? 1 : 0;

  setHour =
    now.hour() % 12;

  if (setHour == 0) {
    setHour = 12;
  }

  setMinute =
    now.minute();

  timeSetStep = 0;
  timeSetting = true;

  Serial.println(
    "TIME SETTING START"
  );
}

// =====================================================
// TIME VALUE UP
// =====================================================

void timeValueUp() {

  if (timeSetStep == 0) {

    setHour++;

    if (setHour > 12) {
      setHour = 1;
    }
  }
  else if (timeSetStep == 1) {

    setMinute++;

    if (setMinute > 59) {
      setMinute = 0;
    }
  }
  else {

    setAMPM = !setAMPM;
  }
}

// =====================================================
// TIME VALUE DOWN
// =====================================================

void timeValueDown() {

  if (timeSetStep == 0) {

    setHour--;

    if (setHour < 1) {
      setHour = 12;
    }
  }
  else if (timeSetStep == 1) {

    setMinute--;

    if (setMinute < 0) {
      setMinute = 59;
    }
  }
  else {

    setAMPM = !setAMPM;
  }
}

// =====================================================
// SAVE TIME
// =====================================================

void saveTime() {

  int hour24;

  if (setAMPM == 0) {
    hour24 = setHour % 12;
  }
  else {
    hour24 = (setHour % 12) + 12;
  }

  DateTime now = rtc.now();

  rtc.adjust(
    DateTime(
      now.year(),
      now.month(),
      now.day(),
      hour24,
      setMinute,
      0
    )
  );

  Serial.println(
    "TIME UPDATED"
  );
}
