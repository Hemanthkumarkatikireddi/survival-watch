#include "alarm_page.h"
#include "display.h"
#include "fonts.h"
#include "state.h"

#include <Arduino.h>


// ============================================================
// ALARM TIME DRAW
// EXACT SAME NUMBER FONT USED BY HOME drawBoldTime()
// ============================================================

static void drawAlarmTime(
    int x,
    int y,
    int hour,
    int minute,
    bool selectHour,
    bool selectMinute
)
{
    // --------------------------------------------------------
    // 24 hour -> 12 hour
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
    // EXACT SAME POSITIONS AS drawBoldTime()
    // --------------------------------------------------------

    int xH1 = x;
    int xH2 = x + 12;

    int xColon = x + 24;

    int xM1 = x + 36;
    int xM2 = x + 48;


    // ========================================================
    // HOUR
    // ========================================================

    if (selectHour)
    {
        // Black selection box
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
    // MINUTE
    // ========================================================

    if (selectMinute)
    {
        // Black selection box
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
// ALARM PAGE
// ============================================================

void drawAlarmPage()
{
    display.clearDisplay();


    // ========================================================
    // TITLE
    // ========================================================

    char title[12];

    snprintf(
        title,
        sizeof(title),
        "ALARM %d",
        selectedAlarmIndex + 1
    );

    drawBoldWatchText(
        17,
        2,
        title,
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
    // ALARM DATA
    // ========================================================

    AlarmData &alarm =
        alarms[selectedAlarmIndex];


    // ========================================================
    // TIME
    // ========================================================

    drawAlarmTime(
        2,
        14,
        alarm.hour,
        alarm.minute,

        alarmSetting &&
        alarmSetStep == 0,

        alarmSetting &&
        alarmSetStep == 1
    );


    // ========================================================
    // AM / PM
    // ========================================================

    bool pm =
        alarm.hour >= 12;


    if (
    alarmSetting &&
    alarmSetStep == 2
    )
    {
        // AM/PM selection background
        display.fillRect(
            63,
            19,
            17,
            9,
            BLACK
        );

        // Center AM/PM inside selection box
        drawBoldWatchText(
            64,
            20,
            pm ? "PM" : "AM",
            1,
            WHITE
        );
      }
      else
      {
          drawBoldWatchText(
              64,
              20,
              pm ? "PM" : "AM",
              1,
            BLACK
          );
      }


    // ========================================================
    // ON / OFF
    // ========================================================

    if (
        alarmSetting &&
        alarmSetStep == 3
    )
    {
        // Selection background
        display.fillRect(
            0,
            39,
            84,
            9,
            BLACK
        );

        // White ON / OFF
        drawBoldWatchText(
            31,
            40,
            alarm.enabled ? "ON" : "OFF",
            1,
            WHITE
        );
    }
    else
    {
        drawBoldWatchText(
            31,
            40,
            alarm.enabled ? "ON" : "OFF",
            1,
            BLACK
        );
    }


    display.display();
}

// ============================================================
// SNOOZE PAGE
// ============================================================

void drawSnoozePage()
{
    display.clearDisplay();

    // ========================================================
    // TITLE
    // ========================================================

    drawBoldWatchText(
        21,
        2,
        "SNOOZE",
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
    // SNOOZE STATUS
    // ========================================================

    if (snoozeEnabled)
    {
        display.fillRect(
            0,
            39,
            84,
            9,
            BLACK
        );

        drawBoldWatchText(
            31,
            40,
            "ON",
            1,
            WHITE
        );
    }
    else
    {
      display.fillRect(
            0,
            39,
            84,
            9,
            BLACK
        );
        drawBoldWatchText(
            31,
            40,
            "OFF",
            1,
            WHITE
        );
    }

    display.display();
}

// ============================================================
// ALARM VALUE UP
// ============================================================

void alarmValueUp()
{
    if (!alarmSetting)
    {
        return;
    }


    AlarmData &alarm =
        alarms[selectedAlarmIndex];


    // --------------------------------------------------------
    // HOUR
    // --------------------------------------------------------

    if (alarmSetStep == 0)
    {
        alarm.hour++;

        if (alarm.hour >= 24)
        {
            alarm.hour = 0;
        }

        return;
    }


    // --------------------------------------------------------
    // MINUTE
    // --------------------------------------------------------

    if (alarmSetStep == 1)
    {
        alarm.minute++;

        if (alarm.minute >= 60)
        {
            alarm.minute = 0;
        }

        return;
    }


    // --------------------------------------------------------
    // AM / PM
    // --------------------------------------------------------

    if (alarmSetStep == 2)
    {
        if (alarm.hour < 12)
        {
            alarm.hour += 12;
        }
        else
        {
            alarm.hour -= 12;
        }

        return;
    }


    // --------------------------------------------------------
    // ON / OFF
    // --------------------------------------------------------

    if (alarmSetStep == 3)
    {
        alarm.enabled =
            !alarm.enabled;

        return;
    }
}


// ============================================================
// ALARM VALUE DOWN
// ============================================================

void alarmValueDown()
{
    if (!alarmSetting)
    {
        return;
    }


    AlarmData &alarm =
        alarms[selectedAlarmIndex];


    // --------------------------------------------------------
    // HOUR
    // --------------------------------------------------------

    if (alarmSetStep == 0)
    {
        if (alarm.hour == 0)
        {
            alarm.hour = 23;
        }
        else
        {
            alarm.hour--;
        }

        return;
    }


    // --------------------------------------------------------
    // MINUTE
    // --------------------------------------------------------

    if (alarmSetStep == 1)
    {
        if (alarm.minute == 0)
        {
            alarm.minute = 59;
        }
        else
        {
            alarm.minute--;
        }

        return;
    }


    // --------------------------------------------------------
    // AM / PM
    // --------------------------------------------------------

    if (alarmSetStep == 2)
    {
        if (alarm.hour < 12)
        {
            alarm.hour += 12;
        }
        else
        {
            alarm.hour -= 12;
        }

        return;
    }


    // --------------------------------------------------------
    // ON / OFF
    // --------------------------------------------------------

    if (alarmSetStep == 3)
    {
        alarm.enabled =
            !alarm.enabled;

        return;
    }
}


// ============================================================
// CHECK ALARM
// ============================================================

void checkAlarm()
{
    // --------------------------------------------------------
    // Prevent repeated triggering within same minute
    // --------------------------------------------------------

    static int lastTriggeredMinute = -1;


    // --------------------------------------------------------
    // Get current time
    // --------------------------------------------------------

    DateTime now = rtc.now();


    int currentMinute =
        now.hour() * 60 +
        now.minute();


    // --------------------------------------------------------
    // Check all 4 alarms
    // --------------------------------------------------------

    for (int i = 0; i < 4; i++)
    {
        if (!alarms[i].enabled)
        {
            continue;
        }


        int alarmMinute =
            alarms[i].hour * 60 +
            alarms[i].minute;


        if (
            currentMinute == alarmMinute &&
            now.second() < 2 &&
            lastTriggeredMinute != currentMinute
        )
        {
            selectedAlarmIndex = i;

            alarmRinging = true;

            lastTriggeredMinute =
                currentMinute;

            Serial.print("ALARM RINGING: ");
            Serial.println(i + 1);

            return;
        }
    }
}


// ============================================================
// SNOOZE ALARM
// ============================================================

void snoozeAlarm()
{
    if (!alarmRinging)
    {
        return;
    }


    alarmRinging = false;


    Serial.println("ALARM SNOOZE");


    // --------------------------------------------------------
    // 5 minute snooze
    // --------------------------------------------------------

    DateTime now = rtc.now();


    int newMinute =
        now.minute() + 5;


    int newHour =
        now.hour();


    if (newMinute >= 60)
    {
        newMinute -= 60;
        newHour++;

        if (newHour >= 24)
        {
            newHour = 0;
        }
    }


    alarms[selectedAlarmIndex].hour =
        newHour;

    alarms[selectedAlarmIndex].minute =
        newMinute;
}


// ============================================================
// STOP ALARM
// ============================================================

void stopAlarm()
{
    if (!alarmRinging)
    {
        return;
    }


    alarmRinging = false;


    Serial.println("ALARM STOPPED");
}