#include "lunar.h"
#include <math.h>

// ============================================================
// COMPACT MOON POSITION
// Meeus-style lunar theory
//
// Output:
// longitude = geocentric tropical ecliptic longitude (degrees)
// latitude  = geocentric ecliptic latitude (degrees)
// distance  = kilometres
// ============================================================

static double normalizeAngle(double angle)
{
    while (angle < 0.0)
        angle += 360.0;

    while (angle >= 360.0)
        angle -= 360.0;

    return angle;
}

// ============================================================
// JULIAN DAY
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

    int A = year / 100;

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
// MOON POSITION
// ============================================================

LunarPosition calculateMoonPosition(
    int year,
    int month,
    int day,
    double hourUTC
)
{
    LunarPosition result;

    double JD =
        julianDay(
            year,
            month,
            day,
            hourUTC
        );

    // Centuries from J2000.0
    double T =
        (JD - 2451545.0) / 36525.0;

    // ========================================================
    // FUNDAMENTAL ARGUMENTS
    // ========================================================

    // Mean elongation of Moon from Sun
    double D =
        297.8501921
        +
        445267.1114034 * T
        -
        0.0018819 * T * T
        +
        T * T * T / 545868.0
        -
        T * T * T * T / 113065000.0;

    // Mean anomaly of Sun
    double M =
        357.5291092
        +
        35999.0502909 * T
        -
        0.0001536 * T * T
        +
        T * T * T / 24490000.0;

    // Mean anomaly of Moon
    double Mp =
        134.9633964
        +
        477198.8675055 * T
        +
        0.0087414 * T * T
        +
        T * T * T / 69699.0
        -
        T * T * T * T / 14712000.0;

    // Moon argument of latitude
    double F =
        93.2720950
        +
        483202.0175233 * T
        -
        0.0036539 * T * T
        -
        T * T * T / 3526000.0
        +
        T * T * T * T / 863310000.0;

    // Longitude of ascending node
    double Omega =
        125.0445550
        -
        1934.1361849 * T
        +
        0.0020762 * T * T
        +
        T * T * T / 467410.0
        -
        T * T * T * T / 60616000.0;

    D = normalizeAngle(D);
    M = normalizeAngle(M);
    Mp = normalizeAngle(Mp);
    F = normalizeAngle(F);
    Omega = normalizeAngle(Omega);

    // ========================================================
    // MEAN MOON LONGITUDE
    //
    // IMPORTANT:
    // Lp must be calculated BEFORE it is used by the
    // additional longitude correction terms.
    // ========================================================

    double Lp =
        218.3164477
        +
        481267.88123421 * T
        -
        0.0015786 * T * T
        +
        T * T * T / 538841.0
        -
        T * T * T * T / 65194000.0;

    Lp = normalizeAngle(Lp);

    // ========================================================
    // ADDITIONAL CORRECTION ARGUMENTS
    // ========================================================

    double A1 =
        119.75 +
        131.849 * T;

    double A2 =
        53.09 +
        479264.29 * T;

    double A3 =
        313.45 +
        481266.484 * T;

    // ========================================================
    // EARTH ECCENTRICITY
    // ========================================================

    double E =
        1.0
        -
        0.002516 * T
        -
        0.0000074 * T * T;

    // ========================================================
    // RADIANS
    // ========================================================

    double Dr =
        D * DEG_TO_RAD;

    double Mr =
        M * DEG_TO_RAD;

    double Mpr =
        Mp * DEG_TO_RAD;

    double Fr =
        F * DEG_TO_RAD;

    // ========================================================
    // MOON LONGITUDE PERIODIC TERMS
    // ========================================================

    struct LongitudeTerm
    {
        int D;
        int M;
        int Mp;
        int F;
        double coefficient;
    };

    static const LongitudeTerm terms[] =
    {
        { 0, 0, 1, 0, 6288774 },
        { 2, 0,-1, 0, 1274027 },
        { 2, 0, 0, 0, 658314 },
        { 0, 0, 2, 0, 213618 },
        { 0, 1, 0, 0,-185116 },
        { 0, 0, 0, 2,-114332 },
        { 2, 0,-2, 0, 58793 },
        { 2,-1,-1, 0, 57066 },
        { 2, 0, 1, 0, 53322 },
        { 2,-1, 0, 0, 45758 },
        { 0, 1,-1, 0,-40923 },
        { 1, 0, 0, 0,-34720 },
        { 0, 1, 1, 0,-30383 },
        { 2, 0, 0,-2,15327 },
        { 0, 0, 1, 2,-12528 },
        { 0, 0, 1,-2,10980 },
        { 4, 0,-1, 0,10675 },
        { 0, 0, 3, 0,10034 },
        { 4, 0,-2, 0,8548 },
        { 2, 1,-1, 0,-7888 },
        { 2, 1, 0, 0,-6766 },
        { 1, 0,-1, 0,-5163 },
        { 1, 1, 0, 0,4987 },
        { 2,-1,1, 0,4036 },
        { 2, 0, 2, 0,3994 },
        { 4, 0, 0, 0,3861 },
        { 2, 0,-3, 0,3665 },
        { 0, 1,-2, 0,-2689 },
        { 2, 0,-1, 2,-2602 },
        { 2,-1,-2, 0,2390 },
        { 1, 0, 1, 0,-2348 },
        { 2,-2, 0, 0,2236 },
        { 0, 1, 2, 0,-2120 },
        { 0, 2, 0, 0,-2069 },
        { 2,-2,-1, 0,2048 },
        { 2, 0, 1,-2,-1773 },
        { 2, 0, 0, 2,-1595 },
        { 4,-1,-1, 0,1215 },
        { 0, 0, 2, 2,-1110 },
        { 3, 0,-1, 0,-892 },
        { 2, 1, 1, 0,-810 },
        { 4,-1,-2, 0,759 },
        { 0, 2,-1, 0,-713 },
        { 2, 2,-1, 0,-700 },
        { 2, 1,-2, 0,691 },
        { 2,-1, 0,-2,596 },
        { 4, 0, 1, 0,549 },
        { 0, 0, 4, 0,537 },
        { 4,-1, 0, 0,520 },
        { 1, 0,-2, 0,-487 },
        { 2, 1, 0,-2,-399 },
        { 0, 0, 2,-2,-381 },
        { 1, 1, 1, 0,351 },
        { 3, 0,-2, 0,-340 },
        { 4, 0,-3, 0,330 },
        { 2,-1,2, 0,327 },
        { 0, 2,1, 0,-323 },
        { 1, 1,-1, 0,299 },
        { 2, 0, 3, 0,294 }
    };

    const int termCount =
        sizeof(terms) /
        sizeof(terms[0]);

    double longitudeCorrection = 0.0;

    for (int i = 0; i < termCount; i++)
    {
        const LongitudeTerm& term =
            terms[i];

        double argument =
            term.D * Dr
            +
            term.M * Mr
            +
            term.Mp * Mpr
            +
            term.F * Fr;

        double factor = 1.0;

        if (term.M == 1 || term.M == -1)
        {
            factor = E;
        }
        else if (term.M == 2 || term.M == -2)
        {
            factor = E * E;
        }

        longitudeCorrection +=
            term.coefficient *
            factor *
            sin(argument);
    }

    // ========================================================
    // ADDITIONAL LONGITUDE CORRECTIONS
    // ========================================================

    longitudeCorrection +=
        3958.0 *
        sin(A1 * DEG_TO_RAD);

    longitudeCorrection +=
        1962.0 *
        sin(Lp * DEG_TO_RAD);

    longitudeCorrection +=
        318.0 *
        sin(A3 * DEG_TO_RAD);

    // ========================================================
    // FINAL MOON LONGITUDE
    // ========================================================

    double longitude =
        Lp +
        longitudeCorrection / 1000000.0;

    longitude =
        normalizeAngle(longitude);

    // ========================================================
    // MOON LATITUDE PERIODIC TERMS
    // ========================================================

    struct LatitudeTerm
    {
        int D;
        int M;
        int Mp;
        int F;
        double coefficient;
    };

    static const LatitudeTerm latTerms[] =
    {
        { 0, 0, 0, 1, 5128122 },
        { 0, 0, 1, 1, 280602 },
        { 0, 0, 1,-1, 277693 },
        { 2, 0, 0,-1, 173237 },
        { 2, 0,-1, 1, 55413 },
        { 2, 0,-1,-1, 46271 },
        { 2, 0, 0, 1, 32573 },
        { 0, 0, 2, 1, 17198 },
        { 2, 0, 1,-1, 9266 },
        { 0, 0, 2,-1, 8822 },
        { 2,-1, 0,-1, 8216 },
        { 2, 0,-2,-1, 4324 },
        { 2, 0, 1, 1, 4200 },
        { 2, 1, 0,-1,-3359 },
        { 2,-1,-1, 1, 2463 },
        { 2,-1, 0, 1, 2211 },
        { 2,-1,-1,-1, 2065 },
        { 0, 1,-1,-1,-1870 },
        { 4, 0,-1,-1, 1828 },
        { 0, 1, 0, 1,-1794 },
        { 0, 1,-1, 1,-1565 },
        { 1, 0, 0, 1,-1491 },
        { 0, 1, 1, 1,-1475 },
        { 0, 1, 1,-1,-1410 },
        { 0, 1, 0,-1,-1344 },
        { 1, 0, 0,-1,-1335 },
        { 0, 0, 3, 1, 1107 },
        { 4, 0, 0,-1, 1021 },
        { 4, 0,-1, 1, 833 },
        { 0, 0, 1,-3, 777 },
        { 4, 0,-2, 1, 671 },
        { 2, 0, 0,-3, 607 },
        { 2, 0, 2,-1, 596 },
        { 2,-1, 1,-1, 491 },
        { 2, 0,-2, 1,-451 },
        { 0, 0, 3,-1, 439 },
        { 2, 0, 2, 1, 422 },
        { 2, 0,-3,-1, 421 },
        { 2, 1,-1, 1,-366 },
        { 2, 1, 0, 1,-351 },
        { 4, 0, 0, 1, 331 },
        { 2,-1, 1, 1, 315 },
        { 2,-2, 0,-1, 302 },
        { 0, 0, 1, 3,-283 },
        { 2, 1, 1,-1,-229 },
        { 1, 1, 0,-1, 223 },
        { 1, 1, 0, 1, 223 },
        { 0, 1,-2,-1,-220 },
        { 2, 1,-1,-1,-220 },
        { 1, 0, 1, 1,-185 },
        { 2,-1,-2,-1, 181 },
        { 0, 1, 2, 1,-177 },
        { 4, 0,-2,-1, 176 },
        { 4,-1,-1,-1, 166 },
        { 1, 0, 1,-1,-164 },
        { 4, 0, 1,-1, 132 },
        { 1, 0,-1,-1,-119 },
        { 4,-1, 0,-1, 115 },
        { 2,-2, 0, 1, 107 }
    };

    const int latTermCount =
        sizeof(latTerms) /
        sizeof(latTerms[0]);

    double latitudeCorrection = 0.0;

    for (int i = 0; i < latTermCount; i++)
    {
        const LatitudeTerm& term =
            latTerms[i];

        double argument =
            term.D * Dr
            +
            term.M * Mr
            +
            term.Mp * Mpr
            +
            term.F * Fr;

        double factor = 1.0;

        if (term.M == 1 || term.M == -1)
        {
            factor = E;
        }
        else if (term.M == 2 || term.M == -2)
        {
            factor = E * E;
        }

        latitudeCorrection +=
            term.coefficient *
            factor *
            sin(argument);
    }

    // ========================================================
    // ADDITIONAL LATITUDE CORRECTIONS
    // ========================================================

    latitudeCorrection +=
        -2235.0 *
        sin(Lp * DEG_TO_RAD);

    latitudeCorrection +=
        382.0 *
        sin(A3 * DEG_TO_RAD);

    latitudeCorrection +=
        175.0 *
        sin((A1 + F) * DEG_TO_RAD);

    latitudeCorrection +=
        175.0 *
        sin((A1 - F) * DEG_TO_RAD);

    latitudeCorrection +=
        127.0 *
        sin((Lp - Mp) * DEG_TO_RAD);

    latitudeCorrection +=
        -115.0 *
        sin((Lp + Mp) * DEG_TO_RAD);

    double latitude =
        latitudeCorrection / 1000000.0;

    // ========================================================
    // MOON DISTANCE
    // ========================================================

    struct DistanceTerm
    {
        int D;
        int M;
        int Mp;
        int F;
        double coefficient;
    };

    static const DistanceTerm distanceTerms[] =
    {
        { 0, 0, 1, 0,-20905355 },
        { 2, 0,-1, 0,-3699111 },
        { 2, 0, 0, 0,-2955968 },
        { 0, 0, 2, 0,-569925 },
        { 0, 1, 0, 0, 48 },
        { 0, 0, 0, 2,-3149 },
        { 2, 0,-2, 0, 246158 },
        { 2,-1,-1, 0,-152138 },
        { 2, 0, 1, 0,-170733 },
        { 2,-1, 0, 0,-204586 },
        { 0, 1,-1, 0,-129620 },
        { 1, 0, 0, 0, 108743 },
        { 0, 1, 1, 0, 104755 },
        { 2, 0, 0,-2, 10321 },
        { 0, 0, 1, 2, 0 },
        { 0, 0, 1,-2, 79661 },
        { 4, 0,-1, 0,-34782 },
        { 0, 0, 3, 0,-23210 },
        { 4, 0,-2, 0,-21636 },
        { 2, 1,-1, 0, 24208 }
    };

    const int distanceTermCount =
        sizeof(distanceTerms) /
        sizeof(distanceTerms[0]);

    double distanceCorrection = 0.0;

    for (int i = 0; i < distanceTermCount; i++)
    {
        const DistanceTerm& term =
            distanceTerms[i];

        double argument =
            term.D * Dr
            +
            term.M * Mr
            +
            term.Mp * Mpr
            +
            term.F * Fr;

        double factor = 1.0;

        if (term.M == 1 || term.M == -1)
        {
            factor = E;
        }
        else if (term.M == 2 || term.M == -2)
        {
            factor = E * E;
        }

        distanceCorrection +=
            term.coefficient *
            factor *
            cos(argument);
    }

    double distance =
        385000.56 +
        distanceCorrection;

    // ========================================================
    // RESULT
    // ========================================================

    result.longitude =
        longitude;

    result.latitude =
        latitude;

    result.distance =
        distance;

    return result;
}

