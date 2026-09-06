#include "home.h"
#include "display.h"
#include "config.h"
#include "state.h"
#include "fonts.h"
#include "moon_data.h"

#include "dummy_data.h"

#include "panchang/timezone.h"
#include "panchang/panchang.h"
#include "panchang/solar.h"

#include <stdio.h>
#include <string.h>

// ============================================================
// HOME SCREEN
// Nokia 5110 = 84 x 48 pixels
// ============================================================


// ============================================================
// HEART / BPM
// ============================================================

int heartBpm = DummyData::HEART_BPM;

bool showHeartIcon = true;

unsigned long lastHeartSwitch = 0;

const unsigned long HEART_SWITCH_INTERVAL = 5000;


// ============================================================
// HOME INFORMATION VIEW
// ============================================================
//
// false = Date + Weather
// true  = Tithi / SR / SS + Location
//
// DOWN changes this view.
//

bool showTithi = false;

bool homeInfoBlank = false;


// ============================================================
// MOON VIEW
// ============================================================

bool moonView = false;


// ============================================================
// MOON TITHI
// ============================================================
//
// Normally this comes automatically from Panchang.
//
// This variable is kept for compatibility with home.h.
//

uint8_t testMoonTithi = 1;


// ============================================================
// DISPLAY WIDTH
// ============================================================

const int DISPLAY_WIDTH = 84;


// ============================================================
// DYNAMIC DISPLAY TEXT
// ============================================================

char dateText[32];

char tithiText[320];

char sunriseSunset[80];

char locationText[120];


// ============================================================
// PANCHANG CACHE
// ============================================================
//
// Panchang astronomy calculation is expensive.
//
// We don't calculate it on every 50 ms screen refresh.
//
// Instead, calculate approximately once per minute.
//

PanchangData currentPanchang;

bool panchangValid = false;

unsigned long lastPanchangCalculation = 0;

const unsigned long PANCHANG_UPDATE_INTERVAL = 60000;


// ============================================================
// CURRENT LOCATION DATA
// ============================================================

double currentLatitude = 0.0;

double currentLongitude = 0.0;

TimezoneInfo currentTimezone;


// ============================================================
// SCROLL POSITIONS
// ============================================================

int tithiScrollX = DISPLAY_WIDTH;

unsigned long lastTithiScroll = 0;


// Location

int locationScrollX = DISPLAY_WIDTH;

unsigned long lastLocationScroll = 0;


// Weather

int weatherScrollX = DISPLAY_WIDTH;

unsigned long lastWeatherScroll = 0;


// Existing scroll speed

const unsigned long SCROLL_INTERVAL = 62;


// ============================================================
// FIRST LINE FIXED POSITIONS
// ============================================================

const int HEART_X = 2;

const int HEART_ICON_X = 10;

const int ALARM_X = 38;

const int GPS_X = 50;

const int COMPASS_X = 62;

const int SENSOR_X = 74;


// ============================================================
// FIRST LINE STATES
// ============================================================

bool compassEnabled = false;

bool otherSensorsEnabled = false;


// ============================================================
// HEART BITMAP
// ============================================================

const unsigned char heartBitmap[] PROGMEM =
{
    0b01100110,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000
};


void drawHeartIcon(
    int x,
    int y
)
{
    display.drawBitmap(
        x,
        y,
        heartBitmap,
        8,
        6,
        BLACK
    );
}


// ============================================================
// DEGREE SYMBOL
// ============================================================

void drawDegreeSymbol(
    int x,
    int y
)
{
    display.drawPixel(x + 1, y, BLACK);
    display.drawPixel(x + 2, y, BLACK);

    display.drawPixel(x, y + 1, BLACK);
    display.drawPixel(x + 3, y + 1, BLACK);

    display.drawPixel(x, y + 2, BLACK);
    display.drawPixel(x + 3, y + 2, BLACK);

    display.drawPixel(x + 1, y + 3, BLACK);
    display.drawPixel(x + 2, y + 3, BLACK);
}


// ============================================================
// SMALL DASH
// ============================================================

void drawSmallDash(
    int x,
    int y
)
{
    display.drawLine(
        x,
        y + 3,
        x + 3,
        y + 3,
        BLACK
    );
}


// ============================================================
// FORMAT TIME
// ============================================================
//
// Example:
// 09:30pm
// 06:32am
//

