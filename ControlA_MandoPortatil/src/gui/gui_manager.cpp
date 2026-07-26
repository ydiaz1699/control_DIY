/**
 * gui_manager.cpp — LVGL GUI Manager Implementation
 * 
 * Carousel-based navigation with swipe gestures
 * Combines OLED Remote's carousel concept with OMOTE's LVGL richness
 */

#include "gui_manager.h"
#include "theme_engine.h"
#include "screens/screen_scenes.h"
#include "screens/screen_media.h"
#include "screens/screen_ir_remote.h"
#include "screens/screen_smarthome.h"
#include "screens/screen_settings.h"
#include "screens/screen_idle.h"
#include "../hal/battery_hal.h"
#include "../comms/wifi_manager.h"
#include "../comms/mqtt_manager.h"
#include "../comms/ble_keyboard.h"

// LVGL tick timer
static unsigned long lastLvglTick = 0;

// Screen management
static lv_obj_t *screen_container = nullptr;
static lv_obj_t *status_bar = nullptr;
static lv_obj_t *lbl_battery = nullptr;
static lv_obj_t *lbl_wifi = nullptr;
static lv_obj_t *lbl_ble = nullptr;
static lv_obj_t *lbl_time = nullptr;
static lv_obj_t *notification_label = nullptr;
static lv_timer_t *notification_timer = nullptr;

static ScreenID currentScreen = SCREEN_SCENES;
static lv_obj_t *screens[SCREEN_COUNT] = {nullptr};

// LVGL tick callback
static uint32_t lv_tick_cb() {
    return millis();
}

// Gesture event callback for carousel swipe
static void gesture_event_cb(lv_event_t *e) {
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_LEFT) {
        gui_navigate_next();
    } else if (dir == LV_DIR_RIGHT) {
        gui_navigate_prev();
    }
}

// Create status bar (top of screen)
static void create_status_bar() {
    status_bar = lv_obj_create(lv_screen_active());
    lv_obj_set_size(status_bar, 240, 24);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_set_style_radius(status_bar, 0, 0);
    lv_obj_set_style_pad_all(status_bar, 2, 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);
    
    // WiFi indicator
    lbl_wifi = lv_label_create(status_bar);
    lv_label_set_text(lbl_wifi, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(lbl_wifi, lv_color_hex(0x4CAF50), 0);
    lv_obj_align(lbl_wifi, LV_ALIGN_LEFT_MID, 2, 0);
    
    // BLE indicator
    lbl_ble = lv_label_create(status_bar);
    lv_label_set_text(lbl_ble, LV_SYMBOL_BLUETOOTH);
    lv_obj_set_style_text_color(lbl_ble, lv_color_hex(0x2196F3), 0);
    lv_obj_align(lbl_ble, LV_ALIGN_LEFT_MID, 22, 0);
    
    // Time
    lbl_time = lv_label_create(status_bar);
    lv_label_set_text(lbl_time, "--:--");
    lv_obj_set_style_text_color(lbl_time, lv_color_hex(0xCCCCCC), 0);
    lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, 0);
    
    // Battery
    lbl_battery = lv_label_create(status_bar);
    lv_label_set_text(lbl_battery, LV_SYMBOL_BATTERY_FULL " 100%");
    lv_obj_set_style_text_color(lbl_battery, lv_color_hex(0x4CAF50), 0);
    lv_obj_align(lbl_battery, LV_ALIGN_RIGHT_MID, -2, 0);
}

