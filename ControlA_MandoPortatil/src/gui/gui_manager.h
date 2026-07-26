/**
 * gui_manager.h — LVGL GUI Manager
 * 
 * Manages all screens: carousel, media player, IR remote, settings
 * Inspired by: OLED Remote carousel + homeThing menu + OMOTE LVGL tabs
 */

#pragma once

#include <Arduino.h>
#include <lvgl.h>

// Screen IDs (carousel pages)
enum ScreenID {
    SCREEN_SCENES = 0,      // Scene selection (carousel)
    SCREEN_MEDIA,           // Media player control
    SCREEN_IR_REMOTE,       // IR Remote (per-device buttons)
    SCREEN_SMART_HOME,      // Smart home entities
    SCREEN_IR_LEARN,        // IR Learning mode
    SCREEN_SETTINGS,        // Settings
    SCREEN_IDLE,            // Idle screen (weather/clock widget)
    SCREEN_COUNT
};

void gui_init();
void gui_loop();
void gui_navigate_to(ScreenID screen);
void gui_navigate_next();
void gui_navigate_prev();
ScreenID gui_get_current_screen();
void gui_update_status_bar();
void gui_show_notification(const char* text, uint16_t duration_ms = 2000);
void gui_show_popup(const char* title, const char* text);
