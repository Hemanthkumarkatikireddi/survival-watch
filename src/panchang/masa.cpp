#include "masa.h"
#include "solar.h"
#include "lunar.h"

#include <math.h>

// ============================================================
// INTERNAL HELPERS
// ============================================================

static double normalizeAngle(double angle)
{
    while (angle < 0.0)
        angle += 360.0;

    while (angle >= 360.0)
        angle -= 360.0;

    return angle;
}

static double normalize180(double angle)
{
    while (angle > 180.0)
        angle -= 360.0;

    while (angle < -180.0)
        angle += 360.0;

    return angle;
}


// ============================================================
// JULIAN DAY
//
// Same convention as existing lunar.cpp
// ============================================================

static double julianDay(
    int year,
    int month,
    int day,
    double hourUTC
)
{
    if (month <= 2)
    {
        year--;
        month += 12;
    }

    int A =
        year / 100;

    int B =
        2 -
        A +
        A / 4;

    return
        floor(365.25 * (year + 4716))
        +
        floor(30.6001 * (month + 1))
        +
        day
        +
        B
        -
        1524.5
        +
        hourUTC / 24.0;
}


// ============================================================
// JD -> DateTime
// ============================================================

static DateTime dateTimeFromJD(
    double jd
)
{
    double Z =
        floor(jd + 0.5);

    double F =
        (jd + 0.5) - Z;

    double A =
        Z;

    if (Z >= 2299161)
    {
        int alpha =
            (int)(
                (Z - 1867216.25)
                / 36524.25
            );

        A =
            Z +
            1 +
            alpha -
            alpha / 4;
    }

    double B =
        A + 1524;

    double C =
        floor(
            (B - 122.1) / 365.25
        );

    double D =
        floor(
            365.25 * C
        );

    double E =
        floor(
            (B - D) / 30.6001
        );

    double dayValue =
        B -
        D -
        floor(30.6001 * E) +
        F;

    int day =
        (int)floor(dayValue);

    int month;

    if (E < 14)
        month = (int)E - 1;
    else
        month = (int)E - 13;

    int year;

    if (month > 2)
        year = (int)C - 4716;
    else
        year = (int)C - 4715;

    double fractionalDay =
        dayValue - day;

    double totalSeconds =
        fractionalDay * 86400.0;

    if (totalSeconds < 0)
        totalSeconds = 0;

    if (totalSeconds >= 86400)
        totalSeconds = 86399;

    int hour =
        (int)(totalSeconds / 3600.0);

    totalSeconds -=
        hour * 3600.0;

    int minute =
        (int)(totalSeconds / 60.0);

    int second =
        (int)(
            totalSeconds -
            minute * 60.0
        );

    return DateTime(
        year,
        month,
        day,
        hour,
        minute,
        second
    );
}


// ============================================================
// MOON-SUN PHASE
//
// 0°   = Amavasya
// 180° = Purnima
// ============================================================

static double lunarPhase(
    DateTime dt
)
{
    double hourUTC =
        dt.hour()
        +
        dt.minute() / 60.0
        +
        dt.second() / 3600.0;

    double sun =
        calculateSunLongitude(
            dt.year(),
            dt.month(),
            dt.day(),
            hourUTC
        );

    double moon =
        calculateMoonLongitude(
            dt.year(),
            dt.month(),
            dt.day(),
            hourUTC
        );

    return normalizeAngle(
        moon - sun
    );
}


// ============================================================
// SIGNED NEW MOON PHASE
// ============================================================

static double signedNewMoonPhase(
    DateTime dt
)
{
    return normalize180(
        lunarPhase(dt)
    );
}


// ============================================================
// SIGNED FULL MOON PHASE
// ============================================================

static double signedFullMoonPhase(
    DateTime dt
)
{
    return normalize180(
        lunarPhase(dt) - 180.0
    );
}


// ============================================================
// CONTINUOUS LUNAR ELONGATION
//
// 0°   = Amavasya
// 180° = Purnima
// 360° = next Amavasya
// ============================================================

static double getLunarElongation(
    DateTime dt
)
{
    double hourUTC =
        dt.hour()
        +
        dt.minute() / 60.0
        +
        dt.second() / 3600.0;

    double sun =
        calculateSunLongitude(
            dt.year(),
            dt.month(),
            dt.day(),
            hourUTC
        );

    double moon =
        calculateMoonLongitude(
            dt.year(),
            dt.month(),
            dt.day(),
            hourUTC
        );

    return normalizeAngle(
        moon - sun
    );
}


