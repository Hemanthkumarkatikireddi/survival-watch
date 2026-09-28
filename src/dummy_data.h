#pragma once

namespace DummyData
{
    // ============================================================
    // ORIGINAL DEFAULT GPS POSITION
    // ============================================================

    static constexpr double GPS_LATITUDE =
        18.1167;

    static constexpr double GPS_LONGITUDE =
        83.4000;


    // ============================================================
    // DUMMY GPS
    // ============================================================

    struct GPSPoint
    {
        double latitude;
        double longitude;
    };

    bool getNextGPSFix(
        double& latitude,
        double& longitude
    );

    void resetGPSSequence();


    // ============================================================
    // TEMPORARY SENSOR VALUES
    // ============================================================

    static constexpr int HEART_BPM =
        87;

    static constexpr int TEMPERATURE_C =
        29;

    static constexpr int HUMIDITY_PERCENT =
        73;

    static constexpr int BATTERY_PERCENT =
        99;
}