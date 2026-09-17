#include "battery.h"
#include "display.h"
#include "fonts.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

// ============================================================
// BATTERY PERCENTAGE DISPLAY
//
// Area:
// X = 42 .. 83
// Y = 39 .. 47
//
// Only percentage text is displayed.
//
// Example:
//
// B: 45%
//
// No battery blocks.
// No animation.
// No loop.
// ============================================================

void drawBattery(
    int x,
    int y,
    int percent
)
{
    // --------------------------------------------------------
    // Limit percentage
    // --------------------------------------------------------

    percent = constrain(
        percent,
        0,
        100
    );


    // --------------------------------------------------------
    // Create text
    // --------------------------------------------------------

    char text[10];

    snprintf(
        text,
        sizeof(text),
        "B:%d%%",
        percent
    );


    // --------------------------------------------------------
    // Battery area
    // --------------------------------------------------------

    const int AREA_WIDTH  = 42;
    const int AREA_HEIGHT = 9;


    // --------------------------------------------------------
    // Character settings
    //
    // 5 pixel character width
    // 1 pixel gap between characters
    // --------------------------------------------------------

    const int CHAR_WIDTH = 5;
    const int LETTER_GAP = 2;
    const int CHAR_ADVANCE = CHAR_WIDTH + LETTER_GAP;


    int charCount = strlen(text);


    // --------------------------------------------------------
    // Calculate complete text width
    // --------------------------------------------------------

    int textWidth =
        (charCount * CHAR_WIDTH) +
        ((charCount - 1) * LETTER_GAP);


    // --------------------------------------------------------
    // Horizontal center
    // --------------------------------------------------------

    int textX =
        x +
        (AREA_WIDTH - textWidth) / 2;


    // --------------------------------------------------------
    // Vertical center
    //
    // 7px font inside 9px area
    // --------------------------------------------------------

    int textY =
        y + 1;


    // --------------------------------------------------------
    // Draw text
    //
    // Existing watch font is used.
    // --------------------------------------------------------

    for (int i = 0; i < charCount; i++)
    {
        drawWatchChar(
            textX,
            textY,
            text[i],
            1
        );

        // ----------------------------------------------------
        // 1px bold effect
        //
        // Second draw is kept inside the character area.
        // ----------------------------------------------------

        drawWatchChar(
            textX + 1,
            textY,
            text[i],
            1
        );

        // ----------------------------------------------------
        // Next character
        // ----------------------------------------------------

        textX += CHAR_ADVANCE;
    }
}