// ============================================================
// FIND AMAVASYA USING CONTINUOUS LUNAR ELONGATION
//
// Amavasya = lunar elongation 0° / 360°
//
// Backward:
//     find the most recent Amavasya.
//
// Forward:
//     find the next Amavasya.
// ============================================================

static DateTime findAmavasya(
    DateTime dateTime,
    bool searchBackward
)
{
    const double STEP_HOURS = 3.0;
    const int MAX_STEPS = 300;

    double startJD =
        julianDay(
            dateTime.year(),
            dateTime.month(),
            dateTime.day(),
            dateTime.hour()
            +
            dateTime.minute() / 60.0
            +
            dateTime.second() / 3600.0
        );

    double previousPhase =
        getLunarElongation(
            dateTime
        );

    for (
        int i = 1;
        i <= MAX_STEPS;
        i++
    )
    {
        double jd;

        if (searchBackward)
        {
            jd =
                startJD -
                i * STEP_HOURS / 24.0;
        }
        else
        {
            jd =
                startJD +
                i * STEP_HOURS / 24.0;
        }

        DateTime sample =
            dateTimeFromJD(
                jd
            );

        double phase =
            getLunarElongation(
                sample
            );

        bool crossing =
            false;


        // ====================================================
        // BACKWARD SEARCH
        //
        // Example:
        //
        // 31 → 20 → 10 → 2 → 359
        //
        // ====================================================

        if (searchBackward)
        {
            if (
                previousPhase < 180.0 &&
                phase > 180.0
            )
            {
                crossing = true;
            }
        }


        // ====================================================
        // FORWARD SEARCH
        //
        // Example:
        //
        // 350 → 359 → 1
        //
        // ====================================================

        else
        {
            if (
                previousPhase > 180.0 &&
                phase < 180.0
            )
            {
                crossing = true;
            }
        }


        if (crossing)
        {
            double lowJD;
            double highJD;

            if (searchBackward)
            {
                lowJD =
                    jd;

                highJD =
                    jd +
                    STEP_HOURS / 24.0;
            }
            else
            {
                lowJD =
                    jd -
                    STEP_HOURS / 24.0;

                highJD =
                    jd;
            }


            // ================================================
            // BINARY SEARCH
            // ================================================

            for (
                int j = 0;
                j < 40;
                j++
            )
            {
                double middleJD =
                    (lowJD + highJD) / 2.0;

                DateTime middle =
                    dateTimeFromJD(
                        middleJD
                    );

                double middlePhase =
                    getLunarElongation(
                        middle
                    );

                if (searchBackward)
                {
                    if (middlePhase > 180.0)
                        lowJD = middleJD;
                    else
                        highJD = middleJD;
                }
                else
                {
                    if (middlePhase > 180.0)
                        lowJD = middleJD;
                    else
                        highJD = middleJD;
                }
            }

            return dateTimeFromJD(
                (lowJD + highJD) / 2.0
            );
        }

        previousPhase =
            phase;
    }

    return dateTime;
}


// ============================================================
// FIND PURNIMA
//
// Finds the first Full Moon AFTER the supplied Amavasya.
// ============================================================

static DateTime findPurnima(
    DateTime amavasya
)
{
    const double STEP_HOURS = 6.0;

    double startJD =
        julianDay(
            amavasya.year(),
            amavasya.month(),
            amavasya.day(),
            amavasya.hour()
            +
            amavasya.minute() / 60.0
            +
            amavasya.second() / 3600.0
        );

    double previousPhase =
        signedFullMoonPhase(
            amavasya
        );

    const int MAX_STEPS = 100;

    for (
        int i = 1;
        i <= MAX_STEPS;
        i++
    )
    {
        double jd =
            startJD +
            i * STEP_HOURS / 24.0;

        DateTime sample =
            dateTimeFromJD(
                jd
            );

        double phase =
            signedFullMoonPhase(
                sample
            );


        // ====================================================
        // PURNIMA CROSSING
        //
        // negative -> 0 -> positive
        // ====================================================

        if (
            previousPhase < 0.0 &&
            phase >= 0.0
        )
        {
            double lowJD =
                jd -
                STEP_HOURS / 24.0;

            double highJD =
                jd;


            // ================================================
            // BINARY SEARCH
            // ================================================

            for (
                int j = 0;
                j < 35;
                j++
            )
            {
                double middleJD =
                    (lowJD + highJD) / 2.0;

                DateTime middle =
                    dateTimeFromJD(
                        middleJD
                    );

                double middlePhase =
                    signedFullMoonPhase(
                        middle
                    );

                if (
                    middlePhase < 0.0
                )
                {
                    lowJD =
                        middleJD;
                }
                else
                {
                    highJD =
                        middleJD;
                }
            }

            return dateTimeFromJD(
                (lowJD + highJD) / 2.0
            );
        }

        previousPhase =
            phase;
    }


    // Should never normally happen
    return amavasya;
}


