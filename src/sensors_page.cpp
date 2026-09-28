#include "sensors_page.h"
#include "display.h"

void drawSensorPage() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(20, 0);
  display.print("SENSORS");

  display.drawLine(
    0,
    9,
    83,
    9,
    BLACK
  );

  display.setCursor(0, 14);
  display.print("TEMP: --.- C");

  display.setCursor(0, 24);
  display.print("HUM : --.- %");

  display.setCursor(0, 34);
  display.print("HR  : ---");

  display.setCursor(0, 44);
  display.print("MENU=BACK");

  display.display();
}
