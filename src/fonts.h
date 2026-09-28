#ifndef FONTS_H
#define FONTS_H

#include <Arduino.h>

// ============================================================
// BOLD PIXEL WATCH FONT
// ============================================================

void drawWatchChar(
    int x,
    int y,
    char c,
    uint8_t scale = 1,
    uint8_t color = 1
);

void drawWatchText(
    int x,
    int y,
    const char* text,
    uint8_t scale = 1,
    uint8_t color = 1
);

void drawSecondsDigit(
    int x,
    int y,
    int digit
);

int watchTextWidth(
    const char* text,
    uint8_t scale = 1
);

void drawBoldDigit(
    int x,
    int y,
    int digit,
    uint8_t scale = 1
);

void drawBoldColon(
    int x,
    int y,
    uint8_t scale = 1
);

void drawBoldTime(
    int x,
    int y,
    int hour,
    int minute
);


void drawBoldWatchChar(
    int x,
    int y,
    char c,
    uint8_t scale,
    uint8_t color
);

void drawBoldWatchText(
    int x,
    int y,
    const char* text,
    uint8_t scale,
    uint8_t color
);

// ============================================================
// COMMON FEATURE BOLD FONT
// ============================================================

void drawFeatureBoldChar(
    int x,
    int y,
    char c,
    uint8_t color = 1
);

void drawFeatureBoldText(
    int x,
    int y,
    const char* text,
    uint8_t color = 1
);

int featureBoldTextWidth(
    const char* text
);

// ============================================================
// LCD STYLE LARGE TIME FONT
// ============================================================

void drawLCDDigit(
    int x,
    int y,
    int digit
);

void drawLCDColon(
    int x,
    int y
);

void drawLCDTime(
    int x,
    int y,
    int hour,
    int minute
);


void drawDateText(
    int x,
    int y,
    const char* text
);

int dateTextWidth(
    const char* text
);

void drawStopwatchDigit(
    int x,
    int y,
    char c,
    uint8_t color
);

#endif
