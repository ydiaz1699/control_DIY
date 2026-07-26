/**
 * pin_config.h — Hardware pin definitions for OMOTE PCB Rev5
 * 
 * Supports both ESP32-S3 (Rev5) and ESP32 (Rev1-4) via HARDWARE_REV define
 */

#pragma once

#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════
// Display — 8-bit Parallel TFT (Rev5) / SPI TFT (Rev1-4)
// ═══════════════════════════════════════════════════════════════
#if (HARDWARE_REV >= 5)
  // 8-bit parallel interface (Rev5 ESP32-S3)
  #define LCD_D0_GPIO     40
  #define LCD_D1_GPIO     39
  #define LCD_D2_GPIO     38
  #define LCD_D3_GPIO     12
  #define LCD_D4_GPIO     11
  #define LCD_D5_GPIO     10
  #define LCD_D6_GPIO     9
  #define LCD_D7_GPIO     46
  #define LCD_WR_GPIO     8
  #define LCD_RD_GPIO     48
  #define LCD_DC_GPIO     47
  #define LCD_CS_GPIO     6
  #define LCD_EN_GPIO     7
  #define LCD_BL_GPIO     45
  #define LCD_RST_GPIO    4
  #define TOUCH_INT_GPIO  3
  #define TOUCH_SDA_GPIO  1
  #define TOUCH_SCL_GPIO  2
#else
  // SPI interface (Rev1-4 ESP32)
  #define LCD_MOSI_GPIO   23
  #define LCD_SCK_GPIO    18
  #define LCD_DC_GPIO     15
  #define LCD_CS_GPIO     5
  #define LCD_EN_GPIO     12
  #define LCD_BL_GPIO     26
  #define TOUCH_INT_GPIO  27
  #define TOUCH_SDA_GPIO  21
  #define TOUCH_SCL_GPIO  22
#endif

// ═══════════════════════════════════════════════════════════════
// IR — Transmitter and Receiver
// ═══════════════════════════════════════════════════════════════
#if (HARDWARE_REV >= 5)
  #define IR_LED_GPIO     5     // IR TX LED
  #define IR_RX_GPIO      15    // IR Receiver data
  #define IR_VCC_GPIO     16    // IR Receiver power enable
#else
  #define IR_LED_GPIO     33    // IR TX LED
  #define IR_RX_GPIO      14    // IR Receiver data
  #define IR_VCC_GPIO     25    // IR Receiver power enable
#endif

// ═══════════════════════════════════════════════════════════════
// IMU — LIS3DH Accelerometer (I2C)
// ═══════════════════════════════════════════════════════════════
#if (HARDWARE_REV >= 5)
  #define ACC_INT_GPIO    2     // IMU interrupt pin
#else
  #define ACC_INT_GPIO    13    // IMU interrupt pin
#endif
#define IMU_I2C_ADDR      0x19  // LIS3DH I2C address

// ═══════════════════════════════════════════════════════════════
// Battery — MAX17048 Fuel Gauge (I2C)
// ═══════════════════════════════════════════════════════════════
#if (HARDWARE_REV >= 5)
  #define CRG_STAT_GPIO   1     // Charge status pin
#elif (HARDWARE_REV == 4)
  #define CRG_STAT_GPIO   21    // Charge status pin
#else
  #define ADC_BAT_GPIO    36    // Battery ADC (Rev1-3 only)
#endif

// ═══════════════════════════════════════════════════════════════
// Keypad — TCA8418 I2C Matrix Controller (Rev5)
// ═══════════════════════════════════════════════════════════════
#if (HARDWARE_REV >= 5)
  #define KEYPAD_INT_GPIO   13    // TCA8418 interrupt
  #define KEYPAD_I2C_ADDR   0x34  // TCA8418 I2C address
#else
  // Direct GPIO matrix (Rev1-4)
  #define SW_1_GPIO       32
  #define SW_2_GPIO       26
  #define SW_3_GPIO       27
  #define SW_4_GPIO       14
  #define SW_5_GPIO       12
  // Row GPIOs for matrix scanning
  #define KEY_ROW_1       19
  #define KEY_ROW_2       4
  #define KEY_ROW_3       0
  #define KEY_ROW_4       2
#endif

// ═══════════════════════════════════════════════════════════════
// Haptic Motor (LRA via DRV2605 or simple PWM)
// ═══════════════════════════════════════════════════════════════
#if (HARDWARE_REV >= 5)
  #define HAPTIC_GPIO     42    // Haptic motor PWM
#else
  #define HAPTIC_GPIO     17    // Haptic motor PWM
#endif

// ═══════════════════════════════════════════════════════════════
// User LED
// ═══════════════════════════════════════════════════════════════
#if (HARDWARE_REV >= 5)
  #define USER_LED_GPIO   41
#else
  #define USER_LED_GPIO   2
#endif

// ═══════════════════════════════════════════════════════════════
// SD Card (Rev5 only)
// ═══════════════════════════════════════════════════════════════
#if (HARDWARE_REV >= 5)
  #define SD_CS_GPIO      37
  #define SD_EN_GPIO      36
  #define SD_MOSI_GPIO    35
  #define SD_MISO_GPIO    14
  #define SD_SCK_GPIO     34
#endif

// ═══════════════════════════════════════════════════════════════
// I2C Bus (shared: Touch, IMU, Fuel Gauge, TCA8418)
// ═══════════════════════════════════════════════════════════════
#if (HARDWARE_REV >= 5)
  #define I2C_SDA_GPIO    1
  #define I2C_SCL_GPIO    2
#else
  #define I2C_SDA_GPIO    21
  #define I2C_SCL_GPIO    22
#endif

// ═══════════════════════════════════════════════════════════════
// Wakeup Button Bitmask (for EXT1 deep sleep wakeup)
// ═══════════════════════════════════════════════════════════════
#if (HARDWARE_REV >= 5)
  // TCA8418 INT + IMU INT
  #define BUTTON_PIN_BITMASK  ((1ULL << KEYPAD_INT_GPIO) | (1ULL << ACC_INT_GPIO))
#else
  // Direct button GPIOs
  #define BUTTON_PIN_BITMASK  ((1ULL << SW_1_GPIO) | (1ULL << SW_2_GPIO) | \
                               (1ULL << SW_3_GPIO) | (1ULL << SW_4_GPIO) | \
                               (1ULL << SW_5_GPIO))
#endif