void formatLocalDateTime(
    DateTime dt,
    char* output,
    size_t outputSize
)
{
    int hour = dt.hour();

    bool pm = hour >= 12;

    if (hour == 0)
    {
        hour = 12;
    }
    else if (hour > 12)
    {
        hour -= 12;
    }

    snprintf(
        output,
        outputSize,
        "%02d:%02d%s",
        hour,
        dt.minute(),
        pm ? "pm" : "am"
    );
}


// ============================================================
// FORMAT DATE
// ============================================================

void updateDateText(
    DateTime dt
)
{
    static const char* months[] =
    {
        "JAN",
        "FEB",
        "MAR",
        "APR",
        "MAY",
        "JUN",
        "JUL",
        "AUG",
        "SEP",
        "OCT",
        "NOV",
        "DEC"
    };

    static const char* days[] =
    {
        "SUN",
        "MON",
        "TUE",
        "WED",
        "THU",
        "FRI",
        "SAT"
    };

    snprintf(
        dateText,
        sizeof(dateText),
        "%04d:%s:%d:%s",
        dt.year(),
        months[dt.month() - 1],
        dt.day(),
        days[dt.dayOfTheWeek()]
    );
}


// ============================================================
// RESET TITHI SCROLL
// ============================================================

void resetTithiScroll()
{
    tithiScrollX = DISPLAY_WIDTH;

    lastTithiScroll = millis();
}


// ============================================================
// RESET LOCATION SCROLL
// ============================================================

void resetLocationScroll()
{
    locationScrollX = DISPLAY_WIDTH;

    lastLocationScroll = millis();
}


// ============================================================
// RESET WEATHER SCROLL
// ============================================================

void resetWeatherScroll()
{
    weatherScrollX = DISPLAY_WIDTH;

    lastWeatherScroll = millis();
}


// ============================================================
// TOGGLE HOME VIEW
// ============================================================

void toggleHomeView()
{
    // Do not change HOME information
    // while Moon View is active.

    if (moonView)
    {
        return;
    }

    showTithi = !showTithi;

    homeInfoBlank = false;

    resetTithiScroll();

    resetLocationScroll();

    resetWeatherScroll();
}


// ============================================================
// TOGGLE MOON VIEW
// ============================================================

void toggleMoonView()
{
    moonView = !moonView;

    // --------------------------------------------------------
    // Enter Moon View
    // --------------------------------------------------------

    if (moonView)
    {
        homeInfoBlank = true;

        Serial.print(
            "MOON VIEW ON - TITHI "
        );

        Serial.println(
            testMoonTithi
        );

        return;
    }


    // --------------------------------------------------------
    // Leave Moon View
    // --------------------------------------------------------

    homeInfoBlank = false;

    resetTithiScroll();

    resetLocationScroll();

    resetWeatherScroll();

    Serial.println(
        "MOON VIEW OFF"
    );
}


// ============================================================
// MOON VIEW STATE
// ============================================================

bool isMoonView()
{
    return moonView;
}


// ============================================================
// COMPATIBILITY FUNCTION
// ============================================================
//
// Kept because home.h already exposes this function.
//
// It can still be used for testing.
// Normally Moon Tithi is automatically updated.
//

void setTestMoonTithi(
    uint8_t tithi
)
{
    if (tithi < 1)
    {
        tithi = 1;
    }

    if (tithi > 30)
    {
        tithi = 30;
    }

    testMoonTithi = tithi;
}


// ============================================================
// PAKSHA NAME
// ============================================================

const char* getPakshaName(
    uint8_t tithi
)
{
    if (tithi <= 15)
    {
        return "Shukla";
    }

    return "Krishna";
}


// ============================================================
// BLANK HOME INFORMATION LINES
// ============================================================

void blankHomeInfoLines()
{
    homeInfoBlank = !homeInfoBlank;

    if (!homeInfoBlank)
    {
        resetTithiScroll();

        resetLocationScroll();

        resetWeatherScroll();
    }
}


// ============================================================
// PANCHANG UPDATE
// ============================================================
//
// IMPORTANT:
//
// RTC stores India time.
//
// 1. India time -> UTC
// 2. UTC -> calculate Tithi/Nakshatra
// 3. UTC -> location local time
// 4. Location local date -> Sunrise/Sunset
//
// ============================================================

