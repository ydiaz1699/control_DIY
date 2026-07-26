/**
 * theme_engine.h — Dynamic Theme Engine
 * 
 * INNOVATION: Changes color scheme based on time of day and activity
 * - Day mode: Light, high contrast, warm tones
 * - Night mode: Dark, OLED-friendly, cool tones
 * - Movie mode: Ultra dark, minimal glare
 */

#pragma once

#include <Arduino.h>
#include <lvgl.h>

enum ThemeMode {
    THEME_DAY = 0,
    THEME_NIGHT,
    THEME_MOVIE,
    THEME_CUSTOM,
};

struct ThemeColors {
    uint32_t bg_primary;
    uint32_t bg_secondary;
    uint32_t text_primary;
    uint32_t text_secondary;
    uint32_t accent;
    uint32_t success;
    uint32_t warning;
    uint32_t error;
    uint32_t card_bg;
    uint32_t border;
};

void theme_engine_init();
void theme_engine_apply();
void theme_engine_set_mode(ThemeMode mode);
ThemeMode theme_engine_get_mode();
void theme_engine_check_time_change();
ThemeColors theme_get_colors();
