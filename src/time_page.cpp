#include "time_page.h"
#include "display.h"
#include "fonts.h"
#include "state.h"

#include <Arduino.h>


// ============================================================
// MODIFY TIME DISPLAY
// SAME NEGATIVE DIGIT STYLE AS ALARM PAGE
// ============================================================

static void drawModifyTime(
    int x,
    int y,
    int hour,
    int minute,
    bool selectHour,
    bool selectMinute
)
{
    // --------------------------------------------------------
    // 12 HOUR DISPLAY
    // --------------------------------------------------------

    int displayHour = hour % 12;

    if (displayHour == 0)
    {
        displayHour = 12;
    }


    int h1 = displayHour / 10;
    int h2 = displayHour % 10;

    int m1 = minute / 10;
    int m2 = minute % 10;


    const uint8_t SCALE = 2;


    // --------------------------------------------------------
    // EXACT SAME POSITIONS AS HOME drawBoldTime()
    // --------------------------------------------------------

    int xH1 = x;
    int xH2 = x + 12;

    int xColon = x + 24;

    int xM1 = x + 36;
    int xM2 = x + 48;


    // ========================================================
    // HOURS
    // ========================================================

    if (selectHour)
    {
        // Same negative selection style as ALARM
        display.fillRect(
            xH1,
            y - 1,
            24,
            16,
            BLACK
        );

        // White digits
        drawWatchChar(
            xH1,
            y,
            '0' + h1,
            SCALE,
            WHITE
        );

        drawWatchChar(
            xH2,
            y,
            '0' + h2,
            SCALE,
            WHITE
        );
    }
    else
    {
        drawBoldDigit(
            xH1,
            y,
            h1,
            SCALE
        );

        drawBoldDigit(
            xH2,
            y,
            h2,
            SCALE
        );
    }


    // ========================================================
    // COLON
    // ========================================================

    drawBoldColon(
        xColon,
        y,
        SCALE
    );


    // ========================================================
    // MINUTES
    // ========================================================

    if (selectMinute)
    {
        // Same negative selection style as ALARM
        display.fillRect(
            xM1 - 1,
            y - 1,
            24,
            16,
            BLACK
        );

        // White digits
        drawWatchChar(
            xM1,
            y,
            '0' + m1,
            SCALE,
            WHITE
        );

        drawWatchChar(
            xM2,
            y,
            '0' + m2,
            SCALE,
            WHITE
        );
    }
    else
    {
        drawBoldDigit(
            xM1,
            y,
            m1,
            SCALE
        );

        drawBoldDigit(
            xM2,
            y,
            m2,
            SCALE
        );
    }
}


// ============================================================
// MODIFY TIME PAGE
// ============================================================

