#include <Arduino.h>
#include "moon_data.h"
#include "display.h"

// ============================================================
// NOKIA 5110 - 25x16 MOON PHASE GENERATOR
// ============================================================
//
// Based directly on the exact HTML/JavaScript algorithm
// used for this project's 30 Tithi moon designs.
//
// Tithi:
//   1  - Shukla Pratipada
//   2  - Shukla Dwitiya
//   ...
//   15 - Purnima
//   16 - Krishna Pratipada
//   ...
//   29 - Krishna Chaturdashi
//   30 - Amavasya
//
// Size:
//   25 x 16 pixels
//
// The outer round border is common to all 30 Tithis.
//
// ============================================================


// ============================================================
// EXACT HTML MOON MASK
// ============================================================
//
// IMPORTANT:
//
// These strings are intentionally kept exactly as supplied
// in the original HTML.
//
// They contain 24 characters even though the drawing loop
// checks x = 0..24.
//
// At x = 24, JavaScript gets undefined and therefore treats
// it as not equal to "1".
//
// We preserve that behavior here by using the same 24-char
// strings.
//
// ============================================================

static const char moonMask[16][25] =
{
    "000001111111111111100000",
    "000110000000000000011000",
    "001100000000000000001100",
    "011000000000000000000110",
    "110000000000000000000011",
    "110000000000000000000011",
    "110000000000000000000011",
    "110000000000000000000011",
    "110000000000000000000011",
    "110000000000000000000011",
    "110000000000000000000011",
    "110000000000000000000011",
    "011000000000000000000110",
    "001100000000000000001100",
    "000110000000000000011000",
    "000001111111111111100000"
};


// ============================================================
// EXACT 30 TITHI PHASE VALUES
// ============================================================

static const float phases[30] =
{
    0.04f,
    0.09f,
    0.14f,
    0.19f,
    0.25f,
    0.31f,
    0.38f,
    0.50f,
    0.61f,
    0.70f,
    0.78f,
    0.85f,
    0.91f,
    0.96f,
    1.00f,

    0.96f,
    0.91f,
    0.85f,
    0.78f,
    0.70f,
    0.61f,
    0.50f,
    0.38f,
    0.31f,
    0.25f,
    0.19f,
    0.14f,
    0.09f,
    0.04f,
    0.00f
};


// ============================================================
// EXACT INSIDE-MOON TEST
// ============================================================
//
// Same values as the original HTML.
//
// cx = 12
// cy = 7.5
// rx = 10.3
// ry = 6.3
//
// ============================================================

static bool insideMoon(
    float px,
    float py
)
{
    const float cx = 12.0f;
    const float cy = 7.5f;

    const float rx = 10.3f;
    const float ry = 6.3f;

    const float dx = (px - cx) / rx;
    const float dy = (py - cy) / ry;

    return ((dx * dx) + (dy * dy)) <= 1.0f;
}


// ============================================================
// DRAW 25x16 MOON FOR A TITHI
// ============================================================
//
// tithi = 1..30
//
// The function reproduces the original JavaScript:
//
// createMoon(tithi)
//
// exactly in C++.
//
// ============================================================

void drawMoon25x16(
    int x,
    int y,
    uint8_t tithi
)
{
    // --------------------------------------------------------
    // Safety
    // --------------------------------------------------------

    if (tithi < 1)
    {
        tithi = 1;
    }

    if (tithi > 30)
    {
        tithi = 30;
    }


    // --------------------------------------------------------
    // JavaScript:
    //
    // const phase = phases[tithi - 1];
    // const isShukla = tithi <= 15;
    // --------------------------------------------------------

    const float phase = phases[tithi - 1];

    const bool isShukla = (tithi <= 15);


    // --------------------------------------------------------
    // 16 rows
    // --------------------------------------------------------

    for (int row = 0; row < 16; row++)
    {
        // ----------------------------------------------------
        // 25 pixels
        // ----------------------------------------------------

        for (int col = 0; col < 25; col++)
        {

            // =================================================
            // 1. OUTER ROUND BORDER
            // =================================================
            //
            // Same as:
            //
            // if (moonMask[y][x] === "1")
            // {
            //     row += "1";
            //     continue;
            // }
            //
            // =================================================

            if (moonMask[row][col] == '1')
            {
                display.drawPixel(
                    x + col,
                    y + row,
                    BLACK
                );

                continue;
            }


            // =================================================
            // 2. OUTSIDE THE MOON
            // =================================================

            if (!insideMoon(
                    (float)col,
                    (float)row
                ))
            {
                continue;
            }


            // =================================================
            // 3. PURNIMA
            // =================================================
            //
            // Tithi 15 = completely illuminated moon.
            //
            // =================================================

            if (tithi == 15)
            {
                display.drawPixel(
                    x + col,
                    y + row,
                    BLACK
                );

                continue;
            }


            // =================================================
            // 4. AMAVASYA
            // =================================================
            //
            // Tithi 30 = completely dark moon.
            //
            // Only the outer border remains.
            //
            // =================================================

            if (tithi == 30)
            {
                continue;
            }


            // =================================================
            // 5. EXACT TERMINATOR CALCULATION
            // =================================================
            //
            // Same as original JavaScript.
            //
            // =================================================

            const float cy = 7.5f;

            const float vertical =
                fabsf(
                    (float)row - cy
                ) / 7.5f;


            const float curve =
                sqrtf(
                    fmaxf(
                        0.0f,
                        1.0f -
                        (vertical * vertical)
                    )
                );


            // -------------------------------------------------
            // Moon left/right boundaries
            // -------------------------------------------------

            const float moonLeft =
                2.0f +
                (1.0f - curve) * 2.0f;


            const float moonRight =
                22.0f -
                (1.0f - curve) * 2.0f;


            bool illuminated = false;


            // =================================================
            // SHUKLA PAKSHA
            // =================================================
            //
            // Right side illuminated.
            //
            // =================================================

            if (isShukla)
            {
                const float terminator =
                    moonRight -
                    (phase * 2.0f * 10.0f);


                const float curvedTerminator =
                    terminator +
                    (1.0f - curve) * 0.8f;


                if ((float)col >= curvedTerminator)
                {
                    illuminated = true;
                }
            }


            // =================================================
            // KRISHNA PAKSHA
            // =================================================
            //
            // Left side illuminated.
            //
            // =================================================

            else
            {
                const float terminator =
                    moonLeft +
                    (phase * 2.0f * 10.0f);


                const float curvedTerminator =
                    terminator -
                    (1.0f - curve) * 0.8f;


                if ((float)col <= curvedTerminator)
                {
                    illuminated = true;
                }
            }


            // =================================================
            // DRAW ILLUMINATED PIXEL
            // =================================================

            if (illuminated)
            {
                display.drawPixel(
                    x + col,
                    y + row,
                    BLACK
                );
            }
        }
    }
}