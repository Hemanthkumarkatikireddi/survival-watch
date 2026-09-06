#pragma once

#include <Arduino.h>
#include <RTClib.h>

// ============================================================
// TITHI
// ============================================================

struct TithiInfo
{
    uint8_t number;

    // Full UTC date + time
    DateTime startUTC;
    DateTime endUTC;

    uint8_t nextNumber;

    // Start time of next Tithi
    DateTime nextStartUTC;
};


// ============================================================
// NAKSHATRA
// ============================================================

struct NakshatraInfo
{
    uint8_t number;

    // Full UTC date + time
    DateTime startUTC;
    DateTime endUTC;
};


// ============================================================
// COMPLETE PANCHANG
// ============================================================

struct PanchangData
{
    TithiInfo tithi;

    NakshatraInfo nakshatra;

    // Local time decimal hours
    double sunrise;
    double sunset;
};


// ============================================================
// CALCULATIONS
// ============================================================

TithiInfo calculateTithi(
    DateTime dateTime
);

NakshatraInfo calculateNakshatra(
    DateTime dateTime
);

PanchangData calculatePanchang(
    DateTime dateTime,
    double latitude,
    double longitude,
    double timezone
);


// ============================================================
// NAMES
// ============================================================

const char* getPanchangTithiName(
    uint8_t tithi
);

const char* getPanchangNakshatraName(
    uint8_t nakshatra
);