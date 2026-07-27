#include "command_router.h"
#include "config_store.h"
#include "../config.h"

// Forward declarations for hardware drivers
extern void ir_send(uint16_t code, uint8_t protocol);
extern void ble_send_key(uint8_t key);
extern void mqtt_publish(const char* topic, const char* payload);
extern void haptic_pulse(int durationMs);

// ============================================================================
// CommandRouter Implementation
// ============================================================================

CommandRouter& CommandRouter::instance() {
    static CommandRouter inst;
    return inst;
}

void CommandRouter::init() {
    // 1. Intentar cargar configuración guardada en NVS
    bool loadedFromNVS = ConfigStore::instance().loadDevices();

    // 2. Si no había nada (primer boot), cargar defaults de fábrica
    if (loadedFromNVS) {
        Serial.println("[CommandRouter] Config cargada desde NVS");
        return;  // NVS ya pobló devices_ vía addDeviceProfile()
    }

    Serial.println("[CommandRouter] Primer boot — cargando defaults de fábrica");

    // Samsung TV
    DeviceProfile samsungTV;
    samsungTV.name = "Samsung TV";
    samsungTV.icon = LV_SYMBOL_IMAGE;  // TV icon
    samsungTV.commands["power"]    = {CommandTarget::IR, "power", "", 0xE0E040BF, 1, false};
    samsungTV.commands["vol_up"]   = {CommandTarget::IR, "vol_up", "", 0xE0E0E01F, 1, true};
    samsungTV.commands["vol_down"] = {CommandTarget::IR, "vol_down", "", 0xE0E0D02F, 1, true};
    samsungTV.commands["ch_up"]    = {CommandTarget::IR, "ch_up", "", 0xE0E048B7, 1, true};
    samsungTV.commands["ch_down"]  = {CommandTarget::IR, "ch_down", "", 0xE0E008F7, 1, true};
    samsungTV.commands["mute"]     = {CommandTarget::IR, "mute", "", 0xE0E0F00F, 1, false};
    samsungTV.commands["source"]   = {CommandTarget::IR, "source", "", 0xE0E0807F, 1, false};
    addDeviceProfile(samsungTV);

    // Home Assistant Media Player (via MQTT)
    DeviceProfile haMedia;
    haMedia.name = "HA Media";
    haMedia.icon = LV_SYMBOL_AUDIO;
    haMedia.commands["play_pause"] = {CommandTarget::HA_SERVICE, "media_player", "media_play_pause", 0, 0, false};
    haMedia.commands["next"]       = {CommandTarget::HA_SERVICE, "media_player", "media_next_track", 0, 0, false};
    haMedia.commands["prev"]       = {CommandTarget::HA_SERVICE, "media_player", "media_previous_track", 0, 0, false};
    haMedia.commands["vol_up"]     = {CommandTarget::HA_SERVICE, "media_player", "volume_up", 0, 0, true};
    haMedia.commands["vol_down"]   = {CommandTarget::HA_SERVICE, "media_player", "volume_down", 0, 0, true};
    addDeviceProfile(haMedia);

    // Apple TV (via BLE)
    DeviceProfile appleTV;
    appleTV.name = "Apple TV";
    appleTV.icon = LV_SYMBOL_VIDEO;
    appleTV.commands["play_pause"] = {CommandTarget::BLE, "play_pause", "", 0xCD, 0, false};
    appleTV.commands["up"]         = {CommandTarget::BLE, "up", "", 0x52, 0, true};
    appleTV.commands["down"]       = {CommandTarget::BLE, "down", "", 0x51, 0, true};
    appleTV.commands["left"]       = {CommandTarget::BLE, "left", "", 0x50, 0, true};
    appleTV.commands["right"]      = {CommandTarget::BLE, "right", "", 0x4F, 0, true};
    appleTV.commands["select"]     = {CommandTarget::BLE, "select", "", 0x28, 0, false};
    appleTV.commands["back"]       = {CommandTarget::BLE, "back", "", 0x29, 0, false};
    addDeviceProfile(appleTV);

    // Set default active
    if (!devices_.empty()) {
        activeDeviceName_ = devices_.begin()->first;
    }

    // Persistir defaults para que la próxima vez ya estén en NVS
    ConfigStore::instance().saveDevices(devices_);
}

void CommandRouter::execute(const String& commandName) {
    DeviceProfile* dev = getActiveDevice();
    if (!dev) return;

    auto it = dev->commands.find(commandName);
    if (it != dev->commands.end()) {
        execute(it->second);
    }
}

void CommandRouter::execute(const Command& cmd) {
    bool success = true;
    String msg = "OK";

    switch (cmd.target) {
        case CommandTarget::IR:
            ir_send(cmd.irCode, cmd.irProtocol);
            break;

        case CommandTarget::BLE:
            ble_send_key(cmd.irCode);  // reusing irCode field for keycode
            break;

        case CommandTarget::MQTT: {
            String topic = String(MQTTConfig::TOPIC_PREFIX) + "/" + cmd.action;
            mqtt_publish(topic.c_str(), cmd.payload.c_str());
            break;
        }

        case CommandTarget::HA_SERVICE:
            callHAService(cmd.action, cmd.payload, "");
            break;

        case CommandTarget::LOCAL:
            // Handle locally (GUI actions)
            break;

        case CommandTarget::SYNC: {
            String topic = SyncConfig::SYNC_TOPIC_CMD;
            mqtt_publish(topic.c_str(), cmd.payload.c_str());
            break;
        }

        default:
            success = false;
            msg = "Unknown target";
            break;
    }

    // Haptic feedback on execution
    if (Features::HAPTIC_ENABLED && success) {
        haptic_pulse(15);
    }

    if (feedbackCb_) {
        feedbackCb_(success, msg);
    }
}

void CommandRouter::addDeviceProfile(const DeviceProfile& profile) {
    devices_[profile.name] = profile;
}

void CommandRouter::setActiveDevice(const String& deviceName) {
    if (devices_.find(deviceName) != devices_.end()) {
        activeDeviceName_ = deviceName;
    }
}

DeviceProfile* CommandRouter::getActiveDevice() {
    auto it = devices_.find(activeDeviceName_);
    if (it != devices_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<String> CommandRouter::getDeviceNames() const {
    std::vector<String> names;
    for (auto& pair : devices_) {
        names.push_back(pair.first);
    }
    return names;
}

void CommandRouter::setQuickAction(int slot, const Command& cmd) {
    if (slot >= 0 && slot < 4) {
        quickActions_[slot] = cmd;
    }
}

void CommandRouter::executeQuickAction(int slot) {
    if (slot >= 0 && slot < 4) {
        execute(quickActions_[slot]);
    }
}

void CommandRouter::startIRLearn(IRLearnCallback callback) {
    learning_ = true;
    learnCb_ = callback;
    // Enable IR receiver in learning mode
}

void CommandRouter::stopIRLearn() {
    learning_ = false;
    learnCb_ = nullptr;
}

void CommandRouter::callHAService(const String& domain, const String& service,
                                   const String& entityId, const String& extraData) {
    // Publish MQTT message for HA service call
    // Format: homeassistant/service/{domain}/{service}
    String topic = String(MQTTConfig::DISCOVERY_PREFIX) + "/service/" + domain + "/" + service;
    String payload = "{";
    if (entityId.length() > 0) {
        payload += "\"entity_id\":\"" + entityId + "\"";
    }
    if (extraData.length() > 0) {
        if (entityId.length() > 0) payload += ",";
        payload += extraData;
    }
    payload += "}";
    mqtt_publish(topic.c_str(), payload.c_str());
}
