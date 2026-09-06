#include "settings_page.h"
#include "display.h"
#include "state.h"

void drawSettingsPage() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(18, 0);
  display.print("SETTINGS");

  display.drawLine(
    0,
    9,
    83,
    9,
    BLACK
  );

  display.setCursor(0, 15);

  display.print("FORMAT: ");

  display.print(
    use24Hour ?
    "24H" :
    "12H"
  );

  display.setCursor(0, 26);
  display.print("BACKLIGHT");

  display.setCursor(0, 37);
  display.print("SYSTEM");

  display.setCursor(0, 44);
  display.print("MENU=BACK");

  display.display();
}
