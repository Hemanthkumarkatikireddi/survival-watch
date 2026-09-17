#pragma once

// =====================================================
// NOKIA 5110
// =====================================================

#define LCD_CLK  9
#define LCD_DIN  8
#define LCD_DC   7
#define LCD_CE   6
#define LCD_RST  5
#define LCD_BL   10

// =====================================================
// RTC - WOKWI CURRENT SETUP = DS1307
// =====================================================

#define RTC_SDA 1
#define RTC_SCL 2

// =====================================================
// BUTTONS
// LEFT -> RIGHT
// 1. UP
// 2. DOWN
// 3. MENU
// 4. SELECT
// =====================================================

#define BTN_UP      12
#define BTN_DOWN    11
#define BTN_MENU    13
#define BTN_SELECT  14

// =====================================================
// BUTTON TIMING
// =====================================================

constexpr unsigned long LONG_PRESS_TIME   = 800;
constexpr unsigned long DOUBLE_CLICK_TIME = 400;

// =====================================================
// BACKLIGHT
// =====================================================

constexpr unsigned long BACKLIGHT_TIMEOUT = 10000;

// =====================================================
// MENU
// =====================================================

constexpr int MENU_VISIBLE = 3;

constexpr int MENU_COUNT = 8;

extern const char* menuItems[MENU_COUNT];

// =====================================================
// TIME ZONE MENU
// =====================================================

constexpr int TIMEZONE_MENU_VISIBLE = 3;

constexpr int TIMEZONE_MENU_COUNT = 14;

extern const char* timezoneMenuItems[TIMEZONE_MENU_COUNT];

// =====================================================
// DAY / MONTH
// =====================================================

extern const char* days[7];
extern const char* months[12];
