#pragma once

#include <Arduino.h>

// ============================================================
// LUNAR CALCULATION
// ============================================================
//
// All astronomical Moon-position calculations use UTC.
//
// Geographic location is NOT required for:
//
//     Moon longitude
//     Moon latitude
//     Moon distance
//     Tropical longitude
//     Sidereal longitude
//
// Geographic location IS required for:
//
//     Moonrise
//     Moonset
//
// ============================================================


// ============================================================
// MOON POSITION
// ============================================================
//
// Geocentric ecliptic Moon position.
//
// longitude:
//     Tropical ecliptic longitude, degrees.
//
// latitude:
//     Geocentric ecliptic latitude, degrees.
//
// distance:
//     Earth-Moon distance, kilometres.
//
// ============================================================

struct LunarPosition
{
    double longitude;
    double latitude;
    double distance;
};


// ============================================================
// CALCULATE MOON POSITION
// ============================================================
//
// year/month/day:
//     UTC calendar date.
//
// hourUTC:
//     Fractional UTC hour.
//
// Example:
//
//     16:30:00 UTC
//     = 16.5
//
// Returns:
//
//     Geocentric tropical ecliptic Moon position.
//
// ============================================================

LunarPosition calculateMoonPosition(
    int year,
    int month,
    int day,
    double hourUTC
);


// ============================================================
// TROPICAL MOON LONGITUDE
// ============================================================
//
// Returns Moon's geocentric tropical ecliptic longitude.
//
// Range:
//
//     0° <= longitude < 360°
//
// Used by:
//
//     Tithi calculation
//
// ============================================================

double calculateMoonLongitude(
    int year,
    int month,
    int day,
    double hourUTC
);


// ============================================================
// SIDEREAL MOON LONGITUDE
// ============================================================
//
// Returns Moon's sidereal longitude.
//
// Current project convention:
//
//     Lahiri / Chitrapaksha approximation
//
// Used by:
//
//     Nakshatra calculation
//
// IMPORTANT:
//
// The currently validated ayanamsa approximation must remain
// unchanged unless we intentionally recalibrate the Panchang
// against a different reference.
//
// ============================================================

double calculateMoonSiderealLongitude(
    int year,
    int month,
    int day,
    double hourUTC
);


// ============================================================
// MOONRISE / MOONSET
// ============================================================
//
// Moonrise and Moonset depend on:
//
//     - local calendar date
//     - latitude
//     - longitude
//     - timezone
//
// year/month/day MUST represent the LOCATION'S LOCAL DATE.
//
// Example:
//
//     Japan local date:
//         2026-09-07
//
//     timezone:
//         +9.0
//
// Output:
//
//     local decimal hours
//
// Example:
//
//     0.50  = 12:30 AM
//     14.35 = 02:21 PM
//
// If an event is not found:
//
//     -1.0
//
// ============================================================

struct LunarTimes
{
    double moonrise;
    double moonset;
};


LunarTimes calculateMoonriseMoonset(
    int year,
    int month,
    int day,
    double latitude,
    double longitude,
    double timezone
);