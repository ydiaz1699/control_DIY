#pragma once

#include <Arduino.h>

// ============================================================================
// Haptic Driver - Vibration motor for tactile feedback
// Novel feature not present in original projects
// ============================================================================

/// Haptic patterns
enum class HapticPattern : uint8_t {
    CLICK,          // Short click (15ms)
    DOUBLE_CLICK,   // Two short pulses
    LONG_PRESS,     // Longer vibration (50ms)
    SUCCESS,        // Rising pattern
    ERROR,          // Rapid triple buzz
    NOTIFICATION,   // Soft pulse
    SCENE_CHANGE    // Medium distinctive pulse
};

class HapticDriver {
public:
    static HapticDriver& instance();

    void init();
    void update();

    // Basic control
    void pulse(int durationMs = 15);
    void setIntensity(int percent);  // 0-100

    // Predefined patterns
    void playPattern(HapticPattern pattern);

    // Enable/disable
    void enable(bool en) { enabled_ = en; }
    bool isEnabled() const { return enabled_; }

private:
    HapticDriver() = default;

    bool enabled_ = true;
    int intensity_ = 80;   // Default 80%
    int channel_ = 1;      // LEDC channel

    // Pattern playback
    struct PatternStep {
        int durationMs;
        bool active;
    };
    const PatternStep* currentPattern_ = nullptr;
    int patternLength_ = 0;
    int patternIndex_ = 0;
    unsigned long patternStartMs_ = 0;
    bool patternPlaying_ = false;

    void motorOn(int intensity);
    void motorOff();
};

// Global helper
void haptic_pulse(int durationMs);
