#include "timezone.h"

#include "../dummy_data.h"

#include <Preferences.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

// ============================================================
// GPS LOCATION
// ============================================================
//
// Temporary GPS source comes from dummy_data.h.
//
// Later:
//     dummy_data.h
//          ↓
//     real GPS module
//          ↓
//     setGPSLocation(latitude, longitude)
//
// All other files can continue using:
//     getGPSLatitude()
//     getGPSLongitude()
//
// ============================================================

static double currentLatitude =
    DummyData::GPS_LATITUDE;

static double currentLongitude =
    DummyData::GPS_LONGITUDE;

// ============================================================
// TIMEZONE SETTINGS
// ============================================================

static Preferences timezonePreferences;

static bool timezoneSettingsLoaded =
    false;

// ============================================================
// DEFAULT TIMEZONE SETTINGS
// ============================================================
//
// Default:
//     Manual mode
//     India
//
// timezoneRegions[0] is India in the current
// timezone_data.cpp.
//

static TimezoneMode activeTimezoneMode =
    TIMEZONE_MANUAL;

static int manualTimezoneIndex =
    0;

// ============================================================
// TIMEZONE CHANGE REQUEST
// ============================================================
//
// This becomes TRUE whenever the user changes:
//     - Manual timezone
//     - Manual / Current Location mode
//
// Panchang can consume this request and recalculate
// immediately in the background.
//

static volatile bool timezoneChangeRequested =
    false;

// ============================================================
// SET GPS LOCATION
// ============================================================

void setGPSLocation(
    double latitude,
    double longitude
)
{
    if (
        latitude < -90.0 ||
        latitude > 90.0
    )
    {
        return;
    }

    if (
        longitude < -180.0 ||
        longitude > 180.0
    )
    {
        return;
    }

    currentLatitude =
        latitude;

    currentLongitude =
        longitude;
}

// ============================================================
// GET GPS LATITUDE
// ============================================================

double getGPSLatitude()
{
    return currentLatitude;
}

// ============================================================
// GET GPS LONGITUDE
// ============================================================

double getGPSLongitude()
{
    return currentLongitude;
}

// ============================================================
// DEFAULT TIMEZONE
// ============================================================
//
// India = UTC+5:30
//
// IMPORTANT:
// This is NOT UTC.
//
// ============================================================

static TimezoneInfo defaultTimezone()
{
    TimezoneInfo result;

    result.countryCode =
        "IN";

    result.countryName =
        "India";

    result.timezoneId =
        "Asia/Kolkata";

    result.utcOffsetMinutes =
        330;

    result.daylightSaving =
        false;

    return result;
}

// ============================================================
// LEAP YEAR
// ============================================================

static bool isLeapYear(
    int year
)
{
    return
        (
            (year % 4 == 0 &&
             year % 100 != 0)
            ||
            (year % 400 == 0)
        );
}

// ============================================================
// LAST SUNDAY
// ============================================================

static int lastSunday(
    int year,
    int month
)
{
    static const int daysInMonth[] =
    {
        0,
        31,
        28,
        31,
        30,
        31,
        30,
        31,
        31,
        30,
        31,
        30,
        31
    };

    int days =
        daysInMonth[month];

    if (
        month == 2 &&
        isLeapYear(year)
    )
    {
        days = 29;
    }

    DateTime date(
        year,
        month,
        days,
        0,
        0,
        0
    );

    return
        days -
        date.dayOfTheWeek();
}

// ============================================================
// EUROPE DST
// ============================================================

static bool isEuropeanDST(
    DateTime utc
)
{
    int year =
        utc.year();

    int marchSunday =
        lastSunday(
            year,
            3
        );

    int octoberSunday =
        lastSunday(
            year,
            10
        );

    DateTime start(
        year,
        3,
        marchSunday,
        1,
        0,
        0
    );

    DateTime end(
        year,
        10,
        octoberSunday,
        1,
        0,
        0
    );

    return
        utc >= start &&
        utc < end;
}

// ============================================================
// SYDNEY DST
// ============================================================