void updatePanchangData(
    bool forceUpdate = false
)
{
    unsigned long nowMillis =
        millis();

    if (
        !forceUpdate &&
        panchangValid &&
        (
            nowMillis -
            lastPanchangCalculation
            <
            PANCHANG_UPDATE_INTERVAL
        )
    )
    {
        return;
    }


    // ========================================================
    // GPS LOCATION
    // ========================================================

    currentLatitude =
        getGPSLatitude();

    currentLongitude =
        getGPSLongitude();


    // ========================================================
    // INDIA RTC TIME
    // ========================================================

    DateTime indiaTime =
        rtc.now();


    // ========================================================
    // INDIA -> UTC
    // ========================================================

    DateTime utcTime =
        convertIndiaTimeToUTC(
            indiaTime
        );


    // ========================================================
    // LOCATION TIMEZONE
    // ========================================================

    currentTimezone =
        getCurrentTimezone(
            utcTime
        );


    // ========================================================
    // LOCATION LOCAL TIME
    // ========================================================

    DateTime locationTime =
        getLocationLocalTime(
            utcTime
        );


    // ========================================================
    // TITHI
    // ========================================================

    currentPanchang.tithi =
        calculateTithi(
            utcTime
        );


    // ========================================================
    // NAKSHATRA
    // ========================================================

    currentPanchang.nakshatra =
        calculateNakshatra(
            utcTime
        );


    // ========================================================
    // SUNRISE / SUNSET
    // ========================================================

    SolarTimes solar =
        calculateSunriseSunset(
            locationTime.year(),
            locationTime.month(),
            locationTime.day(),
            currentLatitude,
            currentLongitude,
            currentTimezone.utcOffsetMinutes / 60.0
        );


    currentPanchang.sunrise =
        solar.sunrise;

    currentPanchang.sunset =
        solar.sunset;


    // ========================================================
    // CURRENT MOON TITHI
    // ========================================================

    testMoonTithi =
        currentPanchang.tithi.number;


    // ========================================================
    // DATE
    // ========================================================

    updateDateText(
        showTithi
            ? locationTime
            : indiaTime
    );


    // ========================================================
    // TITHI NAMES
    // ========================================================

    const char* tithiName =
        getPanchangTithiName(
            currentPanchang.tithi.number
        );


    const char* nextTithiName =
        getPanchangTithiName(
            currentPanchang.tithi.nextNumber
        );


    // ========================================================
    // TITHI LOCAL TIMES
    // ========================================================

    DateTime tithiStartLocal =
        getLocationLocalTime(
            currentPanchang.tithi.startUTC
        );


    DateTime tithiEndLocal =
        getLocationLocalTime(
            currentPanchang.tithi.endUTC
        );


    DateTime nextTithiStartLocal =
        getLocationLocalTime(
            currentPanchang.tithi.nextStartUTC
        );


    char tithiStartText[16];

    char tithiEndText[16];

    char nextTithiStartText[16];


    formatLocalDateTime(
        tithiStartLocal,
        tithiStartText,
        sizeof(tithiStartText)
    );


    formatLocalDateTime(
        tithiEndLocal,
        tithiEndText,
        sizeof(tithiEndText)
    );


    formatLocalDateTime(
        nextTithiStartLocal,
        nextTithiStartText,
        sizeof(nextTithiStartText)
    );


    // ========================================================
    // TITHI START LABEL
    // ========================================================
    //
    // Same local date  -> TS
    // Previous date    -> YS
    // Future date      -> ToS
    //
    // ========================================================

    const char* tithiStartLabel;

    if (
        tithiStartLocal.year() ==
            locationTime.year() &&
        tithiStartLocal.month() ==
            locationTime.month() &&
        tithiStartLocal.day() ==
            locationTime.day()
    )
    {
        tithiStartLabel = "TS";
    }
    else if (
        tithiStartLocal < locationTime
    )
    {
        tithiStartLabel = "YS";
    }
    else
    {
        tithiStartLabel = "ToS";
    }


    // ========================================================
    // TITHI END LABEL
    // ========================================================

    const char* tithiEndLabel;

    if (
        tithiEndLocal.year() ==
            locationTime.year() &&
        tithiEndLocal.month() ==
            locationTime.month() &&
        tithiEndLocal.day() ==
            locationTime.day()
    )
    {
        tithiEndLabel = "TE";
    }
    else if (
        tithiEndLocal > locationTime
    )
    {
        tithiEndLabel = "ToE";
    }
    else
    {
        tithiEndLabel = "YE";
    }


    // ========================================================
    // NEXT TITHI START LABEL
    // ========================================================

    const char* nextTithiStartLabel;

    if (
        nextTithiStartLocal.year() ==
            locationTime.year() &&
        nextTithiStartLocal.month() ==
            locationTime.month() &&
        nextTithiStartLocal.day() ==
            locationTime.day()
    )
    {
        nextTithiStartLabel = "TS";
    }
    else if (
        nextTithiStartLocal >
            locationTime
    )
    {
        nextTithiStartLabel = "ToS";
    }
    else
    {
        nextTithiStartLabel = "YS";
    }


    // ========================================================
    // CURRENT NAKSHATRA
    // ========================================================

    const char* nakshatraName =
        getPanchangNakshatraName(
            currentPanchang.nakshatra.number
        );


    DateTime nakshatraStartLocal =
        getLocationLocalTime(
            currentPanchang.nakshatra.startUTC
        );


    DateTime nakshatraEndLocal =
        getLocationLocalTime(
            currentPanchang.nakshatra.endUTC
        );


    char nakshatraStartText[16];

    char nakshatraEndText[16];


    formatLocalDateTime(
        nakshatraStartLocal,
        nakshatraStartText,
        sizeof(nakshatraStartText)
    );


    formatLocalDateTime(
        nakshatraEndLocal,
        nakshatraEndText,
        sizeof(nakshatraEndText)
    );


    // ========================================================
    // FIND NEXT NAKSHATRA
    // ========================================================
    //
    // Current Nakshatra ends at endUTC.
    //
    // Calculate just AFTER that boundary.
    // The returned Nakshatra is the next one.
    //
    // ========================================================

    DateTime nextNakshatraCheckUTC =
        currentPanchang.nakshatra.endUTC
        +
        TimeSpan(
            0,
            0,
            0,
            1
        );


    NakshatraInfo nextNakshatra =
        calculateNakshatra(
            nextNakshatraCheckUTC
        );


    const char* nextNakshatraName =
        getPanchangNakshatraName(
            nextNakshatra.number
        );


    DateTime nextNakshatraStartLocal =
        getLocationLocalTime(
            nextNakshatra.startUTC
        );


    char nextNakshatraStartText[16];


    formatLocalDateTime(
        nextNakshatraStartLocal,
        nextNakshatraStartText,
        sizeof(nextNakshatraStartText)
    );


    // ========================================================
    // NAKSHATRA START LABEL
    // ========================================================

    const char* nakshatraStartLabel;

    if (
        nakshatraStartLocal.year() ==
            locationTime.year() &&
        nakshatraStartLocal.month() ==
            locationTime.month() &&
        nakshatraStartLocal.day() ==
            locationTime.day()
    )
    {
        nakshatraStartLabel = "TS";
    }
    else if (
        nakshatraStartLocal <
            locationTime
    )
    {
        nakshatraStartLabel = "YS";
    }
    else
    {
        nakshatraStartLabel = "ToS";
    }


    // ========================================================
    // NAKSHATRA END LABEL
    // ========================================================

    const char* nakshatraEndLabel;

    if (
        nakshatraEndLocal.year() ==
            locationTime.year() &&
        nakshatraEndLocal.month() ==
            locationTime.month() &&
        nakshatraEndLocal.day() ==
            locationTime.day()
    )
    {
        nakshatraEndLabel = "TE";
    }
    else if (
        nakshatraEndLocal >
            locationTime
    )
    {
        nakshatraEndLabel = "ToE";
    }
    else
    {
        nakshatraEndLabel = "YE";
    }


    // ========================================================
    // NEXT NAKSHATRA START LABEL
    // ========================================================

    const char* nextNakshatraStartLabel;

    if (
        nextNakshatraStartLocal.year() ==
            locationTime.year() &&
        nextNakshatraStartLocal.month() ==
            locationTime.month() &&
        nextNakshatraStartLocal.day() ==
            locationTime.day()
    )
    {
        nextNakshatraStartLabel = "TS";
    }
    else if (
        nextNakshatraStartLocal >
            locationTime
    )
    {
        nextNakshatraStartLabel = "ToS";
    }
    else
    {
        nextNakshatraStartLabel = "YS";
    }


    // ========================================================
    // SUNRISE / SUNSET
    // ========================================================

    char sunriseText[16];

    char sunsetText[16];


    formatSolarTime(
        currentPanchang.sunrise,
        sunriseText,
        sizeof(sunriseText)
    );


    formatSolarTime(
        currentPanchang.sunset,
        sunsetText,
        sizeof(sunsetText)
    );


    // ========================================================
    // FINAL TITHI / NAKSHATRA SCROLL STRING
    // ========================================================
    //
    // FINAL FORMAT:
    //
    // SR-06:xxam SS-05:xxpm /#/
    //
    // Tidhi-Chaturthi: YS-06:xxpm TE-06:xxpm,
    // Nxt-Tidhi:Panchami TS-06:xxpm /#/
    //
    // Nak-Rohini: YS-10:xxpm TE-10:xxpm,
    // Nxt-Nak:Kruthika TS-10:xxpm
    //
    // ========================================================

    snprintf(
        tithiText,
        sizeof(tithiText),

        "SR-%s SS-%s /#/ "
        "Tidhi-%s: %s-%s %s-%s, "
        "Nxt-Tidhi:%s %s-%s /#/ "
        "Nak-%s: %s-%s %s-%s, "
        "Nxt-Nak:%s %s-%s  ",

        // ----------------------------------------------------
        // SUNRISE / SUNSET
        // ----------------------------------------------------

        sunriseText,
        sunsetText,

        // ----------------------------------------------------
        // CURRENT TITHI
        // ----------------------------------------------------

        tithiName,

        tithiStartLabel,
        tithiStartText,

        tithiEndLabel,
        tithiEndText,

        // ----------------------------------------------------
        // NEXT TITHI
        // ----------------------------------------------------

        nextTithiName,

        nextTithiStartLabel,
        nextTithiStartText,

        // ----------------------------------------------------
        // CURRENT NAKSHATRA
        // ----------------------------------------------------

        nakshatraName,

        nakshatraStartLabel,
        nakshatraStartText,

        nakshatraEndLabel,
        nakshatraEndText,

        // ----------------------------------------------------
        // NEXT NAKSHATRA
        // ----------------------------------------------------

        nextNakshatraName,

        nextNakshatraStartLabel,
        nextNakshatraStartText
    );


    // ========================================================
    // LOCATION STRING
    // ========================================================

    snprintf(
        locationText,
        sizeof(locationText),
        "Lat:%.4f Lon:%.4f %s %s UTC%+.1f  ",
        currentLatitude,
        currentLongitude,
        currentTimezone.countryName,
        currentTimezone.timezoneId,
        currentTimezone.utcOffsetMinutes / 60.0
    );


    // ========================================================
    // VALID
    // ========================================================

    panchangValid =
        true;

    lastPanchangCalculation =
        nowMillis;


    // ========================================================
    // SERIAL DEBUG
    // ========================================================

    Serial.println();

    Serial.println(
        "===== HOME PANCHANG UPDATE ====="
    );

    Serial.print("Tithi: ");
    Serial.println(
        tithiName
    );

    Serial.print("Tithi Start: ");
    Serial.print(
        tithiStartLabel
    );
    Serial.print(" ");
    Serial.println(
        tithiStartText
    );

    Serial.print("Tithi End: ");
    Serial.print(
        tithiEndLabel
    );
    Serial.print(" ");
    Serial.println(
        tithiEndText
    );

    Serial.print("Next Tithi: ");
    Serial.print(
        nextTithiName
    );
    Serial.print(" ");
    Serial.print(
        nextTithiStartLabel
    );
    Serial.print(" ");
    Serial.println(
        nextTithiStartText
    );

    Serial.print("Nakshatra: ");
    Serial.println(
        nakshatraName
    );

    Serial.print("Nak Start: ");
    Serial.print(
        nakshatraStartLabel
    );
    Serial.print(" ");
    Serial.println(
        nakshatraStartText
    );

    Serial.print("Nak End: ");
    Serial.print(
        nakshatraEndLabel
    );
    Serial.print(" ");
    Serial.println(
        nakshatraEndText
    );

    Serial.print("Next Nakshatra: ");
    Serial.print(
        nextNakshatraName
    );
    Serial.print(" ");
    Serial.print(
        nextNakshatraStartLabel
    );
    Serial.print(" ");
    Serial.println(
        nextNakshatraStartText
    );

    Serial.print("Sunrise: ");
    Serial.println(
        sunriseText
    );

    Serial.print("Sunset: ");
    Serial.println(
        sunsetText
    );

    Serial.println(
        "================================="
    );
}

