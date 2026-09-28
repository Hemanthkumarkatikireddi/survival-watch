#include <Arduino.h>

#include "display.h"
#include "fonts.h"
#include "state.h"
#include "stopwatch_page.h"


// ============================================================
// STOPWATCH TOGGLE
//
// MENU:
// ZERO + STOPPED  -> START
// RUNNING         -> STOP
// TIME + STOPPED  -> RESUME
// ============================================================

void stopwatchToggle()
{
    if (stopwatchRunning)
    {
        // ----------------------------------------------------
        // STOP
        // Store the current running interval.
        // ----------------------------------------------------

        stopwatchElapsed +=
            millis() - stopwatchStart;

        stopwatchRunning = false;

        stopwatchStart = 0;
    }
    else
    {
        // ----------------------------------------------------
        // START / RESUME
        //
        // IMPORTANT:
        // stopwatchElapsed is NOT cleared.
        // ----------------------------------------------------

        stopwatchStart = millis();

        stopwatchRunning = true;
    }
}


// ============================================================
// STOPWATCH RESET
//
// UP = RESET
// ============================================================

void stopwatchReset()
{
    stopwatchRunning = false;

    stopwatchStart = 0;

    stopwatchElapsed = 0;
}


// ============================================================
// DRAW STOPWATCH TIME
//
// FORMAT:
//
//        00:00:00
//
// MM = minutes
// SS = seconds
// CC = 1/100 second
//
// IMPORTANT:
// Uses the SAME actual number font used by HOME:
//
// drawWatchChar()
// scale = 2
// ============================================================

static void drawStopwatchTime(
    unsigned long minutes,
    unsigned long seconds,
    unsigned long centiseconds
)
{
    const int Y = 18;

    // 9 px digit + 1 px gap
    const int STEP = 10;

    const int X = 2;


    // ========================================================
    // MINUTES
    // ========================================================

    drawStopwatchDigit(
        X,
        Y,
        '0' + (minutes / 10),
        BLACK
    );

    drawStopwatchDigit(
        X + STEP,
        Y,
        '0' + (minutes % 10),
        BLACK
    );


    // ========================================================
    // FIRST COLON
    // ========================================================

    drawWatchChar(
        X + STEP * 2,
        Y,
        ':',
        2,
        BLACK
    );


    // ========================================================
    // SECONDS
    // ========================================================

    drawStopwatchDigit(
        X + STEP * 3,
        Y,
        '0' + (seconds / 10),
        BLACK
    );

    drawStopwatchDigit(
        X + STEP * 4,
        Y,
        '0' + (seconds % 10),
        BLACK
    );


    // ========================================================
    // SECOND COLON
    // ========================================================

    drawWatchChar(
        X + STEP * 5,
        Y,
        ':',
        2,
        BLACK
    );


    // ========================================================
    // CENTISECONDS
    // ========================================================

    drawStopwatchDigit(
        X + STEP * 6,
        Y,
        '0' + (centiseconds / 10),
        BLACK
    );

    drawStopwatchDigit(
        X + STEP * 7,
        Y,
        '0' + (centiseconds % 10),
        BLACK
    );
}

// ============================================================
// STOPWATCH SCREEN
// ============================================================

void drawStopwatchPage()
{
    display.clearDisplay();


    // ========================================================
    // TITLE
    // ========================================================

    drawBoldWatchText(
        2,
        2,
        "STOP WATCH",
        1,
        BLACK
    );


    // ========================================================
    // SEPARATOR
    // ========================================================

    display.drawLine(
        0,
        11,
        83,
        11,
        BLACK
    );


    // ========================================================
    // CURRENT ELAPSED TIME
    // ========================================================

    unsigned long elapsedMs =
        stopwatchElapsed;


    // --------------------------------------------------------
    // If running, add the current active interval.
    // --------------------------------------------------------

    if (stopwatchRunning)
    {
        elapsedMs +=
            millis() - stopwatchStart;
    }


    // ========================================================
    // CONVERT TO CASIO-STYLE VALUES
    //
    // MM : SS : CC
    // ========================================================

    unsigned long minutes =
        (elapsedMs / 60000UL) % 100UL;

    unsigned long seconds =
        (elapsedMs / 1000UL) % 60UL;

    unsigned long centiseconds =
        (elapsedMs / 10UL) % 100UL;


    // ========================================================
    // DRAW TIME
    // ========================================================

    drawStopwatchTime(
        minutes,
        seconds,
        centiseconds
    );


    // ========================================================
    // STATUS
    // ========================================================

    const char* buttonText;


    if (stopwatchRunning)
    {
        buttonText = "STOP";
    }
    else if (elapsedMs > 0)
    {
        buttonText = "RESUME";
    }
    else
    {
        buttonText = "START";
    }


    // ========================================================
    // CASIO-STYLE NEGATIVE BUTTON AREA
    // ========================================================

    display.fillRect(
        0,
        39,
        84,
        9,
        BLACK
    );


    // ========================================================
    // BUTTON TEXT
    // ========================================================

    int buttonWidth =
        watchTextWidth(
            buttonText,
            1
        );


    int textX =
        (84 - buttonWidth) / 2;


    drawBoldWatchText(
        textX-2,
        40,
        buttonText,
        1,
        WHITE
    );


    // ========================================================
    // UPDATE DISPLAY
    // ========================================================

    display.display();
}