// ============================================================
// MOON TROPICAL LONGITUDE
// ============================================================

double calculateMoonLongitude(
    int year,
    int month,
    int day,
    double hourUTC
)
{
    LunarPosition moon =
        calculateMoonPosition(
            year,
            month,
            day,
            hourUTC
        );

    return moon.longitude;
}

// ============================================================
// MOON SIDEREAL LONGITUDE
// Lahiri / Chitrapaksha approximation
// ============================================================

double calculateMoonSiderealLongitude(
    int year,
    int month,
    int day,
    double hourUTC
)
{
    double tropicalLongitude =
        calculateMoonLongitude(
            year,
            month,
            day,
            hourUTC
        );

    double JD =
        julianDay(
            year,
            month,
            day,
            hourUTC
        );

    double T =
        (JD - 2451545.0) /
        36525.0;

    // Approximate Lahiri ayanamsa
    double ayanamsa =
        23.85675 +
        1.3968 * T;

    return normalizeAngle(
        tropicalLongitude -
        ayanamsa
    );
}


// ============================================================
// MOONRISE / MOONSET CALCULATION
//
// Separate calculation.
// Existing Moon position, Tithi and Nakshatra calculations
// are NOT modified.
//
// Output:
// moonrise = local decimal hour
// moonset  = local decimal hour
//
// Example:
// 0.50  = 12:30 AM
// 14.35 = 02:21 PM
//
// Returns -1.0 when the event is not found.
// ============================================================

