#include "menu.h"
#include "display.h"
#include "config.h"
#include "state.h"
#include "fonts.h"

#include "panchang/timezone.h"

#include <Arduino.h>


// =====================================================
// MAIN MENU ITEMS
// =====================================================




// =====================================================
// TIME ZONE MENU ITEMS
// =====================================================

const char* timezoneMenuItems[TIMEZONE_MENU_COUNT] =
{
    "Current Location",
    "Default / India",
    "Japan",
    "United Kingdom",
    "Central Europe",
    "Eastern Europe",
    "China",
    "Australia",
    "New Zealand",
    "South Africa",
    "UAE",
    "Singapore",
    "Thailand",
    "South Korea"
};


// =====================================================
// TIMEZONE MENU STATE
// =====================================================

int timezoneMenuIndex = 0;

int timezoneMenuTop = 0;


// =====================================================
// DRAW MAIN MENU
// =====================================================

void drawMenuScreen()
{
    display.clearDisplay();


    // =================================================
    // TITLE
    // =================================================

    drawWatchText(
        25,
        0,
        "MENU",
        1
    );


    // =================================================
    // SEPARATOR
    // =================================================

    display.drawLine(
        0,
        9,
        83,
        9,
        BLACK
    );


    // =================================================
    // MENU ITEMS
    // =================================================

    for (
        int row = 0;
        row < MENU_VISIBLE;
        row++
    )
    {
        int index =
            menuTop + row;


        if (
            index >= MENU_COUNT
        )
        {
            break;
        }


        int y =
            11 + (row * 9);


        // ---------------------------------------------
        // SELECTED ITEM
        // ---------------------------------------------

        if (
            index == menuIndex
        )
        {
            drawWatchText(
                0,
                y,
                ">",
                1
            );
        }
        else
        {
            drawWatchText(
                0,
                y,
                " ",
                1
            );
        }


        // ---------------------------------------------
        // MENU TEXT
        // ---------------------------------------------

        drawWatchText(
            9,
            y,
            menuItems[index],
            1
        );
    }


    // =================================================
    // SCROLL UP
    // =================================================

    if (
        menuTop > 0
    )
    {
        drawWatchText(
            76,
            10,
            "^",
            1
        );
    }


    // =================================================
    // SCROLL DOWN
    // =================================================

    if (
        menuTop + MENU_VISIBLE <
        MENU_COUNT
    )
    {
        drawWatchText(
            76,
            37,
            "v",
            1
        );
    }


    display.display();
}


// =====================================================
// DRAW TIMEZONE MENU
// =====================================================

void drawTimezoneMenuScreen()
{
    display.clearDisplay();


    // =================================================
    // TITLE
    // =================================================

    drawWatchText(
        20,
        0,
        "TIME ZONES",
        1
    );


    // =================================================
    // SEPARATOR
    // =================================================

    display.drawLine(
        0,
        9,
        83,
        9,
        BLACK
    );


    // =================================================
    // TIMEZONE ITEMS
    // =================================================

    for (
        int row = 0;
        row < TIMEZONE_MENU_VISIBLE;
        row++
    )
    {
        int index =
            timezoneMenuTop + row;


        if (
            index >= TIMEZONE_MENU_COUNT
        )
        {
            break;
        }


        int y =
            11 + (row * 9);


        // ---------------------------------------------
        // SELECTED
        // ---------------------------------------------

        if (
            index == timezoneMenuIndex
        )
        {
            drawWatchText(
                0,
                y,
                ">",
                1
            );
        }
        else
        {
            drawWatchText(
                0,
                y,
                " ",
                1
            );
        }


        // ---------------------------------------------
        // ITEM
        // ---------------------------------------------

        drawWatchText(
            9,
            y,
            timezoneMenuItems[index],
            1
        );
    }


    // =================================================
    // SCROLL UP
    // =================================================

    if (
        timezoneMenuTop > 0
    )
    {
        drawWatchText(
            76,
            10,
            "^",
            1
        );
    }


    // =================================================
    // SCROLL DOWN
    // =================================================

    if (
        timezoneMenuTop +
        TIMEZONE_MENU_VISIBLE <
        TIMEZONE_MENU_COUNT
    )
    {
        drawWatchText(
            76,
            37,
            "v",
            1
        );
    }


    display.display();
}


