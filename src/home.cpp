#include "home.h"
#include "display.h"
#include "config.h"
#include "state.h"
#include "fonts.h"
#include "moon_data.h"

#include "panchang/timezone.h"
#include "panchang/panchang.h"

// ============================================================
// HOME SCREEN
// Nokia 5110 = 84 x 48 pixels
// ============================================================


// ============================================================
// HEART / BPM
// ============================================================

int heartBpm = 87;

bool showHeartIcon = true;

unsigned long lastHeartSwitch = 0;

const unsigned long HEART_SWITCH_INTERVAL = 5000;


// ============================================================
// HOME INFORMATION VIEW
// ============================================================
//
// false = Date + Weather
// true  = Tithi/SR/SS + Location
//
// Changes ONLY when DOWN is pressed.
//

bool showTithi = false;

bool homeInfoBlank = false;


// ============================================================
// MOON VIEW
// ============================================================
//
// false = normal HOME
// true  = Moon View
//

bool moonView = false;


// ============================================================
// TEMPORARY TEST TITHI
// ============================================================
//
// Tithi 4 = Chaturthi
//
// Later this value will come from tithi.cpp / SD card.
//

uint8_t testMoonTithi = 4;


// ============================================================
// DISPLAY WIDTH
// ============================================================

const int DISPLAY_WIDTH = 84;


// ============================================================
// LINE 3 TEXT
// ============================================================

const char* dateText =
    "2026:SEP:2:THU";

const char* tithiText =
    "Chavithi: YS-9:30pm Nxt: Panchami T-8:50am  ";

const char* sunriseSunset =
    "/#/  SR-6:32  SS-5:42";


// ============================================================
// LINE 4 TEXT
// ============================================================

const char* locationText =
    "Vizianagram PC:535002 St:kanu.st";


// ============================================================
// SCROLL POSITIONS
// ============================================================

// Tithi + SR/SS
int tithiScrollX = DISPLAY_WIDTH;

unsigned long lastTithiScroll = 0;


// Location
int locationScrollX = DISPLAY_WIDTH;

unsigned long lastLocationScroll = 0;


// Weather
int weatherScrollX = DISPLAY_WIDTH;

unsigned long lastWeatherScroll = 0;


// Existing scroll speed
const unsigned long SCROLL_INTERVAL = 62;


// ============================================================
// FIRST LINE FIXED POSITIONS
// ============================================================
//
// BPM:
//     x = 2
//
// Heart:
//     x = 10
//
// Alarm A:
//     x = 38
//
// GPS G:
//     x = 50
//
// Compass C:
//     x = 62
//
// Other Sensors S:
//     x = 74
//
// These positions NEVER change.
//

const int HEART_X = 2;

const int HEART_ICON_X = 10;

const int ALARM_X = 38;
const int GPS_X = 50;
const int COMPASS_X = 62;
const int SENSOR_X = 74;


// ============================================================
// FIRST LINE STATES
// ============================================================

bool compassEnabled = false;

bool otherSensorsEnabled = false;


// ============================================================
// HEART BITMAP
// ============================================================

const unsigned char heartBitmap[] PROGMEM =
{
    0b01100110,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000
};


void drawHeartIcon(
    int x,
    int y
)
{
    display.drawBitmap(
        x,
        y,
        heartBitmap,
        8,
        6,
        BLACK
    );
}


// ============================================================
// DEGREE SYMBOL
// ============================================================

void drawDegreeSymbol(
    int x,
    int y
)
{
    display.drawPixel(x + 1, y, BLACK);
    display.drawPixel(x + 2, y, BLACK);

    display.drawPixel(x, y + 1, BLACK);
    display.drawPixel(x + 3, y + 1, BLACK);

    display.drawPixel(x, y + 2, BLACK);
    display.drawPixel(x + 3, y + 2, BLACK);

    display.drawPixel(x + 1, y + 3, BLACK);
    display.drawPixel(x + 2, y + 3, BLACK);
}


// ============================================================
// SMALL DASH
// ============================================================

void drawSmallDash(
    int x,
    int y
)
{
    display.drawLine(
        x,
        y + 3,
        x + 3,
        y + 3,
        BLACK
    );
}


