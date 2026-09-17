#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

extern Adafruit_PCD8544 display;

void initDisplay();

void setBacklight(bool state);
void updateBacklight();

void getDisplayHour(
  int hour24,
  int &hourDisplay,
  bool &pm
);