// ============================================================
// NORMALIZE +/-180 DEGREES
// ============================================================

static double normalize180(double angle)
{
    while (angle > 180.0)
        angle -= 360.0;

    while (angle < -180.0)
        angle += 360.0;

    return angle;
}

// ============================================================
// MOON EQUATORIAL POSITION
//
// Converts ecliptic longitude/latitude to:
//
// Right Ascension = degrees
// Declination     = degrees
//
// Uses the same Moon model already present above.
// ============================================================

static void calculateMoonEquatorialPosition(
    int year,
    int month,
    int day,
    double hourUTC,
    double& rightAscension,
    double& declination
)
{
    LunarPosition moon =
        calculateMoonPosition(
            year,
            month,
            day,
            hourUTC
        );

    double JD =
        julianDay(
            year,
            month,
            day,
            hourUTC
        );

    // Mean obliquity of the ecliptic
    double T =
        (JD - 2451545.0) / 36525.0;

    double epsilon =
        23.439291 -
        0.0130042 * T;

    double lambda =
        moon.longitude *
        DEG_TO_RAD;

    double beta =
        moon.latitude *
        DEG_TO_RAD;

    double eps =
        epsilon *
        DEG_TO_RAD;

    // ========================================================
    // ECLIPTIC -> EQUATORIAL
    // ========================================================

    double sinRA =
        cos(beta) *
        sin(lambda) *
        cos(eps)
        -
        sin(beta) *
        sin(eps);

    double cosRA =
        cos(beta) *
        cos(lambda);

    double alpha =
        atan2(
            sinRA,
            cosRA
        );

    double delta =
        asin(
            sin(beta) *
            cos(eps)
            +
            cos(beta) *
            sin(eps) *
            sin(lambda)
        );

    rightAscension =
        alpha / DEG_TO_RAD;

    declination =
        delta / DEG_TO_RAD;

    rightAscension =
        normalizeAngle(
            rightAscension
        );
}

