/**
 * mqtt_manager.cpp — MQTT Client Manager Implementation
 * 
 * Based on OMOTE mqtt_hal_esp32.cpp with enhanced auto-reconnect
 */

#include "mqtt_manager.h"
#include "wifi_manager.h"
#include "../config/user_config.h"
#include <WiFi.h>
#include <PubSubClient.h>

#if ENABLE_MQTT

static WiFiClient espClient;
static PubSubClient mqttClient(espClient);
static MQTTMessageCallback userCallback = nullptr;
static unsigned long lastReconnectAttempt = 0;
static const unsigned long RECONNECT_INTERVAL = 5000;

// Internal callback bridge
static void mqttCallbackBridge(char* topic, byte* payload, unsigned int length) {
    if (userCallback) {
        char payloadStr[512];
        int len = min((unsigned int)511, length);
        memcpy(payloadStr, payload, len);
        payloadStr[len] = '\0';
        userCallback(topic, payloadStr);
    }
}

void mqtt_manager_init() {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallbackBridge);
    mqttClient.setBufferSize(1024);
    
    Serial.printf("[MQTT] Configured: %s:%d\n", MQTT_SERVER, MQTT_PORT);
}

static bool connectToMQTT() {
    if (!wifi_is_connected()) return false;
    
    String clientId = String(MQTT_CLIENT_ID) + "_" + WiFi.macAddress();
    
    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
        Serial.println("[MQTT] Connected to broker");
        
        // Subscribe to state topics
        mqtt_subscribe("homeassistant/status");
        mqtt_subscribe("control_a/#");
        
        // Announce presence
        mqtt_publish("control_a/status", "online");
        
        return true;
    }
    return false;
}

void mqtt_manager_loop() {
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > RECONNECT_INTERVAL) {
            lastReconnectAttempt = now;
            connectToMQTT();
        }
    } else {
        mqttClient.loop();
    }
}

bool mqtt_is_connected() {
    return mqttClient.connected();
}

bool mqtt_publish(const char* topic, const char* payload) {
    if (!mqttClient.connected()) return false;
    return mqttClient.publish(topic, payload);
}

void mqtt_subscribe(const char* topic) {
    if (mqttClient.connected()) {
        mqttClient.subscribe(topic);
    }
}

void mqtt_set_callback(MQTTMessageCallback cb) {
    userCallback = cb;
}

void mqtt_shutdown() {
    mqtt_publish("control_a/status", "offline");
    mqttClient.disconnect();
}

#else
// Stub implementations when MQTT is disabled
void mqtt_manager_init() {}
void mqtt_manager_loop() {}
bool mqtt_is_connected() { return false; }
bool mqtt_publish(const char* topic, const char* payload) { return false; }
void mqtt_subscribe(const char* topic) {}
void mqtt_set_callback(MQTTMessageCallback cb) {}
void mqtt_shutdown() {}
#endif
