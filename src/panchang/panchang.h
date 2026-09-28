#pragma once

#include <Arduino.h>
#include <RTClib.h>

// ============================================================
// TITHI INFORMATION
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

struct NakshatraInfo
{
    uint8_t number;

    // Current Nakshatra start/end in UTC
    DateTime startUTC;
    DateTime endUTC;

    // Next Nakshatra
    uint8_t nextNumber;
    DateTime nextStartUTC;
};


// ============================================================
// PANCHANG DATA
// ============================================================

struct PanchangData
{
    TithiInfo tithi;

    NakshatraInfo nakshatra;

    // Local decimal hours
    double sunrise;
    double sunset;

    uint8_t masaNumber;
    bool adhikaMasa;
    uint8_t rutuNumber;
};


// ============================================================
// TITHI CALCULATION
// ============================================================

TithiInfo calculateTithi(
    DateTime dateTime
);


// ============================================================
// NAKSHATRA CALCULATION
// ============================================================

NakshatraInfo calculateNakshatra(
    DateTime dateTime
);


// ============================================================
// COMPLETE PANCHANG CALCULATION
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

const char* getPanchangTithiName(
    uint8_t tithi
);


// ============================================================
// NAKSHATRA NAME
// ============================================================

const char* getPanchangNakshatraName(
    uint8_t nakshatra
);