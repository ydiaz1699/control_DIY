#include "haptic_driver.h"
#include "../config.h"

// Pattern definitions
static const HapticDriver::PatternStep patClick[] = {{15, true}};
static const HapticDriver::PatternStep patDoubleClick[] = {{10, true}, {30, false}, {10, true}};
static const HapticDriver::PatternStep patLongPress[] = {{50, true}};
static const HapticDriver::PatternStep patSuccess[] = {{20, true}, {20, false}, {30, true}};
static const HapticDriver::PatternStep patError[] = {{10, true}, {10, false}, {10, true}, {10, false}, {10, true}};
static const HapticDriver::PatternStep patNotification[] = {{30, true}};
static const HapticDriver::PatternStep patSceneChange[] = {{40, true}, {30, false}, {20, true}};

HapticDriver& HapticDriver::instance() {
    static HapticDriver inst;
    return inst;
}

void HapticDriver::init() {
#if ENABLE_HAPTIC
    ledcSetup(channel_, 20000, 8);  // 20kHz PWM, 8-bit
    ledcAttachPin(Pins::HAPTIC, channel_);
    ledcWrite(channel_, 0);
    enabled_ = true;
#else
    enabled_ = false;
#endif
}

void HapticDriver::update() {
    if (!patternPlaying_) return;

    unsigned long elapsed = millis() - patternStartMs_;
    unsigned long accumulated = 0;

    for (int i = 0; i <= patternIndex_; i++) {
        accumulated += currentPattern_[i].durationMs;
    }

    if (elapsed >= accumulated) {
        patternIndex_++;
        if (patternIndex_ >= patternLength_) {
            // Pattern complete
            motorOff();
            patternPlaying_ = false;
            return;
        }
        if (currentPattern_[patternIndex_].active) {
            motorOn(intensity_);
        } else {
            motorOff();
        }
    }
}

void HapticDriver::pulse(int durationMs) {
    if (!enabled_) return;
    motorOn(intensity_);
    delay(durationMs);
    motorOff();
}

void HapticDriver::setIntensity(int percent) {
    intensity_ = constrain(percent, 0, 100);
}

void HapticDriver::playPattern(HapticPattern pattern) {
    if (!enabled_) return;

    switch (pattern) {
        case HapticPattern::CLICK:
            currentPattern_ = patClick;
            patternLength_ = 1;
            break;
        case HapticPattern::DOUBLE_CLICK:
            currentPattern_ = patDoubleClick;
            patternLength_ = 3;
            break;
        case HapticPattern::LONG_PRESS:
            currentPattern_ = patLongPress;
            patternLength_ = 1;
            break;
        case HapticPattern::SUCCESS:
            currentPattern_ = patSuccess;
            patternLength_ = 3;
            break;
        case HapticPattern::ERROR:
            currentPattern_ = patError;
            patternLength_ = 5;
            break;
        case HapticPattern::NOTIFICATION:
            currentPattern_ = patNotification;
            patternLength_ = 1;
            break;
        case HapticPattern::SCENE_CHANGE:
            currentPattern_ = patSceneChange;
            patternLength_ = 3;
            break;
    }

    patternIndex_ = 0;
    patternStartMs_ = millis();
    patternPlaying_ = true;

    if (currentPattern_[0].active) {
        motorOn(intensity_);
    }
}

void HapticDriver::motorOn(int intensityPercent) {
#if ENABLE_HAPTIC
    int pwm = map(intensityPercent, 0, 100, 0, 255);
    ledcWrite(channel_, pwm);
#endif
}

void HapticDriver::motorOff() {
#if ENABLE_HAPTIC
    ledcWrite(channel_, 0);
#endif
}

// Global helper
void haptic_pulse(int durationMs) {
    HapticDriver::instance().pulse(durationMs);
}