static bool isSydneyDST(
    DateTime utc
)
{
    int year =
        utc.year();

    DateTime octoberFirst(
        year,
        10,
        1,
        0,
        0,
        0
    );

    int startDay =
        1 +
        (
            7 -
            octoberFirst.dayOfTheWeek()
        ) % 7;

    DateTime aprilFirst(
        year,
        4,
        1,
        0,
        0,
        0
    );

    int endDay =
        1 +
        (
            7 -
            aprilFirst.dayOfTheWeek()
        ) % 7;

    DateTime start(
        year,
        10,
        startDay,
        16,
        0,
        0
    );

    DateTime end(
        year,
        4,
        endDay,
        16,
        0,
        0
    );

    if (utc >= start)
    {
        return true;
    }

    if (utc < end)
    {
        return true;
    }

    return false;
}

// ============================================================
// NEW ZEALAND DST
// ============================================================

static bool isNewZealandDST(
    DateTime utc
)
{
    int year =
        utc.year();

    int septemberSunday =
        lastSunday(
            year,
            9
        );

    DateTime aprilFirst(
        year,
        4,
        1,
        0,
        0,
        0
    );

    int aprilSunday =
        1 +
        (
            7 -
            aprilFirst.dayOfTheWeek()
        ) % 7;

    DateTime start(
        year,
        9,
        septemberSunday,
        12,
        0,
        0
    );

    DateTime end(
        year,
        4,
        aprilSunday,
        12,
        0,
        0
    );

    if (utc >= start)
    {
        return true;
    }

    if (utc < end)
    {
        return true;
    }

    return false;
}

// ============================================================
// APPLY DST
// ============================================================
//
// This helper applies the same DST rules already used by
// getCurrentTimezone().
//
// ============================================================

static void applyDST(
    TimezoneInfo& result,
    DateTime utcDateTime
)
{
    // ========================================================
    // EUROPE
    // ========================================================

    if (
        (
            strcmp(
                result.timezoneId,
                "Europe/London"
            ) == 0

            ||

            strcmp(
                result.timezoneId,
                "Europe/Berlin"
            ) == 0

            ||

            strcmp(
                result.timezoneId,
                "Europe/Athens"
            ) == 0
        )
    )
    {
        if (
            isEuropeanDST(
                utcDateTime
            )
        )
        {
            result.utcOffsetMinutes +=
                60;

            result.daylightSaving =
                true;
        }
    }

    // ========================================================
    // SYDNEY
    // ========================================================

    if (
        strcmp(
            result.timezoneId,
            "Australia/Sydney"
        ) == 0
    )
    {
        if (
            isSydneyDST(
                utcDateTime
            )
        )
        {
            result.utcOffsetMinutes +=
                60;

            result.daylightSaving =
                true;
        }
    }

    // ========================================================
    // NEW ZEALAND
    // ========================================================

    if (
        strcmp(
            result.timezoneId,
            "Pacific/Auckland"
        ) == 0
    )
    {
        if (
            isNewZealandDST(
                utcDateTime
            )
        )
        {
            result.utcOffsetMinutes +=
                60;

            result.daylightSaving =
                true;
        }
    }
}

// ============================================================
// TIMEZONE FROM REGION
// ============================================================
//
// Converts one timezoneRegions[] entry into TimezoneInfo.
//
// This is used by:
//     getCurrentTimezone()
//     getActiveTimezone()
//
// ============================================================

static TimezoneInfo timezoneFromRegion(
    const TimezoneRegion& region,
    DateTime utcDateTime
)
{
    TimezoneInfo result;

    result.countryCode =
        region.countryCode;

    result.countryName =
        region.countryName;

    result.timezoneId =
        region.timezoneId;

    result.utcOffsetMinutes =
        region.standardOffsetMinutes;

    result.daylightSaving =
        false;

    if (region.hasDST)
    {
        applyDST(
            result,
            utcDateTime
        );
    }

    return result;
}

// ============================================================
// DETECT TIMEZONE FROM GPS LOCATION
// ============================================================
//
// IMPORTANT:
// This function ALWAYS uses GPS location.
//
// It does NOT care whether the user selected:
//     MANUAL
// or
//     CURRENT_LOCATION
//
// ============================================================

TimezoneInfo getCurrentTimezone(
    DateTime utcDateTime
)
{
    TimezoneInfo result =
        defaultTimezone();

    for (
        int i = 0;
        i < timezoneRegionCount;
        i++
    )
    {
        const TimezoneRegion& region =
            timezoneRegions[i];

        if (
            currentLatitude >= region.minLat &&
            currentLatitude <= region.maxLat &&
            currentLongitude >= region.minLon &&
            currentLongitude <= region.maxLon
        )
        {
            return
                timezoneFromRegion(
                    region,
                    utcDateTime
                );
        }
    }

    // ========================================================
    // No matching region
    //
    // Keep India as safe default.
    // ========================================================

    return result;
}

