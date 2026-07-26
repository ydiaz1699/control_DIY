/**
 * display_hal.cpp — Display HAL Implementation
 * 
 * LovyanGFX driver for OMOTE TFT + LVGL integration
 */

#include "display_hal.h"
#include "../config/pin_config.h"
#include "../config/user_config.h"

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// ═══════════════════════════════════════════════════════════════
// LovyanGFX Display Configuration
// ═══════════════════════════════════════════════════════════════

class LGFX_OMOTE : public lgfx::LGFX_Device {
    #if (HARDWARE_REV >= 5)
    lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Bus_Parallel8 _bus_instance;
    #else
    lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    #endif
    lgfx::Light_PWM _light_instance;
    lgfx::Touch_FT5x06 _touch_instance;

public:
    LGFX_OMOTE(void) {
        #if (HARDWARE_REV >= 5)
        // 8-bit parallel bus configuration (Rev5)
        {
            auto cfg = _bus_instance.config();
            cfg.freq_write = 16000000;
            cfg.pin_wr = LCD_WR_GPIO;
            cfg.pin_rd = LCD_RD_GPIO;
            cfg.pin_rs = LCD_DC_GPIO;
            cfg.pin_d0 = LCD_D0_GPIO;
            cfg.pin_d1 = LCD_D1_GPIO;
            cfg.pin_d2 = LCD_D2_GPIO;
            cfg.pin_d3 = LCD_D3_GPIO;
            cfg.pin_d4 = LCD_D4_GPIO;
            cfg.pin_d5 = LCD_D5_GPIO;
            cfg.pin_d6 = LCD_D6_GPIO;
            cfg.pin_d7 = LCD_D7_GPIO;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        #else
        // SPI bus configuration (Rev1-4)
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = VSPI_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.pin_sclk = LCD_SCK_GPIO;
            cfg.pin_mosi = LCD_MOSI_GPIO;
            cfg.pin_miso = -1;
            cfg.pin_dc = LCD_DC_GPIO;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        #endif

        // Panel configuration
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = LCD_CS_GPIO;
            cfg.pin_rst = -1;
            cfg.pin_busy = -1;
            cfg.memory_width = DISPLAY_WIDTH;
            cfg.memory_height = DISPLAY_HEIGHT;
            cfg.panel_width = DISPLAY_WIDTH;
            cfg.panel_height = DISPLAY_HEIGHT;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = true;
            cfg.invert = false;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;
            _panel_instance.config(cfg);
        }

        // Backlight configuration
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = LCD_BL_GPIO;
            cfg.invert = true;  // Active low
            cfg.freq = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        // Touchscreen configuration (FT5x06 capacitive)
        {
            auto cfg = _touch_instance.config();
            cfg.x_min = 0;
            cfg.x_max = DISPLAY_WIDTH - 1;
            cfg.y_min = 0;
            cfg.y_max = DISPLAY_HEIGHT - 1;
            cfg.pin_int = TOUCH_INT_GPIO;
            cfg.bus_shared = false;
            cfg.offset_rotation = 0;
            cfg.i2c_port = 0;
            cfg.i2c_addr = 0x38;
            cfg.pin_sda = I2C_SDA_GPIO;
            cfg.pin_scl = I2C_SCL_GPIO;
            cfg.freq = 400000;
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};

// ═══════════════════════════════════════════════════════════════
// Global instances
// ═══════════════════════════════════════════════════════════════

static LGFX_OMOTE tft;
static lv_display_t *lvgl_display = nullptr;
static lv_indev_t *lvgl_touch = nullptr;
static uint8_t *draw_buf1 = nullptr;
static uint8_t *draw_buf2 = nullptr;
static uint8_t current_brightness = BRIGHTNESS_MAX;

// Touch state
static bool touch_pressed = false;
static int16_t touch_x = 0, touch_y = 0;

// ═══════════════════════════════════════════════════════════════
// LVGL Callbacks
// ═══════════════════════════════════════════════════════════════

void display_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.writePixels((lgfx::rgb565_t *)px_map, w * h);
    tft.endWrite();
    
    lv_display_flush_ready(disp);
}

void touchpad_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    uint16_t tx, ty;
    if (tft.getTouch(&tx, &ty)) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = tx;
        data->point.y = ty;
        touch_pressed = true;
        touch_x = tx;
        touch_y = ty;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
        touch_pressed = false;
    }
}

// ═══════════════════════════════════════════════════════════════
// Public API
// ═══════════════════════════════════════════════════════════════

void display_init() {
    // Power on display
    #if (HARDWARE_REV >= 5)
    pinMode(LCD_EN_GPIO, OUTPUT);
    digitalWrite(LCD_EN_GPIO, LOW);  // Enable display power
    #else
    pinMode(LCD_EN_GPIO, OUTPUT);
    digitalWrite(LCD_EN_GPIO, LOW);
    #endif
    
    delay(50);
    
    // Initialize LovyanGFX
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setBrightness(BRIGHTNESS_MAX);
    current_brightness = BRIGHTNESS_MAX;
    
    // Initialize LVGL
    lv_init();
    
    // Allocate draw buffers (in PSRAM if available)
    #ifdef BOARD_HAS_PSRAM
    draw_buf1 = (uint8_t *)ps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t));
    draw_buf2 = (uint8_t *)ps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t));
    #else
    draw_buf1 = (uint8_t *)malloc(DISP_BUF_SIZE * sizeof(lv_color_t));
    draw_buf2 = nullptr;  // Single buffer on non-PSRAM
    #endif
    
    // Create LVGL display
    lvgl_display = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_display_set_flush_cb(lvgl_display, display_flush_cb);
    
    if (draw_buf2) {
        lv_display_set_buffers(lvgl_display, draw_buf1, draw_buf2, 
                              DISP_BUF_SIZE * sizeof(lv_color_t), 
                              LV_DISPLAY_RENDER_MODE_PARTIAL);
    } else {
        lv_display_set_buffers(lvgl_display, draw_buf1, nullptr,
                              DISP_BUF_SIZE * sizeof(lv_color_t),
                              LV_DISPLAY_RENDER_MODE_PARTIAL);
    }
    
    // Create touch input device
    lvgl_touch = lv_indev_create();
    lv_indev_set_type(lvgl_touch, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(lvgl_touch, touchpad_read_cb);
    
    Serial.println("[DISPLAY] Initialized: 320x240 TFT + Touch + LVGL");
}

void display_set_brightness(uint8_t brightness) {
    current_brightness = brightness;
    tft.setBrightness(brightness);
}

uint8_t display_get_brightness() {
    return current_brightness;
}

void display_fade_brightness(uint8_t target, uint16_t duration_ms) {
    int steps = 20;
    int delay_per_step = duration_ms / steps;
    int diff = (int)target - (int)current_brightness;
    
    for (int i = 1; i <= steps; i++) {
        uint8_t val = current_brightness + (diff * i / steps);
        tft.setBrightness(val);
        delay(delay_per_step);
    }
    current_brightness = target;
}

void display_off() {
    display_fade_brightness(0, 200);
    tft.sleep();
}

void display_on() {
    tft.wakeup();
    display_fade_brightness(BRIGHTNESS_MAX, 150);
}