// ============================================================
// RESET TITHI SCROLL
// ============================================================

void resetTithiScroll()
{
    tithiScrollX = DISPLAY_WIDTH;

    lastTithiScroll = millis();
}


// ============================================================
// RESET LOCATION SCROLL
// ============================================================

void resetLocationScroll()
{
    locationScrollX = DISPLAY_WIDTH;

    lastLocationScroll = millis();
}


// ============================================================
// RESET WEATHER SCROLL
// ============================================================

void resetWeatherScroll()
{
    weatherScrollX = DISPLAY_WIDTH;

    lastWeatherScroll = millis();
}


// ============================================================
// TOGGLE HOME VIEW
// ============================================================
//
// DOWN:
//
// Date + Weather
//       ↕
// Tithi + Location
//

void toggleHomeView()
{
    // Do not change HOME information while Moon View
    // is active.
    if (moonView)
    {
        return;
    }

    showTithi = !showTithi;

    homeInfoBlank = false;

    resetTithiScroll();
    resetLocationScroll();
    resetWeatherScroll();
}


// ============================================================
// TOGGLE MOON VIEW
// ============================================================
//
// BACK/SELECT from HOME:
//
// Normal HOME -> Moon
// Moon        -> Normal HOME
//

void toggleMoonView()
{
    moonView = !moonView;

    // When entering Moon View
    if (moonView)
    {
        homeInfoBlank = true;

        Serial.print(
            "MOON VIEW ON - TITHI "
        );

        Serial.println(
            testMoonTithi
        );

        return;
    }


    // When leaving Moon View
    homeInfoBlank = false;

    resetTithiScroll();
    resetLocationScroll();
    resetWeatherScroll();

    Serial.println(
        "MOON VIEW OFF"
    );
}


// ============================================================
// MOON VIEW STATE
// ============================================================

bool isMoonView()
{
    return moonView;
}


// ============================================================
// TEMPORARY TEST TITHI
// ============================================================

void setTestMoonTithi(
    uint8_t tithi
)
{
    if (tithi < 1)
    {
        tithi = 1;
    }

    if (tithi > 30)
    {
        tithi = 30;
    }

    testMoonTithi = tithi;
}


// ============================================================
// TITHI NAME
// ============================================================
//
// 1-15  = Shukla Paksha
// 16-30 = Krishna Paksha
//
// The Tithi number is continuous 1..30,
// while the Tithi name starts again at Pratipada
// after Purnima.
//

const char* getTithiName(
    uint8_t tithi
)
{
    switch (tithi)
    {
        case 1:
        case 16:
            return "Pratipada";

        case 2:
        case 17:
            return "Dwitiya";

        case 3:
        case 18:
            return "Tritiya";

        case 4:
        case 19:
            return "Chaturthi";

        case 5:
        case 20:
            return "Panchami";

        case 6:
        case 21:
            return "Shashthi";

        case 7:
        case 22:
            return "Saptami";

        case 8:
        case 23:
            return "Ashtami";

        case 9:
        case 24:
            return "Navami";

        case 10:
        case 25:
            return "Dashami";

        case 11:
        case 26:
            return "Ekadashi";

        case 12:
        case 27:
            return "Dwadashi";

        case 13:
        case 28:
            return "Trayodashi";

        case 14:
        case 29:
            return "Chaturdashi";

        case 15:
            return "Purnima";

        case 30:
            return "Amavasya";

        default:
            return "Unknown";
    }
}


// ============================================================
// PAKSHA NAME
// ============================================================

const char* getPakshaName(
    uint8_t tithi
)
{
    if (tithi <= 15)
    {
        return "Shukla";
    }

    return "Krishna";
}


// ============================================================
// BLANK HOME INFORMATION LINES
// ============================================================
//
// Kept for compatibility.
//
// Existing code can still call this.
//

void blankHomeInfoLines()
{
    homeInfoBlank = !homeInfoBlank;

    if (!homeInfoBlank)
    {
        resetTithiScroll();
        resetLocationScroll();
        resetWeatherScroll();
    }
}


