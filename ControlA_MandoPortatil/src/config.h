#pragma once

// ============================================================================
// Control A - Mando Portátil Universal
// Configuración centralizada
// ============================================================================

// --- Hardware Revision ---
#ifndef OMOTE_HARDWARE_REV
#define OMOTE_HARDWARE_REV 5
#endif

// --- Display ---
#ifndef SCR_WIDTH
#define SCR_WIDTH 240
#endif
#ifndef SCR_HEIGHT
#define SCR_HEIGHT 320
#endif

// --- Pin Definitions (OMOTE Rev5 - ESP32-S3) ---
namespace Pins {
    // Display (SPI)
    constexpr int TFT_CS    = 12;
    constexpr int TFT_DC    = 13;
    constexpr int TFT_RST   = 14;
    constexpr int TFT_MOSI  = 11;
    constexpr int TFT_SCLK  = 10;
    constexpr int TFT_BL    = 9;

    // Touch (I2C shared)
    constexpr int TOUCH_SDA = 8;
    constexpr int TOUCH_SCL = 18;
    constexpr int TOUCH_INT = 3;

    // I2C Bus (shared: IMU, Fuel Gauge, TCA8418, NFC)
    constexpr int I2C_SDA   = 8;
    constexpr int I2C_SCL   = 18;

    // IR
    constexpr int IR_TX     = 5;
    constexpr int IR_RX     = 15;

    // IMU (LIS3DH) - interrupt
    constexpr int IMU_INT   = 4;

    // TCA8418 Keypad - interrupt
    constexpr int KBD_INT   = 6;

    // Haptic Motor (PWM)
    constexpr int HAPTIC    = 7;

    // Battery Fuel Gauge - alert
    constexpr int BATT_ALERT = 16;

    // User LED
    constexpr int USER_LED  = 17;

    // NFC (optional, SPI or I2C)
    constexpr int NFC_IRQ   = 38;
    constexpr int NFC_RST   = 39;

    // Microphone I2S (optional)
    constexpr int MIC_WS    = 40;
    constexpr int MIC_SD    = 41;
    constexpr int MIC_SCK   = 42;
}

// --- WiFi ---
namespace WiFiConfig {
    constexpr const char* DEFAULT_SSID     = "";  // Set in secrets or web portal
    constexpr const char* DEFAULT_PASSWORD = "";
    constexpr int CONNECT_TIMEOUT_MS       = 10000;
    constexpr int RECONNECT_INTERVAL_MS    = 30000;
}

// --- MQTT ---
namespace MQTTConfig {
    constexpr const char* DEFAULT_SERVER    = "homeassistant.local";
    constexpr int DEFAULT_PORT             = 1883;
    constexpr const char* DEFAULT_USER     = "";
    constexpr const char* DEFAULT_PASSWORD = "";
    constexpr const char* TOPIC_PREFIX     = "controldiy/mando";
    constexpr const char* DISCOVERY_PREFIX = "homeassistant";
}

// --- Power Management ---
namespace PowerConfig {
    constexpr unsigned long IDLE_TIMEOUT_MS       = 120000;   // 2 min to dim
    constexpr unsigned long SLEEP_TIMEOUT_MS      = 300000;   // 5 min to deep sleep
    constexpr unsigned long DEEP_SLEEP_DURATION_S = 0;        // 0 = indefinite
    constexpr int BACKLIGHT_MAX                   = 255;
    constexpr int BACKLIGHT_DIM                   = 50;
    constexpr int BATTERY_LOW_PERCENT             = 15;
    constexpr int BATTERY_CRITICAL_PERCENT        = 5;
}

// --- Sync Protocol (between Control A and Control B) ---
namespace SyncConfig {
    constexpr const char* SYNC_TOPIC_STATE  = "controldiy/sync/state";
    constexpr const char* SYNC_TOPIC_CMD    = "controldiy/sync/cmd";
    constexpr const char* SYNC_TOPIC_SCENE  = "controldiy/sync/scene";
    constexpr int SYNC_INTERVAL_MS          = 1000;
}

// --- GUI ---
namespace GUIConfig {
    constexpr int CAROUSEL_ANIM_MS  = 300;
    constexpr int SCREEN_TRANSITION = 250;
    constexpr int TOAST_DURATION_MS = 2000;
    constexpr int IDLE_WIDGET_DELAY = 60000;  // Show widget after 1 min idle
}

// --- Feature Flags ---
namespace Features {
    #if ENABLE_HAPTIC
    constexpr bool HAPTIC_ENABLED = true;
    #else
    constexpr bool HAPTIC_ENABLED = false;
    #endif

    #if ENABLE_NFC
    constexpr bool NFC_ENABLED = true;
    #else
    constexpr bool NFC_ENABLED = false;
    #endif

    #if ENABLE_VOICE
    constexpr bool VOICE_ENABLED = true;
    #else
    constexpr bool VOICE_ENABLED = false;
    #endif

    #if ENABLE_SYNC
    constexpr bool SYNC_ENABLED = true;
    #else
    constexpr bool SYNC_ENABLED = false;
    #endif
}
