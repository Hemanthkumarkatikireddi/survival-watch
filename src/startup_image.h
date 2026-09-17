#ifndef STARTUP_IMAGE_H
#define STARTUP_IMAGE_H

#include <Arduino.h>

extern const uint8_t startupImage1[] PROGMEM;

void showStartupImages();

void updateStartupLoading();

void startupDelay(
    unsigned long duration
);

void setStartupProgress(
    uint8_t percent
);

void stopStartupLoading();

bool isStartupLoading();

#endif