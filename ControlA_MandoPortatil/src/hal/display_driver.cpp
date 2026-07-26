#include "display_driver.h"
#include "../config.h"

// Static members
lv_color_t* DisplayDriver::buf1_ = nullptr;
lv_color_t* DisplayDriver::buf2_ = nullptr;

// ============================================================================
// LovyanGFX Configuration for OMOTE
// ============================================================================

LGFX_OMOTE::LGFX_OMOTE() {
    {
        auto cfg = _bus_instance.config();
        cfg.spi_host = SPI2_HOST;
        cfg.spi_mode = 0;
        cfg.freq_write = 40000000;
        cfg.freq_read  = 16000000;
        cfg.pin_sclk = Pins::TFT_SCLK;
        cfg.pin_mosi = Pins::TFT_MOSI;
        cfg.pin_miso = -1;
        cfg.pin_dc   = Pins::TFT_DC;
        _bus_instance.config(cfg);
        _panel_instance.setBus(&_bus_instance);
    }
    {
        auto cfg = _panel_instance.config();
        cfg.pin_cs   = Pins::TFT_CS;
        cfg.pin_rst  = Pins::TFT_RST;
        cfg.pin_busy = -1;
        cfg.memory_width  = SCR_WIDTH;
        cfg.memory_height = SCR_HEIGHT;
        cfg.panel_width   = SCR_WIDTH;
        cfg.panel_height  = SCR_HEIGHT;
        cfg.offset_x = 0;
        cfg.offset_y = 0;
        cfg.offset_rotation = 0;
        cfg.readable = false;
        cfg.invert = false;
        cfg.rgb_order = false;
        cfg.dlen_16bit = false;
        cfg.bus_shared = false;
        _panel_instance.config(cfg);
    }
    {
        auto cfg = _light_instance.config();
        cfg.pin_bl = Pins::TFT_BL;
        cfg.invert = false;
        cfg.freq = 44100;
        cfg.pwm_channel = 0;
        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
}

// ============================================================================
// DisplayDriver Implementation
// ============================================================================

DisplayDriver& DisplayDriver::instance() {
    static DisplayDriver inst;
    return inst;
}

void DisplayDriver::init() {
    // Initialize TFT
    tft_.init();
    tft_.setRotation(0);
    tft_.setBrightness(255);

    // Initialize LVGL
    lv_init();

    // Allocate draw buffers (use PSRAM on Rev5)
    size_t bufSize = SCR_WIDTH * 40;  // 40 lines buffer

    #if defined(BOARD_HAS_PSRAM)
    buf1_ = (lv_color_t*)ps_malloc(bufSize * sizeof(lv_color_t));
    buf2_ = (lv_color_t*)ps_malloc(bufSize * sizeof(lv_color_t));
    #else
    buf1_ = (lv_color_t*)malloc(bufSize * sizeof(lv_color_t));
    buf2_ = nullptr;  // Single buffer for non-PSRAM boards
    #endif

    lv_disp_draw_buf_init(&drawBuf_, buf1_, buf2_, bufSize);

    // Register display driver
    lv_disp_drv_init(&dispDrv_);
    dispDrv_.hor_res = SCR_WIDTH;
    dispDrv_.ver_res = SCR_HEIGHT;
    dispDrv_.flush_cb = lvFlushCb;
    dispDrv_.draw_buf = &drawBuf_;
    dispDrv_.user_data = &tft_;
    lvDisplay_ = lv_disp_drv_register(&dispDrv_);
}

void DisplayDriver::update() {
    lv_timer_handler();
}

void DisplayDriver::setBacklight(int brightness) {
    backlightLevel_ = constrain(brightness, 0, 255);
    tft_.setBrightness(backlightLevel_);
}

void DisplayDriver::takeScreenshot() {
    // Output framebuffer as base64 via serial (like OLED Remote diagnostic)
    Serial.println("SCREENSHOT_BEGIN");
    // In a real implementation, read back the framebuffer
    Serial.println("SCREENSHOT_END");
}

void DisplayDriver::lvFlushCb(lv_disp_drv_t* drv, const lv_area_t* area, lv_color_t* color_p) {
    LGFX_OMOTE* gfx = (LGFX_OMOTE*)drv->user_data;
    int w = (area->x2 - area->x1 + 1);
    int h = (area->y2 - area->y1 + 1);
    gfx->startWrite();
    gfx->setAddrWindow(area->x1, area->y1, w, h);
    gfx->writePixels((lgfx::rgb565_t*)&color_p->full, w * h);
    gfx->endWrite();
    lv_disp_flush_ready(drv);
}

// Global accessor
void backlight_set(int brightness) {
    DisplayDriver::instance().setBacklight(brightness);
}