// ============================================================
// LAHIRI AYANAMSA
//
// Same approximation used by lunar.cpp.
// ============================================================

static double calculateLahiriAyanamsa(
    double JD
)
{
    double T =
        (JD - 2451545.0) /
        36525.0;

    return
        23.85675 +
        1.3968 * T;
}


// ============================================================
// SIDEREAL SUN LONGITUDE
// ============================================================

static double calculateSunSiderealLongitude(
    DateTime dt
)
{
    double hourUTC =
        dt.hour()
        +
        dt.minute() / 60.0
        +
        dt.second() / 3600.0;

    double tropical =
        calculateSunLongitude(
            dt.year(),
            dt.month(),
            dt.day(),
            hourUTC
        );

    double JD =
        julianDay(
            dt.year(),
            dt.month(),
            dt.day(),
            hourUTC
        );

    double ayanamsa =
        calculateLahiriAyanamsa(
            JD
        );

    return normalizeAngle(
        tropical -
        ayanamsa
    );
}


// ============================================================
// SOLAR RASHI
//
// 0  = Mesha
// 1  = Vrishabha
// 2  = Mithuna
// 3  = Karkataka
// 4  = Simha
// 5  = Kanya
// 6  = Tula
// 7  = Vrischika
// 8  = Dhanus
// 9  = Makara
// 10 = Kumbha
// 11 = Meena
// ============================================================

static uint8_t getSolarRashi(
    DateTime dt
)
{
    double longitude =
        calculateSunSiderealLongitude(
            dt
        );

    int rashi =
        (int)floor(
            longitude / 30.0
        );

    if (rashi < 0)
        rashi = 0;

    if (rashi > 11)
        rashi = 11;

    return (uint8_t)rashi;
}


// ============================================================
// MONTH NUMBER FROM SOLAR RASHI
//
// Mesha      -> Chaitra
// Vrishabha  -> Vaishakha
// Mithuna    -> Jyeshtha
// Karkataka  -> Ashadha
// Simha      -> Shravana
// Kanya      -> Bhadrapada
// Tula       -> Ashwayuja
// Vrischika  -> Kartika
// Dhanus     -> Margashira
// Makara     -> Pushya
// Kumbha     -> Magha
// Meena      -> Phalguna
// ============================================================

static uint8_t masaFromRashi(uint8_t rashi)
{
    // Telugu Amanta Masa mapping
    //
    // Mesha     -> Vaishakha
    // Vrishabha -> Jyeshtha
    // Mithuna   -> Ashadha
    // Karka     -> Shravana
    // Simha     -> Bhadrapada
    // Kanya     -> Ashwayuja
    // Tula      -> Kartika
    // Vrischika -> Margashira
    // Dhanus    -> Pushya
    // Makara    -> Magha
    // Kumbha    -> Phalguna
    // Meena     -> Chaitra

    uint8_t masa =
        (uint8_t)((rashi + 2) % 12);

    return masa;
}

// ============================================================
// CHECK SOLAR SANKRANTI BETWEEN TWO AMAVASYAS
//
// If Sun changes sidereal sign between the two New Moons,
// this is a normal lunar month.
//
// If it does not change sign,
// this lunar month is Adhika.
// ============================================================

static bool hasSolarSankranti(
    DateTime start,
    DateTime end
)
{
    uint8_t startRashi =
        getSolarRashi(
            start
        );

    uint8_t endRashi =
        getSolarRashi(
            end
        );

    if (
        startRashi !=
        endRashi
    )
    {
        return true;
    }


    // ========================================================
    // Extra protection
    //
    // Sample every 12 hours so a Sankranti very close to
    // an Amavasya is not missed.
    // ========================================================

    double startJD =
        julianDay(
            start.year(),
            start.month(),
            start.day(),
            start.hour()
            +
            start.minute() / 60.0
            +
            start.second() / 3600.0
        );

    double endJD =
        julianDay(
            end.year(),
            end.month(),
            end.day(),
            end.hour()
            +
            end.minute() / 60.0
            +
            end.second() / 3600.0
        );

    uint8_t previousRashi =
        startRashi;


    for (
        double jd =
            startJD + 0.5;

        jd < endJD;

        jd += 0.5
    )
    {
        DateTime sample =
            dateTimeFromJD(
                jd
            );

        uint8_t rashi =
            getSolarRashi(
                sample
            );

        if (
            rashi !=
            previousRashi
        )
        {
            return true;
        }

        previousRashi =
            rashi;
    }

    return false;
}


