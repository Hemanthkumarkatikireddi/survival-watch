#pragma once

#include <Arduino.h>

void drawHomeScreen();

void toggleHomeView();

void toggleMoonView();

void blankHomeInfoLines();

bool isMoonView();

void updatePanchangData(bool forceUpdate);

void setBackBlankView(
    bool enabled
);

bool isBackBlankView();

void setHomeTithiView(bool enabled);
bool isHomeTithiView();

void setBackBlankView(bool enabled);
bool isBackBlankView();

void setTestMoonTithi(
    uint8_t tithi
);
void startPanchangBackgroundTask();