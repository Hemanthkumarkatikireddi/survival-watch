#include "compass_page.h"
#include "display.h"
#include "state.h"

void drawCompassPage() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(18, 0);
  display.print("COMPASS");

  display.drawLine(
    0,
    9,
    83,
    9,
    BLACK
  );

  display.setTextSize(2);

  display.setCursor(28, 15);
  display.print("N");

  display.setTextSize(1);

  display.setCursor(5, 36);

  display.print(
    compassActive ?
    "ACTIVE" :
    "OFF"
  );

  display.setCursor(5, 44);
  display.print("SEL=ON/OFF");

  display.display();
}
