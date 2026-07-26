/**
 * imu_hal.h — IMU (Inertial Measurement Unit) HAL
 * 
 * LIS3DH accelerometer for:
 * - Lift-to-wake detection
 * - Motion activity tracking
 * - Gesture recognition (tilt, shake)
 */

#pragma once

#include <Arduino.h>

enum WakeupReason {
    WAKEUP_BY_RESET = 0,
    WAKEUP_BY_IMU,
    WAKEUP_BY_KEYPAD,
    WAKEUP_BY_TIMER,
    WAKEUP_BY_TOUCH
};

void imu_init();
void imu_check_activity();
void imu_configure_sleep_interrupt();
WakeupReason imu_get_wakeup_reason();
void imu_set_activity_timestamp();
uint32_t imu_get_last_activity();
bool imu_is_face_up();
bool imu_detect_shake();