// ============================================================
// DATE LINE
// ============================================================

void drawDateLine()
{
    int textWidth =
        watchTextWidth(
            dateText,
            1
        );

    int x =
        (DISPLAY_WIDTH - textWidth) / 2;

    if (x < 0)
    {
        x = 0;
    }

    drawWatchText(
        x,
        30,
        dateText,
        1
    );
}


// ============================================================
// TITHI + SR/SS WIDTH
// ============================================================

int getTithiLineWidth()
{
    return
        watchTextWidth(
            tithiText,
            1
        )
        +
        watchTextWidth(
            sunriseSunset,
            1
        );
}


// ============================================================
// DRAW TITHI + SR/SS
// ============================================================

void drawTithiLine()
{
    int x =
        tithiScrollX;


    drawWatchText(
        x,
        30,
        tithiText,
        1
    );


    int tithiWidth =
        watchTextWidth(
            tithiText,
            1
        );


    drawWatchText(
        x + tithiWidth,
        30,
        sunriseSunset,
        1
    );
}


// ============================================================
// UPDATE TITHI SCROLL
// ============================================================

void updateTithiScroll(
    unsigned long currentMillis
)
{
    if (
        currentMillis -
        lastTithiScroll
        <
        SCROLL_INTERVAL
    )
    {
        return;
    }

    lastTithiScroll =
        currentMillis;

    tithiScrollX--;


    int textWidth =
        getTithiLineWidth();


    if (
        tithiScrollX +
        textWidth
        <=
        0
    )
    {
        tithiScrollX =
            DISPLAY_WIDTH;
    }
}