// ============================================================
// INITIALIZE TIMEZONE SETTINGS
// ============================================================
//
// Preferences namespace:
//     "timezone"
//
// Keys:
//     mode
//     zone
//
// Default:
//     mode = MANUAL
//     zone = India (index 0)
//
// ============================================================

void initializeTimezoneSettings()
{
    if (timezoneSettingsLoaded)
    {
        return;
    }

    timezonePreferences.begin(
        "timezone",
        false
    );

    int savedMode =
        timezonePreferences.getUChar(
            "mode",
            (uint8_t)TIMEZONE_MANUAL
        );

    int savedZone =
        timezonePreferences.getUChar(
            "zone",
            0
        );

    // ========================================================
    // Validate mode
    // ========================================================

    if (
        savedMode != TIMEZONE_MANUAL &&
        savedMode != TIMEZONE_CURRENT_LOCATION
    )
    {
        savedMode =
            TIMEZONE_MANUAL;
    }

    // ========================================================
    // Validate timezone index
    // ========================================================

    if (
        savedZone < 0 ||
        savedZone >= timezoneRegionCount
    )
    {
        savedZone =
            0;
    }

    activeTimezoneMode =
        (TimezoneMode)savedMode;

    manualTimezoneIndex =
        savedZone;

    timezoneSettingsLoaded =
        true;
}

// ============================================================
// ENSURE SETTINGS LOADED
// ============================================================

static void ensureTimezoneSettingsLoaded()
{
    if (!timezoneSettingsLoaded)
    {
        initializeTimezoneSettings();
    }
}

// ============================================================
// GET TIMEZONE MODE
// ============================================================

TimezoneMode getTimezoneMode()
{
    ensureTimezoneSettingsLoaded();

    return activeTimezoneMode;
}

// ============================================================
// SET TIMEZONE MODE
// ============================================================

void setTimezoneMode(
    TimezoneMode mode
)
{
    ensureTimezoneSettingsLoaded();

    if (
        mode != TIMEZONE_MANUAL &&
        mode != TIMEZONE_CURRENT_LOCATION
    )
    {
        return;
    }

    // No change
    if (
        activeTimezoneMode ==
        mode
    )
    {
        return;
    }

    activeTimezoneMode =
        mode;

    timezonePreferences.putUChar(
        "mode",
        (uint8_t)activeTimezoneMode
    );

    // ========================================================
    // Tell Panchang that timezone changed.
    // ========================================================

    timezoneChangeRequested =
        true;
}

// ============================================================
// GET MANUAL TIMEZONE INDEX
// ============================================================

int getManualTimezoneIndex()
{
    ensureTimezoneSettingsLoaded();

    return manualTimezoneIndex;
}

// ============================================================
// SET MANUAL TIMEZONE INDEX
// ============================================================

bool setManualTimezoneIndex(
    int index
)
{
    ensureTimezoneSettingsLoaded();

    if (
        index < 0 ||
        index >= timezoneRegionCount
    )
    {
        return false;
    }

    // ========================================================
    // Save even if already selected.
    // This keeps the function deterministic.
    // ========================================================

    manualTimezoneIndex =
        index;

    timezonePreferences.putUChar(
        "zone",
        (uint8_t)manualTimezoneIndex
    );

    // ========================================================
    // Manual timezone selection should activate MANUAL mode.
    //
    // Example:
    // Current Location → Japan selected
    //
    // Result:
    // Manual mode + Japan
    // ========================================================

    if (
        activeTimezoneMode !=
        TIMEZONE_MANUAL
    )
    {
        activeTimezoneMode =
            TIMEZONE_MANUAL;

        timezonePreferences.putUChar(
            "mode",
            (uint8_t)TIMEZONE_MANUAL
        );
    }

    // ========================================================
    // Request immediate Panchang recalculation.
    // ========================================================

    timezoneChangeRequested =
        true;

    return true;
}

// ============================================================
// CONSUME TIMEZONE CHANGE REQUEST
// ============================================================
//
// Returns TRUE once when a timezone change has been requested.
//
// Example:
//
// if (consumeTimezoneChangeRequest())
// {
//     // Start Panchang background recalculation
// }
//
// ============================================================

bool consumeTimezoneChangeRequest()
{
    if (
        timezoneChangeRequested
    )
    {
        timezoneChangeRequested =
            false;

        return true;
    }

    return false;
}

