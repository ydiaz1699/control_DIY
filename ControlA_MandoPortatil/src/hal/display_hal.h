/**
 * display_hal.h — Display Hardware Abstraction Layer
 * 
 * Drives TFT 2.8" 320x240 via LovyanGFX (8-bit parallel Rev5 / SPI Rev1-4)
 * Integrates with LVGL for GUI rendering
 */

#pragma once

#include <Arduino.h>
#include <lvgl.h>

// Display dimensions
#define DISPLAY_WIDTH   240
#define DISPLAY_HEIGHT  320

// Display buffer size (using PSRAM on S3)
#define DISP_BUF_SIZE   (DISPLAY_WIDTH * 40)

/**
 * Initialize display hardware and LVGL display driver
 */
void display_init();

/**
 * Set backlight brightness (0-255)
 */
void display_set_brightness(uint8_t brightness);

/**
 * Get current backlight brightness
 */
uint8_t display_get_brightness();

/**
 * Fade backlight to target brightness over duration_ms
 */
void display_fade_brightness(uint8_t target, uint16_t duration_ms);

/**
 * Turn display completely off (before sleep)
 */
void display_off();

/**
 * Turn display on after wakeup
 */
void display_on();

/**
 * LVGL flush callback (internal, registered with lv_display)
 */
void display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

/**
 * LVGL touch read callback
 */
void touchpad_read_cb(lv_indev_t *indev, lv_indev_data_t *data);
