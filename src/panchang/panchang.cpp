#include "panchang.h"

#include "solar.h"
#include "lunar.h"

#include <math.h>

// ============================================================
// CONSTANTS
// ============================================================

static const double TITHI_SIZE = 12.0;

static const double NAKSHATRA_SIZE =
    13.333333333333333;


// ============================================================
// PANCHANG REFERENCE CORRECTIONS
// ============================================================
//
// IMPORTANT:
// These corrections are already verified against the
// Panchang reference timings used in this project.
//
// DO NOT change these values unless we intentionally
// recalibrate the astronomical model.
//
// Tithi      : -120 seconds
// Nakshatra  : -60 seconds
// ============================================================

static const int32_t TITHI_TIME_CORRECTION_SECONDS = -120;

static const int32_t NAKSHATRA_TIME_CORRECTION_SECONDS = -60;


// ============================================================
// NORMALIZE ANGLE
// ============================================================

static double normalizeAngle(double angle)
{
    while (angle < 0.0)
    {
        angle += 360.0;
    }

    while (angle >= 360.0)
    {
        angle -= 360.0;
    }

    return angle;
}


// ============================================================
// SUN-MOON ELONGATION
// ============================================================
//
// Tithi is based on:
//
//     Moon longitude - Sun longitude
//
// Normalized to:
//
//     0° ... <360°
//
// Every 12° represents one Tithi.
// ============================================================

static double lunarPhaseAngle(DateTime dt)
{
    double hourUTC =
        dt.hour()
        + dt.minute() / 60.0
        + dt.second() / 3600.0;

    double sunLongitude =
        calculateSunLongitude(
            dt.year(),
            dt.month(),
            dt.day(),
            hourUTC
        );

    double moonLongitude =
        calculateMoonLongitude(
            dt.year(),
            dt.month(),
            dt.day(),
            hourUTC
        );

    return normalizeAngle(
        moonLongitude - sunLongitude
    );
}


// ============================================================
// TITHI NUMBER FROM ELONGATION
// ============================================================

static uint8_t tithiFromAngle(double angle)
{
    int value =
        (int)floor(
            angle / TITHI_SIZE
        );

    return (uint8_t)(value + 1);
}


// ============================================================
// NAKSHATRA NUMBER FROM SIDEREAL LONGITUDE
// ============================================================

static uint8_t nakshatraFromLongitude(double longitude)
{
    int value =
        (int)floor(
            longitude / NAKSHATRA_SIZE
        );

    return (uint8_t)(value + 1);
}


// ============================================================
// GET TITHI AT A UTC DATETIME
// ============================================================

static uint8_t getTithiAt(DateTime dt)
{
    return tithiFromAngle(
        lunarPhaseAngle(dt)
    );
}


// ============================================================
// GET NAKSHATRA AT A UTC DATETIME
// ============================================================

static uint8_t getNakshatraAt(DateTime dt)
{
    double hourUTC =
        dt.hour()
        + dt.minute() / 60.0
        + dt.second() / 3600.0;

    double moonSidereal =
        calculateMoonSiderealLongitude(
            dt.year(),
            dt.month(),
            dt.day(),
            hourUTC
        );

    return nakshatraFromLongitude(
        moonSidereal
    );
}


// ============================================================
// FIND TITHI START
// ============================================================
//
// Search backward in one-hour steps until the previous
// Tithi is found.
//
// Then perform a binary search to locate the boundary
// to approximately one-second resolution.
// ============================================================

static DateTime findTithiStart(DateTime center)
{
    uint8_t currentTithi =
        getTithiAt(center);

    DateTime high = center;

    DateTime low =
        center - TimeSpan(3600);

    // --------------------------------------------------------
    // Search backward
    // --------------------------------------------------------

    for (int i = 0; i < 36; i++)
    {
        uint8_t previousTithi =
            getTithiAt(low);

        if (previousTithi != currentTithi)
        {
            break;
        }

        high = low;

        low =
            low - TimeSpan(3600);
    }

    // --------------------------------------------------------
    // Binary search
    // --------------------------------------------------------

    for (int i = 0; i < 30; i++)
    {
        int32_t difference =
            high.unixtime() -
            low.unixtime();

        if (difference <= 1)
        {
            break;
        }

        DateTime middle =
            low +
            TimeSpan(
                difference / 2
            );

        if (
            getTithiAt(middle) ==
            currentTithi
        )
        {
            high = middle;
        }
        else
        {
            low = middle;
        }
    }

    return high;
}


// ============================================================
// FIND TITHI END
// ============================================================