// ============================================================
// DATE LINE
// ============================================================

void drawDateLine()
{
    int textWidth =
        watchTextWidth(
            dateText,
            1
        );

    int x =
        (DISPLAY_WIDTH - textWidth) / 2;

    if (x < 0)
    {
        x = 0;
    }

    drawWatchText(
        x,
        30,
        dateText,
        1
    );
}


// ============================================================
// TITHI + SR/SS WIDTH
// ============================================================

int getTithiLineWidth()
{
    return
        watchTextWidth(
            tithiText,
            1
        ) +
        watchTextWidth(
            sunriseSunset,
            1
        );
}


// ============================================================
// DRAW TITHI + SR/SS
// ============================================================

void drawTithiLine()
{
    int x = tithiScrollX;


    drawWatchText(
        x,
        30,
        tithiText,
        1
    );


    int tithiWidth =
        watchTextWidth(
            tithiText,
            1
        );


    drawWatchText(
        x + tithiWidth,
        30,
        sunriseSunset,
        1
    );
}


// ============================================================
// UPDATE TITHI SCROLL
// ============================================================

void updateTithiScroll(
    unsigned long currentMillis
)
{
    if (
        currentMillis -
        lastTithiScroll <
        SCROLL_INTERVAL
    )
    {
        return;
    }

    lastTithiScroll =
        currentMillis;


    tithiScrollX--;


    int textWidth =
        getTithiLineWidth();


    if (
        tithiScrollX +
        textWidth <= 0
    )
    {
        tithiScrollX =
            DISPLAY_WIDTH;
    }
}


// ============================================================
// WEATHER WIDTH
// ============================================================

int getWeatherWidth()
{
    int width = 0;


    width +=
        watchTextWidth(
            "Tem: 29",
            1
        );


    width += 5;


    width +=
        watchTextWidth(
            "C Hum: 73% Feels like 35",
            1
        );


    width += 5;


    width +=
        watchTextWidth(
            "C-36",
            1
        );


    width += 5;


    width +=
        watchTextWidth(
            "C",
            1
        );


    return width;
}


// ============================================================
// DRAW WEATHER
// ============================================================

void drawWeatherLine()
{
    int x =
        weatherScrollX;

    int currentX =
        x;


    drawWatchText(
        currentX,
        40,
        "Tem: 29",
        1
    );


    currentX +=
        watchTextWidth(
            "Tem: 29",
            1
        );


    drawDegreeSymbol(
        currentX + 1,
        40
    );

    currentX += 5;


    drawWatchText(
        currentX,
        40,
        "C Hum: 73% Feels like 35",
        1
    );


    currentX +=
        watchTextWidth(
            "C Hum: 73% Feels like 35",
            1
        );


    drawDegreeSymbol(
        currentX + 1,
        40
    );

    currentX += 5;


    drawWatchText(
        currentX,
        40,
        "C-36",
        1
    );


    currentX +=
        watchTextWidth(
            "C-36",
            1
        );


    drawDegreeSymbol(
        currentX + 1,
        40
    );

    currentX += 5;


    drawWatchText(
        currentX,
        40,
        "C",
        1
    );
}


// ============================================================
// UPDATE WEATHER SCROLL
// ============================================================

void updateWeatherScroll(
    unsigned long currentMillis
)
{
    if (
        currentMillis -
        lastWeatherScroll <
        SCROLL_INTERVAL
    )
    {
        return;
    }

    lastWeatherScroll =
        currentMillis;


    weatherScrollX--;


    int weatherWidth =
        getWeatherWidth();


    if (
        weatherScrollX +
        weatherWidth <= 0
    )
    {
        weatherScrollX =
            DISPLAY_WIDTH;
    }
}


// ============================================================
// UPDATE LOCATION SCROLL
// ============================================================

void updateLocationScroll(
    unsigned long currentMillis
)
{
    if (
        currentMillis -
        lastLocationScroll <
        SCROLL_INTERVAL
    )
    {
        return;
    }

    lastLocationScroll =
        currentMillis;


    locationScrollX--;


    int locationWidth =
        watchTextWidth(
            locationText,
            1
        );


    if (
        locationScrollX +
        locationWidth <= 0
    )
    {
        locationScrollX =
            DISPLAY_WIDTH;
    }
}


