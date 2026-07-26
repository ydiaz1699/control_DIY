/**
 * espnow_sync.cpp — ESP-NOW Inter-Device Sync Implementation
 * 
 * Allows Control A and Control B to communicate directly via ESP-NOW
 * without requiring WiFi router infrastructure
 */

#include "espnow_sync.h"
#include "../config/user_config.h"
#include "../hal/battery_hal.h"
#include <WiFi.h>
#include <esp_now.h>

#if ENABLE_ESPNOW

static uint8_t panelMacAddr[] = PANEL_MAC_ADDR;
static SyncRecvCallback recvCallback = nullptr;
static esp_now_peer_info_t peerInfo = {};
static bool peerAdded = false;

// ESP-NOW send callback
static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS) {
        // Silent fail — ESP-NOW is best-effort
    }
}

// ESP-NOW receive callback
static void onDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
    if (len < sizeof(SyncPacket)) return;
    
    SyncPacket packet;
    memcpy(&packet, data, sizeof(SyncPacket));
    
    if (recvCallback) {
        recvCallback(packet);
    }
}

void espnow_sync_init() {
    // ESP-NOW requires WiFi to be initialized (STA or AP mode)
    if (WiFi.getMode() == WIFI_OFF) {
        WiFi.mode(WIFI_STA);
    }
    
    if (esp_now_init() != ESP_OK) {
        Serial.println("[ESP-NOW] Init FAILED!");
        return;
    }
    
    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);
    
    // Add peer (Panel de Mesa)
    memcpy(peerInfo.peer_addr, panelMacAddr, 6);
    peerInfo.channel = 0;  // Use current channel
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
        peerAdded = true;
        Serial.println("[ESP-NOW] Peer added (Control B Panel)");
    } else {
        Serial.println("[ESP-NOW] Peer add failed (will broadcast)");
    }
    
    Serial.println("[ESP-NOW] Initialized — inter-device sync ready");
}

void espnow_broadcast_state() {
    SyncPacket packet = {};
    packet.msg_type = SYNC_HEARTBEAT;
    packet.device_id = 0; // Control A
    packet.battery_pct = battery_get_percentage();
    packet.active_scene = 0;
    packet.active_mode = 0;
    packet.wifi_rssi = (uint8_t)abs(WiFi.RSSI());
    packet.uptime_min = millis() / 60000;
    strncpy(packet.payload, "alive", sizeof(packet.payload));
    
    if (peerAdded) {
        esp_now_send(panelMacAddr, (uint8_t*)&packet, sizeof(SyncPacket));
    } else {
        // Broadcast to all
        esp_now_send(nullptr, (uint8_t*)&packet, sizeof(SyncPacket));
    }
}

void espnow_send_scene_change(uint8_t scene_id, const char* scene_name) {
    SyncPacket packet = {};
    packet.msg_type = SYNC_SCENE_CHANGE;
    packet.device_id = 0;
    packet.active_scene = scene_id;
    strncpy(packet.payload, scene_name, sizeof(packet.payload) - 1);
    
    if (peerAdded) {
        esp_now_send(panelMacAddr, (uint8_t*)&packet, sizeof(SyncPacket));
    }
}

void espnow_send_command(const char* cmd) {
    SyncPacket packet = {};
    packet.msg_type = SYNC_COMMAND;
    packet.device_id = 0;
    strncpy(packet.payload, cmd, sizeof(packet.payload) - 1);
    
    if (peerAdded) {
        esp_now_send(panelMacAddr, (uint8_t*)&packet, sizeof(SyncPacket));
    }
}

void espnow_set_recv_callback(SyncRecvCallback cb) {
    recvCallback = cb;
}

#else
// Stubs
void espnow_sync_init() {}
void espnow_broadcast_state() {}
void espnow_send_scene_change(uint8_t scene_id, const char* scene_name) {}
void espnow_send_command(const char* cmd) {}
void espnow_set_recv_callback(SyncRecvCallback cb) {}
#endif