void drawTimePage()
{
    display.clearDisplay();


    // ========================================================
    // TITLE
    // ========================================================

    drawBoldWatchText(
        10,
        2,
        "SET TIME",
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
    // CURRENT / EDITING TIME
    // ========================================================

    DateTime now = rtc.now();


    int hourToDisplay;
    bool pm;


    if (timeSetting)
    {
        // ----------------------------------------------------
        // During editing use selected values
        // ----------------------------------------------------

        int editHour24;

        if (setAMPM == 0)
        {
            editHour24 = setHour % 12;
        }
        else
        {
            editHour24 = (setHour % 12) + 12;
        }

        hourToDisplay = setHour;
        pm = (setAMPM == 1);
    }
    else
    {
        // ----------------------------------------------------
        // Normal running clock
        // ----------------------------------------------------

        getDisplayHour(
            now.hour(),
            hourToDisplay,
            pm
        );
    }


    // ========================================================
    // TIME
    // ========================================================

    if (timeSetting)
    {
        drawModifyTime(
            2,
            18,
            (setAMPM == 0)
                ? (setHour % 12)
                : ((setHour % 12) + 12),
            setMinute,

            timeSetStep == 0,
            timeSetStep == 1
        );
    }
    else
    {
        drawBoldTime(
            2,
            18,
            hourToDisplay,
            now.minute()
        );
    }


    // ========================================================
    // SECONDS
    // EXACT SAME POSITION AS HOME
    // ========================================================

    drawSecondsDigit(
        66,
        25,
        now.second() / 10
    );

    drawSecondsDigit(
        72,
        25,
        now.second() % 10
    );


    // ========================================================
    // AM / PM
    // ========================================================

    bool showPM;

    if (timeSetting)
    {
        showPM = (setAMPM == 1);
    }
    else
    {
        showPM = pm;
    }


    if (
        timeSetting &&
        timeSetStep == 2
    )
    {
        // Same negative style as ALARM AM/PM

        display.fillRect(
            65,
            14,
            17,
            9,
            BLACK
        );

        drawBoldWatchText(
            66,
            15,
            showPM ? "PM" : "AM",
            1,
            WHITE
        );
    }
    else
    {
        drawBoldWatchText(
            66,
            15,
            showPM ? "PM" : "AM",
            1,
            BLACK
        );
    }


    // ========================================================
    // SELECTION BAR
    // ========================================================

    if (timeSetting)
    {
        display.fillRect(
            0,
            39,
            84,
            9,
            BLACK
        );


        if (timeSetStep == 0)
        {
            drawBoldWatchText(
                22,
                40,
                "HOURS",
                1,
                WHITE
            );
        }
        else if (timeSetStep == 1)
        {
            drawBoldWatchText(
                16,
                40,
                "MINUTES",
                1,
                WHITE
            );
        }
        else if (timeSetStep == 2)
          {
              drawBoldWatchText(
                  14,
                  40,
                  "(AM/PM)",
                  1,
                  WHITE
              );
              }
              else
              {
                  drawBoldWatchText(
                      35,
                      40,
                      "OK",
                      1,
                      WHITE
                  );
              }
    }


    display.display();
}


// ============================================================
// START TIME SETTING
// ============================================================

void startTimeSetting()
{
    DateTime now = rtc.now();


    setAMPM =
        now.hour() >= 12 ? 1 : 0;


    setHour =
        now.hour() % 12;


    if (setHour == 0)
    {
        setHour = 12;
    }


    setMinute =
        now.minute();


    timeSetStep = 0;

    timeSetting = true;


    Serial.println(
        "TIME SETTING START"
    );
}


// ============================================================
// TIME VALUE UP
// ============================================================

void timeValueUp()
{
    if (!timeSetting)
    {
        return;
    }


    // --------------------------------------------------------
    // HOURS
    // --------------------------------------------------------

    if (timeSetStep == 0)
    {
        setHour++;

        if (setHour > 12)
        {
            setHour = 1;
        }

        return;
    }


    // --------------------------------------------------------
    // MINUTES
    // --------------------------------------------------------

    if (timeSetStep == 1)
    {
        setMinute++;

        if (setMinute > 59)
        {
            setMinute = 0;
        }

        return;
    }


    // --------------------------------------------------------
    // AM / PM
    // --------------------------------------------------------

    if (timeSetStep == 2)
    {
        setAMPM =
            !setAMPM;

        return;
    }
}


// ============================================================
// TIME VALUE DOWN
// ============================================================

void timeValueDown()
{
    if (!timeSetting)
    {
        return;
    }


    // --------------------------------------------------------
    // HOURS
    // --------------------------------------------------------

    if (timeSetStep == 0)
    {
        setHour--;

        if (setHour < 1)
        {
            setHour = 12;
        }

        return;
    }


    // --------------------------------------------------------
    // MINUTES
    // --------------------------------------------------------

    if (timeSetStep == 1)
    {
        setMinute--;

        if (setMinute < 0)
        {
            setMinute = 59;
        }

        return;
    }


    // --------------------------------------------------------
    // AM / PM
    // --------------------------------------------------------

    if (timeSetStep == 2)
    {
        setAMPM =
            !setAMPM;

        return;
    }
}


// ============================================================
// MENU = NEXT SELECTION
// ============================================================

void timeNextStep()
{
    if (!timeSetting)
        return;

    if (timeSetStep < 3)
    {
        timeSetStep++;
        return;
    }

    // ==========================================
    // OK selected -> MENU confirms the new time
    // ==========================================

    saveTime();

    currentScreen = HOME;

    Serial.println("TIME CONFIRMED -> HOME");
}


// ============================================================
// SAVE TIME
// ============================================================

void saveTime()
{
    if (!timeSetting)
    {
        return;
    }


    // --------------------------------------------------------
    // Convert 12-hour setting to 24-hour
    // --------------------------------------------------------

    int hour24;

    if (setAMPM == 0)
    {
        hour24 = setHour % 12;
    }
    else
    {
        hour24 = (setHour % 12) + 12;
    }


    // --------------------------------------------------------
    // Read CURRENT RTC time
    // --------------------------------------------------------

    DateTime now = rtc.now();


    // --------------------------------------------------------
    // Save new hour/minute
    //
    // IMPORTANT:
    // Keep current seconds.
    // Do NOT reset seconds to 0.
    // --------------------------------------------------------

    rtc.adjust(
        DateTime(
            now.year(),
            now.month(),
            now.day(),
            hour24,
            setMinute,
            now.second()
        )
    );


    // --------------------------------------------------------
    // Exit setting mode
    // --------------------------------------------------------

    timeSetting = false;
    timeSetStep = 0;


    Serial.println(
        "TIME UPDATED"
    );
}