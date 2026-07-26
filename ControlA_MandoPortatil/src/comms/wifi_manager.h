/**
 * wifi_manager.h — WiFi Connection Manager
 */

#pragma once

#include <Arduino.h>

enum WiFiState {
    WIFI_STATE_DISCONNECTED = 0,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_ERROR,
};

void wifi_manager_init();
void wifi_manager_loop();
bool wifi_is_connected();
WiFiState wifi_get_state();
String wifi_get_ip();
int wifi_get_rssi();
void wifi_shutdown();
