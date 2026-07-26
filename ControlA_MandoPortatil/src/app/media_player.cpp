/**
 * media_player.cpp — Media Player Controller Implementation
 * 
 * Uses MQTT to control HA media_player entities
 */

#include "media_player.h"
#include "../config/user_config.h"
#include "../comms/mqtt_manager.h"
#include "../gui/gui_manager.h"
#include "../gui/screens/screen_media.h"

static MediaState currentState = {"", "", "", 50, false, false};
static int currentPlayerIndex = 0;

void media_player_init() {
    memset(&currentState, 0, sizeof(MediaState));
    currentState.volume = 50;
    
    // Subscribe to media player state updates
    if (NUM_MEDIA_PLAYERS > 0) {
        char topic[128];
        snprintf(topic, sizeof(topic), "homeassistant/media_player/%s/state", 
                 MEDIA_PLAYERS[currentPlayerIndex] + strlen("media_player."));
        mqtt_subscribe(topic);
    }
    
    Serial.printf("[MEDIA] Initialized with %d players\n", NUM_MEDIA_PLAYERS);
}

void media_player_poll_state() {
    // In a real implementation, this would query HA REST API or process MQTT state topics
    // For now, update GUI with whatever state we have
    screen_media_update_state(currentState.title, currentState.artist,
                             currentState.source, currentState.volume,
                             currentState.is_playing);
}

void media_player_toggle_play() {
    char topic[128];
    snprintf(topic, sizeof(topic), "homeassistant/media_player/%s/command",
             MEDIA_PLAYERS[currentPlayerIndex] + strlen("media_player."));
    
    mqtt_publish(topic, currentState.is_playing ? "pause" : "play");
    currentState.is_playing = !currentState.is_playing;
}

void media_player_next() {
    char topic[128];
    snprintf(topic, sizeof(topic), "homeassistant/media_player/%s/command",
             MEDIA_PLAYERS[currentPlayerIndex] + strlen("media_player."));
    mqtt_publish(topic, "next");
}

void media_player_prev() {
    char topic[128];
    snprintf(topic, sizeof(topic), "homeassistant/media_player/%s/command",
             MEDIA_PLAYERS[currentPlayerIndex] + strlen("media_player."));
    mqtt_publish(topic, "previous");
}

void media_player_set_volume(int vol) {
    currentState.volume = constrain(vol, 0, 100);
    
    char topic[128];
    char payload[16];
    snprintf(topic, sizeof(topic), "homeassistant/media_player/%s/volume",
             MEDIA_PLAYERS[currentPlayerIndex] + strlen("media_player."));
    snprintf(payload, sizeof(payload), "%d", vol);
    mqtt_publish(topic, payload);
}

void media_player_select_entity(int index) {
    if (index >= 0 && index < NUM_MEDIA_PLAYERS) {
        currentPlayerIndex = index;
        Serial.printf("[MEDIA] Selected player: %s\n", MEDIA_PLAYERS[index]);
    }
}

MediaState media_player_get_state() {
    return currentState;
}
