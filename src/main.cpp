#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

#include "config.h"
#include "state.h"
#include "display.h"
#include "buttons.h"
#include "alarm_page.h"
#include "screens.h"
#include "home.h"

#include "startup_image.h"

#include "panchang/masa.h"


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
// - Startup loading progress
//
// Panchang calculation is prepared BEFORE HOME appears.
// ============================================================


// ============================================================
// STARTUP STATE
// ============================================================

static bool startupFinished = false;


// ============================================================
// SETUP
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);


    // ========================================================
    // I2C
    // ========================================================

    Wire.begin(
        RTC_SDA,
        RTC_SCL
    );


    // ========================================================
    // BUTTONS
    // ========================================================

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


    // ========================================================
    // BACKLIGHT
    // ========================================================

    pinMode(
        LCD_BL,
        OUTPUT
    );

    setBacklight(false);


    // ========================================================
    // DISPLAY
    // ========================================================

    initDisplay();


    // ========================================================
    // STARTUP IMAGE + LOADING BAR
    // ========================================================

    showStartupImages();

    // 0%
    setStartupProgress(0);


    // ========================================================
    // INITIAL STARTUP
    // ========================================================

    startupDelay(300);

    setStartupProgress(10);


    startupDelay(300);

    setStartupProgress(20);


    startupDelay(300);

    setStartupProgress(30);


    // ========================================================
    // RTC
    // ========================================================

    if (!rtc.begin())
    {
        stopStartupLoading();

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


        while (true)
        {
            delay(100);
        }
    }


    Serial.println(
        "DS1307 FOUND!"
    );


    // ========================================================
    // RTC READY
    // ========================================================

    setStartupProgress(40);

    startupDelay(300);


    Serial.println(
        "RTC READY!"
    );


    setStartupProgress(50);

    startupDelay(300);


    // ========================================================
    // PANCHANG PREPARATION
    //
    // Do NOT perform heavy Panchang/Masa calculation here.
    // HOME must appear quickly.
    // ========================================================

    Serial.println(
    "PANCHANG WILL UPDATE IN BACKGROUND"
    );

    setStartupProgress(60);

    


    // ========================================================
    // FINAL STARTUP
    // ========================================================

    startupDelay(300);

    setStartupProgress(90);


    startupDelay(300);


    Serial.println(
        "WATCH MENU READY!"
    );

    Serial.println("PANCHANG READY!");


    // ========================================================
    // 100% COMPLETE
    //
    // All boxes will now be filled.
    // ========================================================

    setStartupProgress(60);


    startupDelay(150);

    startPanchangBackgroundTask();

    // ========================================================
    // Allow loop() to draw HOME
    // ========================================================

    startupFinished = true;
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
    // ========================================================
    // FIRST HOME DRAW
    //
    // Panchang is already calculated in setup().
    // ========================================================

    if (!startupFinished)
    {
        updateStartupLoading();

        return;
    }


    // ========================================================
    // NORMAL BUTTONS
    // ========================================================

    handleButtons();

    updateDoubleClick();

    checkAlarm();

    updateBacklight();


    // ========================================================
    // DRAW CURRENT SCREEN
    // ========================================================

    drawCurrentScreen();


    // ========================================================
    // HOME IS NOW DRAWN
    //
    // Stop startup loading after the first LCD frame.
    // ========================================================

    if (isStartupLoading())
    {
        stopStartupLoading();
    }


    // ========================================================
    // NORMAL LOOP DELAY
    // ========================================================

    delay(50);
}