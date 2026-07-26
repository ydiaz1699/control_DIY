/**
 * theme_engine.cpp — Dynamic Theme Engine Implementation
 * 
 * Auto-switches between day/night themes based on configured hours
 */

#include "theme_engine.h"
#include "../config/user_config.h"
#include <time.h>

static ThemeMode currentMode = THEME_NIGHT;
static int lastCheckedHour = -1;

// Theme color palettes
static const ThemeColors THEMES[] = {
    // THEME_DAY
    {
        .bg_primary = 0xF5F5F5,
        .bg_secondary = 0xE0E0E0,
        .text_primary = 0x212121,
        .text_secondary = 0x757575,
        .accent = 0x1976D2,
        .success = 0x388E3C,
        .warning = 0xF57C00,
        .error = 0xD32F2F,
        .card_bg = 0xFFFFFF,
        .border = 0xBDBDBD,
    },
    // THEME_NIGHT
    {
        .bg_primary = 0x121212,
        .bg_secondary = 0x1E1E2E,
        .text_primary = 0xE0E0E0,
        .text_secondary = 0x9E9E9E,
        .accent = 0x7C4DFF,
        .success = 0x00E676,
        .warning = 0xFFAB00,
        .error = 0xFF5252,
        .card_bg = 0x1E1E2E,
        .border = 0x333355,
    },
    // THEME_MOVIE (ultra dark)
    {
        .bg_primary = 0x000000,
        .bg_secondary = 0x0A0A15,
        .text_primary = 0x888888,
        .text_secondary = 0x555555,
        .accent = 0x4A148C,
        .success = 0x1B5E20,
        .warning = 0xE65100,
        .error = 0xB71C1C,
        .card_bg = 0x0A0A15,
        .border = 0x1A1A2A,
    },
    // THEME_CUSTOM (same as night for now)
    {
        .bg_primary = 0x0D1117,
        .bg_secondary = 0x161B22,
        .text_primary = 0xC9D1D9,
        .text_secondary = 0x8B949E,
        .accent = 0x58A6FF,
        .success = 0x3FB950,
        .warning = 0xD29922,
        .error = 0xF85149,
        .card_bg = 0x21262D,
        .border = 0x30363D,
    },
};

void theme_engine_init() {
    // Determine initial theme based on time (if available)
    if (THEME_AUTO_CHANGE) {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 0)) {
            int hour = timeinfo.tm_hour;
            if (hour >= THEME_DAY_START_HOUR && hour < THEME_NIGHT_START_HOUR) {
                currentMode = THEME_DAY;
            } else {
                currentMode = THEME_NIGHT;
            }
            lastCheckedHour = hour;
        }
    }
    
    Serial.printf("[THEME] Initialized, mode: %d\n", currentMode);
}

void theme_engine_apply() {
    const ThemeColors &colors = THEMES[currentMode];
    
    // Apply to default LVGL style
    static lv_style_t style_default;
    lv_style_init(&style_default);
    lv_style_set_bg_color(&style_default, lv_color_hex(colors.bg_primary));
    lv_style_set_text_color(&style_default, lv_color_hex(colors.text_primary));
    
    // Apply to active screen
    lv_obj_t *scr = lv_screen_active();
    if (scr) {
        lv_obj_set_style_bg_color(scr, lv_color_hex(colors.bg_primary), 0);
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    }
}

void theme_engine_set_mode(ThemeMode mode) {
    if (mode == currentMode) return;
    currentMode = mode;
    theme_engine_apply();
    Serial.printf("[THEME] Mode changed to: %d\n", mode);
}

ThemeMode theme_engine_get_mode() {
    return currentMode;
}

void theme_engine_check_time_change() {
    if (!THEME_AUTO_CHANGE) return;
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 0)) return;
    
    int hour = timeinfo.tm_hour;
    if (hour == lastCheckedHour) return;
    lastCheckedHour = hour;
    
    ThemeMode newMode;
    if (hour >= THEME_DAY_START_HOUR && hour < THEME_NIGHT_START_HOUR) {
        newMode = THEME_DAY;
    } else {
        newMode = THEME_NIGHT;
    }
    
    if (newMode != currentMode) {
        theme_engine_set_mode(newMode);
    }
}

ThemeColors theme_get_colors() {
    return THEMES[currentMode];
}
