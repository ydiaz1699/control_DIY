#include "sync_protocol.h"
#include "mqtt_client.h"
#include "../config.h"

SyncProtocol& SyncProtocol::instance() {
    static SyncProtocol inst;
    return inst;
}

void SyncProtocol::init() {
    if (!Features::SYNC_ENABLED) return;

    deviceRole_ = "mando";
    subscribeSyncTopics();

    // Send initial heartbeat
    sendHeartbeat();
}

void SyncProtocol::update() {
    if (!Features::SYNC_ENABLED) return;

    unsigned long now = millis();

    // Send heartbeat every 30 seconds
    if (now - lastHeartbeatMs_ > 30000) {
        sendHeartbeat();
        lastHeartbeatMs_ = now;
    }

    // Check peer timeout (60 seconds)
    if (peerOnline_ && (now - peerLastSeenMs_ > 60000)) {
        peerOnline_ = false;
    }
}

void SyncProtocol::broadcastState(const String& key, const String& value) {
    if (!Features::SYNC_ENABLED) return;

    JsonDocument doc;
    doc["type"] = (int)SyncMsgType::STATE_UPDATE;
    doc["from"] = deviceRole_;
    doc["key"] = key;
    doc["value"] = value;
    doc["ts"] = millis();

    String payload;
    serializeJson(doc, payload);
    MQTTClient::instance().publish(String(SyncConfig::SYNC_TOPIC_STATE), payload);
}

void SyncProtocol::broadcastSceneChange(const String& sceneName) {
    if (!Features::SYNC_ENABLED) return;

    JsonDocument doc;
    doc["type"] = (int)SyncMsgType::SCENE_CHANGE;
    doc["from"] = deviceRole_;
    doc["scene"] = sceneName;
    doc["ts"] = millis();

    String payload;
    serializeJson(doc, payload);
    MQTTClient::instance().publish(String(SyncConfig::SYNC_TOPIC_SCENE), payload);
}

void SyncProtocol::relayCommand(const String& command, const String& cmdPayload) {
    if (!Features::SYNC_ENABLED) return;

    JsonDocument doc;
    doc["type"] = (int)SyncMsgType::COMMAND_RELAY;
    doc["from"] = deviceRole_;
    doc["command"] = command;
    doc["payload"] = cmdPayload;

    String payload;
    serializeJson(doc, payload);
    MQTTClient::instance().publish(String(SyncConfig::SYNC_TOPIC_CMD), payload);
}

void SyncProtocol::sendNotification(const String& title, const String& message) {
    if (!Features::SYNC_ENABLED) return;

    JsonDocument doc;
    doc["type"] = (int)SyncMsgType::NOTIFICATION;
    doc["from"] = deviceRole_;
    doc["title"] = title;
    doc["message"] = message;

    String topic = String("controldiy/sync/notification");
    String payload;
    serializeJson(doc, payload);
    MQTTClient::instance().publish(topic, payload);
}

void SyncProtocol::sendHeartbeat() {
    JsonDocument doc;
    doc["type"] = (int)SyncMsgType::HEARTBEAT;
    doc["from"] = deviceRole_;
    doc["ts"] = millis();

    String topic = String("controldiy/sync/heartbeat");
    String payload;
    serializeJson(doc, payload);
    MQTTClient::instance().publish(topic, payload);
}

void SyncProtocol::handleSyncMessage(const String& topic, const String& payload) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) return;

    // Ignore own messages
    String from = doc["from"].as<String>();
    if (from == deviceRole_) return;

    // Update peer status
    peerOnline_ = true;
    peerLastSeenMs_ = millis();
    peerName_ = from;

    SyncMsgType type = (SyncMsgType)doc["type"].as<int>();

    switch (type) {
        case SyncMsgType::STATE_UPDATE:
            if (stateCb_) {
                stateCb_(doc["key"].as<String>(), doc["value"].as<String>());
            }
            break;

        case SyncMsgType::SCENE_CHANGE:
            if (sceneCb_) {
                sceneCb_(doc["scene"].as<String>());
            }
            break;

        case SyncMsgType::COMMAND_RELAY:
            if (commandCb_) {
                commandCb_(doc["command"].as<String>(), doc["payload"].as<String>());
            }
            break;

        case SyncMsgType::NOTIFICATION:
            if (notifCb_) {
                notifCb_(doc["title"].as<String>(), doc["message"].as<String>());
            }
            break;

        case SyncMsgType::HEARTBEAT:
            // Just update peer status (already done above)
            break;
    }
}

void SyncProtocol::subscribeSyncTopics() {
    auto& mqtt = MQTTClient::instance();

    // Subscribe to all sync topics
    mqtt.onMessage("controldiy/sync/#", [this](const String& topic, const String& payload) {
        handleSyncMessage(topic, payload);
    });
}
