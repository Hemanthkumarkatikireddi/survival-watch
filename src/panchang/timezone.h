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
// TIMEZONE MODE
// ============================================================
//
// MANUAL:
//     User-selected timezone stays fixed.
//
// CURRENT_LOCATION:
//     Timezone follows GPS location.
//

enum TimezoneMode
{
    TIMEZONE_MANUAL = 0,
    TIMEZONE_CURRENT_LOCATION = 1
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
// GPS CACHE
// ============================================================
//
// Stores the last valid GPS location in ESP32 RTC memory.
//
// This allows the last GPS location to remain available
// after a software reset / restart when RTC memory is retained.
//
// IMPORTANT:
// This is NOT permanent storage.
// Complete power loss can clear this cache.
//
// ============================================================

struct GPSCache
{
    uint32_t magic;

    double latitude;
    double longitude;

    bool valid;
};

// ============================================================
// GPS CACHE MAGIC
// ============================================================
//
// Used to determine whether RTC memory contains a valid
// GPS cache or uninitialized/random data.
//

#define GPS_CACHE_MAGIC 0x47505331UL

// ============================================================
// GPS CACHE STORAGE
// ============================================================
//
// RTC_NOINIT_ATTR prevents normal startup initialization
// of this variable, allowing the previous RTC-memory value
// to be checked after reset.
//
// ============================================================

extern GPSCache lastGPSCache;

// ============================================================
// GPS CACHE FUNCTIONS
// ============================================================

void saveGPSCache(
    double latitude,
    double longitude
);

bool restoreGPSCache();

bool hasValidGPSCache();



// ============================================================
// GPS-BASED TIMEZONE DETECTION
// ============================================================
//
// This always detects timezone from GPS coordinates.
//
// It does NOT depend on Manual / Current Location mode.
//

TimezoneInfo getCurrentTimezone(
    DateTime utcDateTime
);

// ============================================================
// ACTIVE TIMEZONE
// ============================================================
//
// This is the timezone that HOME CLOCK and PANCHANG
// should use.
//
// MANUAL:
//     selected manual timezone.
//
// CURRENT LOCATION:
//     timezone detected from GPS.
//

TimezoneInfo getActiveTimezone(
    DateTime utcDateTime
);

// ============================================================
// TIMEZONE MODE
// ============================================================

TimezoneMode getTimezoneMode();

void setTimezoneMode(
    TimezoneMode mode
);

// ============================================================
// MANUAL TIMEZONE SELECTION
// ============================================================
//
// Index corresponds directly to timezoneRegions[].
//
// Default:
//     0 = India
//

int getManualTimezoneIndex();

bool setManualTimezoneIndex(
    int index
);

// ============================================================
// TIMEZONE SETTINGS INITIALIZATION
// ============================================================
//
// Loads saved timezone mode and selected timezone
// from ESP32 Preferences / NVS.
//
// Default if nothing is saved:
//     MANUAL
//     India
//

void initializeTimezoneSettings();

// ============================================================
// TIMEZONE CHANGE NOTIFICATION
// ============================================================
//
// When user changes timezone from the menu,
// this flag becomes true.
//
// Panchang code can consume this request and
// immediately start background recalculation.
//

bool consumeTimezoneChangeRequest();

// ============================================================
// UTC -> ACTIVE LOCAL TIME
// ============================================================
//
// Used by HOME CLOCK / PANCHANG local civil time.
//

DateTime getActiveLocalTime(
    DateTime utcDateTime
);

// ============================================================
// UTC -> GPS LOCATION LOCAL TIME
// ============================================================
//
// This remains GPS-location based and is intentionally
// separate from getActiveLocalTime().
//
// Useful when the actual GPS location timezone is required.
//

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
// FORMAT SOLAR TIME
// ============================================================

void formatSolarTime(
    double decimalHour,
    char* output,
    size_t outputSize
);

