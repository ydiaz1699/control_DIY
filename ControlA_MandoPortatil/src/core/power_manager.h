#pragma once

#include <Arduino.h>
#include <functional>

// ============================================================================
// Power Manager - Battery monitoring, deep sleep, brightness control
// Combines: OMOTE fuel gauge + Everything Remote deep sleep + OLED Remote battery
// ============================================================================

/// Battery state
struct BatteryState {
    float voltage;      // Volts
    float percent;      // 0-100
    float chargeRate;   // mA (positive = charging)
    bool isCharging;
    bool isLow;
    bool isCritical;
};

/// Wakeup reason
enum class WakeupReason : uint8_t {
    POWER_ON,
    BUTTON_PRESS,
    IMU_MOTION,     // Lift-to-wake
    TIMER,
    TOUCH,
    UNKNOWN
};

/// Power Manager
class PowerManager {
public:
    static PowerManager& instance();

    void init();
    void update();

    // Sleep control
    void enterLightSleep();
    void enterDeepSleep();
    void preventSleep(bool prevent) { sleepPrevented_ = prevent; }
    WakeupReason getWakeupReason() const { return wakeupReason_; }

    // Backlight
    void setBacklight(int brightness);  // 0-255
    void fadeBacklight(int target, int durationMs);
    int getBacklight() const { return currentBrightness_; }
    void dimBacklight();
    void restoreBacklight();

    // Battery
    BatteryState getBatteryState() const { return battery_; }
    void updateBattery();

    // Activity tracking
    void onActivity();
    bool shouldDim() const;
    bool shouldSleep() const;

    // Callbacks
    using BatteryCallback = std::function<void(const BatteryState&)>;
    using SleepCallback = std::function<void()>;
    void onBatteryUpdate(BatteryCallback cb) { batteryCb_ = cb; }
    void onBeforeSleep(SleepCallback cb) { sleepCb_ = cb; }

private:
    PowerManager() = default;

    BatteryState battery_ = {};
    WakeupReason wakeupReason_ = WakeupReason::UNKNOWN;
    int currentBrightness_ = 255;
    int targetBrightness_ = 255;
    int savedBrightness_ = 255;
    bool isDimmed_ = false;
    bool sleepPrevented_ = false;

    unsigned long lastActivityMs_ = 0;
    unsigned long lastBatteryReadMs_ = 0;
    unsigned long fadeStartMs_ = 0;
    int fadeFromBrightness_ = 0;
    int fadeDurationMs_ = 0;

    BatteryCallback batteryCb_ = nullptr;
    SleepCallback sleepCb_ = nullptr;

    void detectWakeupReason();
    void configureSleepWakeup();
};
