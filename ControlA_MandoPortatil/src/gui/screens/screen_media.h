/**
 * screen_media.h — Media Player Control Screen
 * 
 * Based on homeThing's Now Playing screen
 * Controls: Play/Pause, Next/Prev, Volume, Source selection
 */

#pragma once

#include <lvgl.h>

lv_obj_t* screen_media_create(lv_obj_t* parent);
void screen_media_update_state(const char* title, const char* artist, 
                               const char* source, int volume, bool playing);
