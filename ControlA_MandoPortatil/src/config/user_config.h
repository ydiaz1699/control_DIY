/**
 * user_config.h — User-editable configuration
 * 
 * Edit this file to customize your remote for your home setup.
 * All user-facing settings are here — no need to touch other files.
 */

#pragma once

// ═══════════════════════════════════════════════════════════════
// WiFi Configuration
// ═══════════════════════════════════════════════════════════════
#define WIFI_SSID           "YOUR_WIFI_SSID"
#define WIFI_PASSWORD       "YOUR_WIFI_PASSWORD"
#define WIFI_HOSTNAME       "control-a-mando"

// ═══════════════════════════════════════════════════════════════
// MQTT Configuration (Home Assistant Mosquitto broker)
// ═══════════════════════════════════════════════════════════════
#define MQTT_SERVER         "homeassistant.local"
#define MQTT_PORT           1883
#define MQTT_USER           "mqtt_user"
#define MQTT_PASSWORD       "mqtt_password"
#define MQTT_CLIENT_ID      "control_a_mando"

// ═══════════════════════════════════════════════════════════════
// Home Assistant REST API (for media player & entity state)
// ═══════════════════════════════════════════════════════════════
#define HA_BASE_URL         "http://homeassistant.local:8123"
#define HA_TOKEN            "YOUR_LONG_LIVED_ACCESS_TOKEN"

// ═══════════════════════════════════════════════════════════════
// BLE Keyboard Settings
// ═══════════════════════════════════════════════════════════════
#define BLE_DEVICE_NAME     "Mando Portátil"
#define BLE_MANUFACTURER    "ControlDIY"

// ═══════════════════════════════════════════════════════════════
// Power Management
// ═══════════════════════════════════════════════════════════════
#define SLEEP_TIMEOUT_MS        20000   // Go to sleep after 20s inactivity
#define DIM_TIMEOUT_MS          10000   // Dim screen after 10s inactivity
#define MOTION_THRESHOLD        80      // IMU motion threshold (lift-to-wake)
#define WAKEUP_BY_IMU_ENABLED   true    // Enable lift-to-wake
#define LOW_BATTERY_THRESHOLD   15      // % to show low battery warning
#define CRITICAL_BATTERY_THRESHOLD 5    // % to force sleep

// ═══════════════════════════════════════════════════════════════
// Display Settings
// ═══════════════════════════════════════════════════════════════
#define BRIGHTNESS_MAX      255
#define BRIGHTNESS_DIM      50
#define BRIGHTNESS_MIN      10

// ═══════════════════════════════════════════════════════════════
// Scenes — Define your quick-access scenes here
// ═══════════════════════════════════════════════════════════════
#define MAX_SCENES 8

struct SceneConfig {
    const char* name;
    const char* icon;       // LVGL symbol or MDI icon name
    const char* mqtt_topic;
    const char* mqtt_payload;
};

static const SceneConfig USER_SCENES[] = {
    {"TV",          "\xEF\x86\xAE", "homeassistant/scene/tv/activate",        "{}"},
    {"Película",    "\xEF\x80\x88", "homeassistant/scene/movie/activate",     "{}"},
    {"Música",      "\xEF\x80\x81", "homeassistant/scene/music/activate",     "{}"},
    {"Luces Off",   "\xEF\x83\xA7", "homeassistant/scene/lights_off/activate","{}"},
    {"Dormir",      "\xEF\x86\xB6", "homeassistant/scene/sleep/activate",     "{}"},
    {"Todo Off",    "\xEF\x80\x91", "homeassistant/scene/all_off/activate",   "{}"},
};
static const int NUM_SCENES = sizeof(USER_SCENES) / sizeof(USER_SCENES[0]);

// ═══════════════════════════════════════════════════════════════
// IR Devices — Pre-configured IR codes
// ═══════════════════════════════════════════════════════════════
struct IRDevice {
    const char* name;
    int protocol;   // IRremoteESP8266 protocol enum
    uint64_t power;
    uint64_t vol_up;
    uint64_t vol_down;
    uint64_t mute;
    uint64_t ch_up;
    uint64_t ch_down;
};

static const IRDevice IR_DEVICES[] = {
    {
        "Samsung TV",
        7,  // SAMSUNG protocol
        0xE0E040BF,  // Power
        0xE0E0E01F,  // Vol+
        0xE0E0D02F,  // Vol-
        0xE0E0F00F,  // Mute
        0xE0E048B7,  // Ch+
        0xE0E008F7,  // Ch-
    },
    {
        "LG TV",
        28, // LG protocol
        0x20DF10EF,  // Power
        0x20DF40BF,  // Vol+
        0x20DFC03F,  // Vol-
        0x20DF906F,  // Mute
        0x20DF00FF,  // Ch+
        0x20DF807F,  // Ch-
    },
};
static const int NUM_IR_DEVICES = sizeof(IR_DEVICES) / sizeof(IR_DEVICES[0]);

// ═══════════════════════════════════════════════════════════════
// Media Players (Home Assistant entities)
// ═══════════════════════════════════════════════════════════════
static const char* MEDIA_PLAYERS[] = {
    "media_player.living_room",
    "media_player.bedroom",
    "media_player.kitchen",
};
static const int NUM_MEDIA_PLAYERS = sizeof(MEDIA_PLAYERS) / sizeof(MEDIA_PLAYERS[0]);

// ═══════════════════════════════════════════════════════════════
// ESP-NOW Sync (Communication with Control B Panel)
// ═══════════════════════════════════════════════════════════════
// MAC address of the Panel de Mesa (Control B) — set after flashing
#define PANEL_MAC_ADDR  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

// ═══════════════════════════════════════════════════════════════
// Theme Settings
// ═══════════════════════════════════════════════════════════════
#define THEME_AUTO_CHANGE       true    // Change theme based on time of day
#define THEME_DAY_START_HOUR    7       // Light theme starts at 7:00
#define THEME_NIGHT_START_HOUR  21      // Dark theme starts at 21:00

// ═══════════════════════════════════════════════════════════════
// Weather Widget (shown on idle screen)
// ═══════════════════════════════════════════════════════════════
#define WEATHER_ENTITY      "weather.home"
#define TEMP_ENTITY         "sensor.outdoor_temperature"
