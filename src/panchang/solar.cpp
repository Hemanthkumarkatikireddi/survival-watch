#include "solar.h"

#include <math.h>


// ============================================================
// INTERNAL HELPERS
// ============================================================


// ============================================================
// NORMALIZE ANGLE
// ============================================================
//
// Keeps an angle inside:
//
//   0° <= angle < 360°
//
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
// JULIAN DAY
// ============================================================
//
// Gregorian calendar Julian Day.
//
// hourUTC:
//     Decimal UTC hours.
//
// Example:
//
//     12:00 UTC = 12.0
//     18:30 UTC = 18.5
//
// ============================================================

static double julianDay(
    int year,
    int month,
    int day,
    double hourUTC
)
{
    if (month <= 2)
    {
        year--;
        month += 12;
    }

    int A =
        year / 100;

    int B =
        2
        - A
        + (A / 4);

    return
        floor(
            365.25 *
            (year + 4716)
        )
        +
        floor(
            30.6001 *
            (month + 1)
        )
        +
        day
        +
        B
        -
        1524.5
        +
        hourUTC / 24.0;
}


// ============================================================
// SUN LONGITUDE
// ============================================================
//
// Approximate geocentric apparent solar longitude.
//
// This is the existing solar model used by the project's
// Tithi calculation.
//
// DO NOT replace this with a different model casually,
// because changing this function changes:
//
//     Sun longitude
//          ↓
//     Moon - Sun elongation
//          ↓
//     Tithi boundary
//
// ============================================================

double calculateSunLongitude(
    int year,
    int month,
    int day,
    double hourUTC
)
{
    double JD =
        julianDay(
            year,
            month,
            day,
            hourUTC
        );

    double T =
        (JD - 2451545.0)
        / 36525.0;


    // --------------------------------------------------------
    // Mean longitude
    // --------------------------------------------------------

    double L0 =
        normalizeAngle(
            280.46646
            +
            36000.76983 * T
            +
            0.0003032 * T * T
        );


    // --------------------------------------------------------
    // Mean anomaly
    // --------------------------------------------------------

    double M =
        normalizeAngle(
            357.52911
            +
            35999.05029 * T
            -
            0.0001537 * T * T
        );


    double Mrad =
        M * DEG_TO_RAD;


    // --------------------------------------------------------
    // Equation of center
    // --------------------------------------------------------

    double C =
        (
            1.914602
            -
            0.004817 * T
            -
            0.000014 * T * T
        )
        *
        sin(Mrad);


    C +=
        (
            0.019993
            -
            0.000101 * T
        )
        *
        sin(2.0 * Mrad);


    C +=
        0.000289 *
        sin(3.0 * Mrad);


    // --------------------------------------------------------
    // True solar longitude
    // --------------------------------------------------------

    double trueLongitude =
        L0 + C;


    return normalizeAngle(
        trueLongitude
    );
}


// ============================================================
// SUNRISE / SUNSET
// ============================================================
//
// Algorithm:
//
//     1. Calculate day of year
//     2. Calculate fractional year
//     3. Calculate solar declination
//     4. Calculate equation of time
//     5. Calculate sunrise/sunset hour angle
//     6. Calculate local solar noon
//     7. Calculate sunrise
//     8. Calculate sunset
//
// Input date is the LOCATION'S LOCAL CIVIL DATE.
//
// Returned times are LOCAL decimal hours.
//
// ============================================================

