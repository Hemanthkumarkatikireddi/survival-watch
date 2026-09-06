#include "timezone.h"

#include "../dummy_data.h"

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
// DETECT TIMEZONE
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


            // =================================================
            // EUROPE
            // =================================================

            if (
                region.hasDST &&
                (
                    strcmp(
                        region.timezoneId,
                        "Europe/London"
                    ) == 0

                    ||

                    strcmp(
                        region.timezoneId,
                        "Europe/Berlin"
                    ) == 0

                    ||

                    strcmp(
                        region.timezoneId,
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


            // =================================================
            // SYDNEY
            // =================================================

            if (
                strcmp(
                    region.timezoneId,
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


            // =================================================
            // NEW ZEALAND
            // =================================================

            if (
                strcmp(
                    region.timezoneId,
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

            return result;
        }
    }

    return result;
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
// UTC -> INDIA
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
// GET LOCATION LOCAL TIME
// ============================================================
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
// GPS location timezone
//      ↓
// Local location time
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
        totalMinutes = 1439;
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