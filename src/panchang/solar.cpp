#include "solar.h"
#include <math.h>

// ============================================================
// CONSTANTS
// ============================================================



// ============================================================
// NORMALIZE ANGLE
// ============================================================

static double normalizeAngle(double angle)
{
    while (angle < 0.0)
        angle += 360.0;

    while (angle >= 360.0)
        angle -= 360.0;

    return angle;
}


// ============================================================
// JULIAN DAY
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

    int A = year / 100;

    int B =
        2 -
        A +
        (A / 4);

    return
        floor(365.25 * (year + 4716)) +
        floor(30.6001 * (month + 1)) +
        day +
        B -
        1524.5 +
        hourUTC / 24.0;
}


// ============================================================
// SUN LONGITUDE
// ============================================================
//
// Approximate geocentric apparent solar longitude.
// This is the first version; later we will improve precision
// when validating against Panchang data.
//

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
        (JD - 2451545.0) / 36525.0;


    // Mean longitude
    double L0 =
        normalizeAngle(
            280.46646 +
            36000.76983 * T +
            0.0003032 * T * T
        );


    // Mean anomaly
    double M =
        normalizeAngle(
            357.52911 +
            35999.05029 * T -
            0.0001537 * T * T
        );


    double Mrad =
        M * DEG_TO_RAD;


    // Equation of center
    double C =
        (1.914602 -
         0.004817 * T -
         0.000014 * T * T)
        * sin(Mrad);

    C +=
        (0.019993 -
         0.000101 * T)
        * sin(2.0 * Mrad);

    C +=
        0.000289 *
        sin(3.0 * Mrad);


    double trueLongitude =
        L0 + C;


    return normalizeAngle(
        trueLongitude
    );
}


// ============================================================
// SUNRISE / SUNSET
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

    result.sunrise = -1.0;
    result.sunset = -1.0;


    // --------------------------------------------------------
    // Day of year
    // --------------------------------------------------------

    static const int daysBeforeMonth[] =
    {
        0,
        31,
        59,
        90,
        120,
        151,
        181,
        212,
        243,
        273,
        304,
        334
    };


    int dayOfYear =
        daysBeforeMonth[month - 1] +
        day;


    bool leap =
        ((year % 4 == 0 && year % 100 != 0) ||
         (year % 400 == 0));


    if (leap && month > 2)
        dayOfYear++;


    // --------------------------------------------------------
    // Fractional year
    // --------------------------------------------------------

    double gamma =
        2.0 * M_PI / 365.0 *
        (dayOfYear - 1);


    // --------------------------------------------------------
    // Solar declination
    // --------------------------------------------------------

    double declination =
        0.006918
        - 0.399912 * cos(gamma)
        + 0.070257 * sin(gamma)
        - 0.006758 * cos(2.0 * gamma)
        + 0.000907 * sin(2.0 * gamma)
        - 0.002697 * cos(3.0 * gamma)
        + 0.00148 * sin(3.0 * gamma);


    // --------------------------------------------------------
    // Equation of time
    // --------------------------------------------------------

    double eqTime =
        229.18 *
        (
            0.000075
            + 0.001868 * cos(gamma)
            - 0.032077 * sin(gamma)
            - 0.014615 * cos(2.0 * gamma)
            - 0.040849 * sin(2.0 * gamma)
        );


    double latRad =
        latitude * DEG_TO_RAD;


    // --------------------------------------------------------
    // Sunrise/sunset solar zenith
    //
    // Standard atmospheric refraction correction:
    // 90.833 degrees
    // --------------------------------------------------------

    double zenith =
        90.833 * DEG_TO_RAD;


    double cosHourAngle =
        (
            cos(zenith) /
            (cos(latRad) * cos(declination))
        )
        -
        tan(latRad) *
        tan(declination);


    // Polar day / polar night
    if (cosHourAngle > 1.0 ||
        cosHourAngle < -1.0)
    {
        return result;
    }


    double hourAngle =
        acos(cosHourAngle) /
        DEG_TO_RAD;


    // --------------------------------------------------------
    // Solar noon
    // --------------------------------------------------------

    double solarNoon =
        (
            720.0
            - 4.0 * longitude
            - eqTime
            + timezone * 60.0
        ) / 1440.0;


    // --------------------------------------------------------
    // Sunrise
    // --------------------------------------------------------

    double sunriseMinutes =
        (
            solarNoon * 1440.0
        )
        -
        hourAngle * 4.0;


    // --------------------------------------------------------
    // Sunset
    // --------------------------------------------------------

    double sunsetMinutes =
        (
            solarNoon * 1440.0
        )
        +
        hourAngle * 4.0;


    result.sunrise =
        sunriseMinutes / 60.0;

    result.sunset =
        sunsetMinutes / 60.0;


    return result;
}