// ============================================================
// GREENWICH MEAN SIDEREAL TIME
// ============================================================

static double calculateGMST(
    double JD
)
{
    double T =
        (JD - 2451545.0) / 36525.0;

    double gmst =
        280.46061837
        +
        360.98564736629 *
        (JD - 2451545.0)
        +
        0.000387933 * T * T
        -
        T * T * T / 38710000.0;

    return normalizeAngle(gmst);
}

// ============================================================
// MOON ALTITUDE
//
// latitude  = observer latitude
// longitude = observer longitude
// hourUTC   = UTC fractional hour
//
// Includes lunar horizontal parallax.
// ============================================================

static double calculateMoonAltitude(
    int year,
    int month,
    int day,
    double hourUTC,
    double latitude,
    double longitude
)
{
    LunarPosition moon =
        calculateMoonPosition(
            year,
            month,
            day,
            hourUTC
        );

    double rightAscension;
    double declination;

    calculateMoonEquatorialPosition(
        year,
        month,
        day,
        hourUTC,
        rightAscension,
        declination
    );

    double JD =
        julianDay(
            year,
            month,
            day,
            hourUTC
        );

    double gmst =
        calculateGMST(JD);

    // Local sidereal time
    double lst =
        normalizeAngle(
            gmst +
            longitude
        );

    double hourAngle =
        normalize180(
            lst -
            rightAscension
        );

    double latRad =
        latitude *
        DEG_TO_RAD;

    double decRad =
        declination *
        DEG_TO_RAD;

    double haRad =
        hourAngle *
        DEG_TO_RAD;

    // Geocentric altitude
    double sinAltitude =
        sin(latRad) *
        sin(decRad)
        +
        cos(latRad) *
        cos(decRad) *
        cos(haRad);

    sinAltitude =
        constrain(
            sinAltitude,
            -1.0,
            1.0
        );

    double altitude =
        asin(
            sinAltitude
        ) / DEG_TO_RAD;

    // ========================================================
    // LUNAR HORIZONTAL PARALLAX
    // ========================================================

    double earthRadius =
        6378.14;

    double ratio =
        earthRadius /
        moon.distance;

    ratio =
        constrain(
            ratio,
            -1.0,
            1.0
        );

    double horizontalParallax =
        asin(
            ratio
        ) / DEG_TO_RAD;

    // ========================================================
    // APPROXIMATE APPARENT MOON ALTITUDE
    //
    // Practical rise/set correction.
    // ========================================================

    double apparentAltitude =
        altitude +
        horizontalParallax *
        cos(
            altitude *
            DEG_TO_RAD
        );

    apparentAltitude -= 0.83;

    return apparentAltitude;
}

