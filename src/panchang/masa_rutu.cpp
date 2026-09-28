#include "masa_rutu.h"

// ============================================================
// TELUGU MASA NAMES
// ============================================================

const char* getTeluguMasaName(int masa)
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
// TELUGU RUTU NUMBER
// ============================================================

int getTeluguRutu(int masa)
{
    switch (masa)
    {
        // ----------------------------------------------------
        // VASANTA RUTU
        // ----------------------------------------------------
        case 1:     // Chaitra
        case 2:     // Vaishakha
            return 0;


        // ----------------------------------------------------
        // GRISHMA RUTU
        // ----------------------------------------------------
        case 3:     // Jyeshtha
        case 4:     // Ashadha
            return 1;


        // ----------------------------------------------------
        // VARSHA RUTU
        // ----------------------------------------------------
        case 5:     // Shravana
        case 6:     // Bhadrapada
            return 2;


        // ----------------------------------------------------
        // SHARAD RUTU
        // ----------------------------------------------------
        case 7:     // Ashwayuja
        case 8:     // Kartika
            return 3;


        // ----------------------------------------------------
        // HEMANTA RUTU
        // ----------------------------------------------------
        case 9:     // Margashira
        case 10:    // Pushya
            return 4;


        // ----------------------------------------------------
        // SHISHIRA RUTU
        // ----------------------------------------------------
        case 11:    // Magha
        case 12:    // Phalguna
            return 5;


        default:
            return -1;
    }
}


// ============================================================
// TELUGU RUTU NAME
// ============================================================

const char* getTeluguRutuName(int masa)
{
    switch (getTeluguRutu(masa))
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