// ============================================================
// CALCULATE TELUGU MASAM
//
// IMPORTANT:
// There is ONLY ONE definition of this function in this file.
//
// Normal Masa:
//     Name is determined from Sun's sidereal Rashi
//     at the Purnima of the lunar month.
//
// Adhika Masa:
//     No Sankranti exists between the two Amavasyas.
//     Therefore use the following normal month's name.
// ============================================================

MasaInfo calculateTeluguMasa(
    DateTime dateTime
)
{
    MasaInfo result;

    result.number = 0;
    result.adhika = false;

    result.amavasyaStartUTC =
        dateTime;

    result.nextAmavasyaUTC =
        dateTime;

    result.purnimaUTC =
        dateTime;


    // ========================================================
    // CURRENT LUNAR MONTH
    // ========================================================

    DateTime currentAmavasya =
        findAmavasya(
            dateTime,
            true
        );


    // ========================================================
    // NEXT AMAVASYA
    // ========================================================

    DateTime nextAmavasya =
        findAmavasya(
            currentAmavasya +
            TimeSpan(1, 0, 0, 0),
            false
        );


    // ========================================================
    // PURNIMA
    // ========================================================

    DateTime purnima =
        findPurnima(
            currentAmavasya +
            TimeSpan(1, 0, 0, 0)
        );


    // ========================================================
    // CHECK SANKRANTI
    // ========================================================

    bool sankranti =
        hasSolarSankranti(
            currentAmavasya,
            nextAmavasya
        );


    uint8_t masa = 0;


    // ========================================================
    // NORMAL MASA
    //
    // Use the Solar Rashi at the CURRENT AMAVASYA.
    //
    // This correctly handles:
    //
    // May 2026  -> Vaishakha
    // June 2026 -> Jyeshtha
    // September -> Bhadrapada
    // ========================================================

    if (sankranti)
    {
        uint8_t rashi =
            getSolarRashi(
                currentAmavasya
            );

        masa =
            masaFromRashi(
                rashi
            );
    }


    // ========================================================
    // ADHIKA MASA
    //
    // No Sankranti between the two Amavasyas.
    //
    // Therefore use the following lunar month's
    // Amavasya Solar Rashi.
    // ========================================================

    else
{
    // ========================================================
    // ADHIKA MASA
    //
    // For an Adhika lunar month, use the Solar Rashi
    // at the Purnima of THIS lunar month.
    //
    // This gives:
    //
    // May 2026 -> Adhika Jyeshtha
    // ========================================================

    uint8_t purnimaRashi =
        getSolarRashi(
            purnima
        );

    masa =
        masaFromRashi(
            purnimaRashi
        );
}


    // ========================================================
    // SAVE RESULT
    // ========================================================

    result.number =
        masa;

    result.adhika =
        !sankranti;

    result.amavasyaStartUTC =
        currentAmavasya;

    result.nextAmavasyaUTC =
        nextAmavasya;

    result.purnimaUTC =
        purnima;


    return result;
}

// ============================================================
// TELUGU MASA NAMES
// ============================================================

const char* getPanchangMasaName(
    uint8_t masa
)
{
    switch (masa)
    {
        case 1:
            return "Chaitra";

        case 2:
            return "Vaishakha";

        case 3:
            return "Jyeshtha";

        case 4:
            return "Ashadha";

        case 5:
            return "Shravana";

        case 6:
            return "Bhadrapada";

        case 7:
            return "Ashwayuja";

        case 8:
            return "Kartika";

        case 9:
            return "Margashira";

        case 10:
            return "Pushya";

        case 11:
            return "Magha";

        case 12:
            return "Phalguna";

        default:
            return "Unknown";
    }
}


// ============================================================
// DISPLAY NAME
// ============================================================

const char* getPanchangMasaDisplayName(
    uint8_t masa,
    bool adhika
)
{
    static char buffer[32];

    const char* name =
        getPanchangMasaName(
            masa
        );

    if (adhika)
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "Adhika %s",
            name
        );
    }
    else
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "%s",
            name
        );
    }

    return buffer;
}


// ============================================================
// RUTU
// ============================================================

