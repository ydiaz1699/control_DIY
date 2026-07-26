/**
 * power_manager.cpp — Power Management Controller
 * 
 * Combines OMOTE's deep sleep + Everything Remote's ESP32 sleep strategy
 * with enhanced dim/sleep/wake transitions
 */

#include "power_manager.h"
#include "../config/user_config.h"
#include "../config/pin_config.h"
#include "../hal/display_hal.h"
#include "../hal/imu_hal.h"
#include "../hal/battery_hal.h"
#include "../hal/haptic_hal.h"
#include "../hal/ir_hal.h"
#include "../comms/wifi_manager.h"
#include "../comms/mqtt_manager.h"
#include "../comms/ble_keyboard.h"

static PowerState currentState = POWER_ACTIVE;
static bool dimmed = false;

void power_manager_init() {
    currentState = POWER_ACTIVE;
    dimmed = false;
    Serial.println("[POWER] Manager initialized");
}

void power_manager_check_sleep() {
    uint32_t idleTime = millis() - imu_get_last_activity();
    BatteryStatus batt = battery_get_status();
    
    // Critical battery: force sleep immediately
    if (batt.is_critical && !batt.is_charging) {
        Serial.println("[POWER] Critical battery! Entering emergency sleep...");
        power_enter_sleep();
        return;
    }
    
    // Normal idle transition: Active → Dim → Sleep
    if (idleTime > SLEEP_TIMEOUT_MS) {
        // Time to sleep
        power_enter_sleep();
    } else if (idleTime > DIM_TIMEOUT_MS && !dimmed) {
        // Dim the display
        dimmed = true;
        display_set_brightness(BRIGHTNESS_DIM);
        currentState = POWER_DIM;
    } else if (idleTime < DIM_TIMEOUT_MS && dimmed) {
        // Activity restored — undim
        dimmed = false;
        display_set_brightness(BRIGHTNESS_MAX);
        currentState = POWER_ACTIVE;
    }
}

void power_enter_sleep() {
    Serial.println("[POWER] Entering deep sleep...");
    currentState = POWER_SLEEP;
    
    // 1. Save preferences (NVS)
    // preferences_save();
    
    // 2. Shutdown communications
    #if ENABLE_MQTT
    mqtt_shutdown();
    #endif
    wifi_shutdown();
    #if ENABLE_BLE_KEYBOARD
    ble_keyboard_shutdown();
    #endif
    
    // 3. Turn off IR receiver
    ir_receiver_stop();
    
    // 4. Turn off display
    display_off();
    
    // 5. Configure IMU interrupt for lift-to-wake
    imu_configure_sleep_interrupt();
    
    // 6. Power down GPIOs
    #if (HARDWARE_REV >= 5)
    digitalWrite(LCD_EN_GPIO, HIGH);   // LCD power off
    digitalWrite(IR_VCC_GPIO, LOW);    // IR receiver off
    #endif
    
    // 7. Configure wakeup sources
    #if (HARDWARE_REV >= 5)
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_LOW);
    #else
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    #endif
    
    // 8. Hold GPIO states
    gpio_hold_en((gpio_num_t)LCD_BL_GPIO);
    gpio_hold_en((gpio_num_t)LCD_EN_GPIO);
    gpio_deep_sleep_hold_en();
    
    // 9. Enter deep sleep
    delay(50);
    esp_deep_sleep_start();
}

PowerState power_get_state() {
    return currentState;
}

void power_wake_activity() {
    imu_set_activity_timestamp();
    if (dimmed) {
        dimmed = false;
        display_set_brightness(BRIGHTNESS_MAX);
        currentState = POWER_ACTIVE;
    }
}
