#pragma once

#include <Arduino.h>

// ============================================================
// LUNAR CALCULATION
// ============================================================

// Moon position result

struct LunarPosition
{
    double longitude;
    double latitude;
    double distance;
};

// Calculate approximate geocentric Moon position.
//
// hourUTC = fractional UTC hour

LunarPosition calculateMoonPosition(
    int year,
    int month,
    int day,
    double hourUTC
);

// Tropical Moon longitude

double calculateMoonLongitude(
    int year,
    int month,
    int day,
    double hourUTC
);

// Sidereal Moon longitude
//
// Ayanamsa will be applied here.
// Later we can select the exact ayanamsa/convention
// used by the Telugu Panchangam we validate against.
//

double calculateMoonSiderealLongitude(
    int year,
    int month,
    int day,
    double hourUTC
);

// ============================================================
// MOONRISE / MOONSET
// ============================================================

struct LunarTimes
{
    double moonrise;
    double moonset;
};

// Calculate approximate Moonrise and Moonset.
//
// Output:
// moonrise = local decimal hour
// moonset  = local decimal hour
//
// Example:
// 12.50 = 12:30 PM
// 14.35 = 02:21 PM
//
// Returns -1.0 if the event is not found for that local day.
//

LunarTimes calculateMoonriseMoonset(
    int year,
    int month,
    int day,
    double latitude,
    double longitude,
    double timezone
);