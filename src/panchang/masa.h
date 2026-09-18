#pragma once

#include <Arduino.h>
#include <RTClib.h>

// ============================================================
// TELUGU LUNAR MASAM
//
// 1  = Chaitra
// 2  = Vaishakha
// 3  = Jyeshtha
// 4  = Ashadha
// 5  = Shravana
// 6  = Bhadrapada
// 7  = Ashwayuja
// 8  = Kartika
// 9  = Margashira
// 10 = Pushya
// 11 = Magha
// 12 = Phalguna
// ============================================================

struct MasaInfo
{
    uint8_t number;
    bool adhika;

    DateTime amavasyaStartUTC;
    DateTime nextAmavasyaUTC;
    DateTime purnimaUTC;
};

// ============================================================
// CALCULATE CURRENT TELUGU LUNAR MASAM
// ============================================================

MasaInfo calculateTeluguMasa(
    DateTime dateTime
);

// ============================================================
// NAMES
// ============================================================

const char* getPanchangMasaName(
    uint8_t masa
);

const char* getPanchangMasaDisplayName(
    uint8_t masa,
    bool adhika
);

// ============================================================
// RUTU
//
// 0 = Vasanta
// 1 = Grishma
// 2 = Varsha
// 3 = Sharad
// 4 = Hemanta
// 5 = Shishira
// ============================================================

uint8_t getPanchangRutu(
    uint8_t masa
);

const char* getPanchangRutuName(
    uint8_t rutu
);

// ============================================================
// MASA MULTI-YEAR TEST
// ============================================================

void runMasaYearTest(
    int startYear,
    int endYear
);