#include "stopwatch_page.h"
#include "display.h"
#include "state.h"

void drawStopwatchPage() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(20, 0);
  display.print("STOPWATCH");

  display.drawLine(
    0,
    9,
    83,
    9,
    BLACK
  );

  unsigned long elapsed =
    stopwatchElapsed;

  if (stopwatchRunning) {

    elapsed =
      millis() -
      stopwatchStart +
      stopwatchElapsed;
  }

  unsigned long totalSeconds =
    elapsed / 1000;

  int minutes =
    totalSeconds / 60;

  int seconds =
    totalSeconds % 60;

  unsigned long milliseconds =
    elapsed % 1000;

  int centiseconds =
    milliseconds / 10;

  display.setTextSize(2);

  display.setCursor(4, 15);

  if (minutes < 10) {
    display.print("0");
  }

  display.print(minutes);
  display.print(":");

  if (seconds < 10) {
    display.print("0");
  }

  display.print(seconds);

  display.setTextSize(1);

  display.setCursor(64, 21);

  if (centiseconds < 10) {
    display.print("0");
  }

  display.print(centiseconds);
  display.print("cs");

  display.setCursor(0, 36);

  if (stopwatchRunning) {
    display.print("RUNNING");
  }
  else {
    display.print("STOPPED");
  }

  display.setCursor(0, 44);

  display.print(
    stopwatchRunning ?
    "SEL=STOP UP=RESET" :
    "SEL=START UP=RESET"
  );

  display.display();
}

// =====================================================
// START / STOP
// =====================================================

void stopwatchSelect() {

  if (!stopwatchRunning) {

    stopwatchStart = millis();
    stopwatchRunning = true;

    Serial.println(
      "STOPWATCH START"
    );
  }
  else {

    stopwatchElapsed +=
      millis() -
      stopwatchStart;

    stopwatchRunning = false;

    Serial.println(
      "STOPWATCH STOP"
    );
  }
}

// =====================================================
// RESET
// =====================================================

void stopwatchReset() {

  stopwatchRunning = false;
  stopwatchStart = 0;
  stopwatchElapsed = 0;

  Serial.println(
    "STOPWATCH RESET"
  );
}
