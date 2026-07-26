/**
 * imu_hal.cpp — IMU Implementation
 * 
 * Based on OMOTE sleep_hal_esp32.cpp with enhanced gesture detection
 */

#include "imu_hal.h"
#include "../config/pin_config.h"
#include "../config/user_config.h"
#include <SparkFunLIS3DH.h>

static LIS3DH IMU(I2C_MODE, IMU_I2C_ADDR);
static WakeupReason wakeup_reason = WAKEUP_BY_RESET;
static uint32_t lastActivityTimestamp = 0;
static int accXold = 0, accYold = 0, accZold = 0;

void imu_init() {
    // Determine wakeup reason
    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
        if (esp_sleep_get_ext1_wakeup_status() == (1ULL << ACC_INT_GPIO)) {
            wakeup_reason = WAKEUP_BY_IMU;
        } else {
            wakeup_reason = WAKEUP_BY_KEYPAD;
        }
    } else {
        wakeup_reason = WAKEUP_BY_RESET;
    }
    
    pinMode(ACC_INT_GPIO, INPUT);
    
    // Release GPIO hold from sleep
    gpio_hold_dis((gpio_num_t)LCD_EN_GPIO);
    gpio_hold_dis((gpio_num_t)LCD_BL_GPIO);
    gpio_deep_sleep_hold_dis();
    
    // Configure IMU
    IMU.settings.accelSampleRate = 50;   // 50 Hz
    IMU.settings.accelRange = 2;          // +/- 2G
    IMU.settings.adcEnabled = 0;
    IMU.settings.tempEnabled = 0;
    IMU.settings.xAccelEnabled = 1;
    IMU.settings.yAccelEnabled = 1;
    IMU.settings.zAccelEnabled = 1;
    IMU.begin();
    
    // Clear any pending interrupts
    uint8_t intData;
    IMU.readRegister(&intData, LIS3DH_INT1_SRC);
    
    lastActivityTimestamp = millis();
    
    Serial.printf("[IMU] Initialized. Wakeup reason: %d\n", wakeup_reason);
}

void imu_check_activity() {
    int accX = IMU.readFloatAccelX() * 1000;
    int accY = IMU.readFloatAccelY() * 1000;
    int accZ = IMU.readFloatAccelZ() * 1000;
    
    int motion = abs(accXold - accX) + abs(accYold - accY) + abs(accZold - accZ);
    
    if (motion > MOTION_THRESHOLD) {
        lastActivityTimestamp = millis();
    }
    
    accXold = accX;
    accYold = accY;
    accZold = accZ;
}

void imu_configure_sleep_interrupt() {
    if (WAKEUP_BY_IMU_ENABLED) {
        IMU.writeRegister(LIS3DH_INT1_CFG, 0b00101010); // X,Y,Z high
    } else {
        IMU.writeRegister(LIS3DH_INT1_CFG, 0b00000000);
    }
    
    IMU.writeRegister(LIS3DH_INT1_THS, 0x45);           // Threshold
    IMU.writeRegister(LIS3DH_INT1_DURATION, 0x00);      // Minimum duration
    
    // Latch interrupt
    uint8_t reg5;
    IMU.readRegister(&reg5, LIS3DH_CTRL_REG5);
    reg5 &= 0xF3;
    reg5 |= 0x08;
    IMU.writeRegister(LIS3DH_CTRL_REG5, reg5);
    
    // Active-low interrupt (Rev5) or active-high (earlier)
    #if (HARDWARE_REV >= 5)
    IMU.writeRegister(LIS3DH_CTRL_REG6, 0x02);
    #else
    IMU.writeRegister(LIS3DH_CTRL_REG6, 0x00);
    #endif
    
    // Route to INT1 pin
    IMU.writeRegister(LIS3DH_CTRL_REG3, 0x40 | 0x20);
}

WakeupReason imu_get_wakeup_reason() {
    return wakeup_reason;
}

void imu_set_activity_timestamp() {
    lastActivityTimestamp = millis();
}

uint32_t imu_get_last_activity() {
    return lastActivityTimestamp;
}

bool imu_is_face_up() {
    int accZ = IMU.readFloatAccelZ() * 1000;
    return accZ > 800; // Roughly pointing up
}

bool imu_detect_shake() {
    int accX = IMU.readFloatAccelX() * 1000;
    int accY = IMU.readFloatAccelY() * 1000;
    int total = abs(accX) + abs(accY);
    return total > 2000; // Strong motion = shake
}
