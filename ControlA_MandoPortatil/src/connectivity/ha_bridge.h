#pragma once

#include <Arduino.h>
#include <functional>
#include <map>
#include <vector>

// ============================================================================
// Home Assistant Bridge - Manages entity states and service calls
// Combines OMOTE MQTT approach + Everything Remote event model + homeThing entity sync
// ============================================================================

/// HA Entity types
enum class HAEntityType : uint8_t {
    LIGHT,
    SWITCH,
    CLIMATE,
    COVER,
    MEDIA_PLAYER,
    SCENE,
    AUTOMATION,
    SENSOR,
    FAN
};

/// Simplified entity state
struct HAEntityState {
    String entityId;
    HAEntityType type;
    String state;           // "on", "off", "playing", etc.
    int brightness;         // For lights (0-255)
    float temperature;      // For climate
    String mediaTitle;      // For media players
    String mediaArtist;
    float position;         // For covers (0-100)
};

/// HA Bridge
class HABridge {
public:
    static HABridge& instance();

    void init();
    void update();

    // Entity state management
    void trackEntity(const String& entityId, HAEntityType type);
    void untrackEntity(const String& entityId);
    HAEntityState* getEntityState(const String& entityId);
    std::vector<HAEntityState*> getEntitiesByType(HAEntityType type);

    // Service calls
    void toggleEntity(const String& entityId);
    void turnOn(const String& entityId, const String& extraData = "");
    void turnOff(const String& entityId);
    void setLightBrightness(const String& entityId, int brightness);
    void setClimateTemp(const String& entityId, float temp);
    void setCoverPosition(const String& entityId, float position);
    void mediaPlayPause(const String& entityId);
    void mediaNext(const String& entityId);
    void mediaPrev(const String& entityId);
    void mediaSetVolume(const String& entityId, float volume);
    void activateScene(const String& entityId);
    void triggerAutomation(const String& entityId);

    // Event emission (ESPHome-style events for HA automations)
    void emitEvent(const String& eventType, const String& data);

    // Callbacks
    using StateChangeCallback = std::function<void(const HAEntityState&)>;
    void onStateChange(StateChangeCallback cb) { stateChangeCb_ = cb; }

private:
    HABridge() = default;

    std::map<String, HAEntityState> entities_;
    StateChangeCallback stateChangeCb_ = nullptr;

    void handleStateUpdate(const String& entityId, const String& stateJson);
    void subscribeToEntityStates();
};
