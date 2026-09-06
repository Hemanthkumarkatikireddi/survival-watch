#include "alarm_page.h"
#include "display.h"
#include "state.h"

void drawAlarmPage() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(25, 0);
  display.print("ALARM");

  display.drawLine(
    0,
    9,
    83,
    9,
    BLACK
  );

  if (alarmRinging) {

    display.setTextSize(2);

    display.setCursor(10, 14);
    display.print("ALARM!");

    display.setTextSize(1);

    display.setCursor(0, 35);
    display.print("UP/DN=SNOOZE");

    display.setCursor(0, 44);
    display.print("SEL=STOP");

  }
  else if (alarmSetting) {

    display.setTextSize(2);

    display.setCursor(7, 14);

    if (alarmHour < 10) {
      display.print("0");
    }

    display.print(alarmHour);
    display.print(":");

    if (alarmMinute < 10) {
      display.print("0");
    }

    display.print(alarmMinute);

    display.setTextSize(1);

    display.setCursor(61, 16);

    display.print(
      alarmAMPM == 0 ? "AM" : "PM"
    );

    display.setCursor(0, 36);

    if (alarmSetStep == 0) {
      display.print("UP/DN = HOUR");
    }
    else if (alarmSetStep == 1) {
      display.print("UP/DN = MIN");
    }
    else {
      display.print("UP/DN = AM/PM");
    }

    display.setCursor(0, 45);
    display.print("SEL = NEXT");

  }
  else {

    display.setTextSize(2);

    display.setCursor(7, 14);

    if (alarmHour < 10) {
      display.print("0");
    }

    display.print(alarmHour);
    display.print(":");

    if (alarmMinute < 10) {
      display.print("0");
    }

    display.print(alarmMinute);

    display.setTextSize(1);

    display.setCursor(61, 16);

    display.print(
      alarmAMPM == 0 ? "AM" : "PM"
    );

    display.setCursor(0, 36);

    display.print(
      alarmEnabled ?
      "ALARM: ON" :
      "ALARM: OFF"
    );

    display.setCursor(0, 45);
    display.print("SEL=SET");
  }

  display.display();
}

// =====================================================
// ALARM VALUE UP
// =====================================================

void alarmValueUp() {

  if (alarmSetStep == 0) {

    alarmHour++;

    if (alarmHour > 12) {
      alarmHour = 1;
    }
  }
  else if (alarmSetStep == 1) {

    alarmMinute++;

    if (alarmMinute > 59) {
      alarmMinute = 0;
    }
  }
  else {

    alarmAMPM = !alarmAMPM;
  }
}

// =====================================================
// ALARM VALUE DOWN
// =====================================================

void alarmValueDown() {

  if (alarmSetStep == 0) {

    alarmHour--;

    if (alarmHour < 1) {
      alarmHour = 12;
    }
  }
  else if (alarmSetStep == 1) {

    alarmMinute--;

    if (alarmMinute < 0) {
      alarmMinute = 59;
    }
  }
  else {

    alarmAMPM = !alarmAMPM;
  }
}

// =====================================================
// CHECK ALARM
// =====================================================

void checkAlarm() {

  if (!alarmEnabled || alarmRinging) {
    return;
  }

  DateTime now = rtc.now();

  int hour12 =
    now.hour() % 12;

  if (hour12 == 0) {
    hour12 = 12;
  }

  int currentAMPM =
    now.hour() >= 12 ? 1 : 0;

  if (
    hour12 == alarmHour &&
    now.minute() == alarmMinute &&
    currentAMPM == alarmAMPM &&
    now.second() == 0
  ) {

    alarmRinging = true;

    Serial.println(
      "ALARM RINGING!"
    );

    setBacklight(true);
  }
}

// =====================================================
// ALARM SNOOZE
// =====================================================

void snoozeAlarm() {

  alarmRinging = false;
  alarmEnabled = true;

  alarmMinute += 10;

  if (alarmMinute >= 60) {

    alarmMinute -= 60;

    alarmHour++;

    if (alarmHour > 12) {
      alarmHour = 1;
    }
  }

  Serial.println(
    "ALARM SNOOZED 10 MIN"
  );

  currentScreen = HOME;
}

// =====================================================
// STOP ALARM
// =====================================================

void stopAlarm() {

  alarmRinging = false;
  alarmEnabled = false;

  Serial.println(
    "ALARM STOPPED"
  );

  currentScreen = HOME;
}
