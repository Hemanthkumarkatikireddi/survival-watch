#pragma once

#include <Arduino.h>
#include <RTClib.h>

// ============================================================
// TITHI INFORMATION
// ============================================================
//
// number:
//   1  - 15  = Shukla Paksha
//   16 - 30  = Krishna Paksha
//
// All astronomical boundary times are stored in UTC.
// The UI can convert them to the required local timezone.
// ============================================================

struct TithiInfo
{
    uint8_t number;

    // Current Tithi start/end in UTC
    DateTime startUTC;
    DateTime endUTC;

    // Next Tithi
    uint8_t nextNumber;
    DateTime nextStartUTC;
};


// ============================================================
// NAKSHATRA INFORMATION
// ============================================================
//
// Nakshatra number:
//   1 - 27
//
// Boundary times are stored in UTC.
// ============================================================

struct NakshatraInfo
{
    uint8_t number;

    // Current Nakshatra start/end in UTC
    DateTime startUTC;
    DateTime endUTC;
};


// ============================================================
// PANCHANG DATA
// ============================================================
//
// This contains the Panchang information required by the watch.
//
// Tithi and Nakshatra:
//   Astronomical boundary times are UTC.
//
// Sunrise / Sunset:
//   Decimal local hours for the requested location/date.
//
// Example:
//   5.75  = 05:45 AM
//   18.25 = 06:15 PM
// ============================================================

struct PanchangData
{
    TithiInfo tithi;

    NakshatraInfo nakshatra;

    // Local decimal hours
    double sunrise;
    double sunset;
};


// ============================================================
// TITHI CALCULATION
// ============================================================
//
// dateTime must represent the UTC instant at which the
// calculation is being performed.
//
// The function determines:
//   - Current Tithi
//   - Current Tithi start
//   - Current Tithi end
//   - Next Tithi
//   - Next Tithi start
// ============================================================

TithiInfo calculateTithi(
    DateTime dateTime
);


// ============================================================
// NAKSHATRA CALCULATION
// ============================================================
//
// dateTime must represent a UTC instant.
//
// Determines:
//   - Current Nakshatra
//   - Nakshatra start
//   - Nakshatra end
// ============================================================

NakshatraInfo calculateNakshatra(
    DateTime dateTime
);


// ============================================================
// COMPLETE PANCHANG CALCULATION
// ============================================================
//
// dateTime:
//   Current UTC instant.
//
// latitude:
//   GPS latitude in degrees.
//
// longitude:
//   GPS longitude in degrees.
//
// timezone:
//   Local timezone offset from UTC in hours.
//
// Example:
//   India       = +5.5
//   Japan       = +9.0
//   UK winter   =  0.0
//   UK summer   = +1.0
//
// IMPORTANT:
// Sunrise / sunset must be calculated for the LOCATION'S
// LOCAL CIVIL DATE, not simply the UTC calendar date.
//
// The implementation will handle this correctly when we
// finish the Panchang/location architecture.
// ============================================================

PanchangData calculatePanchang(
    DateTime dateTime,
    double latitude,
    double longitude,
    double timezone
);


// ============================================================
// TITHI NAME
// ============================================================
//
// Input:
//   1  - 30
//
// Returns English Panchang name.
//
// Examples:
//   1  -> Pratipada
//   4  -> Chaturthi
//   15 -> Purnima
//   30 -> Amavasya
// ============================================================

const char* getPanchangTithiName(
    uint8_t tithi
);


// ============================================================
// NAKSHATRA NAME
// ============================================================
//
// Input:
//   1 - 27
//
// Returns the standard Nakshatra name.
// ============================================================

const char* getPanchangNakshatraName(
    uint8_t nakshatra
);