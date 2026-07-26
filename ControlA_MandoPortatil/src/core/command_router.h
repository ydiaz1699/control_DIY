#pragma once

#include <Arduino.h>
#include <functional>
#include <map>
#include <string>
#include <vector>

// ============================================================================
// Command Router - Routes abstract commands to hardware actions
// Inspired by OMOTE's commandHandler and OLED Remote's button routing
// ============================================================================

/// Command targets
enum class CommandTarget : uint8_t {
    IR,         // Infrared transmit
    BLE,        // Bluetooth keyboard/media
    MQTT,       // MQTT publish
    HA_SERVICE, // Home Assistant service call via MQTT
    LOCAL,      // Local action (GUI, settings)
    SYNC        // Sync to other controls
};

/// A single command definition
struct Command {
    CommandTarget target;
    String action;      // e.g., "samsung_tv/power", "media_player/toggle"
    String payload;     // JSON or raw data
    uint16_t irCode;    // For IR commands
    uint8_t irProtocol; // IR protocol ID
    bool repeat;        // Allow key repeat
};

/// Device profile - groups commands for a specific device
struct DeviceProfile {
    String name;        // e.g., "Samsung TV", "Yamaha AVR"
    String icon;        // LVGL symbol
    std::map<String, Command> commands;  // action_name -> command
};

/// Command Router
class CommandRouter {
public:
    static CommandRouter& instance();

    void init();

    // Execute a command by name for the active device
    void execute(const String& commandName);
    void execute(const Command& cmd);

    // Device profile management
    void addDeviceProfile(const DeviceProfile& profile);
    void setActiveDevice(const String& deviceName);
    DeviceProfile* getActiveDevice();
    std::vector<String> getDeviceNames() const;

    // Quick actions (shortcuts mapped to buttons)
    void setQuickAction(int slot, const Command& cmd);
    void executeQuickAction(int slot);

    // IR learning
    using IRLearnCallback = std::function<void(uint16_t code, uint8_t protocol)>;
    void startIRLearn(IRLearnCallback callback);
    void stopIRLearn();
    bool isLearning() const { return learning_; }

    // HA service call helper
    void callHAService(const String& domain, const String& service,
                       const String& entityId, const String& extraData = "");

    // Callback for command execution feedback
    using CommandFeedback = std::function<void(bool success, const String& msg)>;
    void onFeedback(CommandFeedback cb) { feedbackCb_ = cb; }

private:
    CommandRouter() = default;

    std::map<String, DeviceProfile> devices_;
    String activeDeviceName_;
    Command quickActions_[4];  // 4 shortcut slots
    bool learning_ = false;
    IRLearnCallback learnCb_ = nullptr;
    CommandFeedback feedbackCb_ = nullptr;
};
