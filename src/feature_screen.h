#ifndef FEATURE_SCREEN_H
#define FEATURE_SCREEN_H

#include <Arduino.h>

// ============================================================
// COMMON FEATURE SCREEN
// ============================================================

// Header
void drawFeatureHeader(const char* title);

// Title only
void drawFeatureTitle(const char* title);

// Separator
void drawFeatureSeparator();

// Normal text
void drawFeatureText(
    int x,
    int y,
    const char* text
);

// Bold text
void drawFeatureBoldText(
    int x,
    int y,
    const char* text
);

// Negative / selected row
void drawFeatureNegativeRow(
    int y,
    const char* text
);

#endif