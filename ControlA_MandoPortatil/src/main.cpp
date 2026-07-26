// ============================================================================
// Control A - Mando Portátil Universal
// Firmware fusionado: OMOTE + homeThing + OLED Remote + Everything Remote
// ============================================================================
// Hardware: OMOTE Rev5 (ESP32-S3 + PSRAM + TFT 2.8" + IR + BLE + IMU)
// 
// Architecture:
// - Core: AppController (state machine) + CommandRouter (action dispatch)
// - HAL: Display, Touch, Keypad, IR, BLE, IMU, Battery, Haptic
// - GUI: LVGL-based screens with carousel navigation
// - Connectivity: WiFi + MQTT + HA Bridge + Sync Protocol
// - Modules: Media, Climate, Lights, Covers, Scenes, Settings
// ============================================================================

#include <Arduino.h>
#include "config.h"

// Core
#include "core/app_controller.h"
#include "core/command_router.h"
#include "core/power_manager.h"
#include "core/scene_manager.h"

// HAL
#include "hal/display_driver.h"
#include "hal/ir_driver.h"
#include "hal/ble_driver.h"
#include "hal/haptic_driver.h"

// Connectivity
#include "connectivity/wifi_manager.h"
#include "connectivity/mqtt_client.h"
#include "connectivity/ha_bridge.h"
#include "connectivity/sync_protocol.h"

// ============================================================================
// Stub functions for battery (implemented via MAX17048 or ADC)
// ============================================================================

#if OMOTE_HARDWARE_REV >= 4
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
static SFE_MAX1704X fuelGauge(MAX1704X_MAX17048);
static bool fuelGaugeOk = false;
#endif

float battery_read_voltage() {
#if OMOTE_HARDWARE_REV >= 4
    if (fuelGaugeOk) return fuelGauge.getVoltage() / 1000.0f;
#endif
    return 3.7f;
}

float battery_read_percent() {
#if OMOTE_HARDWARE_REV >= 4
    if (fuelGaugeOk) return fuelGauge.getSOC();
#endif
    return 50.0f;
}

float battery_read_charge_rate() {
#if OMOTE_HARDWARE_REV >= 4
    if (fuelGaugeOk) return fuelGauge.getChangeRate();
#endif
    return 0.0f;
}

bool battery_is_charging() {
    return battery_read_charge_rate() > 0.1f;
}

// ============================================================================
// Main Setup
// ============================================================================

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n====================================");
    Serial.println("  Control DIY - Mando Portatil v1");
    Serial.println("  Hardware: OMOTE Rev" + String(OMOTE_HARDWARE_REV));
    Serial.println("====================================\n");

    // --- Hardware Layer Init ---
    Serial.println("[INIT] Display...");
    DisplayDriver::instance().init();

    Serial.println("[INIT] Power Manager...");
    PowerManager::instance().init();

    // Init fuel gauge
#if OMOTE_HARDWARE_REV >= 4
    Wire.begin(Pins::I2C_SDA, Pins::I2C_SCL);
    fuelGaugeOk = fuelGauge.begin();
    if (fuelGaugeOk) {
        fuelGauge.quickStart();
        Serial.println("[INIT] MAX17048 Fuel Gauge OK");
    } else {
        Serial.println("[WARN] MAX17048 not found");
    }
#endif

    Serial.println("[INIT] IR Driver...");
    IRDriver::instance().init();

#if ENABLE_BLE
    Serial.println("[INIT] BLE Driver...");
    BLEDriver::instance().init();
#endif

#if ENABLE_HAPTIC
    Serial.println("[INIT] Haptic Driver...");
    HapticDriver::instance().init();
    HapticDriver::instance().playPattern(HapticPattern::SUCCESS);
#endif

    // --- Connectivity ---
    Serial.println("[INIT] WiFi...");
    WiFiManager::instance().init();

#if ENABLE_MQTT
    Serial.println("[INIT] MQTT...");
    MQTTClient::instance().init();
#endif

    Serial.println("[INIT] HA Bridge...");
    HABridge::instance().init();

#if ENABLE_SYNC
    Serial.println("[INIT] Sync Protocol...");
    SyncProtocol::instance().init();
#endif

    // --- Application Layer ---
    Serial.println("[INIT] Command Router...");
    CommandRouter::instance().init();

    Serial.println("[INIT] Scene Manager...");
    SceneManager::instance().init();

    Serial.println("[INIT] App Controller...");
    AppController::instance().init();

    // --- Register Power Manager Callbacks ---
    PowerManager::instance().onBeforeSleep([]() {
        // Disconnect WiFi cleanly before sleep
        MQTTClient::instance().disconnect();
        WiFiManager::instance().disconnect();
        Serial.println("[SLEEP] Entering deep sleep...");
        Serial.flush();
    });

    PowerManager::instance().onBatteryUpdate([](const BatteryState& batt) {
        // Report battery to HA
        MQTTClient::instance().reportState("battery", String((int)batt.percent));
    });

    // --- Sync Protocol Callbacks ---
    SyncProtocol::instance().onSceneReceived([](const String& sceneName) {
        // Panel activated a scene, sync it locally
        Serial.println("[SYNC] Scene from panel: " + sceneName);
        SceneManager::instance().activateScene(sceneName);
    });

    SyncProtocol::instance().onNotificationReceived([](const String& title, const String& msg) {
        Serial.println("[SYNC] Notification: " + title + " - " + msg);
        // TODO: Show toast on GUI
    });

    // --- Done ---
    unsigned long bootTime = millis();
    Serial.printf("[INIT] Setup complete in %lu ms\n", bootTime);
    Serial.printf("[INFO] Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("[INFO] PSRAM: %d bytes\n", ESP.getFreePsram());
}

// ============================================================================
// Main Loop
// ============================================================================

static unsigned long lastLoopMs = 0;
static unsigned long statusReportMs = 0;

void loop() {
    unsigned long now = millis();

    // --- High frequency (every loop iteration) ---

    // Update LVGL display
    DisplayDriver::instance().update();

    // Update IR receiver (if learning)
    IRDriver::instance().update();

    // Update haptic pattern playback
    HapticDriver::instance().update();

    // --- Medium frequency (~every 10ms) ---
    if (now - lastLoopMs >= 10) {
        lastLoopMs = now;

        // Update app controller (processes module updates)
        AppController::instance().update();

        // Power management (dim/sleep checks)
        PowerManager::instance().update();

        // WiFi connection management
        WiFiManager::instance().update();

        // MQTT loop (receive messages)
        MQTTClient::instance().update();

        // Sync protocol heartbeat
        SyncProtocol::instance().update();
    }

    // --- Low frequency (~every 30 seconds) ---
    if (now - statusReportMs >= 30000) {
        statusReportMs = now;

        // Report WiFi signal strength
        if (WiFiManager::instance().isConnected()) {
            MQTTClient::instance().reportState("rssi", String(WiFiManager::instance().getRSSI()));
        }

        // Log status
        BatteryState batt = PowerManager::instance().getBatteryState();
        Serial.printf("[STATUS] Battery: %.0f%% (%.2fV) %s | WiFi: %s (%ddBm) | MQTT: %s | Peer: %s\n",
            batt.percent, batt.voltage,
            batt.isCharging ? "CHG" : "",
            WiFiManager::instance().isConnected() ? "OK" : "DISC",
            WiFiManager::instance().getRSSI(),
            MQTTClient::instance().isConnected() ? "OK" : "DISC",
            SyncProtocol::instance().isPeerOnline() ? "ONLINE" : "OFFLINE");
    }
}
