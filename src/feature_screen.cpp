#include "feature_screen.h"

#include "display.h"
#include "fonts.h"

#include <Arduino.h>

// ============================================================
// COMMON FEATURE SCREEN
// ============================================================


// ============================================================
// FEATURE TITLE
// ============================================================

void drawFeatureTitle(const char* title)
{
    if (title == nullptr)
        return;

    int width = watchTextWidth(title, 1);

    int x = (84 - width) / 2;

    if (x < 0)
        x = 0;

    drawBoldWatchText(
        19,
        2,
        title,
        1,
        BLACK
    );
}


// ============================================================
// SEPARATOR
// ============================================================

void drawFeatureSeparator()
{
    display.drawLine(
        0,
        11,
        83,
        11,
        BLACK
    );
}


// ============================================================
// NORMAL TEXT
// ============================================================

void drawFeatureText(
    int x,
    int y,
    const char* text
)
{
    if (text == nullptr)
        return;

    drawWatchText(
        5,
        y,
        text,
        1,
        BLACK
    );
}


// ============================================================
// BOLD TEXT
// ============================================================

void drawFeatureBoldText(
    int x,
    int y,
    const char* text
)
{
    if (text == nullptr)
        return;

    /*
     * IMPORTANT:
     *
     * Actual common bold font renderer will be connected here.
     * For now this uses the existing watch font so that this
     * framework does not depend on menu.cpp.
     */

    drawWatchText(
        0,
        39,
        text,
        1,
        BLACK
    );
}


// ============================================================
// NEGATIVE / SELECTED ROW
// ============================================================

void drawFeatureNegativeRow(
    int y,
    const char* text
)
{
    if (text == nullptr)
        return;

    // --------------------------------------------------------
    // Black background
    // --------------------------------------------------------

    display.fillRect(
        0,
        39,
        84,
        9,
        BLACK
    );

    // --------------------------------------------------------
    // White text
    // --------------------------------------------------------

    drawWatchText(
        0,
        y,
        text,
        1,
        WHITE
    );
}


// ============================================================
// COMPLETE FEATURE HEADER
// ============================================================

void drawFeatureHeader(const char* title)
{
    display.clearDisplay();

    drawFeatureTitle(title);

    drawFeatureSeparator();
}