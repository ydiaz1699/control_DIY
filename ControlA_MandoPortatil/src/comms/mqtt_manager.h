/**
 * mqtt_manager.h — MQTT Client Manager
 * 
 * Connects to MQTT broker (Home Assistant Mosquitto)
 * Publishes commands and subscribes to state topics
 */

#pragma once

#include <Arduino.h>

typedef void (*MQTTMessageCallback)(const char* topic, const char* payload);

void mqtt_manager_init();
void mqtt_manager_loop();
bool mqtt_is_connected();
bool mqtt_publish(const char* topic, const char* payload);
void mqtt_subscribe(const char* topic);
void mqtt_set_callback(MQTTMessageCallback cb);
void mqtt_shutdown();
