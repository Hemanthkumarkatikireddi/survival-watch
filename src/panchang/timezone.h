#pragma once

#include <Arduino.h>
#include <RTClib.h>

// ============================================================
// TIMEZONE REGION
// ============================================================
//
// Temporary offline geographic timezone region.
//
// IMPORTANT:
// This is currently a simplified region table.
//
// It is NOT a final worldwide timezone boundary database.
//
// Later this can be replaced by a compact SD-card based
// geographic timezone database.
// ============================================================

struct TimezoneRegion
{
    double minLat;
    double maxLat;

    double minLon;
    double maxLon;

    const char* countryCode;
    const char* countryName;

    const char* timezoneId;

    int standardOffsetMinutes;

    bool hasDST;
};


// ============================================================
// TIMEZONE INFORMATION
// ============================================================

struct TimezoneInfo
{
    const char* countryCode;
    const char* countryName;

    const char* timezoneId;

    int utcOffsetMinutes;

    bool daylightSaving;
};


// ============================================================
// TIMEZONE REGION DATA
// ============================================================

extern const TimezoneRegion timezoneRegions[];

extern const int timezoneRegionCount;


// ============================================================
// GPS LOCATION
// ============================================================
//
// The GPS module will eventually call:
//
//     setGPSLocation(latitude, longitude);
//
// Latitude:
//
//     -90 ... +90
//
// Longitude:
//
//     -180 ... +180
//
// The timezone module does NOT own the GPS hardware.
// It only stores the latest valid GPS position.
//
// ============================================================

void setGPSLocation(
    double latitude,
    double longitude
);

double getGPSLatitude();

double getGPSLongitude();


// ============================================================
// TIMEZONE DETECTION
// ============================================================
//
// Input:
//     UTC date/time.
//
// The current GPS coordinates are used to determine the
// timezone.
//
// UTC is required because DST status can depend on the
// exact UTC instant.
//
// ============================================================

TimezoneInfo getCurrentTimezone(
    DateTime utcDateTime
);


// ============================================================
// UTC → LOCATION LOCAL TIME
// ============================================================
//
// Converts a UTC instant to the local civil time at the
// current GPS location.
//
// Example:
//
//     UTC:
//         2026-09-06 18:30
//
//     Japan:
//         2026-09-07 03:30
//
// ============================================================

DateTime getLocationLocalTime(
    DateTime utcDateTime
);


// ============================================================
// UTC → INDIA TIME
// ============================================================
//
// India is always:
//
//     UTC + 05:30
//
// No DST.
//
// This is used for the normal HOME clock.
//
// ============================================================

DateTime convertUTCToIndiaTime(
    DateTime utcDateTime
);


// ============================================================
// INDIA TIME → UTC
// ============================================================
//
// Converts India Standard Time to UTC.
//
//     IST - 05:30 = UTC
//
// ============================================================

DateTime convertIndiaTimeToUTC(
    DateTime indiaDateTime
);


// ============================================================
// FORMAT 12-HOUR TIME
// ============================================================
//
// Example:
//
//     21:35
//
// becomes:
//
//     09:35pm
//
// ============================================================

void format12HourTime(
    DateTime dt,
    char* output,
    size_t outputSize
);


// ============================================================
// FORMAT SOLAR TIME
// ============================================================
//
// Input:
//
//     decimal local hour
//
// Examples:
//
//     5.50  → 05:30am
//     18.25 → 06:15pm
//
// Invalid:
//
//     < 0
//     >= 24
//
// gives:
//
//     --:--
//
// ============================================================

void formatSolarTime(
    double decimalHour,
    char* output,
    size_t outputSize
);