// ============================================================
// GET ACTIVE TIMEZONE
// ============================================================
//
// MANUAL:
//     selected timezone.
//
// CURRENT LOCATION:
//     GPS detected timezone.
//
// ============================================================

TimezoneInfo getActiveTimezone(
    DateTime utcDateTime
)
{
    ensureTimezoneSettingsLoaded();

    // ========================================================
    // CURRENT LOCATION MODE
    // ========================================================

    if (
        activeTimezoneMode ==
        TIMEZONE_CURRENT_LOCATION
    )
    {
        return
            getCurrentTimezone(
                utcDateTime
            );
    }

    // ========================================================
    // MANUAL MODE
    // ========================================================

    if (
        manualTimezoneIndex < 0 ||
        manualTimezoneIndex >= timezoneRegionCount
    )
    {
        manualTimezoneIndex =
            0;
    }

    return
        timezoneFromRegion(
            timezoneRegions[
                manualTimezoneIndex
            ],
            utcDateTime
        );
}

// ============================================================
// UTC -> LOCAL
// ============================================================

static DateTime convertUTCToLocal(
    DateTime utcDateTime,
    int utcOffsetMinutes
)
{
    int hours =
        utcOffsetMinutes / 60;

    int minutes =
        utcOffsetMinutes % 60;

    return
        utcDateTime +
        TimeSpan(
            0,
            hours,
            minutes,
            0
        );
}

// ============================================================
// UTC -> ACTIVE LOCAL TIME
// ============================================================
//
// This is the important new function.
//
// HOME CLOCK should use this.
//
// PANCHANG local civil time should use this.
//
// GPS coordinates remain independent.
//
// ============================================================

DateTime getActiveLocalTime(
    DateTime utcDateTime
)
{
    TimezoneInfo timezone =
        getActiveTimezone(
            utcDateTime
        );

    return
        convertUTCToLocal(
            utcDateTime,
            timezone.utcOffsetMinutes
        );
}

// ============================================================
// UTC -> INDIA
// ============================================================
//
// India = UTC+5:30
//
// ============================================================

DateTime convertUTCToIndiaTime(
    DateTime utcDateTime
)
{
    return
        utcDateTime +
        TimeSpan(
            0,
            5,
            30,
            0
        );
}

// ============================================================
// INDIA -> UTC
// ============================================================
//
// India = UTC+5:30
//
// ============================================================

DateTime convertIndiaTimeToUTC(
    DateTime indiaDateTime
)
{
    return
        indiaDateTime -
        TimeSpan(
            0,
            5,
            30,
            0
        );
}

// ============================================================
// GET GPS LOCATION LOCAL TIME
// ============================================================
//
// IMPORTANT:
//
// This function intentionally remains GPS based.
//
// Flow:
//
// RTC India time
//      ↓
// convertIndiaTimeToUTC()
//      ↓
// UTC
//      ↓
// getCurrentTimezone()
//      ↓
// GPS timezone
//      ↓
// Local GPS location time
//
// For HOME / PANCHANG active timezone use:
//     getActiveLocalTime()
//
// ============================================================

DateTime getLocationLocalTime(
    DateTime utcDateTime
)
{
    TimezoneInfo timezone =
        getCurrentTimezone(
            utcDateTime
        );

    return
        convertUTCToLocal(
            utcDateTime,
            timezone.utcOffsetMinutes
        );
}

// ============================================================
// FORMAT 12-HOUR DATETIME
// ============================================================

void format12HourTime(
    DateTime dt,
    char* output,
    size_t outputSize
)
{
    int hour =
        dt.hour();

    bool pm =
        hour >= 12;

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
// FORMAT SUNRISE / SUNSET
// ============================================================

void formatSolarTime(
    double decimalHour,
    char* output,
    size_t outputSize
)
{
    if (
        decimalHour < 0.0 ||
        decimalHour >= 24.0
    )
    {
        snprintf(
            output,
            outputSize,
            "--:--"
        );

        return;
    }

    int totalMinutes =
        (int)round(
            decimalHour * 60.0
        );

    if (totalMinutes >= 1440)
    {
        totalMinutes =
            1439;
    }

    int hour =
        totalMinutes / 60;

    int minute =
        totalMinutes % 60;

    bool pm =
        hour >= 12;

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
        minute,
        pm ? "pm" : "am"
    );
}