/**
 * screen_idle.h — Idle/Screensaver Screen
 * 
 * INNOVATION: Shows weather + clock widget when remote is idle
 * (just before going to sleep), similar to an always-on display
 */

#pragma once

#include <lvgl.h>

lv_obj_t* screen_idle_create(lv_obj_t* parent);
void screen_idle_update_weather(const char* condition, float temperature);
void screen_idle_update_time(int hour, int minute);