// ============================================================
// WEATHER WIDTH
// ============================================================

int getWeatherWidth()
{
    int width = 0;


    char temperatureText[32];

    char humidityText[40];

    char feelsLikeText[32];


    snprintf(
        temperatureText,
        sizeof(temperatureText),
        "Tem: %d",
        DummyData::TEMPERATURE_C
    );


    snprintf(
        humidityText,
        sizeof(humidityText),
        "C Hum: %d%%",
        DummyData::HUMIDITY_PERCENT
    );


    snprintf(
        feelsLikeText,
        sizeof(feelsLikeText),
        "Feels like %d",
        DummyData::TEMPERATURE_C
    );


    width +=
        watchTextWidth(
            temperatureText,
            1
        );

    width += 5;


    width +=
        watchTextWidth(
            humidityText,
            1
        );

    width += 5;


    width +=
        watchTextWidth(
            feelsLikeText,
            1
        );


    return width;
}


// ============================================================
// DRAW WEATHER
// ============================================================

void drawWeatherLine()
{
    char temperatureText[32];

    char humidityText[40];

    char feelsLikeText[32];


    snprintf(
        temperatureText,
        sizeof(temperatureText),
        "Tem: %d",
        DummyData::TEMPERATURE_C
    );


    snprintf(
        humidityText,
        sizeof(humidityText),
        "C Hum: %d%%",
        DummyData::HUMIDITY_PERCENT
    );


    snprintf(
        feelsLikeText,
        sizeof(feelsLikeText),
        "Feels like %d",
        DummyData::TEMPERATURE_C
    );


    int currentX =
        weatherScrollX;


    drawWatchText(
        currentX,
        40,
        temperatureText,
        1
    );


    currentX +=
        watchTextWidth(
            temperatureText,
            1
        );


    drawDegreeSymbol(
        currentX + 1,
        40
    );


    currentX += 5;


    drawWatchText(
        currentX,
        40,
        humidityText,
        1
    );


    currentX +=
        watchTextWidth(
            humidityText,
            1
        );


    currentX += 5;


    drawWatchText(
        currentX,
        40,
        feelsLikeText,
        1
    );
}


