#include "power_manager.h"
#include "../config.h"
#include <esp_sleep.h>
#include <driver/rtc_io.h>

// External hardware drivers
extern float battery_read_voltage();
extern float battery_read_percent();
extern float battery_read_charge_rate();
extern bool battery_is_charging();
extern void backlight_set(int brightness);

// ============================================================================
// PowerManager Implementation
// ============================================================================

PowerManager& PowerManager::instance() {
    static PowerManager inst;
    return inst;
}

void PowerManager::init() {
    detectWakeupReason();
    lastActivityMs_ = millis();
    lastBatteryReadMs_ = 0;  // Force first read
    currentBrightness_ = PowerConfig::BACKLIGHT_MAX;
    targetBrightness_ = PowerConfig::BACKLIGHT_MAX;
    savedBrightness_ = PowerConfig::BACKLIGHT_MAX;

    // Initial battery read
    updateBattery();

    // Set initial backlight
    backlight_set(currentBrightness_);
}

void PowerManager::update() {
    unsigned long now = millis();

    // Update battery every 60 seconds
    if (now - lastBatteryReadMs_ >= 60000) {
        updateBattery();
        lastBatteryReadMs_ = now;
    }

    // Handle backlight fade animation
    if (fadeDurationMs_ > 0 && currentBrightness_ != targetBrightness_) {
        unsigned long elapsed = now - fadeStartMs_;
        if (elapsed >= (unsigned long)fadeDurationMs_) {
            currentBrightness_ = targetBrightness_;
            fadeDurationMs_ = 0;
        } else {
            float progress = (float)elapsed / fadeDurationMs_;
            currentBrightness_ = fadeFromBrightness_ +
                (int)((targetBrightness_ - fadeFromBrightness_) * progress);
        }
        backlight_set(currentBrightness_);
    }

    // Auto-dim after idle timeout
    if (!sleepPrevented_) {
        if (shouldSleep()) {
            enterDeepSleep();
        } else if (shouldDim() && !isDimmed_) {
            dimBacklight();
        }
    }
}

void PowerManager::enterLightSleep() {
    if (sleepPrevented_) return;

    // Save state
    if (sleepCb_) sleepCb_();

    // Turn off backlight
    backlight_set(0);

    // Configure wakeup sources
    configureSleepWakeup();

    // Enter light sleep
    esp_light_sleep_start();

    // --- Wakes up here ---
    onActivity();
    restoreBacklight();
}

void PowerManager::enterDeepSleep() {
    if (sleepPrevented_) return;

    // Notify before sleep
    if (sleepCb_) sleepCb_();

    // Turn off backlight
    backlight_set(0);

    // Configure wakeup sources
    configureSleepWakeup();

    // If deep sleep duration is set, add timer wakeup
    if (PowerConfig::DEEP_SLEEP_DURATION_S > 0) {
        esp_sleep_enable_timer_wakeup(
            (uint64_t)PowerConfig::DEEP_SLEEP_DURATION_S * 1000000ULL);
    }

    // Enter deep sleep
    esp_deep_sleep_start();
}

void PowerManager::setBacklight(int brightness) {
    brightness = constrain(brightness, 0, 255);
    currentBrightness_ = brightness;
    targetBrightness_ = brightness;
    savedBrightness_ = brightness;
    fadeDurationMs_ = 0;
    backlight_set(brightness);
}

void PowerManager::fadeBacklight(int target, int durationMs) {
    target = constrain(target, 0, 255);
    targetBrightness_ = target;
    fadeFromBrightness_ = currentBrightness_;
    fadeDurationMs_ = durationMs;
    fadeStartMs_ = millis();
}

void PowerManager::dimBacklight() {
    if (isDimmed_) return;
    savedBrightness_ = currentBrightness_;
    fadeBacklight(PowerConfig::BACKLIGHT_DIM, 500);
    isDimmed_ = true;
}

void PowerManager::restoreBacklight() {
    if (!isDimmed_) return;
    fadeBacklight(savedBrightness_, 200);
    isDimmed_ = false;
}

void PowerManager::updateBattery() {
    battery_.voltage = battery_read_voltage();
    battery_.percent = battery_read_percent();
    battery_.chargeRate = battery_read_charge_rate();
    battery_.isCharging = battery_is_charging();
    battery_.isLow = battery_.percent <= PowerConfig::BATTERY_LOW_PERCENT;
    battery_.isCritical = battery_.percent <= PowerConfig::BATTERY_CRITICAL_PERCENT;

    if (batteryCb_) {
        batteryCb_(battery_);
    }

    // Force sleep on critical battery
    if (battery_.isCritical && !battery_.isCharging) {
        enterDeepSleep();
    }
}

void PowerManager::onActivity() {
    lastActivityMs_ = millis();
    if (isDimmed_) {
        restoreBacklight();
    }
}

bool PowerManager::shouldDim() const {
    return (millis() - lastActivityMs_) > PowerConfig::IDLE_TIMEOUT_MS;
}

bool PowerManager::shouldSleep() const {
    return (millis() - lastActivityMs_) > PowerConfig::SLEEP_TIMEOUT_MS;
}

void PowerManager::detectWakeupReason() {
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    switch (cause) {
        case ESP_SLEEP_WAKEUP_EXT0:
        case ESP_SLEEP_WAKEUP_EXT1:
            wakeupReason_ = WakeupReason::BUTTON_PRESS;
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            wakeupReason_ = WakeupReason::TIMER;
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            wakeupReason_ = WakeupReason::TOUCH;
            break;
        case ESP_SLEEP_WAKEUP_GPIO:
            // Could be IMU interrupt (lift-to-wake)
            wakeupReason_ = WakeupReason::IMU_MOTION;
            break;
        default:
            wakeupReason_ = WakeupReason::POWER_ON;
            break;
    }
}

void PowerManager::configureSleepWakeup() {
    // Wakeup by IMU motion (lift-to-wake) on GPIO4
    esp_sleep_enable_ext0_wakeup((gpio_num_t)Pins::IMU_INT, 0);

    // Wakeup by keypad interrupt on GPIO6
    esp_sleep_enable_ext1_wakeup(1ULL << Pins::KBD_INT, ESP_EXT1_WAKEUP_ALL_LOW);
}
