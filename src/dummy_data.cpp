#include "dummy_data.h"
#include <Arduino.h>

namespace DummyData
{

// ============================================================
// DUMMY GPS POSITIONS
// ============================================================
//
// World-location Panchang test.
//
// Every new GPS fix is generated once every 60 seconds.
//
// GPS #0 -> Vizianagaram, India
// GPS #1 -> Ahmedabad, Gujarat, India
// GPS #2 -> Jakarta, Indonesia
// GPS #3 -> Tokyo, Japan
// GPS #4 -> Sydney, Australia
// GPS #5 -> New York, USA
//
// Every movement is far greater than 70 km.
//
// This allows us to verify that:
//   - GPS location changes
//   - >70 km movement is detected
//   - background Panchang calculation starts
//   - Sunrise changes
//   - Sunset changes
//   - Tithi / Nakshatra data is recalculated
//   - location-dependent Panchang data belongs to NEW location
//
// ============================================================

static const GPSPoint gpsPoints[] =
{
    // ========================================================
    // GPS #0
    // Vizianagaram, India
    // ========================================================
    {
        18.1167,
        83.4000
    },

    // ========================================================
    // GPS #1
    // Tokyo, Japan
    // ========================================================
    {
        35.6762,
        139.6503
    },

    // ========================================================
    // GPS #2
    // New York, USA
    // ========================================================
    {
        40.7128,
        -74.0060
    },

    // ========================================================
    // GPS #3
    // Honolulu, Hawaii
    // ========================================================
    {
        21.3069,
        -157.8583
    },

    // ========================================================
    // GPS #4
    // Auckland, New Zealand
    // ========================================================
    {
        -36.8509,
        174.7645
    },

    // ========================================================
    // GPS #5
    // Pago Pago, American Samoa
    // ========================================================
    {
        -14.2756,
        -170.7020
    }
};
// ============================================================
// NUMBER OF GPS POINTS
// ============================================================

static constexpr int GPS_POINT_COUNT =
    sizeof(gpsPoints) /
    sizeof(gpsPoints[0]);


// ============================================================
// GPS TEST INTERVAL
// ============================================================
//
// A new dummy GPS fix is generated once every 60 seconds.
//
// ============================================================

static constexpr unsigned long GPS_TEST_INTERVAL =
    60000UL;


// ============================================================
// CURRENT GPS INDEX
// ============================================================

static int currentGPSIndex = 0;


// ============================================================
// LAST GPS FIX TIME
// ============================================================

static unsigned long lastGPSFixTime = 0;


// ============================================================
// FIRST FIX FLAG
// ============================================================

static bool firstGPSFix = true;


// ============================================================
// GET NEXT DUMMY GPS FIX
// ============================================================

// ============================================================
// GET NEXT DUMMY GPS FIX
// ============================================================
//
// Current dummy location:
// Vizianagaram, Andhra Pradesh, India
//
// GPS is kept fixed at this location for normal development.
// ============================================================

bool getNextGPSFix(
    double& latitude,
    double& longitude
)
{
    latitude =
        gpsPoints[0].latitude;

    longitude =
        gpsPoints[0].longitude;

    if (firstGPSFix)
    {
        firstGPSFix = false;

        Serial.println();
        Serial.println(
            "DUMMY GPS: First fix"
        );

        Serial.print(
            "DUMMY GPS: LAT = "
        );

        Serial.println(
            latitude,
            6
        );

        Serial.print(
            "DUMMY GPS: LON = "
        );

        Serial.println(
            longitude,
            6
        );

        return true;
    }

    return false;
}
// ============================================================
// RESET GPS SEQUENCE
// ============================================================

void resetGPSSequence()
{
    currentGPSIndex = 0;

    lastGPSFixTime = 0;

    firstGPSFix = true;
}

}