// ============================================================
// DRAW LOCATION
// ============================================================

void drawLocationLine()
{
    drawWatchText(
        locationScrollX,
        40,
        locationText,
        1
    );
}


// ============================================================
// MAIN TIME
// ============================================================
DateTime getHomeDisplayTime()
{
    // ========================================================
    // NORMAL HOME
    // Always India time
    // ========================================================

    if (!showTithi)
    {
        return rtc.now();
    }

    // ========================================================
    // DOWN / LOCATION VIEW
    // GPS location local time
    // ========================================================

    DateTime indiaTime =
        rtc.now();

    DateTime utcTime =
        convertIndiaTimeToUTC(
            indiaTime
        );

    return getLocationLocalTime(
        utcTime
    );
}




void drawMainTime(
    DateTime now
)
{
    int displayHour;

    bool pm;


    getDisplayHour(
        now.hour(),
        displayHour,
        pm
    );


    char timeText[6];


    sprintf(
        timeText,
        "%02d:%02d",
        displayHour,
        now.minute()
    );


    display.setTextSize(2);


    display.setCursor(
        0,
        12
    );


    display.print(
        timeText
    );


    display.setTextSize(1);


    display.setCursor(
        61,
        18
    );


    if (
        now.second() < 10
    )
    {
        display.print("0");
    }


    display.print(
        now.second()
    );


    if (!use24Hour)
    {
        display.setTextSize(0);


        display.setCursor(
            78,
            11
        );


        display.print(
            pm ? "P" : "A"
        );
    }
}


// ============================================================
// BORDER BELOW TIME
// ============================================================

void drawTimeBorder()
{
    display.drawLine(
        0,
        28,
        83,
        28,
        BLACK
    );
}


// ============================================================
// DRAW FIRST LINE
// ============================================================

void drawFirstLine()
{
    display.setTextSize(1);

    display.setTextWrap(false);


    // ========================================================
    // HEART / BPM
    // ========================================================

    if (showHeartIcon)
    {
        display.drawBitmap(
            HEART_ICON_X,
            1,
            heartBitmap,
            8,
            6,
            BLACK
        );
    }
    else
    {
        display.setCursor(
            HEART_X,
            1
        );


        display.print(
            heartBpm
        );


        display.print(
            "bpm"
        );
    }


    // ========================================================
    // ALARM = A
    // ========================================================

    if (alarmEnabled)
    {
        display.setCursor(
            ALARM_X,
            1
        );

        display.print("A");
    }


    // ========================================================
    // GPS = G
    // ========================================================

    if (gpsActive)
    {
        display.setCursor(
            GPS_X,
            1
        );

        display.print("G");
    }


    // ========================================================
    // COMPASS = C
    // ========================================================

    if (compassEnabled)
    {
        display.setCursor(
            COMPASS_X,
            1
        );

        display.print("C");
    }


    // ========================================================
    // OTHER SENSORS = S
    // ========================================================

    if (otherSensorsEnabled)
    {
        display.setCursor(
            SENSOR_X,
            1
        );

        display.print("S");
    }
}


