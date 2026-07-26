/**
 * haptic_hal.cpp — Haptic Feedback Implementation
 * 
 * Simple PWM-driven LRA motor with pattern support
 */

#include "haptic_hal.h"
#include "../config/pin_config.h"

#define HAPTIC_PWM_CHANNEL  3
#define HAPTIC_PWM_FREQ     200   // Hz
#define HAPTIC_PWM_RES      8     // 8-bit resolution

static bool hapticEnabled = true;
static TaskHandle_t hapticTaskHandle = nullptr;

// Pattern definitions: {duty_cycle (0-255), duration_ms}
struct PulseStep {
    uint8_t duty;
    uint16_t duration;
};

static const PulseStep PATTERN_CLICK[] = {
    {200, 12}, {0, 0}
};

static const PulseStep PATTERN_LONG_PRESS[] = {
    {180, 15}, {0, 30}, {180, 15}, {0, 0}
};

static const PulseStep PATTERN_MODE_CHANGE[] = {
    {100, 10}, {150, 10}, {200, 15}, {0, 0}
};

static const PulseStep PATTERN_ERROR[] = {
    {200, 8}, {0, 20}, {200, 8}, {0, 20}, {200, 8}, {0, 0}
};

static const PulseStep PATTERN_CONFIRM[] = {
    {150, 30}, {0, 0}
};

static const PulseStep PATTERN_STARTUP[] = {
    {80, 20}, {120, 20}, {160, 20}, {200, 30}, {0, 0}
};

static const PulseStep PATTERN_NOTIFICATION[] = {
    {130, 20}, {0, 50}, {130, 20}, {0, 0}
};

static void playPattern(const PulseStep* pattern) {
    for (int i = 0; pattern[i].duty != 0 || pattern[i].duration != 0; i++) {
        ledcWrite(HAPTIC_PWM_CHANNEL, pattern[i].duty);
        if (pattern[i].duration > 0) {
            delay(pattern[i].duration);
        }
    }
    ledcWrite(HAPTIC_PWM_CHANNEL, 0);
}

void haptic_init() {
    #if ENABLE_HAPTIC
    ledcSetup(HAPTIC_PWM_CHANNEL, HAPTIC_PWM_FREQ, HAPTIC_PWM_RES);
    ledcAttachPin(HAPTIC_GPIO, HAPTIC_PWM_CHANNEL);
    ledcWrite(HAPTIC_PWM_CHANNEL, 0);
    Serial.println("[HAPTIC] Initialized on GPIO " + String(HAPTIC_GPIO));
    #endif
}

void haptic_pulse(HapticPattern pattern) {
    #if ENABLE_HAPTIC
    if (!hapticEnabled) return;
    
    switch (pattern) {
        case HAPTIC_CLICK:        playPattern(PATTERN_CLICK); break;
        case HAPTIC_LONG_PRESS:   playPattern(PATTERN_LONG_PRESS); break;
        case HAPTIC_MODE_CHANGE:  playPattern(PATTERN_MODE_CHANGE); break;
        case HAPTIC_ERROR:        playPattern(PATTERN_ERROR); break;
        case HAPTIC_CONFIRM:      playPattern(PATTERN_CONFIRM); break;
        case HAPTIC_STARTUP:      playPattern(PATTERN_STARTUP); break;
        case HAPTIC_NOTIFICATION: playPattern(PATTERN_NOTIFICATION); break;
    }
    #endif
}

void haptic_set_enabled(bool enabled) {
    hapticEnabled = enabled;
}

bool haptic_is_enabled() {
    return hapticEnabled;
}
