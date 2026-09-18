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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "battery.h"

#include "startup_image.h"

#include "panchang/panchang.h"
#include "panchang/masa.h"


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

bool showTithi = false;
bool homeInfoBlank = false;
bool moonView = false;
bool backBlankView = false;
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
// LAST VALID GPS LOCATION
// ============================================================
//
// Latest successfully acquired GPS fix.
//
// This is separate from:
// - currentLatitude/currentLongitude
// - lastAstronomyLatitude/lastAstronomyLongitude
//
// These values may be restored from the GPS RTC cache at startup
// ============================================================

double lastValidLatitude = 0.0;

double lastValidLongitude = 0.0;


// ========================================================
// GPS → ASTRONOMY LOCATION TRACKING
// ========================================================

bool astronomyLocationValid = false;

double lastAstronomyLatitude = 0.0;
double lastAstronomyLongitude = 0.0;

const double GPS_ASTRONOMY_DISTANCE_KM = 70.0;

// ============================================================
// BACKGROUND PANCHANG TASK
// ============================================================

TaskHandle_t panchangTaskHandle = NULL;

volatile bool panchangTaskRunning = false;
volatile bool panchangTaskRequest = false;

double panchangRequestedLatitude = 0.0;
double panchangRequestedLongitude = 0.0;

bool timezoneRecalculationPending = false;

// ============================================================
// SCROLL POSITIONS
// ============================================================

int tithiScrollX = 42;

unsigned long lastTithiScroll = 0;


// Location

int locationScrollX = 42;

unsigned long lastLocationScroll = 0;


// Rutu / Masa / Season
char masaRutuSeasonText[160];

int masaRutuSeasonScrollX = 42;

unsigned long lastMasaRutuSeasonScroll = 0;



// Weather

int weatherScrollX = 42;

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
    tithiScrollX = 42;

    lastTithiScroll = millis();
}
// ============================================================
// MASA / RUTU / SEASON SCROLL
// ============================================================

void resetMasaRutuSeasonScroll()
{
    masaRutuSeasonScrollX = 42;
    lastMasaRutuSeasonScroll = millis();
}

// ============================================================
// RESET LOCATION SCROLL
// ============================================================

void resetLocationScroll()
{
    locationScrollX = 42;

    lastLocationScroll = millis();
}


// ============================================================
// RESET WEATHER SCROLL
// ============================================================

void resetWeatherScroll()
{
    weatherScrollX = 42;

    lastWeatherScroll = millis();
}


// ============================================================
// TOGGLE HOME VIEW
// ============================================================

