#include "mqtt_client.h"
#include "../config.h"
#include "wifi_manager.h"
#include <ArduinoJson.h>

MQTTClient& MQTTClient::instance() {
    static MQTTClient inst;
    return inst;
}

void MQTTClient::init() {
    mqttClient_.setServer(MQTTConfig::DEFAULT_SERVER, MQTTConfig::DEFAULT_PORT);
    mqttClient_.setCallback(staticCallback);
    mqttClient_.setBufferSize(1024);
}

void MQTTClient::update() {
    if (!WiFiManager::instance().isConnected()) return;

    if (!mqttClient_.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectMs_ > 5000) {
            lastReconnectMs_ = now;
            connect();
        }
    } else {
        mqttClient_.loop();
    }

    // Track connection state changes
    bool connected = isConnected();
    if (connected != wasConnected_) {
        wasConnected_ = connected;
        if (connected) {
            // Re-subscribe to all topics
            for (auto& sub : subscriptions_) {
                mqttClient_.subscribe(sub.topicFilter.c_str());
            }
            // Publish discovery
            publishDiscovery();
            // Report online
            reportState("status", "online");
        }
    }
}

bool MQTTClient::connect() {
    String clientId = "controldiy-mando-" + String(random(0xffff), HEX);
    String willTopic = String(MQTTConfig::TOPIC_PREFIX) + "/status";

    bool result;
    if (strlen(MQTTConfig::DEFAULT_USER) > 0) {
        result = mqttClient_.connect(clientId.c_str(),
            MQTTConfig::DEFAULT_USER, MQTTConfig::DEFAULT_PASSWORD,
            willTopic.c_str(), 1, true, "offline");
    } else {
        result = mqttClient_.connect(clientId.c_str(),
            nullptr, nullptr,
            willTopic.c_str(), 1, true, "offline");
    }

    return result;
}

void MQTTClient::disconnect() {
    reportState("status", "offline");
    mqttClient_.disconnect();
}

bool MQTTClient::isConnected() const {
    return mqttClient_.connected();
}

void MQTTClient::publish(const String& topic, const String& payload, bool retain) {
    if (!isConnected()) return;
    mqttClient_.publish(topic.c_str(), payload.c_str(), retain);
}

void MQTTClient::subscribe(const String& topic) {
    Subscription sub;
    sub.topicFilter = topic;
    subscriptions_.push_back(sub);

    if (isConnected()) {
        mqttClient_.subscribe(topic.c_str());
    }
}

void MQTTClient::unsubscribe(const String& topic) {
    mqttClient_.unsubscribe(topic.c_str());
    subscriptions_.erase(
        std::remove_if(subscriptions_.begin(), subscriptions_.end(),
            [&topic](const Subscription& s) { return s.topicFilter == topic; }),
        subscriptions_.end());
}

void MQTTClient::onMessage(const String& topicFilter, MessageCallback cb) {
    for (auto& sub : subscriptions_) {
        if (sub.topicFilter == topicFilter) {
            sub.callback = cb;
            return;
        }
    }
    Subscription sub;
    sub.topicFilter = topicFilter;
    sub.callback = cb;
    subscriptions_.push_back(sub);
    subscribe(topicFilter);
}

void MQTTClient::publishDiscovery() {
    // HA MQTT Discovery - register device
    String baseTopic = String(MQTTConfig::DISCOVERY_PREFIX);

    // Battery sensor
    {
        String topic = baseTopic + "/sensor/controldiy_mando/battery/config";
        JsonDocument doc;
        doc["name"] = "Mando Battery";
        doc["unique_id"] = "controldiy_mando_battery";
        doc["state_topic"] = String(MQTTConfig::TOPIC_PREFIX) + "/battery";
        doc["unit_of_measurement"] = "%";
        doc["device_class"] = "battery";
        doc["icon"] = "mdi:battery";
        JsonObject device = doc["device"].to<JsonObject>();
        device["identifiers"][0] = "controldiy_mando";
        device["name"] = "Control DIY Mando";
        device["model"] = "Mando Portatil v1";
        device["manufacturer"] = "DIY";

        String payload;
        serializeJson(doc, payload);
        publish(topic, payload, true);
    }

    // WiFi signal sensor
    {
        String topic = baseTopic + "/sensor/controldiy_mando/rssi/config";
        JsonDocument doc;
        doc["name"] = "Mando WiFi Signal";
        doc["unique_id"] = "controldiy_mando_rssi";
        doc["state_topic"] = String(MQTTConfig::TOPIC_PREFIX) + "/rssi";
        doc["unit_of_measurement"] = "dBm";
        doc["device_class"] = "signal_strength";
        JsonObject device = doc["device"].to<JsonObject>();
        device["identifiers"][0] = "controldiy_mando";
        device["name"] = "Control DIY Mando";

        String payload;
        serializeJson(doc, payload);
        publish(topic, payload, true);
    }

    // Active scene sensor
    {
        String topic = baseTopic + "/sensor/controldiy_mando/scene/config";
        JsonDocument doc;
        doc["name"] = "Mando Active Scene";
        doc["unique_id"] = "controldiy_mando_scene";
        doc["state_topic"] = String(MQTTConfig::TOPIC_PREFIX) + "/scene";
        doc["icon"] = "mdi:remote";
        JsonObject device = doc["device"].to<JsonObject>();
        device["identifiers"][0] = "controldiy_mando";
        device["name"] = "Control DIY Mando";

        String payload;
        serializeJson(doc, payload);
        publish(topic, payload, true);
    }
}

void MQTTClient::callService(const String& domain, const String& service,
                              const String& entityId, const String& data) {
    // Use HA MQTT service call format
    String topic = String("homeassistant/service/") + domain + "/" + service;
    String payload = "{\"entity_id\":\"" + entityId + "\"";
    if (data.length() > 0) {
        payload += "," + data;
    }
    payload += "}";
    publish(topic, payload);
}

void MQTTClient::reportState(const String& key, const String& value) {
    String topic = String(MQTTConfig::TOPIC_PREFIX) + "/" + key;
    publish(topic, value, true);
}

void MQTTClient::staticCallback(char* topic, byte* payload, unsigned int length) {
    String t(topic);
    String p;
    p.reserve(length);
    for (unsigned int i = 0; i < length; i++) {
        p += (char)payload[i];
    }
    MQTTClient::instance().handleMessage(t, p);
}

void MQTTClient::handleMessage(const String& topic, const String& payload) {
    for (auto& sub : subscriptions_) {
        if (sub.callback) {
            // Simple topic matching (no wildcards for now)
            if (topic == sub.topicFilter || sub.topicFilter.endsWith("#")) {
                sub.callback(topic, payload);
            }
        }
    }
}

// Global helper
void mqtt_publish(const char* topic, const char* payload) {
    MQTTClient::instance().publish(String(topic), String(payload));
}
