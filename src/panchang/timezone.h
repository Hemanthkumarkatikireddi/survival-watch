#pragma once

#include <Arduino.h>
#include <RTClib.h>

// ============================================================
// TIMEZONE REGION
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

void setGPSLocation(
    double latitude,
    double longitude
);

double getGPSLatitude();

double getGPSLongitude();

// ============================================================
// TIMEZONE DETECTION
// ============================================================

TimezoneInfo getCurrentTimezone(
    DateTime utcDateTime
);

// ============================================================
// UTC -> LOCATION LOCAL TIME
// ============================================================

DateTime getLocationLocalTime(
    DateTime utcDateTime
);

// ============================================================
// UTC -> INDIA TIME
// ============================================================

DateTime convertUTCToIndiaTime(
    DateTime utcDateTime
);

// ============================================================
// INDIA TIME -> UTC
// ============================================================

DateTime convertIndiaTimeToUTC(
    DateTime indiaDateTime
);

// ============================================================
// FORMAT 12-HOUR TIME
// ============================================================

void format12HourTime(
    DateTime dt,
    char* output,
    size_t outputSize
);

// ============================================================
// FORMAT SUNRISE / SUNSET
// ============================================================

void formatSolarTime(
    double decimalHour,
    char* output,
    size_t outputSize
);