// Create notification popup area
static void create_notification_area() {
    notification_label = lv_label_create(lv_screen_active());
    lv_obj_set_size(notification_label, 200, LV_SIZE_CONTENT);
    lv_obj_align(notification_label, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(notification_label, lv_color_hex(0x333366), 0);
    lv_obj_set_style_bg_opa(notification_label, LV_OPA_90, 0);
    lv_obj_set_style_text_color(notification_label, lv_color_white(), 0);
    lv_obj_set_style_pad_all(notification_label, 8, 0);
    lv_obj_set_style_radius(notification_label, 8, 0);
    lv_obj_set_style_text_align(notification_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(notification_label, "");
    lv_obj_add_flag(notification_label, LV_OBJ_FLAG_HIDDEN);
}

void gui_init() {
    // Set LVGL tick
    lv_tick_set_cb(lv_tick_cb);
    
    // Apply theme
    theme_engine_apply();
    
    // Create status bar
    create_status_bar();
    
    // Create main content container (below status bar)
    screen_container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(screen_container, 240, 296); // 320 - 24 status bar
    lv_obj_align(screen_container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(screen_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(screen_container, 0, 0);
    lv_obj_set_style_pad_all(screen_container, 0, 0);
    lv_obj_clear_flag(screen_container, LV_OBJ_FLAG_SCROLLABLE);
    
    // Add gesture detection for carousel swipe
    lv_obj_add_event_cb(screen_container, gesture_event_cb, LV_EVENT_GESTURE, nullptr);
    
    // Initialize all screens
    screens[SCREEN_SCENES] = screen_scenes_create(screen_container);
    screens[SCREEN_MEDIA] = screen_media_create(screen_container);
    screens[SCREEN_IR_REMOTE] = screen_ir_remote_create(screen_container);
    screens[SCREEN_SMART_HOME] = screen_smarthome_create(screen_container);
    screens[SCREEN_SETTINGS] = screen_settings_create(screen_container);
    screens[SCREEN_IDLE] = screen_idle_create(screen_container);
    
    // Show initial screen, hide others
    for (int i = 0; i < SCREEN_COUNT; i++) {
        if (screens[i]) {
            if (i == SCREEN_SCENES) {
                lv_obj_clear_flag(screens[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(screens[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    
    // Create notification area
    create_notification_area();
    
    Serial.println("[GUI] Initialized with carousel navigation");
}

void gui_loop() {
    lv_timer_handler();
}

void gui_navigate_to(ScreenID screen) {
    if (screen >= SCREEN_COUNT || screen == currentScreen) return;
    
    // Hide current
    if (screens[currentScreen]) {
        lv_obj_add_flag(screens[currentScreen], LV_OBJ_FLAG_HIDDEN);
    }
    
    // Show new
    currentScreen = screen;
    if (screens[currentScreen]) {
        lv_obj_clear_flag(screens[currentScreen], LV_OBJ_FLAG_HIDDEN);
    }
    
    Serial.printf("[GUI] Navigated to screen %d\n", screen);
}

void gui_navigate_next() {
    int next = (currentScreen + 1) % SCREEN_COUNT;
    // Skip IDLE in normal navigation
    if (next == SCREEN_IDLE) next = (next + 1) % SCREEN_COUNT;
    gui_navigate_to((ScreenID)next);
}

void gui_navigate_prev() {
    int prev = (currentScreen - 1 + SCREEN_COUNT) % SCREEN_COUNT;
    if (prev == SCREEN_IDLE) prev = (prev - 1 + SCREEN_COUNT) % SCREEN_COUNT;
    gui_navigate_to((ScreenID)prev);
}

ScreenID gui_get_current_screen() {
    return currentScreen;
}

void gui_update_status_bar() {
    // Battery
    BatteryStatus batt = battery_get_status();
    char battStr[16];
    const char* battIcon = LV_SYMBOL_BATTERY_FULL;
    if (batt.percentage < 20) battIcon = LV_SYMBOL_BATTERY_1;
    else if (batt.percentage < 50) battIcon = LV_SYMBOL_BATTERY_2;
    else if (batt.percentage < 80) battIcon = LV_SYMBOL_BATTERY_3;
    
    if (batt.is_charging) {
        snprintf(battStr, sizeof(battStr), LV_SYMBOL_CHARGE " %d%%", batt.percentage);
        lv_obj_set_style_text_color(lbl_battery, lv_color_hex(0xFFEB3B), 0);
    } else {
        snprintf(battStr, sizeof(battStr), "%s %d%%", battIcon, batt.percentage);
        lv_color_t col = (batt.percentage < 20) ? lv_color_hex(0xF44336) : lv_color_hex(0x4CAF50);
        lv_obj_set_style_text_color(lbl_battery, col, 0);
    }
    lv_label_set_text(lbl_battery, battStr);
    
    // WiFi
    lv_color_t wifiColor = wifi_is_connected() ? lv_color_hex(0x4CAF50) : lv_color_hex(0x666666);
    lv_obj_set_style_text_color(lbl_wifi, wifiColor, 0);
    
    // BLE
    lv_color_t bleColor = ble_keyboard_is_connected() ? lv_color_hex(0x2196F3) : lv_color_hex(0x666666);
    lv_obj_set_style_text_color(lbl_ble, bleColor, 0);
}

// Hide notification timer callback
static void hide_notification_cb(lv_timer_t *timer) {
    lv_obj_add_flag(notification_label, LV_OBJ_FLAG_HIDDEN);
    lv_timer_delete(timer);
    notification_timer = nullptr;
}

void gui_show_notification(const char* text, uint16_t duration_ms) {
    if (!notification_label) return;
    
    lv_label_set_text(notification_label, text);
    lv_obj_clear_flag(notification_label, LV_OBJ_FLAG_HIDDEN);
    
    // Cancel previous timer
    if (notification_timer) {
        lv_timer_delete(notification_timer);
    }
    notification_timer = lv_timer_create(hide_notification_cb, duration_ms, nullptr);
    lv_timer_set_repeat_count(notification_timer, 1);
}

void gui_show_popup(const char* title, const char* text) {
    // Simple popup using msgbox
    lv_obj_t *mbox = lv_msgbox_create(lv_screen_active());
    lv_msgbox_add_title(mbox, title);
    lv_msgbox_add_text(mbox, text);
    lv_msgbox_add_close_button(mbox);
    lv_obj_center(mbox);
}