// ============================================================
// MOON VIEW
// ============================================================
//
// Nokia 5110 available area:
//
// X = 0..83
// Y = 29..47
//
// Left area:
//
// X = 0..41
// Y = 29..47
//
// Moon:
//
// 25 x 16
//
// Positioned approximately centered inside left 42x19 area.
//
// ============================================================
void drawMoonView()
{
    // --------------------------------------------------------
    // LEFT SIDE - MOON
    // --------------------------------------------------------

    drawMoon25x16(
        8,
        30,
        testMoonTithi
    );


    // --------------------------------------------------------
    // VERTICAL SEPARATOR
    // --------------------------------------------------------

    display.drawLine(
        42,
        29,
        42,
        47,
        BLACK
    );


    // --------------------------------------------------------
    // HORIZONTAL SEPARATOR
    // --------------------------------------------------------
    //
    // Right-side box:
    //
    // TOP    = 29..37
    // BORDER = 38
    // BOTTOM = 39..47
    //

    display.drawLine(
        43,
        38,
        83,
        38,
        BLACK
    );


    // --------------------------------------------------------
    // RIGHT SIDE
    // --------------------------------------------------------

    display.setTextSize(1);
    display.setTextWrap(false);


    // ========================================================
    // TOP BOX
    // ========================================================
    //
    // Only Paksha
    //
    // Shukla / Krishna
    //

    display.setCursor(
        48,
        30
    );

    if (testMoonTithi <= 15)
    {
        display.print("Shukla");
    }
    else
    {
        display.print("Krishna");
    }


    // ========================================================
    // BOTTOM BOX
    // ========================================================
    //
    // Existing HOME Tithi information is reused.
    //
    // Example:
    //
    // Chavithi: YS-9:30pm
    // Nxt: Panchami T-8:50am
    //
    // Sunrise / Sunset:
    //
    // SR-6:32 SS-5:42
    //
    // Because the right side is only 41 pixels wide,
    // this information scrolls horizontally.
    //
    // For now we display the beginning portion.
    //


    // --------------------------------------------------------
    // TITHI DATA
    // --------------------------------------------------------

    display.setCursor(
        43,
        40
    );

    display.print(
        tithiText
    );


    // --------------------------------------------------------
    // SUNRISE / SUNSET
    // --------------------------------------------------------

    display.setCursor(
        43,
        47
    );

    display.print(
        sunriseSunset
    );
}

// ============================================================
// HOME SCREEN
// ============================================================

void drawHomeScreen()
{
    unsigned long currentMillis =
        millis();


    // ========================================================
    // RTC
    // ========================================================

    DateTime now =
    getHomeDisplayTime();


    // ========================================================
    // HEART / BPM SWITCH
    // ========================================================

    if (
        currentMillis -
        lastHeartSwitch >=
        HEART_SWITCH_INTERVAL
    )
    {
        lastHeartSwitch =
            currentMillis;

        showHeartIcon =
            !showHeartIcon;
    }


    // ========================================================
    // UPDATE SCROLL
    // ========================================================
    //
    // Do NOT scroll while Moon View is active.
    //

    if (!moonView && !homeInfoBlank)
    {
        if (!showTithi)
        {
            updateWeatherScroll(
                currentMillis
            );
        }
        else
        {
            updateTithiScroll(
                currentMillis
            );

            updateLocationScroll(
                currentMillis
            );
        }
    }


    // ========================================================
    // CLEAR
    // ========================================================

    display.clearDisplay();

    display.setTextColor(BLACK);

    display.setTextSize(1);

    display.setTextWrap(false);


    // ========================================================
    // LINE 1
    // ========================================================

    drawFirstLine();


    // ========================================================
    // BORDER 1
    // ========================================================

    display.drawLine(
        0,
        9,
        83,
        9,
        BLACK
    );


    // ========================================================
    // MAIN TIME
    // ========================================================

    drawMainTime(
        now
    );


    // ========================================================
    // BORDER BELOW TIME
    // ========================================================

    drawTimeBorder();


    // ========================================================
    // MOON VIEW
    // ========================================================

    if (moonView)
    {
        drawMoonView();

        display.display();

        return;
    }


    // ========================================================
    // NORMAL HOME INFORMATION
    // ========================================================

    if (!homeInfoBlank)
    {
        // ----------------------------------------------------
        // LINE 3
        // ----------------------------------------------------

        if (!showTithi)
        {
            drawDateLine();
        }
        else
        {
            drawTithiLine();
        }


        // ----------------------------------------------------
        // BORDER ABOVE LINE 4
        // ----------------------------------------------------

        display.drawLine(
            0,
            38,
            83,
            38,
            BLACK
        );


        // ----------------------------------------------------
        // LINE 4
        // ----------------------------------------------------

        if (!showTithi)
        {
            drawWeatherLine();
        }
        else
        {
            drawLocationLine();
        }
    }


    // ========================================================
    // DISPLAY
    // ========================================================

    display.display();
}