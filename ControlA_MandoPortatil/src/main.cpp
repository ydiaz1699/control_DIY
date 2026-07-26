/**
 * Control A — "Mando Portátil" — Main Entry Point
 * 
 * Firmware fusionado: OMOTE + homeThing + OLED Remote + The-Everything-Remote
 * Hardware: ESP32-S3, TFT 2.8" 320x240, IR TX/RX, BLE, WiFi, IMU, Fuel Gauge
 * 
 * (c) 2024-2026 — GPL v3
 * Basado en trabajo de OMOTE Community, homeThing, Paweł Lugowski, TheStockPot
 */

#include <Arduino.h>
#include "config/user_config.h"
#include "config/pin_config.h"
#include "hal/display_hal.h"
#include "hal/battery_hal.h"
#include "hal/imu_hal.h"
#include "hal/keypad_hal.h"
#include "hal/haptic_hal.h"
#include "hal/ir_hal.h"
#include "comms/wifi_manager.h"
#include "comms/mqtt_manager.h"
#include "comms/ble_keyboard.h"
#include "comms/espnow_sync.h"
#include "app/scene_manager.h"
#include "app/media_player.h"
#include "app/ir_controller.h"
#include "app/power_manager.h"
#include "gui/gui_manager.h"
#include "gui/theme_engine.h"

// --- Timers ---
static unsigned long lastGUIUpdate = 0;
static unsigned long lastStatusUpdate = 0;
static unsigned long lastIMUCheck = 0;
static unsigned long lastSyncBroadcast = 0;
static const unsigned long GUI_INTERVAL = 5;       // 5ms = ~200fps LVGL tick
static const unsigned long STATUS_INTERVAL = 1000; // 1s status updates
static const unsigned long IMU_INTERVAL = 100;     // 100ms motion check
static const unsigned long SYNC_INTERVAL = 5000;   // 5s ESP-NOW heartbeat

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n╔══════════════════════════════════════╗");
    Serial.println("║  Control A — Mando Portátil v1.0     ║");
    Serial.println("║  Fusión: OMOTE+homeThing+OLEDRemote  ║");
    Serial.println("╚══════════════════════════════════════╝");

    // --- Phase 1: Hardware Init ---
    Serial.println("[INIT] Phase 1: Hardware...");
    display_init();
    battery_init();
    imu_init();
    keypad_init();
    haptic_init();
    ir_init();
    
    // --- Phase 2: Power & Sleep ---
    Serial.println("[INIT] Phase 2: Power Manager...");
    power_manager_init();
    
    // --- Phase 3: GUI ---
    Serial.println("[INIT] Phase 3: GUI (LVGL)...");
    gui_init();
    theme_engine_init();
    
    // --- Phase 4: Communications ---
    Serial.println("[INIT] Phase 4: Communications...");
    wifi_manager_init();
    
    #if ENABLE_MQTT
    mqtt_manager_init();
    #endif
    
    #if ENABLE_BLE_KEYBOARD
    ble_keyboard_init();
    #endif
    
    #if ENABLE_ESPNOW
    espnow_sync_init();
    #endif
    
    // --- Phase 5: Application Layer ---
    Serial.println("[INIT] Phase 5: Application...");
    scene_manager_init();
    media_player_init();
    ir_controller_init();
    
    // --- Haptic feedback: startup complete ---
    haptic_pulse(HAPTIC_STARTUP);
    
    Serial.printf("[INIT] Complete! Free heap: %d bytes, PSRAM: %d bytes\n", 
                  ESP.getFreeHeap(), ESP.getFreePsram());
    Serial.printf("[INIT] Boot time: %lu ms\n", millis());
}

void loop() {
    unsigned long now = millis();
    
    // --- GUI Update (highest priority, 200fps) ---
    if (now - lastGUIUpdate >= GUI_INTERVAL) {
        lastGUIUpdate = now;
        gui_loop();
    }
    
    // --- IMU Activity Check (100ms) ---
    if (now - lastIMUCheck >= IMU_INTERVAL) {
        lastIMUCheck = now;
        imu_check_activity();
        power_manager_check_sleep();
    }
    
    // --- Keypad scan ---
    keypad_loop();
    
    // --- Communications (non-blocking) ---
    #if ENABLE_MQTT
    mqtt_manager_loop();
    #endif
    
    #if ENABLE_BLE_KEYBOARD
    ble_keyboard_loop();
    #endif
    
    // --- Status updates (1s) ---
    if (now - lastStatusUpdate >= STATUS_INTERVAL) {
        lastStatusUpdate = now;
        battery_update();
        gui_update_status_bar();
        theme_engine_check_time_change();
        media_player_poll_state();
    }
    
    // --- ESP-NOW sync heartbeat (5s) ---
    #if ENABLE_ESPNOW
    if (now - lastSyncBroadcast >= SYNC_INTERVAL) {
        lastSyncBroadcast = now;
        espnow_broadcast_state();
    }
    #endif
    
    // --- IR Receiver processing ---
    ir_receiver_loop();
    
    // Small yield to prevent WDT
    yield();
}