static DateTime findTithiEnd(DateTime center)
{
    uint8_t currentTithi =
        getTithiAt(center);

    DateTime low = center;

    DateTime high =
        center + TimeSpan(3600);

    // --------------------------------------------------------
    // Search forward
    // --------------------------------------------------------

    for (int i = 0; i < 36; i++)
    {
        uint8_t nextTithi =
            getTithiAt(high);

        if (nextTithi != currentTithi)
        {
            break;
        }

        low = high;

        high =
            high + TimeSpan(3600);
    }

    // --------------------------------------------------------
    // Binary search
    // --------------------------------------------------------

    for (int i = 0; i < 30; i++)
    {
        int32_t difference =
            high.unixtime() -
            low.unixtime();

        if (difference <= 1)
        {
            break;
        }

        DateTime middle =
            low +
            TimeSpan(
                difference / 2
            );

        if (
            getTithiAt(middle) ==
            currentTithi
        )
        {
            low = middle;
        }
        else
        {
            high = middle;
        }
    }

    return high;
}


// ============================================================
// CALCULATE TITHI
// ============================================================

TithiInfo calculateTithi(DateTime dateTime)
{
    TithiInfo result;

    // Current Tithi
    result.number =
        getTithiAt(dateTime);

    // --------------------------------------------------------
    // Current Tithi start
    // --------------------------------------------------------

    result.startUTC =
        findTithiStart(dateTime)
        +
        TimeSpan(
            TITHI_TIME_CORRECTION_SECONDS
        );

    // --------------------------------------------------------
    // Current Tithi end
    // --------------------------------------------------------

    result.endUTC =
        findTithiEnd(dateTime)
        +
        TimeSpan(
            TITHI_TIME_CORRECTION_SECONDS
        );

    // --------------------------------------------------------
    // Next Tithi
    // --------------------------------------------------------

    result.nextNumber =
        result.number + 1;

    if (result.nextNumber > 30)
    {
        result.nextNumber = 1;
    }

    // The next Tithi begins exactly when the current
    // Tithi ends.
    result.nextStartUTC =
        result.endUTC;

    return result;
}


// ============================================================
// FIND NAKSHATRA START
// ============================================================

static DateTime findNakshatraStart(DateTime center)
{
    uint8_t current =
        getNakshatraAt(center);

    DateTime high = center;

    DateTime low =
        center - TimeSpan(3600);

    // --------------------------------------------------------
    // Search backward
    // --------------------------------------------------------

    for (int i = 0; i < 36; i++)
    {
        uint8_t previous =
            getNakshatraAt(low);

        if (previous != current)
        {
            break;
        }

        high = low;

        low =
            low - TimeSpan(3600);
    }

    // --------------------------------------------------------
    // Binary search
    // --------------------------------------------------------

    for (int i = 0; i < 30; i++)
    {
        int32_t difference =
            high.unixtime() -
            low.unixtime();

        if (difference <= 1)
        {
            break;
        }

        DateTime middle =
            low +
            TimeSpan(
                difference / 2
            );

        if (
            getNakshatraAt(middle) ==
            current
        )
        {
            high = middle;
        }
        else
        {
            low = middle;
        }
    }

    return high;
}


// ============================================================
// FIND NAKSHATRA END
// ============================================================

static DateTime findNakshatraEnd(DateTime center)
{
    uint8_t current =
        getNakshatraAt(center);

    DateTime low = center;

    DateTime high =
        center + TimeSpan(3600);

    // --------------------------------------------------------
    // Search forward
    // --------------------------------------------------------

    for (int i = 0; i < 36; i++)
    {
        uint8_t next =
            getNakshatraAt(high);

        if (next != current)
        {
            break;
        }

        low = high;

        high =
            high + TimeSpan(3600);
    }

    // --------------------------------------------------------
    // Binary search
    // --------------------------------------------------------

    for (int i = 0; i < 30; i++)
    {
        int32_t difference =
            high.unixtime() -
            low.unixtime();

        if (difference <= 1)
        {
            break;
        }

        DateTime middle =
            low +
            TimeSpan(
                difference / 2
            );

        if (
            getNakshatraAt(middle) ==
            current
        )
        {
            low = middle;
        }
        else
        {
            high = middle;
        }
    }

    return high;
}


// ============================================================
// CALCULATE NAKSHATRA
// ============================================================

NakshatraInfo calculateNakshatra(DateTime dateTime)
{
    NakshatraInfo result;

    // Current Nakshatra
    result.number =
        getNakshatraAt(dateTime);

    // --------------------------------------------------------
    // Nakshatra start
    // --------------------------------------------------------

    result.startUTC =
        findNakshatraStart(dateTime)
        +
        TimeSpan(
            NAKSHATRA_TIME_CORRECTION_SECONDS
        );

    // --------------------------------------------------------
    // Nakshatra end
    // --------------------------------------------------------

    result.endUTC =
        findNakshatraEnd(dateTime)
        +
        TimeSpan(
            NAKSHATRA_TIME_CORRECTION_SECONDS
        );

    return result;
}


