#include "gps_page.h"
#include "display.h"
#include "state.h"

void drawGPSPage() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(27, 0);
  display.print("GPS");

  display.drawLine(
    0,
    9,
    83,
    9,
    BLACK
  );

  display.setCursor(5, 16);

  display.print("STATUS:");

  display.print(
    gpsActive ?
    " ON" :
    " OFF"
  );

  display.setCursor(5, 27);
  display.print("LAT: ---");

  display.setCursor(5, 36);
  display.print("LON: ---");

  display.setCursor(5, 44);
  display.print("SEL=ON/OFF");

  display.display();
}
