#pragma once

#include <Arduino.h>

// ============================================================
// SOLAR CALCULATION
// ============================================================

// Sunrise and sunset result
struct SolarTimes
{
    double sunrise;
    double sunset;
};

// Calculate Sun's approximate ecliptic longitude
double calculateSunLongitude(
    int year,
    int month,
    int day,
    double hourUTC
);

// Calculate sunrise and sunset
//
// latitude  = degrees
// longitude = degrees
// timezone  = hours from UTC
//
// Example India:
// timezone = 5.5
//
SolarTimes calculateSunriseSunset(
    int year,
    int month,
    int day,
    double latitude,
    double longitude,
    double timezone
);