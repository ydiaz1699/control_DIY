/**
 * panel_widgets.cpp — Panel Widget Overlay Implementation
 * 
 * Slide-out widget panel with gesture detection and scene buttons
 */

#include "panel_widgets.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace panel_widgets {

static const char *TAG = "panel_widgets";

void PanelWidgets::setup() {
    ESP_LOGI(TAG, "Panel Widgets initialized with %d scenes", this->scenes_.size());
    ESP_LOGI(TAG, "Bar width: %d, position: %s", 
             this->bar_width_, this->bar_on_right_ ? "right" : "left");
    
    this->state_ = WIDGET_HIDDEN;
    this->slide_progress_ = 0.0f;
}

void PanelWidgets::loop() {
    // Process gesture detection
    this->detect_gestures_();
    
    // Handle animation
    if (this->state_ == WIDGET_SLIDING_IN || this->state_ == WIDGET_SLIDING_OUT) {
        this->animate_slide_();
    }
    
    // Render widget bar if visible
    if (this->slide_progress_ > 0.01f) {
        this->render_widget_bar_();
    }
}

void PanelWidgets::add_scene(const std::string &name, const std::string &icon,
                             const std::string &service, const std::string &entity) {
    SceneEntry entry;
    entry.name = name;
    entry.icon = icon;
    entry.service = service;
    entry.entity = entity;
    this->scenes_.push_back(entry);
}

void PanelWidgets::toggle_visibility() {
    if (this->state_ == WIDGET_HIDDEN || this->state_ == WIDGET_SLIDING_OUT) {
        this->show();
    } else {
        this->hide();
    }
}

void PanelWidgets::show() {
    if (this->state_ == WIDGET_VISIBLE) return;
    this->state_ = WIDGET_SLIDING_IN;
    this->animation_start_ = millis();
    ESP_LOGI(TAG, "Widget bar sliding in");
}

void PanelWidgets::hide() {
    if (this->state_ == WIDGET_HIDDEN) return;
    this->state_ = WIDGET_SLIDING_OUT;
    this->animation_start_ = millis();
    ESP_LOGI(TAG, "Widget bar sliding out");
}

void PanelWidgets::animate_slide_() {
    uint32_t elapsed = millis() - this->animation_start_;
    float progress = (float)elapsed / (float)ANIMATION_DURATION;
    
    if (progress >= 1.0f) {
        progress = 1.0f;
        if (this->state_ == WIDGET_SLIDING_IN) {
            this->state_ = WIDGET_VISIBLE;
            this->slide_progress_ = 1.0f;
        } else if (this->state_ == WIDGET_SLIDING_OUT) {
            this->state_ = WIDGET_HIDDEN;
            this->slide_progress_ = 0.0f;
        }
    } else {
        // Ease-out cubic: 1 - (1 - t)^3
        float eased = 1.0f - (1.0f - progress) * (1.0f - progress) * (1.0f - progress);
        
        if (this->state_ == WIDGET_SLIDING_IN) {
            this->slide_progress_ = eased;
        } else {
            this->slide_progress_ = 1.0f - eased;
        }
    }
}

void PanelWidgets::detect_gestures_() {
    // This would integrate with the touchscreen component
    // to detect swipe from edge, long press, etc.
    // 
    // Simplified gesture detection logic:
    // 1. Touch starts near left/right edge → potential swipe-to-reveal
    // 2. Touch moves >50px horizontally → swipe detected
    // 3. Touch held >600ms without moving → long press
    
    // In ESPHome, touch events come through the touchscreen component callbacks.
    // This is a framework for the gesture engine.
    
    this->last_gesture_ = GESTURE_NONE;
    
    // Check if touch is on the widget bar (for scene button taps)
    if (this->state_ == WIDGET_VISIBLE) {
        // Check tap within bar area to handle scene activation
        // (In a full implementation, this reads from touchscreen component)
    }
}

void PanelWidgets::render_widget_bar_() {
    if (this->display_ == nullptr) return;
    
    // Calculate bar position based on slide progress
    int bar_x;
    int visible_width = (int)(this->bar_width_ * this->slide_progress_);
    
    if (this->bar_on_right_) {
        bar_x = 480 - visible_width;
    } else {
        bar_x = visible_width - this->bar_width_;
    }
    
    // Draw semi-transparent background panel
    // (In ESPHome display component, direct pixel manipulation)
    this->display_->filled_rectangle(
        bar_x, 0, this->bar_width_, 480,
        Color(0x1A, 0x1A, 0x2E)  // Dark blue-gray
    );
    
    // Draw separator line
    int sep_x = this->bar_on_right_ ? bar_x : bar_x + this->bar_width_;
    this->display_->line(sep_x, 0, sep_x, 480, Color(0x7C, 0x4D, 0xFF));
    
    // Render sub-elements
    this->render_clock_widget_();
    this->render_scene_buttons_();
    this->render_status_bar_();
}

void PanelWidgets::render_clock_widget_() {
    // Time display at top of widget bar
    // (In production, would use LVGL labels or display->printf)
}

void PanelWidgets::render_scene_buttons_() {
    // Draw scene buttons vertically in the bar
    int y_offset = 60;
    int button_height = 60;
    int button_margin = 8;
    
    for (size_t i = 0; i < this->scenes_.size() && i < 6; i++) {
        int btn_y = y_offset + i * (button_height + button_margin);
        int btn_x = this->bar_on_right_ ? (480 - this->bar_width_ + 5) : 5;
        
        // Button background (rounded rectangle)
        this->display_->filled_rectangle(
            btn_x, btn_y, this->bar_width_ - 10, button_height,
            Color(0x21, 0x21, 0x3E)
        );
        
        // Scene name (centered)
        this->display_->printf(
            btn_x + (this->bar_width_ - 10) / 2,
            btn_y + button_height / 2,
            nullptr,  // Would use configured font
            Color(0xE0, 0xE0, 0xE0),
            display::TextAlign::CENTER,
            "%s", this->scenes_[i].name.c_str()
        );
    }
}

void PanelWidgets::render_status_bar_() {
    // Status info at bottom: Control A battery, WiFi, etc.
    int status_y = 420;
    
    this->display_->printf(
        this->bar_on_right_ ? 420 : 10,
        status_y,
        nullptr,
        Color(0x9E, 0x9E, 0x9E),
        display::TextAlign::TOP_LEFT,
        "Mando: --%% "
    );
}

void PanelWidgets::handle_scene_tap_(int index) {
    if (index < 0 || index >= (int)this->scenes_.size()) return;
    
    const SceneEntry &scene = this->scenes_[index];
    ESP_LOGI(TAG, "Scene activated: %s → %s(%s)", 
             scene.name.c_str(), scene.service.c_str(), scene.entity.c_str());
    
    // Call HA service via ESPHome API
    // In production: api::global_api_server->call_homeassistant_service(...)
}

}  // namespace panel_widgets
}  // namespace esphome