// ============================================================
// JULIAN DAY -> CALENDAR DATE
// ============================================================

static void calculateDateFromJD(
    double jd,
    int& outYear,
    int& outMonth,
    int& outDay,
    double& outHourUTC
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
                /
                36524.25
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
            (B - 122.1) /
            365.25
        );

    double D =
        floor(
            365.25 * C
        );

    double E =
        floor(
            (B - D) /
            30.6001
        );

    double dayValue =
        B -
        D -
        floor(
            30.6001 * E
        ) +
        F;

    outDay =
        (int)floor(
            dayValue
        );

    double fractionalDay =
        dayValue -
        outDay;

    if (E < 14)
        outMonth =
            (int)E - 1;
    else
        outMonth =
            (int)E - 13;

    if (outMonth > 2)
        outYear =
            (int)C - 4716;
    else
        outYear =
            (int)C - 4715;

    outHourUTC =
        fractionalDay *
        24.0;

    if (outHourUTC < 0.0)
        outHourUTC = 0.0;

    if (outHourUTC >= 24.0)
        outHourUTC = 23.999999;
}

// ============================================================
// MOONRISE / MOONSET
// ============================================================

LunarTimes calculateMoonriseMoonset(
    int year,
    int month,
    int day,
    double latitude,
    double longitude,
    double timezone
)
{
    LunarTimes result;

    result.moonrise = -1.0;
    result.moonset = -1.0;

    // ========================================================
    // LOCATION VALIDATION
    // ========================================================

    if (latitude < -90.0 ||
        latitude > 90.0 ||
        longitude < -180.0 ||
        longitude > 180.0)
    {
        return result;
    }

    // ========================================================
    // LOCAL MIDNIGHT -> UTC
    //
    // Example:
    // India = UTC+5:30
    //
    // Local 00:00
    // UTC = previous day 18:30
    // ========================================================

    double utcStart =
        -timezone;

    // ========================================================
    // SCAN WINDOW
    //
    // Start 3 hours before local midnight
    // End 27 hours after local midnight
    //
    // This allows events very close to midnight to be found.
    // ========================================================

    double scanStart =
        utcStart - 3.0;

    double scanEnd =
        utcStart + 27.0;

    const double stepMinutes =
        10.0;

    const double stepHours =
        stepMinutes / 60.0;

    double baseJD =
        julianDay(
            year,
            month,
            day,
            0.0
        );

    double startJD =
        baseJD +
        scanStart / 24.0;

    double endJD =
        baseJD +
        scanEnd / 24.0;

    double previousJD =
        startJD;

    double previousAltitude =
        0.0;

    bool firstPoint =
        true;

    // ========================================================
    // SCAN EVERY 10 MINUTES
    // ========================================================

    for (
        double jd = startJD;
        jd <= endJD;
        jd += stepHours / 24.0
    )
    {
        int sampleYear;
        int sampleMonth;
        int sampleDay;

        double sampleHourUTC;

        calculateDateFromJD(
            jd,
            sampleYear,
            sampleMonth,
            sampleDay,
            sampleHourUTC
        );

        double altitude =
            calculateMoonAltitude(
                sampleYear,
                sampleMonth,
                sampleDay,
                sampleHourUTC,
                latitude,
                longitude
            );

        if (firstPoint)
        {
            previousJD =
                jd;

            previousAltitude =
                altitude;

            firstPoint =
                false;

            continue;
        }

        // ====================================================
        // MOONRISE
        //
        // Negative -> Positive
        // ====================================================

        if (previousAltitude < 0.0 &&
            altitude >= 0.0 &&
            result.moonrise < 0.0)
        {
            double low =
                previousJD;

            double high =
                jd;

            // Binary search
            for (int i = 0; i < 25; i++)
            {
                double middle =
                    (low + high) / 2.0;

                int y;
                int m;
                int d;

                double h;

                calculateDateFromJD(
                    middle,
                    y,
                    m,
                    d,
                    h
                );

                double middleAltitude =
                    calculateMoonAltitude(
                        y,
                        m,
                        d,
                        h,
                        latitude,
                        longitude
                    );

                if (middleAltitude >= 0.0)
                    high = middle;
                else
                    low = middle;
            }

            double eventJD =
                (low + high) / 2.0;

            // Convert to local decimal hour
            double localHours =
                (eventJD - baseJD) *
                24.0
                +
                timezone;

            while (localHours < 0.0)
                localHours += 24.0;

            while (localHours >= 24.0)
                localHours -= 24.0;

            result.moonrise =
                localHours;
        }

        // ====================================================
        // MOONSET
        //
        // Positive -> Negative
        // ====================================================

        if (previousAltitude >= 0.0 &&
            altitude < 0.0 &&
            result.moonset < 0.0)
        {
            double low =
                previousJD;

            double high =
                jd;

            // Binary search
            for (int i = 0; i < 25; i++)
            {
                double middle =
                    (low + high) / 2.0;

                int y;
                int m;
                int d;

                double h;

                calculateDateFromJD(
                    middle,
                    y,
                    m,
                    d,
                    h
                );

                double middleAltitude =
                    calculateMoonAltitude(
                        y,
                        m,
                        d,
                        h,
                        latitude,
                        longitude
                    );

                if (middleAltitude < 0.0)
                    high = middle;
                else
                    low = middle;
            }

            double eventJD =
                (low + high) / 2.0;

            // Convert to local decimal hour
            double localHours =
                (eventJD - baseJD) *
                24.0
                +
                timezone;

            while (localHours < 0.0)
                localHours += 24.0;

            while (localHours >= 24.0)
                localHours -= 24.0;

            result.moonset =
                localHours;
        }

        previousJD =
            jd;

        previousAltitude =
            altitude;
    }

    return result;
}