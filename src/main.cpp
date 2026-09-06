
#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "state.h"
#include "display.h"
#include "buttons.h"
#include "alarm_page.h"
#include "screens.h"

#include "panchang/panchang.h"
#include "panchang/timezone.h"

#include "panchang/solar.h"
#include "panchang/lunar.h"

// ============================================================
// PRINT 12-HOUR TIME
// ============================================================

void print12Hour(DateTime dt)
{
  int hour =
    dt.hour();

  bool pm =
    (hour >= 12);

  if (hour == 0)
  {
    hour = 12;
  }
  else if (hour > 12)
  {
    hour -= 12;
  }

  if (hour < 10)
  {
    Serial.print("0");
  }

  Serial.print(hour);

  Serial.print(":");

  if (dt.minute() < 10)
  {
    Serial.print("0");
  }

  Serial.print(dt.minute());

  Serial.print(
    pm ? "pm" : "am"
  );
}

// ============================================================
// PRINT DATE + TIME
// ============================================================

void printDateTime(DateTime dt)
{
  Serial.print(
    dt.year()
  );

  Serial.print("-");

  if (dt.month() < 10)
  {
    Serial.print("0");
  }

  Serial.print(
    dt.month()
  );

  Serial.print("-");

  if (dt.day() < 10)
  {
    Serial.print("0");
  }

  Serial.print(
    dt.day()
  );

  Serial.print(" ");

  print12Hour(dt);
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  // ==========================================================
  // I2C
  // ==========================================================

  Wire.begin(
    RTC_SDA,
    RTC_SCL
  );

  // ==========================================================
  // BUTTONS
  // ==========================================================

  pinMode(
    BTN_UP,
    INPUT_PULLUP
  );

  pinMode(
    BTN_DOWN,
    INPUT_PULLUP
  );

  pinMode(
    BTN_MENU,
    INPUT_PULLUP
  );

  pinMode(
    BTN_SELECT,
    INPUT_PULLUP
  );

  // ==========================================================
  // BACKLIGHT
  // ==========================================================

  pinMode(
    LCD_BL,
    OUTPUT
  );

  setBacklight(false);

  // ==========================================================
  // DISPLAY
  // ==========================================================

  initDisplay();

  // ==========================================================
  // RTC
  // ==========================================================

  if (!rtc.begin())
  {
    Serial.println(
      "RTC ERROR!"
    );

    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(
      0,
      10
    );

    display.println(
      "RTC ERROR!"
    );

    display.setCursor(
      0,
      25
    );

    display.println(
      "CHECK WIRING"
    );

    display.display();

    while (true)
    {
      delay(100);
    }
  }

  Serial.println(
    "DS1307 FOUND!"
  );

  // ==========================================================
  // RTC START
  // ==========================================================

  if (!rtc.isrunning())
  {
    rtc.adjust(
      DateTime(
        F(__DATE__),
        F(__TIME__)
      )
    );
  }

  Serial.println(
    "RTC READY!"
  );

  Serial.println(
    "WATCH MENU READY!"
  );

  // ==========================================================
  // PANCHANG ENGINE TEST
  // ==========================================================

  Serial.println();
  Serial.println("================================");
  Serial.println(" PANCHANG ENGINE TEST");
  Serial.println("================================");

  // ==========================================================
  // INDIA RTC TIME
  // ==========================================================

  DateTime indiaDateTime =
    rtc.now();

  Serial.print(
    "RTC India Time: "
  );

  printDateTime(
    indiaDateTime
  );

  Serial.println();

  // ==========================================================
  // INDIA TIME -> UTC
  // ==========================================================

  DateTime utcDateTime =
    convertIndiaTimeToUTC(
      indiaDateTime
    );

  Serial.print("India Unix: ");
Serial.println(indiaDateTime.unixtime());

Serial.print("UTC Unix:   ");
Serial.println(utcDateTime.unixtime());

Serial.print("Difference: ");
Serial.println(
  indiaDateTime.unixtime() -
  utcDateTime.unixtime()
);  

  Serial.print(
    "UTC Time:       "
  );

  printDateTime(
    utcDateTime
  );

  Serial.println();

  // ==========================================================
  // GPS LOCATION
  // ==========================================================
  //
  // Temporary Wokwi location:
  //
  // Vizianagaram, Andhra Pradesh, India
  //
  // Later real GPS module will provide these values.
  //
  // ==========================================================

  double latitude =
    18.1167;

  double longitude =
    83.4000;

  setGPSLocation(
    latitude,
    longitude
  );

  Serial.println();
  Serial.println("--------- GPS LOCATION ---------");

  Serial.print(
    "Latitude:  "
  );

  Serial.println(
    getGPSLatitude(),
    6
  );

  Serial.print(
    "Longitude: "
  );

  Serial.println(
    getGPSLongitude(),
    6
  );

  // ==========================================================
  // DETECT LOCATION TIMEZONE
  // ==========================================================

  TimezoneInfo locationTimezone =
    getCurrentTimezone(
      utcDateTime
    );

  // ==========================================================
  // UTC -> LOCATION LOCAL TIME
  // ==========================================================

  DateTime locationDateTime =
    getLocationLocalTime(
      utcDateTime
    );

  Serial.println();
  Serial.println("--------- TIMEZONE ---------");

  Serial.print(
    "Country: "
  );

  Serial.println(
    locationTimezone.countryName
  );

  Serial.print(
    "Country Code: "
  );

  Serial.println(
    locationTimezone.countryCode
  );

  Serial.print(
    "Timezone: "
  );

  Serial.println(
    locationTimezone.timezoneId
  );

  Serial.print(
    "UTC Offset Minutes: "
  );

  Serial.println(
    locationTimezone.utcOffsetMinutes
  );

  Serial.print(
    "DST: "
  );

  Serial.println(
    locationTimezone.daylightSaving
      ? "YES"
      : "NO"
  );

  // ==========================================================
  // TIME COMPARISON
  // ==========================================================

  Serial.println();
  Serial.println("--------- TIME ---------");

  Serial.print(
    "India Time:    "
  );

  print12Hour(
    indiaDateTime
  );

  Serial.println();

  Serial.print(
    "UTC Time:      "
  );

  print12Hour(
    utcDateTime
  );

  Serial.println();

  Serial.print(
    "Location Time: "
  );

  print12Hour(
    locationDateTime
  );

  Serial.println();

  // ==========================================================
  // PANCHANG CALCULATION
  // ==========================================================
  //
  // IMPORTANT:
  //
  // Tithi / Nakshatra calculation uses UTC instant.
  //
  // Sunrise / Sunset uses GPS location and timezone.
  //
  // ==========================================================

  PanchangData testData =
    calculatePanchang(
      utcDateTime,
      latitude,
      longitude,
      locationTimezone.utcOffsetMinutes / 60.0
    );

    // ============================================================
// TITHI BOUNDARY DEBUG
// ============================================================

// Reference Tithi transition:
// 2026-09-05 09:53 PM IST
// = 2026-09-05 04:23 PM UTC

DateTime boundaryUTC(
    2026,
    9,
    5,
    16,
    23,
    0
);

double boundaryHourUTC =
    boundaryUTC.hour()
    + boundaryUTC.minute() / 60.0
    + boundaryUTC.second() / 3600.0;


double sunLon =
    calculateSunLongitude(
        boundaryUTC.year(),
        boundaryUTC.month(),
        boundaryUTC.day(),
        boundaryHourUTC
    );


double moonLon =
    calculateMoonLongitude(
        boundaryUTC.year(),
        boundaryUTC.month(),
        boundaryUTC.day(),
        boundaryHourUTC
    );


double elongation =
    moonLon - sunLon;


while (elongation < 0)
    elongation += 360.0;

while (elongation >= 360.0)
    elongation -= 360.0;


Serial.println();
Serial.println("--------- TITHI BOUNDARY DEBUG ---------");

Serial.println("Reference: Navami -> Dashami");
Serial.println("Reference IST: 2026-09-05 09:53pm");
Serial.println("Test UTC:      2026-09-05 04:23pm");

Serial.print("Sun Longitude:  ");
Serial.println(sunLon, 6);

Serial.print("Moon Longitude: ");
Serial.println(moonLon, 6);

Serial.print("Elongation:     ");
Serial.println(elongation, 6);

Serial.print("Expected:       288.000000");

Serial.println();

Serial.print("Angle Error:    ");
Serial.println(elongation - 288.0, 6);

Serial.print("Calculated Tithi: ");
Serial.println(
    (int)(floor(elongation / 12.0) + 1)
);

// ============================================================
// MULTIPLE TITHI BOUNDARY TEST
// ============================================================

Serial.println();
Serial.println("========= MULTIPLE TITHI TEST ==========");

// ------------------------------------------------------------
// TEST 1
// Sep 05 2026 - Navami -> Dashami
// Reference: 09:53 PM IST
// ------------------------------------------------------------

DateTime test1IST(
    2026,
    9,
    5,
    21,
    53,
    0
);

DateTime test1UTC =
    convertIndiaTimeToUTC(test1IST);

TithiInfo test1 =
    calculateTithi(test1UTC);

Serial.println();
Serial.println("TEST 1: Sep 05 Navami -> Dashami");

Serial.print("Reference IST: ");
printDateTime(test1IST);
Serial.println();

Serial.print("Engine Tithi: ");
Serial.println(
    getPanchangTithiName(test1.number)
);

Serial.print("Engine Next Tithi: ");
Serial.println(
    getPanchangTithiName(test1.nextNumber)
);

Serial.print("Engine Transition Local: ");
printDateTime(
    getLocationLocalTime(test1.nextStartUTC)
);
Serial.println();


// ------------------------------------------------------------
// TEST 2
// Sep 06 2026 - Dashami -> Ekadashi
// Reference: 07:29 PM IST
// ------------------------------------------------------------

DateTime test2IST(
    2026,
    9,
    6,
    19,
    29,
    0
);

DateTime test2UTC =
    convertIndiaTimeToUTC(test2IST);

TithiInfo test2 =
    calculateTithi(test2UTC);

Serial.println();
Serial.println("TEST 2: Sep 06 Dashami -> Ekadashi");

Serial.print("Reference IST: ");
printDateTime(test2IST);
Serial.println();

Serial.print("Engine Tithi: ");
Serial.println(
    getPanchangTithiName(test2.number)
);

Serial.print("Engine Next Tithi: ");
Serial.println(
    getPanchangTithiName(test2.nextNumber)
);

Serial.print("Engine Transition Local: ");
printDateTime(
    getLocationLocalTime(test2.nextStartUTC)
);
Serial.println();


// ------------------------------------------------------------
// TEST 3
// Sep 15 2026 - Tritiya -> Chaturthi
// Reference: 07:44 AM IST
// ------------------------------------------------------------

// ------------------------------------------------------------
// TEST 3
// Dec 07 2026 - Trayodashi -> Chaturdashi
// Reference: 02:22 AM IST
// ------------------------------------------------------------

DateTime test3IST(
    2026,
    12,
    7,
    2,
    22,
    0
);

DateTime test3UTC =
    convertIndiaTimeToUTC(test3IST);

TithiInfo test3 =
    calculateTithi(test3UTC);

Serial.println();
Serial.println("TEST 3: Dec 07 Trayodashi -> Chaturdashi");

Serial.print("Reference IST: ");
printDateTime(test3IST);
Serial.println();

Serial.print("Engine Tithi: ");
Serial.println(
    getPanchangTithiName(test3.number)
);

Serial.print("Engine Next Tithi: ");
Serial.println(
    getPanchangTithiName(test3.nextNumber)
);

Serial.print("Engine Transition Local: ");
printDateTime(
    getLocationLocalTime(test3.nextStartUTC)
);
Serial.println();


// ============================================================
// SUN / MOON LONGITUDE DIAGNOSTIC
// ============================================================

Serial.println();
Serial.println("--------- SUN / MOON DIAGNOSTIC ---------");

// Sun longitude
Serial.print("Sun Longitude at boundary:  ");
Serial.println(sunLon, 9);

// Moon longitude
Serial.print("Moon Longitude at boundary: ");
Serial.println(moonLon, 9);

// Moon - Sun
Serial.print("Moon - Sun:                  ");
Serial.println(elongation, 9);

// Required Moon longitude if Sun is correct
double requiredMoonLon =
    sunLon + 288.0;

while (requiredMoonLon >= 360.0)
    requiredMoonLon -= 360.0;

Serial.print("Required Moon Longitude:     ");
Serial.println(requiredMoonLon, 9);

// Required Sun longitude if Moon is correct
double requiredSunLon =
    moonLon - 288.0;

while (requiredSunLon < 0)
    requiredSunLon += 360.0;

Serial.print("Required Sun Longitude:      ");
Serial.println(requiredSunLon, 9);

// Difference
Serial.print("Moon Longitude Error:        ");
Serial.println(
    moonLon - requiredMoonLon,
    9
);

Serial.print("Sun Longitude Error:         ");
Serial.println(
    sunLon - requiredSunLon,
    9
);    

  // ==========================================================
  // TITHI
  // ==========================================================

  Serial.println();
  Serial.println("----------- TITHI -----------");

  Serial.print(
    "Tithi Number: "
  );

  Serial.println(
    testData.tithi.number
  );

  Serial.print(
    "Tithi Name: "
  );

  Serial.println(
    getPanchangTithiName(
      testData.tithi.number
    )
  );

  // ==========================================================
  // TITHI START UTC
  // ==========================================================

  Serial.print(
    "Tithi Start UTC:   "
  );

  printDateTime(
    testData.tithi.startUTC
  );

  Serial.println();

  // ==========================================================
  // TITHI START LOCAL
  // ==========================================================

  DateTime tithiStartLocal =
    getLocationLocalTime(
      testData.tithi.startUTC
    );

  Serial.print(
    "Tithi Start Local: "
  );

  printDateTime(
    tithiStartLocal
  );

  Serial.println();

  // ==========================================================
  // TITHI END UTC
  // ==========================================================

  Serial.print(
    "Tithi End UTC:     "
  );

  printDateTime(
    testData.tithi.endUTC
  );

  Serial.println();

  // ==========================================================
  // TITHI END LOCAL
  // ==========================================================

  DateTime tithiEndLocal =
    getLocationLocalTime(
      testData.tithi.endUTC
    );

  Serial.print(
    "Tithi End Local:   "
  );

  printDateTime(
    tithiEndLocal
  );

  Serial.println();

  // ==========================================================
  // NEXT TITHI
  // ==========================================================

  Serial.print(
    "Next Tithi: "
  );

  Serial.println(
    getPanchangTithiName(
      testData.tithi.nextNumber
    )
  );

  // ==========================================================
  // NEXT TITHI START UTC
  // ==========================================================

  Serial.print(
    "Next Tithi Start UTC:   "
  );

  printDateTime(
    testData.tithi.nextStartUTC
  );

  Serial.println();

  // ==========================================================
  // NEXT TITHI START LOCAL
  // ==========================================================

  DateTime nextTithiStartLocal =
    getLocationLocalTime(
      testData.tithi.nextStartUTC
    );

  Serial.print(
    "Next Tithi Start Local: "
  );

  printDateTime(
    nextTithiStartLocal
  );

  Serial.println();

  // ==========================================================
  // NAKSHATRA
  // ==========================================================

  Serial.println();
  Serial.println("--------- NAKSHATRA ---------");

  Serial.print(
    "Nakshatra Number: "
  );

  Serial.println(
    testData.nakshatra.number
  );

  Serial.print(
    "Nakshatra Name: "
  );

  Serial.println(
    getPanchangNakshatraName(
      testData.nakshatra.number
    )
  );

  // ==========================================================
  // NAKSHATRA START UTC
  // ==========================================================

  Serial.print(
    "Nakshatra Start UTC:   "
  );

  printDateTime(
    testData.nakshatra.startUTC
  );

  Serial.println();

  // ==========================================================
  // NAKSHATRA START LOCAL
  // ==========================================================

  DateTime nakshatraStartLocal =
    getLocationLocalTime(
      testData.nakshatra.startUTC
    );

  Serial.print(
    "Nakshatra Start Local: "
  );

  printDateTime(
    nakshatraStartLocal
  );

  Serial.println();

  // ==========================================================
  // NAKSHATRA END UTC
  // ==========================================================

  Serial.print(
    "Nakshatra End UTC:     "
  );

  printDateTime(
    testData.nakshatra.endUTC
  );

  Serial.println();


  

  // ==========================================================
  // NAKSHATRA END LOCAL
  // ==========================================================

  DateTime nakshatraEndLocal =
    getLocationLocalTime(
      testData.nakshatra.endUTC
    );

  Serial.print(
    "Nakshatra End Local:   "
  );

  printDateTime(
    nakshatraEndLocal
  );

  Serial.println();

  // ==========================================================
  // SUNRISE / SUNSET
  // ==========================================================

  Serial.println();
  Serial.println("------ SUNRISE / SUNSET ------");

  char sunriseText[16];
  char sunsetText[16];

  formatSolarTime(
    testData.sunrise,
    sunriseText,
    sizeof(sunriseText)
  );

  formatSolarTime(
    testData.sunset,
    sunsetText,
    sizeof(sunsetText)
  );

  Serial.print(
    "Sunrise: "
  );

  Serial.println(
    sunriseText
  );

  Serial.print(
    "Sunset:  "
  );

  Serial.println(
    sunsetText
  );  

  // ==========================================================
  // FINAL TEST SUMMARY
  // ==========================================================

  Serial.println();
  Serial.println("================================");
  Serial.println(" PANCHANG TEST COMPLETE");
  Serial.println("================================");

  Serial.println();

LunarTimes moonTimes =
    calculateMoonriseMoonset(
        2026,
        9,
        6,
        18.1167,   // Vizianagaram latitude
        83.4000,   // Vizianagaram longitude
        5.5        // IST
    );

Serial.println();
Serial.println("------ MOONRISE / MOONSET ------");

Serial.print("Moonrise decimal: ");
Serial.println(moonTimes.moonrise, 6);

Serial.print("Moonset decimal:  ");
Serial.println(moonTimes.moonset, 6);




}

// ============================================================
// LOOP
// ============================================================

void loop()
{
  handleButtons();

  updateDoubleClick();

  checkAlarm();

  updateBacklight();

  drawCurrentScreen();

  delay(50);
}