uint8_t getPanchangRutu(
    uint8_t masa
)
{
    switch (masa)
    {
        case 1:
        case 2:
            return 0;       // Vasanta

        case 3:
        case 4:
            return 1;       // Grishma

        case 5:
        case 6:
            return 2;       // Varsha

        case 7:
        case 8:
            return 3;       // Sharad

        case 9:
        case 10:
            return 4;       // Hemanta

        case 11:
        case 12:
            return 5;       // Shishira

        default:
            return 255;
    }
}


// ============================================================
// RUTU NAME
// ============================================================

const char* getPanchangRutuName(
    uint8_t rutu
)
{
    switch (rutu)
    {
        case 0:
            return "Vasanta";

        case 1:
            return "Grishma";

        case 2:
            return "Varsha";

        case 3:
            return "Sharad";

        case 4:
            return "Hemanta";

        case 5:
            return "Shishira";

        default:
            return "Unknown";
    }
}
// ============================================================
// MASA YEAR TEST
// ============================================================

void runMasaYearTest(
    int startYear,
    int endYear
)
{
    Serial.println();
    Serial.println("================================================");
    Serial.println("        TELUGU MASA TEST");
    Serial.println("================================================");

    Serial.println();

    Serial.println(
        "Date        Masa              Adhika   Rutu"
    );

    Serial.println(
        "------------------------------------------------"
    );

    for (
        int year = startYear;
        year <= endYear;
        year++
    )
    {
        // ====================================================
        // TEST 1 : MAY 15
        // ====================================================

        DateTime testDate1(
            year,
            5,
            15,
            12,
            0,
            0
        );

        MasaInfo info1 =
            calculateTeluguMasa(testDate1);

        uint8_t rutu1 =
            getPanchangRutu(info1.number);

        Serial.print(year);
        Serial.print("-05-15   ");

        if (info1.adhika)
            Serial.print("Adhika ");

        Serial.print(
            getPanchangMasaName(info1.number)
        );

        Serial.print("   ");

        if (info1.adhika)
            Serial.print("YES");
        else
            Serial.print("NO ");

        Serial.print("      ");

        Serial.println(
            getPanchangRutuName(rutu1)
        );


        // ====================================================
        // TEST 2 : MAY 30
        // ====================================================

        DateTime testDate2(
            year,
            5,
            30,
            12,
            0,
            0
        );

        MasaInfo info2 =
            calculateTeluguMasa(testDate2);

        uint8_t rutu2 =
            getPanchangRutu(info2.number);

        Serial.print(year);
        Serial.print("-05-30   ");

        if (info2.adhika)
            Serial.print("Adhika ");

        Serial.print(
            getPanchangMasaName(info2.number)
        );

        Serial.print("   ");

        if (info2.adhika)
            Serial.print("YES");
        else
            Serial.print("NO ");

        Serial.print("      ");

        Serial.println(
            getPanchangRutuName(rutu2)
        );


        // ====================================================
        // TEST 3 : JUNE 15
        // ====================================================

        DateTime testDate3(
            year,
            6,
            15,
            12,
            0,
            0
        );

        MasaInfo info3 =
            calculateTeluguMasa(testDate3);

        uint8_t rutu3 =
            getPanchangRutu(info3.number);

        Serial.print(year);
        Serial.print("-06-15   ");

        if (info3.adhika)
            Serial.print("Adhika ");

        Serial.print(
            getPanchangMasaName(info3.number)
        );

        Serial.print("   ");

        if (info3.adhika)
            Serial.print("YES");
        else
            Serial.print("NO ");

        Serial.print("      ");

        Serial.println(
            getPanchangRutuName(rutu3)
        );


        // ====================================================
        // TEST 4 : SEPTEMBER 13
        // ====================================================

        DateTime testDate4(
            year,
            9,
            13,
            12,
            0,
            0
        );

        MasaInfo info4 =
            calculateTeluguMasa(testDate4);

        uint8_t rutu4 =
            getPanchangRutu(info4.number);

        Serial.print(year);
        Serial.print("-09-13   ");

        if (info4.adhika)
            Serial.print("Adhika ");

        Serial.print(
            getPanchangMasaName(info4.number)
        );

        Serial.print("   ");

        if (info4.adhika)
            Serial.print("YES");
        else
            Serial.print("NO ");

        Serial.print("      ");

        Serial.println(
            getPanchangRutuName(rutu4)
        );
    }

    Serial.println(
        "================================================"
    );

    Serial.println(
        "        MASA TEST COMPLETE"
    );

    Serial.println(
        "================================================"
    );

    Serial.println();
}