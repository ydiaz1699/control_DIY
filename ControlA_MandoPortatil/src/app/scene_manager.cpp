/**
 * scene_manager.cpp — Scene Management
 * 
 * Activates scenes via MQTT and syncs with Control B via ESP-NOW
 */

#include "scene_manager.h"
#include "../config/user_config.h"
#include "../comms/mqtt_manager.h"
#include "../comms/espnow_sync.h"
#include "../hal/haptic_hal.h"
#include "../gui/gui_manager.h"

static int activeScene = -1;

void scene_manager_init() {
    Serial.println("[SCENES] Initialized with " + String(NUM_SCENES) + " scenes");
}

void scene_activate(int index) {
    if (index < 0 || index >= NUM_SCENES) return;
    
    const SceneConfig &scene = USER_SCENES[index];
    
    // Publish via MQTT
    if (mqtt_is_connected()) {
        mqtt_publish(scene.mqtt_topic, scene.mqtt_payload);
        Serial.printf("[SCENES] Activated: %s (MQTT: %s)\n", scene.name, scene.mqtt_topic);
    } else {
        Serial.printf("[SCENES] MQTT not connected, cannot activate: %s\n", scene.name);
        haptic_pulse(HAPTIC_ERROR);
        return;
    }
    
    activeScene = index;
    
    // Sync with Control B (Panel de Mesa)
    #if ENABLE_ESPNOW
    espnow_send_scene_change(index, scene.name);
    #endif
    
    // Show notification
    char msg[64];
    snprintf(msg, sizeof(msg), "%s activada", scene.name);
    gui_show_notification(msg, 1500);
    
    haptic_pulse(HAPTIC_CONFIRM);
}

int scene_get_active() {
    return activeScene;
}

const char* scene_get_name(int index) {
    if (index < 0 || index >= NUM_SCENES) return "—";
    return USER_SCENES[index].name;
}
