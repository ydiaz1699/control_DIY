/**
 * power_manager.h — Power Management
 * 
 * Handles sleep/wake lifecycle: Active → Dim → Deep Sleep
 * Wakeup sources: IMU (lift), keypad buttons
 */

#pragma once

#include <Arduino.h>

enum PowerState {
    POWER_ACTIVE = 0,
    POWER_DIM,
    POWER_SLEEP,
};

void power_manager_init();
void power_manager_check_sleep();
void power_enter_sleep();
PowerState power_get_state();
void power_wake_activity();
