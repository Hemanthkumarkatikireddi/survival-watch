#pragma once

#include <Arduino.h>

// ============================================================
// SOLAR CALCULATION
// ============================================================
//
// All solar calculations in this module use:
//
//   Latitude  = degrees
//   Longitude = degrees
//   Timezone  = hours from UTC
//
// Example:
//
//   India  = +5.5
//   Japan  = +9.0
//   UTC    =  0.0
//
// ============================================================


// ============================================================
// SUNRISE / SUNSET RESULT
// ============================================================
//
// Values are returned as decimal LOCAL hours.
//
// Examples:
//
//   5.50  = 05:30 AM
//   6.25  = 06:15 AM
//   18.50 = 06:30 PM
//
// If sunrise/sunset does not occur on that date
// (polar day / polar night), the value is:
//
//   -1.0
//
// ============================================================

struct SolarTimes
{
    double sunrise;
    double sunset;
};


// ============================================================
// SUN ECLIPTIC LONGITUDE
// ============================================================
//
// Calculates the Sun's approximate geocentric apparent
// ecliptic longitude.
//
// IMPORTANT:
//
// hourUTC MUST be UTC decimal hours.
//
// Example:
//
//   16:30:00 UTC
//
// becomes:
//
//   16.5
//
// Returned value:
//
//   0° ... <360°
//
// This function is also used by the Tithi calculation.
// Therefore its existing astronomical behavior must remain
// stable.
// ============================================================

double calculateSunLongitude(
    int year,
    int month,
    int day,
    double hourUTC
);


// ============================================================
// SUNRISE / SUNSET
// ============================================================
//
// Calculates sunrise and sunset for a specific LOCAL
// calendar date and geographic location.
//
// Parameters:
//
//   year
//       Local calendar year.
//
//   month
//       Local calendar month.
//
//   day
//       Local calendar day.
//
//   latitude
//       Geographic latitude in degrees.
//
//   longitude
//       Geographic longitude in degrees.
//
//   timezone
//       Local UTC offset in decimal hours.
//
// Examples:
//
//   India  = +5.5
//   Japan  = +9.0
//   UK     =  0.0
//
// IMPORTANT:
//
// The date passed here must be the LOCATION'S LOCAL DATE.
//
// Example:
//
//   UTC date/time:
//       2026-09-06 18:30 UTC
//
//   Japan local time:
//       2026-09-07 03:30 JST
//
// Then this function must receive:
//
//   year  = 2026
//   month = 9
//   day   = 7
//   timezone = +9.0
//
// The function returns decimal LOCAL hours.
//
// ============================================================

SolarTimes calculateSunriseSunset(
    int year,
    int month,
    int day,
    double latitude,
    double longitude,
    double timezone
);