// =====================================================
// OPEN MAIN MENU ITEM
// =====================================================

void openMenuItem()
{
    switch (
        menuIndex
    )
    {
        case 0:

            // -----------------------------------------
            // TIME ZONES
            // -----------------------------------------

            currentScreen =
                TIMEZONE_PAGE;

            timezoneMenuIndex =
                0;

            timezoneMenuTop =
                0;

            break;


        case 1:

            currentScreen =
                TIME_PAGE;

            break;


        case 2:

            currentScreen =
                ALARM_PAGE;

            break;


        case 3:

            currentScreen =
                STOPWATCH_PAGE;

            break;


        case 4:

            currentScreen =
                COMPASS_PAGE;

            break;


        case 5:

            currentScreen =
                GPS_PAGE;

            break;


        case 6:

            currentScreen =
                SENSOR_PAGE;

            break;


        case 7:

            currentScreen =
                SETTINGS_PAGE;

            break;
    }


    Serial.print(
        "OPEN: "
    );

    Serial.println(
        menuItems[menuIndex]
    );
}


// =====================================================
// MAIN MENU UP
// =====================================================

void menuUp()
{
    menuIndex--;


    if (
        menuIndex < 0
    )
    {
        menuIndex =
            MENU_COUNT - 1;
    }


    if (
        menuIndex < menuTop
    )
    {
        menuTop =
            menuIndex;
    }


    if (
        menuIndex ==
        MENU_COUNT - 1
    )
    {
        menuTop =
            MENU_COUNT -
            MENU_VISIBLE;
    }


    if (
        menuTop < 0
    )
    {
        menuTop = 0;
    }
}


// =====================================================
// MAIN MENU DOWN
// =====================================================

void menuDown()
{
    menuIndex++;


    if (
        menuIndex >= MENU_COUNT
    )
    {
        menuIndex = 0;

        menuTop = 0;

        return;
    }


    if (
        menuIndex >=
        menuTop + MENU_VISIBLE
    )
    {
        menuTop =
            menuIndex -
            MENU_VISIBLE +
            1;
    }
}


// =====================================================
// TIMEZONE MENU UP
// =====================================================

void timezoneMenuUp()
{
    timezoneMenuIndex--;


    if (
        timezoneMenuIndex < 0
    )
    {
        timezoneMenuIndex =
            TIMEZONE_MENU_COUNT - 1;
    }


    if (
        timezoneMenuIndex <
        timezoneMenuTop
    )
    {
        timezoneMenuTop =
            timezoneMenuIndex;
    }


    if (
        timezoneMenuIndex ==
        TIMEZONE_MENU_COUNT - 1
    )
    {
        timezoneMenuTop =
            TIMEZONE_MENU_COUNT -
            TIMEZONE_MENU_VISIBLE;
    }


    if (
        timezoneMenuTop < 0
    )
    {
        timezoneMenuTop = 0;
    }
}


// =====================================================
// TIMEZONE MENU DOWN
// =====================================================

void timezoneMenuDown()
{
    timezoneMenuIndex++;


    if (
        timezoneMenuIndex >=
        TIMEZONE_MENU_COUNT
    )
    {
        timezoneMenuIndex = 0;

        timezoneMenuTop = 0;

        return;
    }


    if (
        timezoneMenuIndex >=
        timezoneMenuTop +
        TIMEZONE_MENU_VISIBLE
    )
    {
        timezoneMenuTop =
            timezoneMenuIndex -
            TIMEZONE_MENU_VISIBLE +
            1;
    }
}


// =====================================================
// SELECT TIMEZONE
// =====================================================

void openTimezoneItem()
{
    Serial.print("TIMEZONE SELECT: ");
    Serial.println(
        timezoneMenuItems[timezoneMenuIndex]
    );

    // Current Location
    if (timezoneMenuIndex == 0)
    {
        setTimezoneMode(
            TIMEZONE_CURRENT_LOCATION
        );

        return;
    }

    // Default / India
    if (timezoneMenuIndex == 1)
    {
        setManualTimezoneIndex(0);
        return;
    }

    // Remaining entries:
    // Menu index 2 = timezoneRegions[1] = Japan
    // Menu index 3 = timezoneRegions[2] = United Kingdom
    // ...
    int manualIndex =
        timezoneMenuIndex - 1;

    setManualTimezoneIndex(
        manualIndex
    );
}