SolarTimes calculateSunriseSunset(
    int year,
    int month,
    int day,
    double latitude,
    double longitude,
    double timezone
)
{
    SolarTimes result;

    // --------------------------------------------------------
    // Default invalid result
    // --------------------------------------------------------
    //
    // Used when the Sun does not rise/set on this date,
    // such as polar day/night.
    //

    result.sunrise = -1.0;
    result.sunset = -1.0;


    // ========================================================
    // DAY OF YEAR
    // ========================================================

    static const int daysBeforeMonth[] =
    {
        0,      // January
        31,     // February
        59,     // March
        90,     // April
        120,    // May
        151,    // June
        181,    // July
        212,    // August
        243,    // September
        273,    // October
        304,    // November
        334     // December
    };


    // --------------------------------------------------------
    // Basic input validation
    // --------------------------------------------------------

    if (
        month < 1 ||
        month > 12 ||
        day < 1 ||
        day > 31
    )
    {
        return result;
    }


    int dayOfYear =
        daysBeforeMonth[month - 1]
        +
        day;


    // --------------------------------------------------------
    // Leap year
    // --------------------------------------------------------

    bool leap =
        (
            (year % 4 == 0 && year % 100 != 0)
            ||
            (year % 400 == 0)
        );


    if (
        leap &&
        month > 2
    )
    {
        dayOfYear++;
    }


    // ========================================================
    // FRACTIONAL YEAR
    // ========================================================

    double gamma =
        2.0 *
        M_PI /
        365.0 *
        (dayOfYear - 1);


    // ========================================================
    // SOLAR DECLINATION
    // ========================================================

    double declination =
        0.006918
        -
        0.399912 * cos(gamma)
        +
        0.070257 * sin(gamma)
        -
        0.006758 * cos(2.0 * gamma)
        +
        0.000907 * sin(2.0 * gamma)
        -
        0.002697 * cos(3.0 * gamma)
        +
        0.00148 * sin(3.0 * gamma);


    // ========================================================
    // EQUATION OF TIME
    // ========================================================

    double eqTime =
        229.18 *
        (
            0.000075
            +
            0.001868 * cos(gamma)
            -
            0.032077 * sin(gamma)
            -
            0.014615 * cos(2.0 * gamma)
            -
            0.040849 * sin(2.0 * gamma)
        );


    // ========================================================
    // LATITUDE
    // ========================================================

    double latRad =
        latitude * DEG_TO_RAD;


    // ========================================================
    // SUNRISE / SUNSET ZENITH
    // ========================================================
    //
    // 90.833° includes the standard atmospheric refraction
    // correction used for sunrise/sunset.
    //
    // ========================================================

    double zenith =
        90.833 * DEG_TO_RAD;


    // ========================================================
    // HOUR ANGLE
    // ========================================================

    double cosHourAngle =
        (
            cos(zenith)
            /
            (
                cos(latRad)
                *
                cos(declination)
            )
        )
        -
        (
            tan(latRad)
            *
            tan(declination)
        );


    // ========================================================
    // POLAR DAY / POLAR NIGHT
    // ========================================================
    //
    // acos() is not possible when the value is outside
    // [-1, +1].
    //
    // Therefore return -1.0 for both sunrise and sunset.
    //
    // ========================================================

    if (
        cosHourAngle > 1.0 ||
        cosHourAngle < -1.0
    )
    {
        return result;
    }


    // ========================================================
    // HOUR ANGLE IN DEGREES
    // ========================================================

    double hourAngle =
        acos(cosHourAngle)
        /
        DEG_TO_RAD;


    // ========================================================
    // SOLAR NOON
    // ========================================================
    //
    // Result is converted to minutes first.
    //
    // timezone is hours from UTC.
    //
    // Example:
    //
    //     India = +5.5
    //
    //     +5.5 * 60 = +330 minutes
    //
    // ========================================================

    double solarNoon =
        (
            720.0
            -
            4.0 * longitude
            -
            eqTime
            +
            timezone * 60.0
        )
        /
        1440.0;


    double solarNoonMinutes =
        solarNoon * 1440.0;


    // ========================================================
    // SUNRISE
    // ========================================================

    double sunriseMinutes =
        solarNoonMinutes
        -
        hourAngle * 4.0;


    // ========================================================
    // SUNSET
    // ========================================================

    double sunsetMinutes =
        solarNoonMinutes
        +
        hourAngle * 4.0;


    // ========================================================
    // CONVERT MINUTES → DECIMAL HOURS
    // ========================================================

    result.sunrise =
        sunriseMinutes / 60.0;

    result.sunset =
        sunsetMinutes / 60.0;


    // ========================================================
    // NORMALIZE RESULT
    // ========================================================
    //
    // In unusual longitude/timezone combinations a value
    // can cross midnight.
    //
    // Keep returned values inside 0...24 hours.
    //
    // ========================================================

    while (result.sunrise < 0.0)
    {
        result.sunrise += 24.0;
    }

    while (result.sunrise >= 24.0)
    {
        result.sunrise -= 24.0;
    }


    while (result.sunset < 0.0)
    {
        result.sunset += 24.0;
    }

    while (result.sunset >= 24.0)
    {
        result.sunset -= 24.0;
    }


    return result;
}