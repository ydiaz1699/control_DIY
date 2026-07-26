#pragma once

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>

// ============================================================================
// Display Driver - TFT ILI9341 via LovyanGFX + LVGL integration
// Based on OMOTE's display implementation
// ============================================================================

/// LovyanGFX display class for OMOTE hardware
class LGFX_OMOTE : public lgfx::LGFX_Device {
public:
    lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;

    LGFX_OMOTE();
};

/// Display Driver - wraps LovyanGFX and provides LVGL display driver
class DisplayDriver {
public:
    static DisplayDriver& instance();

    void init();
    void update();  // Call lv_timer_handler()

    // LVGL integration
    lv_disp_t* getLvDisplay() { return lvDisplay_; }
    
    // Backlight control
    void setBacklight(int brightness);  // 0-255
    int getBacklight() const { return backlightLevel_; }

    // Screen info
    int getWidth() const { return SCR_WIDTH; }
    int getHeight() const { return SCR_HEIGHT; }

    // Screenshot (for debugging, like OLED Remote)
    void takeScreenshot();

private:
    DisplayDriver() = default;

    LGFX_OMOTE tft_;
    lv_disp_t* lvDisplay_ = nullptr;
    lv_disp_draw_buf_t drawBuf_;
    lv_disp_drv_t dispDrv_;

    int backlightLevel_ = 255;

    // LVGL buffers (in PSRAM for Rev5)
    static lv_color_t* buf1_;
    static lv_color_t* buf2_;

    // LVGL flush callback
    static void lvFlushCb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p);
};

// Global accessor for power manager
void backlight_set(int brightness);
