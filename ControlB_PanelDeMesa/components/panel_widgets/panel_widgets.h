/**
 * panel_widgets.h — Panel Widget Overlay Component
 * 
 * Provides a slide-out widget panel overlaid on the RemoteWebView output.
 * Features:
 * - Swipe from left edge to reveal quick-action panel
 * - Scene buttons with icons
 * - Clock/weather display
 * - Control A status (battery, mode)
 * - Gesture detection (swipe, long-press)
 */

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display.h"
#include "esphome/components/touchscreen/touchscreen.h"
#include <vector>
#include <string>

namespace esphome {
namespace panel_widgets {

struct SceneEntry {
    std::string name;
    std::string icon;
    std::string service;
    std::string entity;
};

enum WidgetState {
    WIDGET_HIDDEN = 0,
    WIDGET_SLIDING_IN,
    WIDGET_VISIBLE,
    WIDGET_SLIDING_OUT,
};

enum GestureType {
    GESTURE_NONE = 0,
    GESTURE_SWIPE_LEFT,
    GESTURE_SWIPE_RIGHT,
    GESTURE_SWIPE_UP,
    GESTURE_SWIPE_DOWN,
    GESTURE_LONG_PRESS,
    GESTURE_PINCH_IN,
    GESTURE_PINCH_OUT,
};

class PanelWidgets : public Component {
 public:
    void setup() override;
    void loop() override;
    float get_setup_priority() const override { return setup_priority::LATE; }

    // Configuration setters (called from __init__.py)
    void set_display(display::Display *disp) { this->display_ = disp; }
    void set_touchscreen(touchscreen::Touchscreen *ts) { this->touchscreen_ = ts; }
    void set_bar_width(int width) { this->bar_width_ = width; }
    void set_bar_position(bool right) { this->bar_on_right_ = right; }
    void add_scene(const std::string &name, const std::string &icon,
                   const std::string &service, const std::string &entity);

    // Public API
    void toggle_visibility();
    void show();
    void hide();
    bool is_visible() const { return state_ == WIDGET_VISIBLE; }
    
    // Gesture detection results
    GestureType get_last_gesture() const { return last_gesture_; }

 protected:
    void detect_gestures_();
    void render_widget_bar_();
    void render_clock_widget_();
    void render_scene_buttons_();
    void render_status_bar_();
    void handle_scene_tap_(int index);
    void animate_slide_();

    display::Display *display_{nullptr};
    touchscreen::Touchscreen *touchscreen_{nullptr};
    
    int bar_width_{80};
    bool bar_on_right_{false};
    
    std::vector<SceneEntry> scenes_;
    WidgetState state_{WIDGET_HIDDEN};
    GestureType last_gesture_{GESTURE_NONE};
    
    // Animation
    float slide_progress_{0.0f};  // 0.0 = hidden, 1.0 = fully visible
    uint32_t animation_start_{0};
    static const uint32_t ANIMATION_DURATION = 250;  // ms
    
    // Gesture tracking
    int16_t touch_start_x_{0};
    int16_t touch_start_y_{0};
    uint32_t touch_start_time_{0};
    bool touch_active_{false};
    bool gesture_detected_{false};
    
    // Edge detection for swipe-to-reveal
    static const int EDGE_THRESHOLD = 30;  // pixels from edge
    static const int SWIPE_MIN_DISTANCE = 50;  // minimum swipe distance
    static const uint32_t LONG_PRESS_TIME = 600;  // ms
};

}  // namespace panel_widgets
}  // namespace esphome
