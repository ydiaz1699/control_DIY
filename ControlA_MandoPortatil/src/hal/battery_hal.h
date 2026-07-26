/**
 * battery_hal.h — Battery Management HAL
 * 
 * MAX17048 fuel gauge (Rev4+) or ADC with voltage divider (Rev1-3)
 */

#pragma once

#include <Arduino.h>

struct BatteryStatus {
    int voltage_mv;      // Battery voltage in millivolts
    int percentage;      // 0-100%
    bool is_charging;    // true if connected to charger
    bool is_critical;    // true if below critical threshold
    bool is_low;         // true if below low threshold
};

void battery_init();
void battery_update();
BatteryStatus battery_get_status();
int battery_get_percentage();
bool battery_is_charging();
