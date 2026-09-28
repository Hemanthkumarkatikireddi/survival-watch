#include "menu.h"
#include "display.h"
#include "config.h"
#include "state.h"
#include "fonts.h"

#include "panchang/timezone.h"
#include "time_page.h"
#include <Arduino.h>


// ============================================================
// MENU HORIZONTAL SCROLL
// ============================================================

int menuScrollX = 0;

unsigned long menuScrollTimer = 0;

const unsigned long MENU_SCROLL_INTERVAL = 120;

bool menuScrollPause = false;

unsigned long menuScrollPauseTimer = 0;

const unsigned long MENU_SCROLL_PAUSE = 700;


// ============================================================
// RESET MAIN MENU HORIZONTAL SCROLL
// ============================================================

void resetMenuScroll()
{
    menuScrollX = 0;

    menuScrollTimer = millis();

    menuScrollPause = false;

    menuScrollPauseTimer = millis();
}


// =====================================================
// TIME ZONE MENU ITEMS
// =====================================================

const char* timezoneMenuItems[TIMEZONE_MENU_COUNT] =
{
    "Current Location",
    "Default/India",
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


// ============================================================
// SUB MENU COMMON STATE
// ============================================================

int subMenuIndex = 0;

int subMenuTop = 0;

int subMenuScrollX = 0;

unsigned long subMenuScrollTimer = 0;

unsigned long subMenuSelectedTime = 0;

int subMenuLastIndex = -1;

const unsigned long SUBMENU_SCROLL_INTERVAL = 120;

const unsigned long SUBMENU_SCROLL_START_DELAY = 3000;


// ============================================================
// RESET SUB MENU HORIZONTAL SCROLL
// ============================================================

void resetSubMenuScroll()
{
    subMenuScrollX = 0;

    subMenuScrollTimer = millis();

    subMenuSelectedTime = millis();

    subMenuLastIndex = -1;
}


// ============================================================
// UPDATE MAIN MENU HORIZONTAL SCROLL
// ============================================================

void updateMenuScroll()
{
    static int lastMenuIndex = -1;
    static unsigned long selectedTime = 0;

    const char* text = menuItems[menuIndex];

    if (text == nullptr)
    {
        menuScrollX = 0;
        return;
    }


    // =========================================================
    // NEW OPTION SELECTED
    // =========================================================

    if (menuIndex != lastMenuIndex)
    {
        lastMenuIndex = menuIndex;

        menuScrollX = 0;

        selectedTime = millis();

        menuScrollTimer = millis();

        return;
    }


    // =========================================================
    // MAIN MENU BOLD FONT
    //
    // 7 px glyph + 1 px gap = 8 px
    // =========================================================

    int textWidth = strlen(text) * 8;


    // =========================================================
    // AVAILABLE MENU WIDTH
    // =========================================================

    const int availableWidth = 74;


    // =========================================================
    // TEXT FITS
    // =========================================================

    if (textWidth <= availableWidth)
    {
        menuScrollX = 0;

        return;
    }


    // =========================================================
    // WAIT 3 SECONDS
    // =========================================================

    const unsigned long SCROLL_START_DELAY = 3000;

    if (millis() - selectedTime < SCROLL_START_DELAY)
    {
        menuScrollX = 0;

        return;
    }


    // =========================================================
    // SCROLL SPEED
    // =========================================================

    if (
        millis() - menuScrollTimer <
        MENU_SCROLL_INTERVAL
    )
    {
        return;
    }

    menuScrollTimer = millis();

    menuScrollX++;


    // =========================================================
    // COMPLETE MARQUEE CYCLE
    // =========================================================

    int cycleWidth = textWidth + 8;

    if (menuScrollX >= cycleWidth)
    {
        menuScrollX = 0;

        selectedTime = millis();
    }
}


// ============================================================
// UPDATE SUB MENU HORIZONTAL SCROLL
// ============================================================
//
// IMPORTANT:
//
// Sub Menu uses NEW NORMAL FONT.
//
// Glyph = 5 x 7
// Gap   = 1 pixel
// Advance = 6 pixels
//
// ============================================================

void updateSubMenuScroll(
    const char* text
)
{
    static const char* lastText = nullptr;
    static unsigned long selectedTime = 0;

    if (text == nullptr)
    {
        subMenuScrollX = 0;

        return;
    }


    // =========================================================
    // NEW SUB-MENU OPTION
    // =========================================================

    if (text != lastText)
    {
        lastText = text;

        subMenuScrollX = 0;

        selectedTime = millis();

        subMenuSelectedTime = millis();

        subMenuScrollTimer = millis();

        return;
    }


    // =========================================================
    // SUB MENU NORMAL FONT
    //
    // 5 px glyph + 1 px gap = 6 px
    // =========================================================

    int textWidth = strlen(text) * 6;

    const int availableWidth = 74;


    // =========================================================
    // TEXT FITS
    // =========================================================

    if (textWidth <= availableWidth)
    {
        subMenuScrollX = 0;

        return;
    }


    // =========================================================
    // WAIT 3 SECONDS
    // =========================================================

    if (
        millis() - selectedTime <
        SUBMENU_SCROLL_START_DELAY
    )
    {
        subMenuScrollX = 0;

        return;
    }


    // =========================================================
    // SCROLL SPEED
    // =========================================================

    if (
        millis() - subMenuScrollTimer <
        SUBMENU_SCROLL_INTERVAL
    )
    {
        return;
    }

    subMenuScrollTimer = millis();

    subMenuScrollX++;


    // =========================================================
    // COMPLETE MARQUEE CYCLE
    // =========================================================

    int cycleWidth = textWidth + 6;

    if (subMenuScrollX >= cycleWidth)
    {
        subMenuScrollX = 0;

        selectedTime = millis();

        subMenuSelectedTime = millis();
    }
}


// ============================================================
// DRAW MAIN MENU BOLD TEXT WITH CLIPPING
// ============================================================
//
// Main Menu:
//
// Font = Bold 7x7
// Advance = 8 px
//
// ============================================================

void drawMenuBoldTextClipped(
    int x,
    int y,
    const char* text,
    uint8_t scale,
    uint8_t color
)
{
    if (text == nullptr || scale == 0)
    {
        return;
    }


    const int CLIP_LEFT  = 10;
    const int CLIP_RIGHT = 83;


    while (*text)
    {
        int charX = x;


        // =====================================================
        // BOLD FONT
        //
        // 7 x 7 glyph
        // =====================================================

        int charWidth = 7 * scale;


        // =====================================================
        // COMPLETE CHARACTER ONLY
        // =====================================================

        if (
            charX >= CLIP_LEFT &&
            charX + charWidth - 1 <= CLIP_RIGHT
        )
        {
            drawBoldWatchChar(
                charX,
                y,
                *text,
                scale,
                color
            );
        }


        // =====================================================
        // 7 px glyph + 1 px gap
        // =====================================================

        x += 8 * scale;

        text++;
    }
}


// ============================================================
// DRAW MAIN MENU
// ============================================================

void drawMenuScreen()
{
    updateMenuScroll();

    display.clearDisplay();


    // ============================================================
    // TITLE
    // ============================================================

    drawBoldWatchText(
        25,
        2,
        "MENU",
        1,
        BLACK
    );


    // ============================================================
    // SEPARATOR
    // ============================================================

    display.drawLine(
        0,
        11,
        83,
        11,
        BLACK
    );


    // ============================================================
    // MENU LAYOUT
    // ============================================================

    const int ROW_HEIGHT    = 10;
    const int ROW_START_Y   = 15;
    const int ROW_BG_HEIGHT = 10;


    // ============================================================
    // MENU ITEMS
    // ============================================================

    for (
        int row = 0;
        row < MENU_VISIBLE;
        row++
    )
    {
        int index =
            menuTop + row;


        if (index >= MENU_COUNT)
        {
            break;
        }


        int y =
            ROW_START_Y +
            (row * ROW_HEIGHT);


        // ========================================================
        // SELECTED OPTION
        // ========================================================

        if (index == menuIndex)
        {
            // ----------------------------------------------------
            // BLACK BACKGROUND
            // ----------------------------------------------------

            display.fillRect(
                0,
                y,
                84,
                ROW_BG_HEIGHT,
                BLACK
            );


            // ----------------------------------------------------
            // CURSOR
            // ----------------------------------------------------

            drawWatchText(
                1,
                y + 1,
                ">",
                1,
                WHITE
            );


            // ----------------------------------------------------
            // SELECTED MENU TEXT
            // ----------------------------------------------------

            const int TEXT_START_X = 10;

            const int TEXT_AREA_W = 74;

            const int TEXT_GAP = 14;


            // ----------------------------------------------------
            // BOLD FONT WIDTH
            // 7x7 + 1 gap = 8
            // ----------------------------------------------------

            int textWidth =
                strlen(menuItems[index]) * 8;


            int scrollX =
                menuScrollX;


            int textX =
                TEXT_START_X -
                scrollX;


            // ----------------------------------------------------
            // FIRST COPY
            // ----------------------------------------------------

            drawMenuBoldTextClipped(
                textX,
                y + 1,
                menuItems[index],
                1,
                WHITE
            );


            // ----------------------------------------------------
            // SECOND COPY
            // ----------------------------------------------------

            if (textWidth > TEXT_AREA_W)
            {
                drawMenuBoldTextClipped(
                    textX +
                    textWidth +
                    TEXT_GAP,
                    y + 1,
                    menuItems[index],
                    1,
                    WHITE
                );
            }
        }


        // ========================================================
        // NORMAL OPTION
        // ========================================================

        else
        {
            drawMenuBoldTextClipped(
                10,
                y + 1,
                menuItems[index],
                1,
                BLACK
            );
        }
    }


    // ============================================================
    // UPDATE DISPLAY
    // ============================================================

    display.display();
}


// ============================================================
// DRAW SUB MENU NORMAL TEXT WITH CLIPPING
// ============================================================
//
// SUB MENU FONT:
//
// 5 x 7 glyph
// 1 px gap
// 6 px advance
//
// This is the NEW font family from fonts.cpp.
//
// Text area:
// x = 10 ... 83
//
// Complete characters only are drawn.
// Therefore no half-letter enters the cursor area.
//
// ============================================================

void drawSubMenuTextClipped(
    int x,
    int y,
    const char* text,
    uint8_t scale,
    uint8_t color
)
{
    if (
        text == nullptr ||
        scale == 0
    )
    {
        return;
    }


    const int CLIP_LEFT  = 10;
    const int CLIP_RIGHT = 83;


    while (*text)
    {
        int charX = x;


        // =====================================================
        // NEW NORMAL FONT
        //
        // 5 x 7
        // =====================================================

        int charWidth =
            5 * scale;


        // =====================================================
        // DRAW ONLY COMPLETE CHARACTER
        // =====================================================

        if (
            charX >= CLIP_LEFT &&
            charX + charWidth - 1 <= CLIP_RIGHT
        )
        {
            drawWatchChar(
                charX,
                y,
                *text,
                scale,
                color
            );
        }


        // =====================================================
        // 5 px glyph + 1 px gap
        // =====================================================

        x += 6 * scale;

        text++;
    }
}

// ============================================================
// DRAW CLOCK MENU
// ============================================================
//
// CLOCK
//   > Time Zones
//     Alarms
//     Stopwatch
//
// Sub-menu font:
// 5 x 7 normal font
// 1 px gap
// 6 px advance
//
// ============================================================

void drawClockMenuScreen()
{
    // =========================================================
    // CURRENT SELECTED TEXT
    // =========================================================

    const char* selectedText =
        clockMenuItems[
            clockMenuIndex
        ];


    // =========================================================
    // UPDATE SUB MENU SCROLL
    // =========================================================

    updateSubMenuScroll(
        selectedText
    );


    // =========================================================
    // CLEAR DISPLAY
    // =========================================================

    display.clearDisplay();


    // =========================================================
    // TITLE
    // =========================================================

    drawBoldWatchText(
        22,
        2,
        "CLOCK",
        1,
        BLACK
    );


    // =========================================================
    // SEPARATOR
    // =========================================================

    display.drawLine(
        0,
        11,
        83,
        11,
        BLACK
    );


    // =========================================================
    // MENU LAYOUT
    // =========================================================

    const int ROW_HEIGHT    = 10;

    const int ROW_START_Y   = 15;

    const int ROW_BG_HEIGHT = 10;


    // =========================================================
    // CLOCK MENU ITEMS
    // =========================================================

    for (
        int row = 0;
        row < CLOCK_MENU_VISIBLE;
        row++
    )
    {
        int index =
            clockMenuTop +
            row;


        if (
            index >=
            CLOCK_MENU_COUNT
        )
        {
            break;
        }


        int y =
            ROW_START_Y +
            (row * ROW_HEIGHT);


        // =====================================================
        // SELECTED OPTION
        // =====================================================

        if (
            index ==
            clockMenuIndex
        )
        {
            // -------------------------------------------------
            // BLACK SELECTED BACKGROUND
            // -------------------------------------------------

            display.fillRect(
                0,
                y,
                84,
                ROW_BG_HEIGHT,
                BLACK
            );


            // -------------------------------------------------
            // CURSOR
            // -------------------------------------------------

            drawWatchText(
                1,
                y + 1,
                ">",
                1,
                WHITE
            );


            // -------------------------------------------------
            // SELECTED TEXT
            //
            // NEW NORMAL 5x7 FONT
            // -------------------------------------------------

            const int TEXT_START_X = 10;


            int textX =
                TEXT_START_X -
                subMenuScrollX;


            drawSubMenuTextClipped(
                textX,
                y + 1,
                clockMenuItems[index],
                1,
                WHITE
            );


            // -------------------------------------------------
            // SECOND COPY FOR MARQUEE
            // -------------------------------------------------

            int textWidth =
                strlen(
                    clockMenuItems[index]
                ) * 6;


            const int TEXT_AREA_W = 74;

            const int TEXT_GAP = 14;


            if (
                textWidth >
                TEXT_AREA_W
            )
            {
                drawSubMenuTextClipped(
                    textX +
                    textWidth +
                    TEXT_GAP,
                    y + 1,
                    clockMenuItems[index],
                    1,
                    WHITE
                );
            }
        }


        // =====================================================
        // NORMAL OPTION
        // =====================================================

        else
        {
            drawSubMenuTextClipped(
                10,
                y + 1,
                clockMenuItems[index],
                1,
                BLACK
            );
        }
    }


    // =========================================================
    // UPDATE DISPLAY
    // =========================================================

    display.display();
}


// ============================================================
// CLOCK MENU UP
// ============================================================

void clockMenuUp()
{
    clockMenuIndex--;


    // =========================================================
    // WRAP
    // =========================================================

    if (
        clockMenuIndex < 0
    )
    {
        clockMenuIndex =
            CLOCK_MENU_COUNT - 1;
    }


    // =========================================================
    // RESET SCROLL
    // =========================================================

    resetSubMenuScroll();


    // =========================================================
    // MOVE TOP
    // =========================================================

    if (
        clockMenuIndex <
        clockMenuTop
    )
    {
        clockMenuTop =
            clockMenuIndex;
    }


    // =========================================================
    // LAST ITEM
    // =========================================================

    if (
        clockMenuIndex ==
        CLOCK_MENU_COUNT - 1
    )
    {
        clockMenuTop =
            CLOCK_MENU_COUNT -
            CLOCK_MENU_VISIBLE;
    }


    // =========================================================
    // SAFETY
    // =========================================================

    if (
        clockMenuTop < 0
    )
    {
        clockMenuTop = 0;
    }
}


// ============================================================
// CLOCK MENU DOWN
// ============================================================

void clockMenuDown()
{
    clockMenuIndex++;


    // =========================================================
    // WRAP
    // =========================================================

    if (
        clockMenuIndex >=
        CLOCK_MENU_COUNT
    )
    {
        clockMenuIndex = 0;

        clockMenuTop = 0;

        resetSubMenuScroll();

        return;
    }


    // =========================================================
    // RESET SCROLL
    // =========================================================

    resetSubMenuScroll();


    // =========================================================
    // MOVE TOP
    // =========================================================

    if (
        clockMenuIndex >=
        clockMenuTop +
        CLOCK_MENU_VISIBLE
    )
    {
        clockMenuTop =
            clockMenuIndex -
            CLOCK_MENU_VISIBLE +
            1;
    }
}


// ============================================================
// SELECT CLOCK MENU ITEM
// ============================================================

void openClockMenuItem()
{
    Serial.print(
        "CLOCK MENU SELECT: "
    );

    Serial.println(
        clockMenuItems[
            clockMenuIndex
        ]
    );


    // =========================================================
    // TIME ZONES
    // =========================================================

    if (
        clockMenuIndex == 0
    )
    {
        currentScreen =
            TIMEZONE_PAGE;


        timezoneMenuIndex =
            0;


        timezoneMenuTop =
            0;


        resetSubMenuScroll();

        return;
    }


    // ============================================================
    // ALARMS
    // ============================================================

    if (
        clockMenuIndex == 1
    )
    {
        currentScreen =
            ALARM_MENU_PAGE;

        alarmMenuIndex =
            0;

        alarmMenuTop =
            0;

        resetSubMenuScroll();

        Serial.println(
            "CLOCK -> ALARMS"
        );

        return;
    }


    // =========================================================
    // STOPWATCH
    // =========================================================

    if (
        clockMenuIndex == 2
    )
    {
        currentScreen =
            STOPWATCH_PAGE;

        return;
    }


    // =========================================================
    // TIME
    // =========================================================

    if (
        clockMenuIndex == 3
    )
    {
        startTimeSetting();
        currentScreen =
            TIME_PAGE;

        return;
    }
}


// =====================================================
// DRAW TIMEZONE MENU
// =====================================================

void drawTimezoneMenuScreen()
{
    // =========================================================
    // CURRENT SELECTED TEXT
    // =========================================================

    const char* selectedText =
        timezoneMenuItems[
            timezoneMenuIndex
        ];


    // =========================================================
    // UPDATE SUB MENU SCROLL
    // =========================================================

    updateSubMenuScroll(
        selectedText
    );


    // =========================================================
    // CLEAR DISPLAY
    // =========================================================

    display.clearDisplay();


    // =========================================================
    // TITLE
    // =========================================================

    drawBoldWatchText(
        3,
        2,
        "TIME ZONES",
        1,
        BLACK
    );


    // =========================================================
    // SEPARATOR
    // ============================================================

    display.drawLine(
        0,
        11,
        83,
        11,
        BLACK
    );


    // =========================================================
    // MENU LAYOUT
    // =========================================================

    const int ROW_HEIGHT    = 11;

    const int ROW_START_Y   = 15;

    const int ROW_BG_HEIGHT = 10;


    // =========================================================
    // TIMEZONE ITEMS
    // =========================================================

    for (
        int row = 0;
        row < TIMEZONE_MENU_VISIBLE;
        row++
    )
    {
        int index =
            timezoneMenuTop +
            row;


        if (
            index >=
            TIMEZONE_MENU_COUNT
        )
        {
            break;
        }


        int y =
            ROW_START_Y +
            (row * ROW_HEIGHT);


        // =====================================================
        // SELECTED OPTION
        // =====================================================

        if (
            index ==
            timezoneMenuIndex
        )
        {
            // -------------------------------------------------
            // BLACK SELECTED BACKGROUND
            // -------------------------------------------------

            display.fillRect(
                0,
                y,
                84,
                ROW_BG_HEIGHT,
                BLACK
            );


            // -------------------------------------------------
            // CURSOR
            // -------------------------------------------------

            drawWatchText(
                1,
                y + 1,
                ">",
                1,
                WHITE
            );


            // -------------------------------------------------
            // SELECTED TEXT
            //
            // NEW NORMAL 5x7 FONT
            // -------------------------------------------------

            const int TEXT_START_X = 10;


            int textX =
                TEXT_START_X -
                subMenuScrollX;


            drawSubMenuTextClipped(
                textX,
                y + 1,
                timezoneMenuItems[index],
                1,
                WHITE
            );


            // -------------------------------------------------
            // SECOND COPY FOR MARQUEE
            //
            // NORMAL FONT:
            // 5 px glyph + 1 px gap = 6 px
            // -------------------------------------------------

            int textWidth =
                strlen(
                    timezoneMenuItems[index]
                ) * 6;


            const int TEXT_AREA_W = 74;

            const int TEXT_GAP = 14;


            if (
                textWidth >
                TEXT_AREA_W
            )
            {
                drawSubMenuTextClipped(
                    textX +
                    textWidth +
                    TEXT_GAP,
                    y + 1,
                    timezoneMenuItems[index],
                    1,
                    WHITE
                );
            }
        }


        // =====================================================
        // NORMAL OPTION
        // =====================================================

        else
        {
            drawSubMenuTextClipped(
                10,
                y + 1,
                timezoneMenuItems[index],
                1,
                BLACK
            );
        }
    }


    // =========================================================
    // UPDATE DISPLAY
    // =========================================================

    display.display();
}

// ============================================================
// DRAW ALARM TIME
// ============================================================

static void drawAlarmTimeText(
    int x,
    int y,
    int index,
    uint8_t color
)
{
    if (
        index < 0 ||
        index >= 4
    )
    {
        return;
    }


    char timeText[12];


    uint8_t hour =
        alarms[index].hour;

    uint8_t minute =
        alarms[index].minute;


    bool pm =
        (hour >= 12);


    uint8_t displayHour =
        hour % 12;


    if (
        displayHour == 0
    )
    {
        displayHour = 12;
    }


    snprintf(
        timeText,
        sizeof(timeText),
        "%d:%02d%s",
        displayHour,
        minute,
        pm ? "pm" : "am"
    );


    // --------------------------------------------------------
    // TIME
    // --------------------------------------------------------

    drawWatchText(
        x,
        y,
        timeText,
        1,
        color
    );

}

// ============================================================
// ALARM ACTIVE BOX
// 1px OUTLINE BOX AT RIGHT SIDE
// ============================================================

static void drawAlarmActiveBox(
    int y,
    bool active,
    bool selected
)
{
    if (!active)
    {
        return;
    }

    // --------------------------------------------------------
    // Alarm option row:
    //
    // row height = 11 px
    // box height = 7 px
    //
    // 2 px top
    // 7 px box
    // 2 px bottom
    // --------------------------------------------------------

    const int BOX_X = 75;
    const int BOX_Y = y + 1;
    const int BOX_W = 7;
    const int BOX_H = 7;


    // --------------------------------------------------------
    // Selected option = negative display
    // --------------------------------------------------------

    if (selected)
    {
        // White box on black selection
        display.fillRect(
            BOX_X,
            BOX_Y,
            BOX_W,
            BOX_H,
            WHITE
        );
    }
    else
    {
        // Black 1px outline box
        display.drawRect(
            BOX_X,
            BOX_Y,
            BOX_W,
            BOX_H,
            BLACK
        );
    }
}

// ============================================================
// ALARM MENU DRAW
// ============================================================

void drawAlarmMenuScreen()
{
    display.clearDisplay();

    // --------------------------------------------------------
    // TITLE
    // --------------------------------------------------------

    drawBoldWatchText(
        20,
        2,
        "ALARMS",
        1,
        BLACK
    );

    // --------------------------------------------------------
    // SEPARATOR
    // --------------------------------------------------------

    display.drawLine(
        0,
        11,
        83,
        11,
        BLACK
    );

    // --------------------------------------------------------
    // ROW SETTINGS
    // --------------------------------------------------------

    const int ROW_HEIGHT = 11;
    const int ROW_BG_HEIGHT = 10;
    const int ROW_START_Y = 13;

    for (
        int row = 0;
        row < 3;
        row++
    )
    {
        int index =
            alarmMenuTop + row;

        if (index < 0 || index >= 5)
        {
            continue;
        }

        int y =
            ROW_START_Y +
            row * ROW_HEIGHT;

        bool selected =
            (index == alarmMenuIndex);

        // ----------------------------------------------------
        // SELECTED BACKGROUND
        // ----------------------------------------------------

        if (selected)
        {
            display.fillRect(
                0,
                y,
                84,
                ROW_BG_HEIGHT,
                BLACK
            );
        }

        uint8_t color =
            selected ? WHITE : BLACK;

        // ----------------------------------------------------
        // ARROW
        // ----------------------------------------------------

        drawWatchText(
            1,
            y + 1,
            ">",
            1,
            color
        );

        // ----------------------------------------------------
        // ALARM 1-4
        // ----------------------------------------------------

        if (index < 4)
        {
            if (
                alarms[index].hour == 0 &&
                alarms[index].minute == 0 &&
                !alarms[index].enabled
            )
            {
                drawWatchText(
                    10,
                    y + 1,
                    "--:--",
                    1,
                    color
                );
            }
            else
            {
                drawAlarmTimeText(
                    10,
                    y + 1,
                    index,
                    color
                );
            }
            // ----------------------------------------------------
            // ACTIVE BOX
            // ----------------------------------------------------

            drawAlarmActiveBox(
                y,
                alarms[index].enabled,
                selected
            );
        }

        // ----------------------------------------------------
        // SNOOZE
        // ----------------------------------------------------

        else
        {
            drawWatchText(
                10,
                y + 1,
                "Snooze",
                1,
                color
            );
            // ------------------------------------------------
            // SNOOZE ACTIVE BOX
            // ------------------------------------------------

            drawAlarmActiveBox(
                y,
                snoozeEnabled,
                selected
            );
        }
        
    }
    display.display();
}

// ============================================================
// ALARM MENU UP
// ============================================================

void alarmMenuUp()
{
    // --------------------------------------------------------
    // MOVE UP
    // --------------------------------------------------------

    alarmMenuIndex--;

    // --------------------------------------------------------
    // WRAP: first -> snooze
    // --------------------------------------------------------

    if (alarmMenuIndex < 0)
    {
        alarmMenuIndex = 4;
    }

    // --------------------------------------------------------
    // EXACT TOP POSITION
    //
    // 0,1,2 -> top 0
    // 3     -> top 1
    // 4     -> top 2
    // --------------------------------------------------------

    if (alarmMenuIndex <= 2)
    {
        alarmMenuTop = 0;
    }
    else if (alarmMenuIndex == 3)
    {
        alarmMenuTop = 1;
    }
    else
    {
        alarmMenuTop = 2;
    }

    resetSubMenuScroll();
}


// ============================================================
// ALARM MENU DOWN
// ============================================================

void alarmMenuDown()
{
    // --------------------------------------------------------
    // MOVE DOWN
    // --------------------------------------------------------

    alarmMenuIndex++;

    // --------------------------------------------------------
    // WRAP: snooze -> first alarm
    // --------------------------------------------------------

    if (alarmMenuIndex > 4)
    {
        alarmMenuIndex = 0;
    }

    // --------------------------------------------------------
    // EXACT TOP POSITION
    // --------------------------------------------------------

    if (alarmMenuIndex <= 2)
    {
        alarmMenuTop = 0;
    }
    else if (alarmMenuIndex == 3)
    {
        alarmMenuTop = 1;
    }
    else
    {
        alarmMenuTop = 2;
    }

    resetSubMenuScroll();
}

// ============================================================
// OPEN ALARM MENU ITEM
// ============================================================

void openAlarmMenuItem()
{
    // --------------------------------------------------------
    // ALARM 1-4
    // --------------------------------------------------------

    if (alarmMenuIndex < 4)
    {
        selectedAlarmIndex = alarmMenuIndex;

        // --------------------------------------------
        // ALARM PAGE OPEN అయిన వెంటనే
        // HOUR selected
        // --------------------------------------------

        alarmSetting = true;
        alarmSetStep = 0;

        currentScreen = ALARM_PAGE;

        resetSubMenuScroll();

        Serial.print("OPEN ALARM ");
        Serial.println(selectedAlarmIndex + 1);

        return;
    }

    // --------------------------------------------------------
    // SNOOZE
    // --------------------------------------------------------

    if (alarmMenuIndex == 4)
    {
        currentScreen = SNOOZE_PAGE;

        resetSubMenuScroll();

        Serial.println("OPEN SNOOZE PAGE");

        return;
    }
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
        // =================================================
        // CLOCK
        // =================================================

        case 0:

            currentScreen =
                CLOCK_MENU_PAGE;

            clockMenuIndex =
                0;

            clockMenuTop =
                0;

            resetSubMenuScroll();

            break;


        // =================================================
        // CALENDAR
        // =================================================

        case 1:

            // TEMPORARY
            // Calendar page will be connected later.

            currentScreen =
                TIME_PAGE;

            break;


        // =================================================
        // GPS
        // =================================================

        case 2:

            currentScreen =
                GPS_PAGE;

            break;


        // =================================================
        // BLUETOOTH
        // =================================================

        case 3:

            // TEMPORARY
            // Bluetooth page will be connected later.

            currentScreen =
                STOPWATCH_PAGE;

            break;


        // =================================================
        // CONTACTS
        // =================================================

        case 4:

            // TEMPORARY

            currentScreen =
                COMPASS_PAGE;

            break;


        // =================================================
        // STEPS
        // =================================================

        case 5:

            // TEMPORARY

            currentScreen =
                SENSOR_PAGE;

            break;


        // =================================================
        // WEATHER
        // =================================================

        case 6:

            // TEMPORARY

            currentScreen =
                SENSOR_PAGE;

            break;


        // =================================================
        // HEART RATE / SPO2
        // =================================================

        case 7:

            // TEMPORARY

            currentScreen =
                SENSOR_PAGE;

            break;


        // =================================================
        // SETTINGS
        // =================================================

        case 8:

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
        menuIndex >=
        MENU_COUNT
    )
    {
        menuIndex = 0;

        menuTop = 0;

        resetMenuScroll();

        return;
    }


    resetMenuScroll();


    if (
        menuIndex >=
        menuTop +
        MENU_VISIBLE
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


    // =========================================================
    // RESET SUB MENU SCROLL
    // =========================================================

    resetSubMenuScroll();


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

        resetSubMenuScroll();

        return;
    }


    // =========================================================
    // RESET SUB MENU SCROLL
    // =========================================================

    resetSubMenuScroll();


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
    Serial.print(
        "TIMEZONE SELECT: "
    );

    Serial.println(
        timezoneMenuItems[
            timezoneMenuIndex
        ]
    );


    // =========================================================
    // CURRENT LOCATION
    // =========================================================

    if (
        timezoneMenuIndex == 0
    )
    {
        setTimezoneMode(
            TIMEZONE_CURRENT_LOCATION
        );

        return;
    }


    // =========================================================
    // DEFAULT / INDIA
    // =========================================================

    if (
        timezoneMenuIndex == 1
    )
    {
        setManualTimezoneIndex(
            0
        );

        return;
    }


    // =========================================================
    // REMAINING TIMEZONES
    //
    // Menu index 2 = timezoneRegions[1] = Japan
    // Menu index 3 = timezoneRegions[2] = UK
    // etc.
    // =========================================================

    int manualIndex =
        timezoneMenuIndex - 1;


    setManualTimezoneIndex(
        manualIndex
    );
}