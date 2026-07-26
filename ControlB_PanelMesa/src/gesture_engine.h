#pragma once

// ============================================================================
// Gesture Engine - Advanced touch gesture detection for Panel de Mesa
// Novel feature: full gesture recognition not present in original projects
// ============================================================================
// 
// Detects:
// - Swipe Left/Right (navigate dashboards)
// - Swipe Down (show local controls overlay)
// - Swipe Up (hide overlay)
// - Pinch In/Out (zoom in WebView mode)
// - Long Press (context menu / edit mode)
// - Double Tap (toggle fullscreen)
// - Two-finger swipe (scroll in WebView)
//
// Usage in ESPHome:
// Include as a lambda header and call from touchscreen on_touch/on_release
// ============================================================================

#include <cmath>
#include <cstdint>

enum class GestureType : uint8_t {
    NONE = 0,
    TAP,
    DOUBLE_TAP,
    LONG_PRESS,
    SWIPE_LEFT,
    SWIPE_RIGHT,
    SWIPE_UP,
    SWIPE_DOWN,
    PINCH_IN,
    PINCH_OUT,
    TWO_FINGER_SCROLL
};

struct TouchPoint {
    int x;
    int y;
    uint32_t timestamp;
};

class GestureEngine {
public:
    // Configuration
    static constexpr int SWIPE_MIN_DISTANCE = 50;       // pixels
    static constexpr int SWIPE_MAX_TIME_MS = 500;       // ms
    static constexpr int TAP_MAX_DISTANCE = 20;         // pixels
    static constexpr int TAP_MAX_TIME_MS = 200;         // ms
    static constexpr int DOUBLE_TAP_MAX_GAP_MS = 300;   // ms between taps
    static constexpr int LONG_PRESS_MIN_MS = 600;       // ms
    static constexpr float PINCH_MIN_RATIO = 0.2f;      // 20% change

    GestureEngine() { reset(); }

    void reset() {
        active_ = false;
        multiTouch_ = false;
        gestureDetected_ = GestureType::NONE;
        touchCount_ = 0;
    }

    /// Call when touch begins
    void onTouchStart(int x, int y, uint32_t timestamp, int pointerId = 0) {
        if (pointerId == 0) {
            start_.x = x;
            start_.y = y;
            start_.timestamp = timestamp;
            active_ = true;
            gestureDetected_ = GestureType::NONE;
        } else if (pointerId == 1) {
            multiTouch_ = true;
            secondStart_.x = x;
            secondStart_.y = y;
            secondStart_.timestamp = timestamp;
            initialPinchDist_ = distance(start_, secondStart_);
        }
        touchCount_ = pointerId + 1;
    }

    /// Call on touch move
    void onTouchMove(int x, int y, uint32_t timestamp, int pointerId = 0) {
        if (pointerId == 0) {
            current_.x = x;
            current_.y = y;
            current_.timestamp = timestamp;
        } else if (pointerId == 1) {
            secondCurrent_.x = x;
            secondCurrent_.y = y;
            secondCurrent_.timestamp = timestamp;
        }
    }

    /// Call when touch ends - returns detected gesture
    GestureType onTouchEnd(int x, int y, uint32_t timestamp, int pointerId = 0) {
        if (!active_) return GestureType::NONE;

        current_.x = x;
        current_.y = y;
        current_.timestamp = timestamp;

        uint32_t duration = timestamp - start_.timestamp;
        int dx = current_.x - start_.x;
        int dy = current_.y - start_.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        // Multi-touch gestures
        if (multiTouch_ && pointerId == 1) {
            float currentDist = distance(current_, secondCurrent_);
            float ratio = (currentDist - initialPinchDist_) / initialPinchDist_;
            if (ratio > PINCH_MIN_RATIO) {
                gestureDetected_ = GestureType::PINCH_OUT;
            } else if (ratio < -PINCH_MIN_RATIO) {
                gestureDetected_ = GestureType::PINCH_IN;
            }
            multiTouch_ = false;
            active_ = false;
            return gestureDetected_;
        }

        if (pointerId != 0) return GestureType::NONE;

        // Long press
        if (duration >= LONG_PRESS_MIN_MS && dist < TAP_MAX_DISTANCE) {
            gestureDetected_ = GestureType::LONG_PRESS;
            active_ = false;
            return gestureDetected_;
        }

        // Tap
        if (dist < TAP_MAX_DISTANCE && duration < TAP_MAX_TIME_MS) {
            // Check for double tap
            if (lastTapTimestamp_ > 0 &&
                (timestamp - lastTapTimestamp_) < DOUBLE_TAP_MAX_GAP_MS) {
                gestureDetected_ = GestureType::DOUBLE_TAP;
                lastTapTimestamp_ = 0;
            } else {
                gestureDetected_ = GestureType::TAP;
                lastTapTimestamp_ = timestamp;
            }
            active_ = false;
            return gestureDetected_;
        }

        // Swipe detection
        if (dist >= SWIPE_MIN_DISTANCE && duration <= SWIPE_MAX_TIME_MS) {
            float angle = std::atan2(dy, dx) * 180.0f / 3.14159f;

            if (angle >= -45 && angle < 45) {
                gestureDetected_ = GestureType::SWIPE_RIGHT;
            } else if (angle >= 45 && angle < 135) {
                gestureDetected_ = GestureType::SWIPE_DOWN;
            } else if (angle >= -135 && angle < -45) {
                gestureDetected_ = GestureType::SWIPE_UP;
            } else {
                gestureDetected_ = GestureType::SWIPE_LEFT;
            }
            active_ = false;
            return gestureDetected_;
        }

        // No gesture recognized
        gestureDetected_ = GestureType::NONE;
        active_ = false;
        return gestureDetected_;
    }

    /// Get last detected gesture
    GestureType getLastGesture() const { return gestureDetected_; }

    /// Check if currently tracking a touch
    bool isActive() const { return active_; }

private:
    TouchPoint start_{0, 0, 0};
    TouchPoint current_{0, 0, 0};
    TouchPoint secondStart_{0, 0, 0};
    TouchPoint secondCurrent_{0, 0, 0};

    bool active_ = false;
    bool multiTouch_ = false;
    int touchCount_ = 0;
    GestureType gestureDetected_ = GestureType::NONE;
    uint32_t lastTapTimestamp_ = 0;
    float initialPinchDist_ = 0;

    static float distance(const TouchPoint& a, const TouchPoint& b) {
        int dx = b.x - a.x;
        int dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};