// ============================================================
// UPDATE WEATHER SCROLL
// ============================================================

void updateWeatherScroll(
    unsigned long currentMillis
)
{
    if (
        currentMillis -
        lastWeatherScroll
        <
        SCROLL_INTERVAL
    )
    {
        return;
    }


    lastWeatherScroll =
        currentMillis;


    weatherScrollX--;


    int weatherWidth =
        getWeatherWidth();


    if (
        weatherScrollX +
        weatherWidth
        <=
        0
    )
    {
        weatherScrollX =
            DISPLAY_WIDTH;
    }
}


// ============================================================
// UPDATE LOCATION SCROLL
// ============================================================

void updateLocationScroll(
    unsigned long currentMillis
)
{
    if (
        currentMillis -
        lastLocationScroll
        <
        SCROLL_INTERVAL
    )
    {
        return;
    }


    lastLocationScroll =
        currentMillis;


    locationScrollX--;


    int locationWidth =
        watchTextWidth(
            locationText,
            1
        );


    if (
        locationScrollX +
        locationWidth
        <=
        0
    )
    {
        locationScrollX =
            DISPLAY_WIDTH;
    }
}


// ============================================================
// DRAW LOCATION
// ============================================================

void drawLocationLine()
{
    drawWatchText(
        locationScrollX,
        40,
        locationText,
        1
    );
}


