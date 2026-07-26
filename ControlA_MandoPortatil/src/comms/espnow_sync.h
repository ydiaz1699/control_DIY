/**
 * espnow_sync.h — ESP-NOW Inter-Device Sync
 * 
 * INNOVATION: Enables communication between Control A (portable) 
 * and Control B (desk panel) for state synchronization.
 * 
 * Features:
 * - Scene sync: Change scene on one = updates the other
 * - State broadcast: Battery, active device, current mode
 * - Low-latency (<5ms) direct communication without router
 */

#pragma once

#include <Arduino.h>

// Sync message types
enum SyncMsgType {
    SYNC_HEARTBEAT = 0,
    SYNC_SCENE_CHANGE,
    SYNC_MEDIA_STATE,
    SYNC_DEVICE_STATE,
    SYNC_NFC_TRIGGER,
    SYNC_COMMAND,
};

// State packet broadcast to other controls
struct SyncPacket {
    uint8_t msg_type;
    uint8_t device_id;      // 0 = Control A, 1 = Control B
    uint8_t battery_pct;
    uint8_t active_scene;
    uint8_t active_mode;
    uint8_t wifi_rssi;
    uint16_t uptime_min;
    char payload[32];       // Additional data (scene name, command, etc.)
};

void espnow_sync_init();
void espnow_broadcast_state();
void espnow_send_scene_change(uint8_t scene_id, const char* scene_name);
void espnow_send_command(const char* cmd);

// Callback for received sync messages
typedef void (*SyncRecvCallback)(SyncPacket packet);
void espnow_set_recv_callback(SyncRecvCallback cb);
