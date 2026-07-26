#pragma once

#include <Arduino.h>
#include <functional>
#include <ArduinoJson.h>

// ============================================================================
// Sync Protocol - Communication between Control A (Mando) and Control B (Panel)
// Novel feature: both controls stay in sync via MQTT
// ============================================================================

/// Sync message types
enum class SyncMsgType : uint8_t {
    STATE_UPDATE,    // General state broadcast
    SCENE_CHANGE,    // Scene was activated
    COMMAND_RELAY,   // Relay a command to the other device
    HEARTBEAT,       // Presence detection
    NOTIFICATION     // Show notification on the other device
};

/// Sync Protocol Manager
class SyncProtocol {
public:
    static SyncProtocol& instance();

    void init();
    void update();

    // Send sync messages
    void broadcastState(const String& key, const String& value);
    void broadcastSceneChange(const String& sceneName);
    void relayCommand(const String& command, const String& payload);
    void sendNotification(const String& title, const String& message);
    void sendHeartbeat();

    // Peer status
    bool isPeerOnline() const { return peerOnline_; }
    unsigned long getPeerLastSeen() const { return peerLastSeenMs_; }
    String getPeerName() const { return peerName_; }

    // Callbacks for received sync messages
    using StateCallback = std::function<void(const String& key, const String& value)>;
    using SceneCallback = std::function<void(const String& sceneName)>;
    using CommandCallback = std::function<void(const String& command, const String& payload)>;
    using NotificationCallback = std::function<void(const String& title, const String& message)>;

    void onStateReceived(StateCallback cb) { stateCb_ = cb; }
    void onSceneReceived(SceneCallback cb) { sceneCb_ = cb; }
    void onCommandReceived(CommandCallback cb) { commandCb_ = cb; }
    void onNotificationReceived(NotificationCallback cb) { notifCb_ = cb; }

private:
    SyncProtocol() = default;

    bool peerOnline_ = false;
    unsigned long peerLastSeenMs_ = 0;
    unsigned long lastHeartbeatMs_ = 0;
    String peerName_ = "Panel";
    String deviceRole_ = "mando";  // "mando" or "panel"

    StateCallback stateCb_ = nullptr;
    SceneCallback sceneCb_ = nullptr;
    CommandCallback commandCb_ = nullptr;
    NotificationCallback notifCb_ = nullptr;

    void handleSyncMessage(const String& topic, const String& payload);
    void subscribeSyncTopics();
};
