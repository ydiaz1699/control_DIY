#include "ha_bridge.h"
#include "mqtt_client.h"
#include "../config.h"
#include <ArduinoJson.h>

HABridge& HABridge::instance() {
    static HABridge inst;
    return inst;
}

void HABridge::init() {
    subscribeToEntityStates();
}

void HABridge::update() {
    // State updates come via MQTT callback, nothing to poll
}

void HABridge::trackEntity(const String& entityId, HAEntityType type) {
    HAEntityState state;
    state.entityId = entityId;
    state.type = type;
    state.state = "unknown";
    state.brightness = 0;
    state.temperature = 0;
    state.position = 0;
    entities_[entityId] = state;

    // Subscribe to state updates for this entity
    String topic = "homeassistant/state/" + entityId;
    MQTTClient::instance().onMessage(topic, [this, entityId](const String& t, const String& payload) {
        handleStateUpdate(entityId, payload);
    });
}

void HABridge::untrackEntity(const String& entityId) {
    String topic = "homeassistant/state/" + entityId;
    MQTTClient::instance().unsubscribe(topic);
    entities_.erase(entityId);
}

HAEntityState* HABridge::getEntityState(const String& entityId) {
    auto it = entities_.find(entityId);
    if (it != entities_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<HAEntityState*> HABridge::getEntitiesByType(HAEntityType type) {
    std::vector<HAEntityState*> result;
    for (auto& pair : entities_) {
        if (pair.second.type == type) {
            result.push_back(&pair.second);
        }
    }
    return result;
}

void HABridge::toggleEntity(const String& entityId) {
    HAEntityState* state = getEntityState(entityId);
    if (!state) return;

    String domain;
    switch (state->type) {
        case HAEntityType::LIGHT: domain = "light"; break;
        case HAEntityType::SWITCH: domain = "switch"; break;
        case HAEntityType::FAN: domain = "fan"; break;
        default: domain = "homeassistant"; break;
    }
    MQTTClient::instance().callService(domain, "toggle", entityId);
}

void HABridge::turnOn(const String& entityId, const String& extraData) {
    HAEntityState* state = getEntityState(entityId);
    if (!state) return;

    String domain;
    switch (state->type) {
        case HAEntityType::LIGHT: domain = "light"; break;
        case HAEntityType::SWITCH: domain = "switch"; break;
        case HAEntityType::FAN: domain = "fan"; break;
        case HAEntityType::CLIMATE: domain = "climate"; break;
        default: domain = "homeassistant"; break;
    }
    MQTTClient::instance().callService(domain, "turn_on", entityId, extraData);
}

void HABridge::turnOff(const String& entityId) {
    HAEntityState* state = getEntityState(entityId);
    if (!state) return;

    String domain;
    switch (state->type) {
        case HAEntityType::LIGHT: domain = "light"; break;
        case HAEntityType::SWITCH: domain = "switch"; break;
        case HAEntityType::FAN: domain = "fan"; break;
        case HAEntityType::CLIMATE: domain = "climate"; break;
        default: domain = "homeassistant"; break;
    }
    MQTTClient::instance().callService(domain, "turn_off", entityId);
}

void HABridge::setLightBrightness(const String& entityId, int brightness) {
    String data = "\"brightness\":" + String(brightness);
    MQTTClient::instance().callService("light", "turn_on", entityId, data);
}

void HABridge::setClimateTemp(const String& entityId, float temp) {
    String data = "\"temperature\":" + String(temp, 1);
    MQTTClient::instance().callService("climate", "set_temperature", entityId, data);
}

void HABridge::setCoverPosition(const String& entityId, float position) {
    String data = "\"position\":" + String((int)position);
    MQTTClient::instance().callService("cover", "set_cover_position", entityId, data);
}

void HABridge::mediaPlayPause(const String& entityId) {
    MQTTClient::instance().callService("media_player", "media_play_pause", entityId);
}

void HABridge::mediaNext(const String& entityId) {
    MQTTClient::instance().callService("media_player", "media_next_track", entityId);
}

void HABridge::mediaPrev(const String& entityId) {
    MQTTClient::instance().callService("media_player", "media_previous_track", entityId);
}

void HABridge::mediaSetVolume(const String& entityId, float volume) {
    String data = "\"volume_level\":" + String(volume, 2);
    MQTTClient::instance().callService("media_player", "volume_set", entityId, data);
}

void HABridge::activateScene(const String& entityId) {
    MQTTClient::instance().callService("scene", "turn_on", entityId);
}

void HABridge::triggerAutomation(const String& entityId) {
    MQTTClient::instance().callService("automation", "trigger", entityId);
}

void HABridge::emitEvent(const String& eventType, const String& data) {
    // Emit ESPHome-style event via MQTT for HA automations
    String topic = String(MQTTConfig::TOPIC_PREFIX) + "/event/" + eventType;
    MQTTClient::instance().publish(topic, data);
}

void HABridge::handleStateUpdate(const String& entityId, const String& stateJson) {
    auto it = entities_.find(entityId);
    if (it == entities_.end()) return;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, stateJson);
    if (err) return;

    HAEntityState& entity = it->second;
    entity.state = doc["state"].as<String>();

    // Parse type-specific attributes
    if (doc.containsKey("attributes")) {
        JsonObject attrs = doc["attributes"];
        switch (entity.type) {
            case HAEntityType::LIGHT:
                if (attrs.containsKey("brightness")) {
                    entity.brightness = attrs["brightness"];
                }
                break;
            case HAEntityType::CLIMATE:
                if (attrs.containsKey("current_temperature")) {
                    entity.temperature = attrs["current_temperature"];
                }
                break;
            case HAEntityType::MEDIA_PLAYER:
                if (attrs.containsKey("media_title")) {
                    entity.mediaTitle = attrs["media_title"].as<String>();
                }
                if (attrs.containsKey("media_artist")) {
                    entity.mediaArtist = attrs["media_artist"].as<String>();
                }
                break;
            case HAEntityType::COVER:
                if (attrs.containsKey("current_position")) {
                    entity.position = attrs["current_position"];
                }
                break;
            default:
                break;
        }
    }

    if (stateChangeCb_) {
        stateChangeCb_(entity);
    }
}

void HABridge::subscribeToEntityStates() {
    // Subscribe to a wildcard for all tracked entities
    String topic = "homeassistant/state/#";
    MQTTClient::instance().subscribe(topic);
}