// ============================================================
// MAIN TIME
// ============================================================

DateTime getHomeDisplayTime()
{
    // ========================================================
    // NORMAL HOME
    // India time
    // ========================================================

    if (!showTithi)
    {
        return rtc.now();
    }


    // ========================================================
    // LOCATION VIEW
    // GPS location local time
    // ========================================================

    DateTime indiaTime =
        rtc.now();


    DateTime utcTime =
        convertIndiaTimeToUTC(
            indiaTime
        );


    return getLocationLocalTime(
        utcTime
    );
}


// ============================================================
// MAIN TIME DRAW
// ============================================================

void drawMainTime(
    DateTime now
)
{
    int displayHour;

    bool pm;


    getDisplayHour(
        now.hour(),
        displayHour,
        pm
    );


    char timeText[6];


    sprintf(
        timeText,
        "%02d:%02d",
        displayHour,
        now.minute()
    );


    display.setTextSize(2);


    display.setCursor(
        0,
        12
    );


    display.print(
        timeText
    );


    display.setTextSize(1);


    display.setCursor(
        61,
        18
    );


    if (
        now.second() < 10
    )
    {
        display.print("0");
    }


    display.print(
        now.second()
    );


    if (!use24Hour)
    {
        display.setTextSize(0);


        display.setCursor(
            78,
            11
        );


        display.print(
            pm ? "P" : "A"
        );
    }
}


// ============================================================
// BORDER BELOW TIME
// ============================================================

void drawTimeBorder()
{
    display.drawLine(
        0,
        28,
        83,
        28,
        BLACK
    );
}


// ============================================================
// DRAW FIRST LINE
// ============================================================

void drawFirstLine()
{
    display.setTextSize(1);

    display.setTextWrap(false);


    // ========================================================
    // HEART / BPM
    // ========================================================

    if (showHeartIcon)
    {
        display.drawBitmap(
            HEART_ICON_X,
            1,
            heartBitmap,
            8,
            6,
            BLACK
        );
    }
    else
    {
        display.setCursor(
            HEART_X,
            1
        );

        display.print(
            heartBpm
        );

        display.print(
            "bpm"
        );
    }


    // ========================================================
    // ALARM = A
    // ========================================================

    if (alarmEnabled)
    {
        display.setCursor(
            ALARM_X,
            1
        );

        display.print("A");
    }


    // ========================================================
    // GPS = G
    // ========================================================

    if (gpsActive)
    {
        display.setCursor(
            GPS_X,
            1
        );

        display.print("G");
    }


    // ========================================================
    // COMPASS = C
    // ========================================================

    if (compassEnabled)
    {
        display.setCursor(
            COMPASS_X,
            1
        );

        display.print("C");
    }


    // ========================================================
    // OTHER SENSORS = S
    // ========================================================

    if (otherSensorsEnabled)
    {
        display.setCursor(
            SENSOR_X,
            1
        );

        display.print("S");
    }
}


