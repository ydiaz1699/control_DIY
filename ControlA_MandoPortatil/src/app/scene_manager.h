/**
 * scene_manager.h — Scene Manager
 * 
 * Manages activation of predefined scenes via MQTT
 * and syncs state with other controls via ESP-NOW
 */

#pragma once

#include <Arduino.h>

void scene_manager_init();
void scene_activate(int index);
int scene_get_active();
const char* scene_get_name(int index);