// ============================================================
// UTC DATE → LOCAL DATE
// ============================================================
//
// This helper is intentionally simple.
//
// timezone is the location's UTC offset in hours.
//
// Example:
//
// India:
//     UTC + 5.5
//
// Japan:
//     UTC + 9.0
//
// UK:
//     UTC + 0 / +1
//
// IMPORTANT:
// This is used ONLY to determine the civil calendar date
// for sunrise/sunset calculation.
//
// Astronomical Tithi/Nakshatra calculations remain in UTC.
// ============================================================

static void getLocalCalendarDate(
    DateTime utcDateTime,
    double timezone,
    int& year,
    int& month,
    int& day
)
{
    int32_t offsetSeconds =
        (int32_t)round(
            timezone * 3600.0
        );

    DateTime localDateTime =
        utcDateTime +
        TimeSpan(offsetSeconds);

    year =
        localDateTime.year();

    month =
        localDateTime.month();

    day =
        localDateTime.day();
}


// ============================================================
// COMPLETE PANCHANG
// ============================================================
//
// dateTime:
//     UTC instant.
//
// latitude:
//     GPS latitude.
//
// longitude:
//     GPS longitude.
//
// timezone:
//     Location timezone offset from UTC.
//
// IMPORTANT ARCHITECTURE:
//
//     UTC instant
//          ↓
//     location timezone
//          ↓
//     location local date
//          ↓
//     sunrise / sunset
//
// Tithi and Nakshatra boundary calculations remain UTC.
// ============================================================

PanchangData calculatePanchang(
    DateTime dateTime,
    double latitude,
    double longitude,
    double timezone
)
{
    PanchangData result;

    // --------------------------------------------------------
    // Tithi
    // --------------------------------------------------------

    result.tithi =
        calculateTithi(dateTime);

    // --------------------------------------------------------
    // Nakshatra
    // --------------------------------------------------------

    result.nakshatra =
        calculateNakshatra(dateTime);

    // --------------------------------------------------------
    // Determine LOCATION LOCAL DATE
    // --------------------------------------------------------

    int localYear;
    int localMonth;
    int localDay;

    getLocalCalendarDate(
        dateTime,
        timezone,
        localYear,
        localMonth,
        localDay
    );

    // --------------------------------------------------------
    // Sunrise / Sunset
    //
    // IMPORTANT:
    // Use the LOCATION'S local calendar date.
    // Do NOT use dateTime.year/month/day directly here.
    // --------------------------------------------------------

    SolarTimes solar =
        calculateSunriseSunset(
            localYear,
            localMonth,
            localDay,
            latitude,
            longitude,
            timezone
        );

    result.sunrise =
        solar.sunrise;

    result.sunset =
        solar.sunset;

    return result;
}


// ============================================================
// TITHI NAMES
// ============================================================

const char* getPanchangTithiName(uint8_t tithi)
{
    switch (tithi)
    {
        case 1:
        case 16:
            return "Pratipada";

        case 2:
        case 17:
            return "Dwitiya";

        case 3:
        case 18:
            return "Tritiya";

        case 4:
        case 19:
            return "Chaturthi";

        case 5:
        case 20:
            return "Panchami";

        case 6:
        case 21:
            return "Shashthi";

        case 7:
        case 22:
            return "Saptami";

        case 8:
        case 23:
            return "Ashtami";

        case 9:
        case 24:
            return "Navami";

        case 10:
        case 25:
            return "Dashami";

        case 11:
        case 26:
            return "Ekadashi";

        case 12:
        case 27:
            return "Dwadashi";

        case 13:
        case 28:
            return "Trayodashi";

        case 14:
        case 29:
            return "Chaturdashi";

        case 15:
            return "Purnima";

        case 30:
            return "Amavasya";

        default:
            return "Unknown";
    }
}


// ============================================================
// NAKSHATRA NAMES
// ============================================================

const char* getPanchangNakshatraName(uint8_t nakshatra)
{
    switch (nakshatra)
    {
        case 1:
            return "Ashwini";

        case 2:
            return "Bharani";

        case 3:
            return "Krittika";

        case 4:
            return "Rohini";

        case 5:
            return "Mrigashira";

        case 6:
            return "Ardra";

        case 7:
            return "Punarvasu";

        case 8:
            return "Pushya";

        case 9:
            return "Ashlesha";

        case 10:
            return "Magha";

        case 11:
            return "Purva Phalguni";

        case 12:
            return "Uttara Phalguni";

        case 13:
            return "Hasta";

        case 14:
            return "Chitra";

        case 15:
            return "Swati";

        case 16:
            return "Vishakha";

        case 17:
            return "Anuradha";

        case 18:
            return "Jyeshtha";

        case 19:
            return "Mula";

        case 20:
            return "Purva Ashadha";

        case 21:
            return "Uttara Ashadha";

        case 22:
            return "Shravana";

        case 23:
            return "Dhanishtha";

        case 24:
            return "Shatabhisha";

        case 25:
            return "Purva Bhadrapada";

        case 26:
            return "Uttara Bhadrapada";

        case 27:
            return "Revati";

        default:
            return "Unknown";
    }
}