// ============================================================
// MOON VIEW
// ============================================================
//
// Nokia 5110:
//
// X = 0..83
// Y = 29..47
//
// Moon:
// 25 x 16
//
// Position:
// X = 8
// Y = 30
//
// ============================================================

void drawMoonView()
{
    // --------------------------------------------------------
    // LEFT SIDE - MOON
    // --------------------------------------------------------

    drawMoon25x16(
        8,
        30,
        testMoonTithi
    );


    // --------------------------------------------------------
    // VERTICAL SEPARATOR
    // --------------------------------------------------------

    display.drawLine(
        42,
        29,
        42,
        47,
        BLACK
    );


    // --------------------------------------------------------
    // HORIZONTAL SEPARATOR
    // --------------------------------------------------------

    display.drawLine(
        43,
        38,
        83,
        38,
        BLACK
    );


    // --------------------------------------------------------
    // RIGHT SIDE
    // --------------------------------------------------------

    display.setTextSize(1);

    display.setTextWrap(false);


    // ========================================================
    // TOP BOX
    // ========================================================

    display.setCursor(
        48,
        30
    );


    if (
        testMoonTithi <= 15
    )
    {
        display.print(
            "Shukla"
        );
    }
    else
    {
        display.print(
            "Krishna"
        );
    }


    // ========================================================
    // BOTTOM BOX
    // ========================================================

    display.setCursor(
        43,
        40
    );

    display.print(
        getPanchangTithiName(
            testMoonTithi
        )
    );


    display.setCursor(
        43,
        47
    );

    display.print(
        "SR"
    );
}


// ============================================================
// HOME SCREEN
// ============================================================

void drawHomeScreen()
{
    unsigned long currentMillis =
        millis();


    // ========================================================
    // UPDATE PANCHANG
    // ========================================================

    updatePanchangData();


    // ========================================================
    // RTC / DISPLAY TIME
    // ========================================================

    DateTime now =
        getHomeDisplayTime();


    // ========================================================
    // HEART / BPM SWITCH
    // ========================================================

    if (
        currentMillis -
        lastHeartSwitch
        >=
        HEART_SWITCH_INTERVAL
    )
    {
        lastHeartSwitch =
            currentMillis;

        showHeartIcon =
            !showHeartIcon;
    }


    // ========================================================
    // UPDATE SCROLL
    // ========================================================

    if (
        !moonView &&
        !homeInfoBlank
    )
    {
        if (!showTithi)
        {
            updateWeatherScroll(
                currentMillis
            );
        }
        else
        {
            updateTithiScroll(
                currentMillis
            );

            updateLocationScroll(
                currentMillis
            );
        }
    }


    // ========================================================
    // CLEAR
    // ========================================================

    display.clearDisplay();

    display.setTextColor(BLACK);

    display.setTextSize(1);

    display.setTextWrap(false);


    // ========================================================
    // LINE 1
    // ========================================================

    drawFirstLine();


    // ========================================================
    // BORDER 1
    // ========================================================

    display.drawLine(
        0,
        9,
        83,
        9,
        BLACK
    );


    // ========================================================
    // MAIN TIME
    // ========================================================

    drawMainTime(
        now
    );


    // ========================================================
    // BORDER BELOW TIME
    // ========================================================

    drawTimeBorder();


    // ========================================================
    // MOON VIEW
    // ========================================================

    if (moonView)
    {
        drawMoonView();

        display.display();

        return;
    }


    // ========================================================
    // NORMAL HOME INFORMATION
    // ========================================================

    if (!homeInfoBlank)
    {
        // ----------------------------------------------------
        // LINE 3
        // ----------------------------------------------------

        if (!showTithi)
        {
            drawDateLine();
        }
        else
        {
            drawTithiLine();
        }


        // ----------------------------------------------------
        // BORDER ABOVE LINE 4
        // ----------------------------------------------------

        display.drawLine(
            0,
            38,
            83,
            38,
            BLACK
        );


        // ----------------------------------------------------
        // LINE 4
        // ----------------------------------------------------

        if (!showTithi)
        {
            drawWeatherLine();
        }
        else
        {
            drawLocationLine();
        }
    }


    // ========================================================
    // DISPLAY
    // ========================================================

    display.display();
}