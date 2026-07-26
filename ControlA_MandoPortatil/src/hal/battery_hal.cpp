/**
 * battery_hal.cpp — Battery Management Implementation
 * 
 * Based on OMOTE firmware battery_hal_esp32.cpp
 * Supports MAX17048 I2C fuel gauge (Rev4+) and ADC (Rev1-3)
 */

#include "battery_hal.h"
#include "../config/pin_config.h"
#include "../config/user_config.h"

#if (HARDWARE_REV >= 4)
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
static SFE_MAX1704X fuelGauge(MAX1704X_MAX17048);
#endif

static BatteryStatus currentStatus = {0, 0, false, false, false};

void battery_init() {
    #if (HARDWARE_REV >= 4)
    pinMode(CRG_STAT_GPIO, INPUT_PULLUP);
    fuelGauge.begin();
    Serial.println("[BATTERY] MAX17048 fuel gauge initialized");
    #else
    pinMode(ADC_BAT_GPIO, INPUT);
    Serial.println("[BATTERY] ADC battery monitoring initialized");
    #endif
    
    battery_update();
}

void battery_update() {
    #if (HARDWARE_REV >= 4)
    currentStatus.voltage_mv = (int)(fuelGauge.getVoltage() * 1000);
    float soc = fuelGauge.getSOC();
    if (soc > 100.0f) soc = 100.0f;
    if (soc < 0.0f) soc = 0.0f;
    currentStatus.percentage = (int)soc;
    currentStatus.is_charging = !digitalRead(CRG_STAT_GPIO);
    #else
    int adc_raw = analogRead(ADC_BAT_GPIO);
    currentStatus.voltage_mv = adc_raw * 2 * 3350 / 4095 + 325;
    currentStatus.percentage = constrain(
        map(currentStatus.voltage_mv, 3700, 4200, 0, 100), 0, 100);
    currentStatus.is_charging = false;
    #endif
    
    currentStatus.is_low = (currentStatus.percentage <= LOW_BATTERY_THRESHOLD);
    currentStatus.is_critical = (currentStatus.percentage <= CRITICAL_BATTERY_THRESHOLD);
}

BatteryStatus battery_get_status() {
    return currentStatus;
}

int battery_get_percentage() {
    return currentStatus.percentage;
}

bool battery_is_charging() {
    return currentStatus.is_charging;
}
