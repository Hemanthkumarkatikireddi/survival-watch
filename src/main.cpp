#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

#include "config.h"
#include "state.h"
#include "display.h"
#include "buttons.h"
#include "alarm_page.h"
#include "screens.h"

// ============================================================
// MAIN
//
// Responsibility:
// - System startup
// - I2C
// - Buttons
// - Backlight
// - Display
// - RTC
//
// Panchang calculations are NOT done here.
// GPS data is NOT handled here.
// Sensor data is NOT handled here.
//
// Those will come from their respective modules.
// ============================================================


// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // SERIAL
    // --------------------------------------------------------

    Serial.begin(115200);

    delay(1000);


    // --------------------------------------------------------
    // I2C
    // --------------------------------------------------------

    Wire.begin(
        RTC_SDA,
        RTC_SCL
    );


    // --------------------------------------------------------
    // BUTTONS
    // --------------------------------------------------------

    pinMode(
        BTN_UP,
        INPUT_PULLUP
    );

    pinMode(
        BTN_DOWN,
        INPUT_PULLUP
    );

    pinMode(
        BTN_MENU,
        INPUT_PULLUP
    );

    pinMode(
        BTN_SELECT,
        INPUT_PULLUP
    );


    // --------------------------------------------------------
    // BACKLIGHT
    // --------------------------------------------------------

    pinMode(
        LCD_BL,
        OUTPUT
    );

    setBacklight(false);


    // --------------------------------------------------------
    // NOKIA 5110 DISPLAY
    // --------------------------------------------------------

    initDisplay();


    // --------------------------------------------------------
    // RTC
    // --------------------------------------------------------

    if (!rtc.begin())
    {
        Serial.println(
            "RTC ERROR!"
        );


        display.clearDisplay();

        display.setTextSize(1);

        display.setCursor(
            0,
            10
        );

        display.println(
            "RTC ERROR!"
        );


        display.setCursor(
            0,
            25
        );

        display.println(
            "CHECK WIRING"
        );


        display.display();


        // Stop system if RTC is not available.

        while (true)
        {
            delay(100);
        }
    }


    Serial.println(
        "DS1307 FOUND!"
    );


    // --------------------------------------------------------
    // RTC START
    // --------------------------------------------------------
    //
    // If RTC lost power / stopped,
    // use the firmware compile date/time.
    //
    // This is only RTC fallback behavior.
    // It is NOT Panchang dummy data.
    //
    // --------------------------------------------------------

    if (!rtc.isrunning())
    {
        rtc.adjust(
            DateTime(
                F(__DATE__),
                F(__TIME__)
            )
        );
    }


    Serial.println(
        "RTC READY!"
    );


    Serial.println(
        "WATCH MENU READY!"
    );
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // --------------------------------------------------------
    // BUTTONS
    // --------------------------------------------------------

    handleButtons();


    // --------------------------------------------------------
    // DOUBLE CLICK
    // --------------------------------------------------------

    updateDoubleClick();


    // --------------------------------------------------------
    // ALARM
    // --------------------------------------------------------

    checkAlarm();


    // --------------------------------------------------------
    // BACKLIGHT
    // --------------------------------------------------------

    updateBacklight();


    // --------------------------------------------------------
    // CURRENT SCREEN
    // --------------------------------------------------------

    drawCurrentScreen();


    // --------------------------------------------------------
    // SMALL LOOP DELAY
    // --------------------------------------------------------

    delay(50);
}