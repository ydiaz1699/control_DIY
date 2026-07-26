#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <functional>
#include <map>
#include <vector>

// ============================================================================
// MQTT Client - Handles HA communication, sync protocol, and discovery
// ============================================================================

class MQTTClient {
public:
    static MQTTClient& instance();

    void init();
    void update();

    // Connection
    bool connect();
    void disconnect();
    bool isConnected() const;

    // Publish/Subscribe
    void publish(const String& topic, const String& payload, bool retain = false);
    void subscribe(const String& topic);
    void unsubscribe(const String& topic);

    // Message handler registration
    using MessageCallback = std::function<void(const String& topic, const String& payload)>;
    void onMessage(const String& topicFilter, MessageCallback cb);

    // HA Discovery (auto-register device in HA)
    void publishDiscovery();

    // HA Service calls
    void callService(const String& domain, const String& service,
                     const String& entityId, const String& data = "");

    // State reporting
    void reportState(const String& key, const String& value);

private:
    MQTTClient() = default;

    WiFiClient wifiClient_;
    PubSubClient mqttClient_{wifiClient_};

    struct Subscription {
        String topicFilter;
        MessageCallback callback;
    };
    std::vector<Subscription> subscriptions_;

    unsigned long lastReconnectMs_ = 0;
    bool wasConnected_ = false;

    static void staticCallback(char* topic, byte* payload, unsigned int length);
    void handleMessage(const String& topic, const String& payload);
};

// Global helper (used by CommandRouter and others)
void mqtt_publish(const char* topic, const char* payload);
