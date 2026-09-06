#include "menu.h"
#include "display.h"
#include "config.h"
#include "state.h"
#include "fonts.h"


// =====================================================
// DRAW MENU SCREEN
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
// OPEN SELECTED MENU ITEM
// =====================================================

void openMenuItem()
{
    switch (
        menuIndex
    )
    {
        case 0:

            currentScreen =
                TIME_PAGE;

            break;


        case 1:

            currentScreen =
                ALARM_PAGE;

            break;


        case 2:

            currentScreen =
                STOPWATCH_PAGE;

            break;


        case 3:

            currentScreen =
                COMPASS_PAGE;

            break;


        case 4:

            currentScreen =
                GPS_PAGE;

            break;


        case 5:

            currentScreen =
                SENSOR_PAGE;

            break;


        case 6:

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
// MENU UP
// =====================================================

void menuUp()
{
    menuIndex--;


    // -----------------------------------------------
    // WRAP TO LAST ITEM
    // -----------------------------------------------

    if (
        menuIndex < 0
    )
    {
        menuIndex =
            MENU_COUNT - 1;
    }


    // -----------------------------------------------
    // MOVE TOP WHEN NEEDED
    // -----------------------------------------------

    if (
        menuIndex < menuTop
    )
    {
        menuTop =
            menuIndex;
    }


    // -----------------------------------------------
    // LAST ITEM
    // -----------------------------------------------

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
// MENU DOWN
// =====================================================

void menuDown()
{
    menuIndex++;


    // -----------------------------------------------
    // WRAP TO FIRST ITEM
    // -----------------------------------------------

    if (
        menuIndex >= MENU_COUNT
    )
    {
        menuIndex = 0;
        menuTop = 0;

        return;
    }


    // -----------------------------------------------
    // SCROLL MENU
    // -----------------------------------------------

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