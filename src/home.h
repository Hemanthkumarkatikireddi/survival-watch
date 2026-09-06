#pragma once

#include <Arduino.h>

void drawHomeScreen();

void toggleHomeView();

void toggleMoonView();

void blankHomeInfoLines();

bool isMoonView();

void setTestMoonTithi(
    uint8_t tithi
);