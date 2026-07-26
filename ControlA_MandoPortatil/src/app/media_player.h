/**
 * media_player.h — Media Player Controller
 * 
 * Based on homeThing's media player integration
 * Controls Spotify, Sonos, Chromecast, etc. via HA REST API / MQTT
 */

#pragma once

#include <Arduino.h>

struct MediaState {
    char title[64];
    char artist[64];
    char source[32];
    int volume;         // 0-100
    bool is_playing;
    bool available;
};

void media_player_init();
void media_player_poll_state();
void media_player_toggle_play();
void media_player_next();
void media_player_prev();
void media_player_set_volume(int vol);
void media_player_select_entity(int index);
MediaState media_player_get_state();
