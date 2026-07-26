/**
 * wifi_manager.cpp — WiFi Connection Manager
 * 
 * Auto-connects, handles reconnection, and supports power saving mode
 */

#include "wifi_manager.h"
#include "../config/user_config.h"
#include <WiFi.h>

static WiFiState currentState = WIFI_STATE_DISCONNECTED;
static unsigned long lastReconnectAttempt = 0;
static const unsigned long RECONNECT_INTERVAL = 5000;

static void onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
            currentState = WIFI_STATE_CONNECTED;
            Serial.printf("[WiFi] Connected! IP: %s, RSSI: %d dBm\n",
                         WiFi.localIP().toString().c_str(), WiFi.RSSI());
            break;
            
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            currentState = WIFI_STATE_DISCONNECTED;
            Serial.println("[WiFi] Disconnected. Will reconnect...");
            break;
            
        default:
            break;
    }
}

void wifi_manager_init() {
    WiFi.setHostname(WIFI_HOSTNAME);
    WiFi.onEvent(onWiFiEvent);
    WiFi.setSleep(true);  // Enable WiFi power saving
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    currentState = WIFI_STATE_CONNECTING;
    
    Serial.printf("[WiFi] Connecting to %s...\n", WIFI_SSID);
}

void wifi_manager_loop() {
    if (currentState == WIFI_STATE_DISCONNECTED) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > RECONNECT_INTERVAL) {
            lastReconnectAttempt = now;
            WiFi.reconnect();
            currentState = WIFI_STATE_CONNECTING;
        }
    }
}

bool wifi_is_connected() {
    return WiFi.isConnected();
}

WiFiState wifi_get_state() {
    return currentState;
}

String wifi_get_ip() {
    return WiFi.localIP().toString();
}

int wifi_get_rssi() {
    return WiFi.RSSI();
}

void wifi_shutdown() {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    currentState = WIFI_STATE_DISCONNECTED;
}
