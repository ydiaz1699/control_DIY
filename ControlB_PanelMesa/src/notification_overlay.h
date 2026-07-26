#pragma once

// ============================================================================
// Notification Overlay - Pop-up notifications from Home Assistant
// Novel feature: toast notifications on the panel display
// ============================================================================

#include <cstdint>
#include <cstring>

enum class NotificationPriority : uint8_t {
    LOW,        // Subtle, auto-dismiss in 3s
    NORMAL,     // Standard toast, 5s
    HIGH,       // Prominent, requires dismiss
    CRITICAL    // Full screen alert
};

enum class NotificationIcon : uint8_t {
    INFO,
    WARNING,
    ERROR,
    DOOR,
    MOTION,
    TEMPERATURE,
    SECURITY,
    PACKAGE,
    CALL,
    MESSAGE
};

struct Notification {
    char title[64];
    char message[128];
    NotificationPriority priority;
    NotificationIcon icon;
    uint32_t timestamp;
    uint32_t color;         // Accent color
    bool dismissed;
    int displayDurationMs;
};

class NotificationOverlay {
public:
    static constexpr int MAX_QUEUE = 5;
    static constexpr int TOAST_WIDTH = 400;
    static constexpr int TOAST_HEIGHT = 80;
    static constexpr int TOAST_MARGIN = 10;
    static constexpr int TOAST_Y_START = 10;

    NotificationOverlay() {
        for (int i = 0; i < MAX_QUEUE; i++) {
            queue_[i].dismissed = true;
        }
    }

    /// Push a new notification
    void push(const char* title, const char* message,
              NotificationPriority priority = NotificationPriority::NORMAL,
              NotificationIcon icon = NotificationIcon::INFO,
              uint32_t color = 0x2196F3) {
        // Find empty slot or oldest
        int slot = findSlot();
        Notification& n = queue_[slot];

        strncpy(n.title, title, sizeof(n.title) - 1);
        strncpy(n.message, message, sizeof(n.message) - 1);
        n.priority = priority;
        n.icon = icon;
        n.timestamp = millis_();
        n.color = color;
        n.dismissed = false;

        switch (priority) {
            case NotificationPriority::LOW:      n.displayDurationMs = 3000; break;
            case NotificationPriority::NORMAL:   n.displayDurationMs = 5000; break;
            case NotificationPriority::HIGH:     n.displayDurationMs = 0; break; // Manual dismiss
            case NotificationPriority::CRITICAL: n.displayDurationMs = 0; break;
        }

        activeCount_++;
    }

    /// Dismiss a notification by index
    void dismiss(int index) {
        if (index >= 0 && index < MAX_QUEUE && !queue_[index].dismissed) {
            queue_[index].dismissed = true;
            if (activeCount_ > 0) activeCount_--;
        }
    }

    /// Dismiss all
    void dismissAll() {
        for (int i = 0; i < MAX_QUEUE; i++) {
            queue_[i].dismissed = true;
        }
        activeCount_ = 0;
    }

    /// Update (auto-dismiss expired notifications)
    void update() {
        uint32_t now = millis_();
        for (int i = 0; i < MAX_QUEUE; i++) {
            if (!queue_[i].dismissed && queue_[i].displayDurationMs > 0) {
                if (now - queue_[i].timestamp >= (uint32_t)queue_[i].displayDurationMs) {
                    dismiss(i);
                }
            }
        }
    }

    /// Check if any notifications are active
    bool hasActive() const { return activeCount_ > 0; }
    int getActiveCount() const { return activeCount_; }

    /// Get notification at index (for rendering)
    const Notification* get(int index) const {
        if (index >= 0 && index < MAX_QUEUE && !queue_[index].dismissed) {
            return &queue_[index];
        }
        return nullptr;
    }

    /// Get the icon string for a notification icon type
    static const char* getIconString(NotificationIcon icon) {
        switch (icon) {
            case NotificationIcon::INFO:        return "\ue88e";  // info
            case NotificationIcon::WARNING:     return "\ue002";  // warning
            case NotificationIcon::ERROR:       return "\ue000";  // error
            case NotificationIcon::DOOR:        return "\ueffc";  // door_front
            case NotificationIcon::MOTION:      return "\ue566";  // directions_walk
            case NotificationIcon::TEMPERATURE: return "\ue1ff";  // thermostat
            case NotificationIcon::SECURITY:    return "\ue32a";  // security
            case NotificationIcon::PACKAGE:     return "\ue54f";  // local_shipping
            case NotificationIcon::CALL:        return "\ue0b0";  // call
            case NotificationIcon::MESSAGE:     return "\ue0c9";  // message
        }
        return "\ue88e";
    }

private:
    Notification queue_[MAX_QUEUE];
    int activeCount_ = 0;

    int findSlot() {
        // Find first dismissed slot
        for (int i = 0; i < MAX_QUEUE; i++) {
            if (queue_[i].dismissed) return i;
        }
        // All full, overwrite oldest
        uint32_t oldest = 0xFFFFFFFF;
        int oldestIdx = 0;
        for (int i = 0; i < MAX_QUEUE; i++) {
            if (queue_[i].timestamp < oldest) {
                oldest = queue_[i].timestamp;
                oldestIdx = i;
            }
        }
        return oldestIdx;
    }

    static uint32_t millis_() {
        extern unsigned long millis();
        return ::millis();
    }
};
