/**
 * haptic_hal.h — Haptic Feedback HAL
 * 
 * Drives an LRA motor via PWM for tactile feedback on key presses,
 * mode changes, errors, and confirmations.
 */

#pragma once

#include <Arduino.h>

enum HapticPattern {
    HAPTIC_CLICK = 0,       // Short 10ms pulse (key press)
    HAPTIC_LONG_PRESS,      // Double pulse (long press detected)
    HAPTIC_MODE_CHANGE,     // Ramp up (mode/scene changed)
    HAPTIC_ERROR,           // Triple short (error/failure)
    HAPTIC_CONFIRM,         // Soft buzz (action confirmed)
    HAPTIC_STARTUP,         // Rising pattern (boot complete)
    HAPTIC_NOTIFICATION,    // Two gentle pulses (incoming notification)
};

void haptic_init();
void haptic_pulse(HapticPattern pattern);
void haptic_set_enabled(bool enabled);
bool haptic_is_enabled();