void toggleHomeView()
{
    if (moonView)
    {
        return;
    }

    // Leaving BACK blank view
    backBlankView = false;

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

    if (moonView)
    {
        // Moon view replaces normal information area
        homeInfoBlank = true;

        // BACK blank view is no longer active
        backBlankView = false;

        Serial.print(
            "MOON VIEW ON - TITHI "
        );

        Serial.println(
            testMoonTithi
        );

        return;
    }

    // Returning from Moon view
    moonView = false;

    // If we are explicitly returning to normal HOME,
    // this will be corrected by the button handler.
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
// HOME TITHI VIEW CONTROL
// ============================================================

void setHomeTithiView(bool enabled)
{
    showTithi = enabled;

    // Leaving BACK blank view
    backBlankView = false;

    // Information area must be visible
    homeInfoBlank = false;

    resetTithiScroll();
    resetLocationScroll();
    resetWeatherScroll();
}


// ============================================================
// HOME TITHI VIEW STATE
// ============================================================

bool isHomeTithiView()
{
    return showTithi;
}


// ============================================================
// BACK BLANK VIEW
// ============================================================

void setBackBlankView(
    bool enabled
)
{
    backBlankView = enabled;

    if (enabled)
    {
        // BACK blank view must not show Moon
        moonView = false;

        // Leave Tithi view also
        showTithi = false;

        // Blank normal information area
        homeInfoBlank = true;

        resetTithiScroll();
        resetLocationScroll();
        resetWeatherScroll();

        Serial.println(
            "BACK BLANK VIEW ON"
        );
    }
    else
    {
        homeInfoBlank = false;

        resetTithiScroll();
        resetLocationScroll();
        resetWeatherScroll();

        Serial.println(
            "BACK BLANK VIEW OFF"
        );
    }
}


// ============================================================
// BACK BLANK VIEW STATE
// ============================================================

bool isBackBlankView()
{
    return backBlankView;
}


// ============================================================
// COMPATIBILITY FUNCTION
// ============================================================

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
//  MASA + PANCHANGA RUTU + ASTRONOMICAL SEASON
// ============================================================

static const char* getAstronomicalSeasonName(
    int month,
    int day
)
{
    // Northern Hemisphere astronomical seasons
    // Approximate Gregorian boundaries.
    //
    // Spring : Mar 20 - Jun 20
    // Summer : Jun 21 - Sep 22
    // Autumn : Sep 23 - Dec 20
    // Winter : Dec 21 - Mar 19

    if (
        (month == 3 && day >= 20) ||
        month == 4 ||
        month == 5 ||
        (month == 6 && day <= 20)
    )
    {
        return "Spring";
    }

    if (
        (month == 6 && day >= 21) ||
        month == 7 ||
        month == 8 ||
        (month == 9 && day <= 22)
    )
    {
        return "Summer";
    }

    if (
        (month == 9 && day >= 23) ||
        month == 10 ||
        month == 11 ||
        (month == 12 && day <= 20)
    )
    {
        return "Autumn";
    }

    return "Winter";
}

// ========================================================
// CALCULATE GPS DISTANCE
// ========================================================

double calculateGPSDistanceKm(
    double lat1,
    double lon1,
    double lat2,
    double lon2
)
{
    const double EARTH_RADIUS_KM = 6371.0;

    double dLat =
        (lat2 - lat1) * DEG_TO_RAD;

    double dLon =
        (lon2 - lon1) * DEG_TO_RAD;

    double a =
        sin(dLat / 2.0) *
        sin(dLat / 2.0)
        +
        cos(lat1 * DEG_TO_RAD) *
        cos(lat2 * DEG_TO_RAD) *
        sin(dLon / 2.0) *
        sin(dLon / 2.0);

    double c =
        2.0 *
        atan2(
            sqrt(a),
            sqrt(1.0 - a)
        );

    return EARTH_RADIUS_KM * c;
}

// ========================================================
// PANCHANG UPDATE FUNCTION DECLARATIONS
// ========================================================

void updatePanchangData(bool forceUpdate);

void updatePanchangDataAtLocation(
    bool forceUpdate,
    double latitude,
    double longitude
);

// ============================================================
// BACKGROUND PANCHANG CALCULATION TASK
// ============================================================

void panchangBackgroundTask(void *parameter)
{
    while (true)
    {
        if (
            panchangTaskRequest &&
            !panchangTaskRunning
        )
        {
            // ------------------------------------------------
            // Consume current request
            // ------------------------------------------------

            panchangTaskRequest =
                false;

            panchangTaskRunning =
                true;

            // ------------------------------------------------
            // Copy requested location
            // ------------------------------------------------

            double latitude =
                panchangRequestedLatitude;

            double longitude =
                panchangRequestedLongitude;

            Serial.println(
                "BACKGROUND: Panchang calculation START"
            );

            // ------------------------------------------------
            // Calculate Panchang
            //
            // This uses the CURRENT active timezone.
            // ------------------------------------------------

            updatePanchangDataAtLocation(
                true,
                latitude,
                longitude
            );

            // ------------------------------------------------
            // Store completed astronomy location
            // ------------------------------------------------

            lastAstronomyLatitude =
                latitude;

            lastAstronomyLongitude =
                longitude;

            astronomyLocationValid =
                true;

            panchangTaskRunning =
                false;

            Serial.println(
                "BACKGROUND: Panchang calculation DONE"
            );

            // ------------------------------------------------
            // If timezone changed while this calculation
            // was running, do another calculation.
            // ------------------------------------------------

            if (
                timezoneRecalculationPending
            )
            {
                timezoneRecalculationPending =
                    false;

                panchangRequestedLatitude =
                    currentLatitude;

                panchangRequestedLongitude =
                    currentLongitude;

                panchangTaskRequest =
                    true;

                Serial.println(
                    "BACKGROUND: Pending timezone recalculation queued"
                );
            }
        }

        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}
// ============================================================
// START BACKGROUND PANCHANG TASK
// ============================================================

void startPanchangBackgroundTask()
{
    if (panchangTaskHandle != NULL)
    {
        return;
    }

    xTaskCreatePinnedToCore(
        panchangBackgroundTask,
        "PanchangTask",
        8192,
        NULL,
        0,
        &panchangTaskHandle,
        0
    );

    Serial.println(
        "BACKGROUND: Panchang task created"
    );
}

// ============================================================
// HANDLE TIMEZONE CHANGE
// ============================================================
//
// A timezone change must recalculate Panchang even when the
// GPS location has NOT moved 70 km.
//
// If a Panchang background task is already running, remember
// the timezone recalculation request and run it again after
// the current task finishes.
//
// ============================================================

void handleTimezoneChange()
{
    if (!consumeTimezoneChangeRequest())
    {
        return;
    }

    Serial.println();
    Serial.println(
        "TIMEZONE: Change detected"
    );

    if (!astronomyLocationValid)
    {
        Serial.println(
            "TIMEZONE: No valid astronomy location yet"
        );

        panchangValid = false;

        return;
    }

    if (panchangTaskRunning)
    {
        Serial.println(
            "TIMEZONE: Panchang task already running"
        );

        timezoneRecalculationPending =
            true;

        panchangValid = false;

        return;
    }

    panchangRequestedLatitude =
        currentLatitude;

    panchangRequestedLongitude =
        currentLongitude;

    panchangTaskRequest =
        true;

    Serial.println(
        "TIMEZONE: Panchang recalculation requested"
    );
}


// ========================================================
// PROCESS NEW GPS FIX
// ========================================================

void processGPSFix(
    double latitude,
    double longitude
)
{
    // ----------------------------------------------------
    // NEW GPS LOCATION
    // ----------------------------------------------------

    currentLatitude = latitude;
    currentLongitude = longitude;

// ----------------------------------------------------
// LAST VALID GPS LOCATION
// ----------------------------------------------------
//
// This is a successfully acquired GPS fix.
//
// Keep a separate copy of the latest valid location.
// This location can later be used when GPS is OFF
// or when the next GPS window gets no fix.
//
// ----------------------------------------------------

lastValidLatitude =
    latitude;

lastValidLongitude =
    longitude;


setGPSLocation(
    latitude,
    longitude
);

    saveGPSCache(
    latitude,
    longitude
    );

    if (!astronomyLocationValid)
    {
        Serial.println(
            "GPS: First valid fix -> FULL astronomy update"
        );

        updatePanchangDataAtLocation(
            true,
            latitude,
            longitude
        );

        lastAstronomyLatitude =
            latitude;

        lastAstronomyLongitude =
            longitude;

        astronomyLocationValid = true;

        return;
    }


    // ----------------------------------------------------
    // DISTANCE FROM LAST FULL ASTRONOMY LOCATION
    // ----------------------------------------------------

    double distanceKm =
        calculateGPSDistanceKm(
            lastAstronomyLatitude,
            lastAstronomyLongitude,
            latitude,
            longitude
        );


    Serial.print(
        "GPS distance from last astronomy location: "
    );

    Serial.print(
        distanceKm,
        2
    );

    Serial.println(" km");


    // ----------------------------------------------------
    // LESS THAN 70 KM
    // ONLY LOCATION DATA UPDATE
    // ----------------------------------------------------

    if (
        distanceKm <
        GPS_ASTRONOMY_DISTANCE_KM
    )
    {
        Serial.println(
            "GPS: < 70 km -> LOCATION ONLY"
        );

        // GPS coordinates already updated above.

        DateTime indiaTime =
            rtc.now();

        DateTime utcTime =
            convertIndiaTimeToUTC(
                indiaTime
            );

        currentTimezone =
            getActiveTimezone(
                utcTime
            );

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
            // IMPORTANT:
    // Do NOT reset location scroll here.
    // GPS can provide fixes repeatedly.

    return;

    }


    // ----------------------------------------------------
    // 70 KM OR MORE
    // FULL ASTRONOMY UPDATE
    // ----------------------------------------------------

    Serial.println(
    "GPS: >= 70 km -> BACKGROUND astronomy update"
);

if (!panchangTaskRunning)
{
    panchangRequestedLatitude =
        latitude;

    panchangRequestedLongitude =
        longitude;

    panchangTaskRequest =
        true;
}
else
{
    Serial.println(
        "BACKGROUND: Panchang calculation already running"
    );
}
}

// ============================================================
// RESTORE LAST VALID GPS LOCATION AT STARTUP
// ============================================================

// ============================================================
// RESTORE LAST VALID GPS FROM CACHE
// ============================================================
//
// If GPS is unavailable during startup,
// use the last valid GPS location stored in RTC cache.
//
// This location becomes:
//     currentLatitude
//     currentLongitude
//     lastValidLatitude
//     lastValidLongitude
//
// Panchang can therefore continue using the last known
// GPS location even when GPS currently has no fix.
//
// ============================================================

void restoreLastValidGPSForHome()
{
    Serial.println();
    Serial.println(
        "GPS CACHE: Restoring last valid location..."
    );

    if (!restoreGPSCache())
    {
        Serial.println(
            "GPS CACHE: No cached GPS location"
        );

        return;
    }

    double cachedLatitude =
        getGPSLatitude();

    double cachedLongitude =
        getGPSLongitude();

    currentLatitude =
        cachedLatitude;

    currentLongitude =
        cachedLongitude;

    lastValidLatitude =
        cachedLatitude;

    lastValidLongitude =
        cachedLongitude;

    setGPSLocation(
        cachedLatitude,
        cachedLongitude
    );

    Serial.println(
        "GPS CACHE: Location restored into HOME"
    );

    Serial.print(
        "GPS CACHE: LAT = "
    );

    Serial.println(
        lastValidLatitude,
        6
    );

    Serial.print(
        "GPS CACHE: LON = "
    );

    Serial.println(
        lastValidLongitude,
        6
    );
}


// ============================================================
// PANCHANG UPDATE
// ============================================================

void updatePanchangDataAtLocation(
    bool forceUpdate,
    double latitude,
    double longitude
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
    latitude;

currentLongitude =
    longitude;

// ========================================================
// IMPORTANT:
// Keep timezone.cpp GPS location synchronized.
// ========================================================

    setGPSLocation(
    latitude,
    longitude
);

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
       getActiveTimezone(
        utcTime
        );


    // ========================================================
    // LOCATION LOCAL TIME
    // ========================================================

    DateTime locationTime =
        getActiveLocalTime(
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
    // TELUGU LUNAR MASA / RUTU
    // ========================================================

    MasaInfo masaInfo =
        calculateTeluguMasa(
            utcTime
        );

    currentPanchang.masaNumber =
        masaInfo.number;

    currentPanchang.adhikaMasa =
        masaInfo.adhika;

    currentPanchang.rutuNumber =
        getPanchangRutu(
            masaInfo.number
        );

    // ========================================================
// SEASON TEST
// ========================================================

const char* masaName =
    getPanchangMasaDisplayName(
        currentPanchang.masaNumber,
        currentPanchang.adhikaMasa
    );

const char* rutuName =
    getPanchangRutuName(
        currentPanchang.rutuNumber
    );

const char* astronomicalSeason =
    getAstronomicalSeasonName(
        locationTime.month(),
        locationTime.day()
    );

snprintf(
    masaRutuSeasonText,
    sizeof(masaRutuSeasonText),

    "M: %s \x01"
    "R: %s \x01"
    "S: %s",

    masaName,
    rutuName,
    astronomicalSeason
);

resetMasaRutuSeasonScroll();    

    Serial.println();
    Serial.println(
        "===== MASA / RUTU / SEASON TEST ====="
    );

    Serial.print("Location Date: ");
    Serial.print(locationTime.year());
    Serial.print("-");
    Serial.print(locationTime.month());
    Serial.print("-");
    Serial.println(locationTime.day());

    Serial.print("Latitude: ");
    Serial.println(currentLatitude, 4);

    Serial.print("Longitude: ");
    Serial.println(currentLongitude, 4);

    Serial.print("Masa:");
    Serial.println(masaName);

    Serial.print("Panchanga Rutu:");
    Serial.println(rutuName);

    Serial.print("Astronomical Season:");
    Serial.println(astronomicalSeason);

    Serial.println(
        "======================================"
    );


    

    // ========================================================
    // CURRENT MOON TITHI
    // ========================================================

    testMoonTithi =
        currentPanchang.tithi.number;


    // ========================================================
    // DATE
    // ========================================================

    updateDateText(
        locationTime
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
        getActiveLocalTime(
            currentPanchang.tithi.startUTC
        );

    DateTime tithiEndLocal =
        getActiveLocalTime(
            currentPanchang.tithi.endUTC
        );

    DateTime nextTithiStartLocal =
        getActiveLocalTime(
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
        getActiveLocalTime(
            currentPanchang.nakshatra.startUTC
        );


    DateTime nakshatraEndLocal =
        getActiveLocalTime(
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

    const char* nextNakshatraName =
        getPanchangNakshatraName(
            currentPanchang.nakshatra.nextNumber
        );


    DateTime nextNakshatraStartLocal =
        getActiveLocalTime(
            currentPanchang.nakshatra.nextStartUTC
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

    snprintf(
        tithiText,
        sizeof(tithiText),

        "SR-%s SS-%s \x01 "
        "Tidhi-%s: %s-%s %s-%s, "
        "Nxt-Tidhi:%s %s-%s \x01 "
        "Nak-%s: %s-%s %s-%s, "
        "Nxt-Nak:%s %s-%s  ",

        sunriseText,
        sunsetText,

        tithiName,

        tithiStartLabel,
        tithiStartText,

        tithiEndLabel,
        tithiEndText,

        nextTithiName,

        nextTithiStartLabel,
        nextTithiStartText,

        nakshatraName,

        nakshatraStartLabel,
        nakshatraStartText,

        nakshatraEndLabel,
        nakshatraEndText,

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
        nextTithiStartLabel
    );
    Serial.print(" ");
    Serial.print(
        nextTithiName
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
        nextNakshatraStartLabel
    );
    Serial.print(" ");
    Serial.print(
        nextNakshatraName
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

        Serial.print("Masa: ");
    Serial.println(
        getPanchangMasaDisplayName(
            currentPanchang.masaNumber,
            currentPanchang.adhikaMasa
        )
    );

    Serial.print("Rutu: ");
    Serial.println(
        getPanchangRutuName(
            currentPanchang.rutuNumber
        )
    );

    Serial.println(
        "================================="
    );
}



// ========================================================
// NORMAL PANCHANG UPDATE
// Uses current GPS location
// ========================================================

void updatePanchangData(
    bool forceUpdate
)
{
    double latitude = 0.0;
    double longitude = 0.0;

    


    // ========================================================
    // GPS FIX
    // ========================================================

    bool newGPSFix =
        DummyData::getNextGPSFix(
            latitude,
            longitude
        );


    // ========================================================
    // NO NEW GPS FIX
    // ========================================================

    if (!newGPSFix)
    {
        return;
    }


    // ========================================================
    // VALID GPS FIX
    //
    // processGPSFix() decides:
    //
    // < 70 km
    //      -> location only
    //
    // >= 70 km
    //      -> full astronomy
    //
    // First fix
    //      -> full astronomy
    // ========================================================

    processGPSFix(
        latitude,
        longitude
    );
}


// ============================================================
// DATE LINE
// ============================================================

void drawDateLine()
{
    // 17 characters × 4 px glyph
    // + 16 × 1 px gap
    // = exactly 84 pixels
    //
    // So start directly at x=0.
    // No left/right extra padding.

    drawDateText(
        2,
        30,
        dateText
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
    int x = tithiScrollX;

    const char* start = tithiText;

    while (*start != '\0')
    {
        // Find graphical separator marker
        const char* separator =
            strchr(start, 0x01);

        // ----------------------------------------------------
        // TEXT BEFORE SEPARATOR
        // ----------------------------------------------------
        if (separator != nullptr)
        {
            char segment[320];

            int len = separator - start;

            if (len > 319)
                len = 319;

            memcpy(segment, start, len);
            segment[len] = '\0';

            drawWatchText(
                x,
                30,
                segment,
                1
            );

            x += watchTextWidth(
                segment,
                1
            );

            // ------------------------------------------------
            // GRAPHICAL ||
            // Touches top and bottom borders
            // ------------------------------------------------
            display.drawLine(
                x,
                28,
                x,
                38,
                BLACK
            );

            display.drawLine(
                x + 1,
                28,
                x + 1,
                38,
                BLACK
            );

            // Move exactly after ||
            x += 2;

            // Continue after separator
            start = separator + 1;
        }
        else
        {
            // ------------------------------------------------
            // LAST TEXT PART
            // ------------------------------------------------
            drawWatchText(
                x,
                30,
                start,
                1
            );

            break;
        }
    }
}

// ============================================================
// DRAW MASA / RUTU / SEASON
// ============================================================

void drawMasaRutuSeasonLine()
{
    drawWatchText(
        masaRutuSeasonScrollX,
        40,
        masaRutuSeasonText,
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
// MASA / RUTU / SEASON SCROLL
// ============================================================

void updateMasaRutuSeasonScroll(unsigned long currentMillis)
{
    if (
        currentMillis -
        lastMasaRutuSeasonScroll
        <
        SCROLL_INTERVAL
    )
    {
        return;
    }

    lastMasaRutuSeasonScroll =
        currentMillis;

    masaRutuSeasonScrollX--;

    int textWidth =
        watchTextWidth(
            masaRutuSeasonText,
            1
        );

    if (
        masaRutuSeasonScrollX +
        textWidth
        <=
        0
    )
    {
        masaRutuSeasonScrollX =
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

// ============================================================
// MAIN HOME DISPLAY TIME
// ============================================================
//
// HOME always starts from the physical RTC which stores
// India time.
//
// India RTC
//     ↓
// UTC
//     ↓
// Active timezone
//     ↓
// Location local time
//
// In CURRENT LOCATION mode, the timezone must come from
// the latest GPS coordinates stored in timezone.cpp.
// ============================================================

DateTime getHomeDisplayTime()
{
    DateTime indiaTime =
    rtc.now();

DateTime utcTime =
    convertIndiaTimeToUTC(
        indiaTime
    );

DateTime localTime =
    getActiveLocalTime(
        utcTime
    );

return localTime;
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


    drawBoldTime(
        2,
        12,
        displayHour,
        now.minute()
    );


    // ========================================================
    // SECONDS
    // ========================================================

    drawSecondsDigit(
        64,
        19,
        now.second() / 10
    );

    drawSecondsDigit(
        70,
        19,
        now.second() % 10
    );


    // ========================================================
    // AM / PM
    // ========================================================

    if (!use24Hour)
    {
        drawWatchText(
            78,
            11,
            pm ? "P" : "A",
            1
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
        char bpmText[16];

        snprintf(
            bpmText,
            sizeof(bpmText),
            "%dbpm",
            heartBpm
        );

        drawWatchText(
            HEART_X,
            1,
            bpmText,
            1
        );
    }


    // ========================================================
    // ALARM = A
    // ========================================================

    if (alarmEnabled)
    {
        drawWatchText(
            ALARM_X,
            1,
            "A",
            1
        );
    }


    // ========================================================
    // GPS = G
    // ========================================================

    if (gpsActive)
    {
        drawWatchText(
            GPS_X,
            1,
            "G",
            1
        );
    }


    // ========================================================
    // COMPASS = C
    // ========================================================

    if (compassEnabled)
    {
        drawWatchText(
            COMPASS_X,
            1,
            "C",
            1
        );
    }


    // ========================================================
    // OTHER SENSORS = S
    // ========================================================

    if (otherSensorsEnabled)
    {
        drawWatchText(
            SENSOR_X,
            1,
            "S",
            1
        );
    }
}


// ============================================================
// BACK BLANK VIEW - BATTERY DISPLAY
//
// BATTERY INDICATOR
//
// Available area : 41 x 9
// Inside area    : 39 x 7
//
// One complete point = 7 x 6 pixels = 20%
//
// 5%  = 3 x 2 bottom
// 10% = 4 x 6
// 15% = 4 x 6 + 3 x 2 bottom-right
// 20% = 7 x 6
//
// Gap between points = 1 pixel
//
// BLINK RULE:
//
// Battery < 20%:
// First/current point blinks.
//
// Examples:
//
// 9%  -> 10% visual -> █      BLINK
// 11% -> 15% visual -> █▀     BLINK
// 16% -> 15% visual -> █▀     BLINK
// 18% -> 20% visual -> ██     BLINK
//
// 20% -> ██             STEADY
// 22% -> ██ ▀           STEADY
//
// Text always shows actual battery percentage.
// ============================================================

void drawBackBlankBattery()
{
    int batteryPercent =
        DummyData::BATTERY_PERCENT;


    // ========================================================
    // CLAMP BATTERY
    // ========================================================

    if (batteryPercent < 0)
    {
        batteryPercent = 0;
    }

    if (batteryPercent > 100)
    {
        batteryPercent = 100;
    }

     // ========================================================
    // LOCATION - LINE 3
    //
    // BACK button view:
    // Location is moved from Line 4 to Line 3.
    // ========================================================

    drawWatchText(
        locationScrollX,
        30,
        locationText,
        1
    );


    // ========================================================
    // BORDER BELOW LOCATION
    // ========================================================

    display.drawLine(
        0,
        38,
        83,
        38,
        BLACK
    );

    // ========================================================
    // TOP BORDER
    // ========================================================

    display.drawLine(
        0,
        38,
        83,
        38,
        BLACK
    );


    // ========================================================
    // LEFT SIDE : B:XX%
    // ========================================================

    char batteryText[16];

    snprintf(
        batteryText,
        sizeof(batteryText),
        "B:%d%%",
        batteryPercent
    );


    int charCount =
        strlen(batteryText);


    // 1 px extra gap between characters
    const int extraGap = 1;


    int baseWidth =
        watchTextWidth(
            batteryText,
            1
        );


    int totalTextWidth =
        baseWidth +
        ((charCount - 1) * extraGap);


    int textX =
        (42 - totalTextWidth) / 2;


    if (textX < 1)
    {
        textX = 1;
    }


    int textY = 40;

    int currentX = textX;


    // ========================================================
    // BOLD BATTERY TEXT
    // ========================================================

    for (int i = 0; i < charCount; i++)
{
    char oneChar[2];

    oneChar[0] =
        batteryText[i];

    oneChar[1] =
        '\0';


    // ====================================================
    // % SYMBOL
    //
    // Draw only once = NORMAL
    // ====================================================

    if (batteryText[i] == '%')
    {
        drawWatchChar(
            currentX,
            textY,
            '%',
            1
        );
    }
    else
    {
        // =================================================
        // B, :, DIGITS
        //
        // Draw twice with 1 px offset = BOLD
        // =================================================

        drawWatchChar(
            currentX,
            textY,
            oneChar[0],
            1
        );

        drawWatchChar(
            currentX + 1,
            textY,
            oneChar[0],
            1
        );
    }


    int charWidth =
        watchTextWidth(
            oneChar,
            1
        );


    currentX +=
        charWidth +
        extraGap;
}


    // ========================================================
    // VERTICAL SEPARATOR
    // ========================================================

    display.drawLine(
        42,
        39,
        42,
        47,
        BLACK
    );


    // ========================================================
    // BATTERY INDICATOR AREA
    //
    // x = 43 ... 83
    // width = 41
    //
    // y = 39 ... 47
    // height = 9
    // ========================================================

    const int areaX = 43;

    const int areaY = 39;

    const int areaWidth = 41;

    const int areaHeight = 9;


    // ========================================================
    // USABLE AREA
    //
    // 1 px gap around the indicator.
    //
    // x = 44 ... 82 = 39 px
    // y = 40 ... 46 = 7 px
    // ========================================================

    const int usableX =
        areaX + 1;

    const int usableY =
        areaY + 1;

    const int usableWidth =
        areaWidth - 2;

    const int usableHeight =
        areaHeight - 2;


    // ========================================================
    // POINT SIZE
    //
    // COMPLETE POINT = 7 x 6
    //
    // Five points:
    //
    // 7 + 1 + 7 + 1 + 7 + 1 + 7 + 1 + 7
    //
    // = 39 px
    //
    // Exactly fits usable width.
    // ========================================================

    const int pointWidth = 7;

    const int pointHeight = 7;

    const int pointGap = 1;


    // ========================================================
    // POINT VERTICAL POSITION
    // ========================================================

    const int pointY =
        usableY +
        (
            (usableHeight - pointHeight)
            / 2
        );


    // ========================================================
    // CONVERT ACTUAL BATTERY TO VISUAL LEVEL
    //
    // We intentionally round UP in selected ranges
    // so the visual matches the requested examples.
    //
    // Within each 20% block:
    //
    // 1..5   -> 5%
    // 6..10  -> 10%
    // 11..17 -> 15%
    // 18..19 -> 20%
    // 20     -> complete point
    //
    // Examples:
    //
    // 9  -> 10
    // 11 -> 15
    // 16 -> 15
    // 18 -> 20
    // 20 -> 20
    // 22 -> 25
    // ========================================================

    int completeBlocks =
        batteryPercent / 20;


    int remainder =
        batteryPercent % 20;


    int visualRemainder = 0;


    if (remainder == 0)
    {
        visualRemainder = 0;
    }
    else if (remainder <= 5)
    {
        visualRemainder = 5;
    }
    else if (remainder <= 10)
    {
        visualRemainder = 10;
    }
    else if (remainder <= 17)
    {
        visualRemainder = 15;
    }
    else
    {
        visualRemainder = 20;
    }


    int indicatorPercent =
        (completeBlocks * 20) +
        visualRemainder;


    if (indicatorPercent < 0)
    {
        indicatorPercent = 0;
    }

    if (indicatorPercent > 100)
    {
        indicatorPercent = 100;
    }


    // ========================================================
    // SPLIT INTO COMPLETE POINTS + PARTIAL POINT
    // ========================================================

    int completePoints =
        indicatorPercent / 20;


    int partialLevel =
        indicatorPercent % 20;


    // ========================================================
    // BLINK STATE
    //
    // Only battery levels BELOW 20% blink.
    //
    // 9%, 11%, 16%, 18% -> first point blinks.
    //
    // 20% and above -> no blinking.
    //
    // Blink interval = 500 ms.
    // ========================================================

    bool blinkCurrentPoint =
        (
            batteryPercent > 0 &&
            batteryPercent < 20
        );


    bool blinkOn =
        (
            (millis() / 500UL) % 2
        ) == 0;


    // ========================================================
    // DRAW COMPLETE POINTS
    //
    // Every complete point = 7 x 6.
    // Gap = 1 px.
    // ========================================================

    for (
        int i = 0;
        i < completePoints;
        i++
    )
    {
        int pointX =
            usableX +
            i *
            (
                pointWidth +
                pointGap
            );


        // ----------------------------------------------------
        // First point blinks when battery < 20%.
        //
        // Example 18%:
        //
        // blink ON  -> ███████
        // blink OFF -> empty
        // ----------------------------------------------------

        if (
            i == 0 &&
            blinkCurrentPoint &&
            !blinkOn
        )
        {
            continue;
        }


        display.fillRect(
            pointX,
            pointY,
            pointWidth,
            pointHeight,
            BLACK
        );
    }


    // ========================================================
    // PARTIAL NEXT POINT
    //
    // 5%:
    //
    // ███
    // ███
    //
    // 10%:
    //
    // ████
    // ████
    // ████
    // ████
    // ████
    // ████
    //
    // 15%:
    //
    // ████
    // ████
    // ████
    // ████
    // ███████
    // ███████
    //
    // 20% is handled as a complete point.
    // ========================================================

    if (
        partialLevel > 0 &&
        completePoints < 5
    )
    {
        int pointX =
            usableX +
            completePoints *
            (
                pointWidth +
                pointGap
            );


        // ====================================================
        // HIDE CURRENT PARTIAL POINT DURING BLINK OFF
        // ====================================================

        bool hidePartial =
            (
                completePoints == 0 &&
                blinkCurrentPoint &&
                !blinkOn
            );


        if (!hidePartial)
        {
            // =================================================
            // 5%
            //
            //    ███
            //    ███
            // =================================================

            if (partialLevel == 5)
            {
                display.fillRect(
                    pointX,
                    pointY + 4,
                    3,
                    2,
                    BLACK
                );
            }


            // =================================================
            // 10%
            //
            //    ████
            //    ████
            //    ████
            //    ████
            //    ████
            //    ████
            // =================================================

            else if (partialLevel == 10)
            {
                display.fillRect(
                    pointX,
                    pointY,
                    4,
                    6,
                    BLACK
                );
            }


            // =================================================
            // 15%
            //
            //    ████
            //    ████
            //    ████
            //    ████
            //    ███████
            //    ███████
            // =================================================

            else if (partialLevel == 15)
            {
                // Left 4 x 6
                display.fillRect(
                    pointX,
                    pointY,
                    4,
                    6,
                    BLACK
                );


                // Bottom-right 3 x 2
                display.fillRect(
                    pointX + 4,
                    pointY + 4,
                    3,
                    2,
                    BLACK
                );
            }
        }
    }
}


// ============================================================
// MOON VIEW
// ============================================================
void drawMoonView()
{
    // ========================================================
    // MOON PHASE
    // ========================================================

    drawMoon25x16(4, 30, testMoonTithi);


    // ========================================================
    // RIGHT SIDE PAKSHA / TITHI AREA
    // ========================================================

    // Moon area reduced.
    // New separator = x33
    display.drawLine(33, 29, 33, 47, BLACK);

    // Horizontal separator
    display.drawLine(34, 38, 83, 38, BLACK);


    // ========================================================
    // RIGHT BOX
    // ========================================================

    const int rightAreaX = 34;
    const int rightAreaWidth = 50;


    // ========================================================
    // PAKSHA
    // ========================================================

    const char* paksha =
        getPakshaName(testMoonTithi);

    // Calculate width using EXACTLY 1 pixel gap
    int pakshaWidth = 0;

    for (int i = 0; paksha[i] != '\0'; i++)
    {
        pakshaWidth += 5;   // character width

        if (paksha[i + 1] != '\0')
            pakshaWidth += 1;   // exactly 1 px gap
    }

    // Center inside 50 px box
    int pakshaX =
        rightAreaX +
        (rightAreaWidth - pakshaWidth) / 2;

    if (pakshaX < rightAreaX + 1)
        pakshaX = rightAreaX + 1;


    // Draw Paksha character by character
    int currentX = pakshaX;

    for (int i = 0; paksha[i] != '\0'; i++)
    {
        drawWatchChar(
            currentX,
            30,
            paksha[i],
            1
        );

        // Character width 5 px + exactly 1 px gap
        currentX += 6;
    }


    // ========================================================
    // TITHI NAME
    // ========================================================

    const char* tithiName =
        getPanchangTithiName(testMoonTithi);

    // Calculate width using EXACTLY 1 pixel gap
    int tithiWidth = 0;

    for (int i = 0; tithiName[i] != '\0'; i++)
    {
        tithiWidth += 5;   // character width

        if (tithiName[i + 1] != '\0')
            tithiWidth += 1;   // exactly 1 px gap
    }

    // Center inside 50 px box
    int tithiX =
        rightAreaX +
        (rightAreaWidth - tithiWidth) / 2;

    if (tithiX < rightAreaX + 1)
        tithiX = rightAreaX + 1;


    // Draw Tithi character by character
    int tithiCurrentX = tithiX;

    for (int i = 0; tithiName[i] != '\0'; i++)
    {
        drawWatchChar(
            tithiCurrentX,
            40,
            tithiName[i],
            1
        );

        // Character width 5 px + exactly 1 px gap
        tithiCurrentX += 6;
    }


    // ========================================================
    // NO BATTERY HERE
    // Bottom area intentionally remains blank.
    // ========================================================
}


// ============================================================
// HOME SCREEN
// ============================================================

void drawHomeScreen() 
{ unsigned long currentMillis = millis(); 
    // ======================================================== 
    // UPDATE PANCHANG 
    // ======================================================== 
    updatePanchangData(false); 
    // ======================================================== 
    // RTC / DISPLAY TIME 
    // ======================================================== 
    DateTime now = getHomeDisplayTime(); 
    // ======================================================== 
    // HEART / BPM SWITCH 
    // ======================================================== 
    if ( currentMillis - lastHeartSwitch >= HEART_SWITCH_INTERVAL ) 
    { lastHeartSwitch = currentMillis; showHeartIcon = !showHeartIcon; } 
    // ======================================================== 
    // UPDATE SCROLL 
    // ======================================================== 
    if (!moonView) 
    { if (backBlankView) { 
    // ==================================================== 
    // BACK VIEW // Location scrolls on Line 3 
    // ==================================================== 
    updateLocationScroll( currentMillis ); } 
    else if (!homeInfoBlank) 
    { if (!showTithi) { updateWeatherScroll( currentMillis ); } 
    else { updateTithiScroll(currentMillis); updateMasaRutuSeasonScroll( currentMillis ); } } } 
    // ======================================================== 
    // CLEAR 
    // ======================================================== 
    display.clearDisplay(); 
    display.setTextColor(BLACK); display.setTextWrap(false); 
    // ======================================================== 
    // LINE 1 
    // ======================================================== 
    drawFirstLine(); 
    // ======================================================== 
    // BORDER 1 
    // ======================================================== 
    display.drawLine( 0, 9, 83, 9, BLACK ); 
    // ======================================================== 
    // MAIN TIME 
    // ======================================================== 
    drawMainTime( now ); 
    // ======================================================== 
    // BORDER BELOW TIME 
    // ======================================================== 
    drawTimeBorder(); 
    // ======================================================== 
    // MOON VIEW 
    // ======================================================== 
    if (moonView) { drawMoonView(); display.display(); return; } 
    // ======================================================== 
    // BACK BLANK VIEW 
    // ======================================================== 
    if (backBlankView) { drawBackBlankBattery(); display.display(); return; } 
    // ======================================================== 
    // NORMAL HOME INFORMATION 
    // ======================================================== 
    if (!homeInfoBlank) { 
    // ---------------------------------------------------- 
    // LINE 3 
    // ---------------------------------------------------- 
    if (!showTithi) { drawDateLine(); } else { drawTithiLine(); } 
    // ---------------------------------------------------- 
    // BORDER ABOVE LINE 4 
    // ---------------------------------------------------- 
    display.drawLine( 0, 38, 83, 38, BLACK ); 
    // ---------------------------------------------------- 
    // LINE 4 
    // ---------------------------------------------------- 
    if (!showTithi) { drawWeatherLine(); } else { drawMasaRutuSeasonLine(); } } 
    // ======================================================== 
    // DISPLAY 
    //======================================================== 
display.display(); }