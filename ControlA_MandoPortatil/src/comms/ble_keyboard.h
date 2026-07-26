/**
 * ble_keyboard.h — BLE HID Keyboard
 * 
 * Based on OMOTE NimBLE BLE keyboard for controlling:
 * - Fire TV, Apple TV, Android TV
 * - Any device accepting BLE HID keyboard input
 */

#pragma once

#include <Arduino.h>

void ble_keyboard_init();
void ble_keyboard_loop();
bool ble_keyboard_is_connected();
void ble_keyboard_write(uint8_t key);
void ble_keyboard_press(uint8_t key);
void ble_keyboard_release(uint8_t key);
void ble_keyboard_release_all();
void ble_keyboard_send_string(const char* str);
void ble_keyboard_media_play_pause();
void ble_keyboard_media_next();
void ble_keyboard_media_prev();
void ble_keyboard_media_vol_up();
void ble_keyboard_media_vol_down();
void ble_keyboard_media_mute();
void ble_keyboard_shutdown();
