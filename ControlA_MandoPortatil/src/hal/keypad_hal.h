/**
 * keypad_hal.h — Keypad Controller HAL
 * 
 * TCA8418 I2C (Rev5) or GPIO matrix (Rev1-4)
 * Provides debounced key events with click, long-press, and repeat support
 */

#pragma once

#include <Arduino.h>

// Timing constants
#define LONG_PRESS_MS       600
#define REPEAT_INTERVAL_MS  150
#define MAX_KEY_EVENTS      8
#define NUM_KEYS            24

// Logical key identifiers (hardware-independent)
enum KeyID {
    KEY_NONE = -1,
    KEY_UP = 0,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_OK,
    KEY_BACK,
    KEY_HOME,
    KEY_MENU,
    KEY_VOL_UP,
    KEY_VOL_DOWN,
    KEY_CH_UP,
    KEY_CH_DOWN,
    KEY_POWER,
    KEY_MUTE,
    KEY_SOURCE,
    KEY_PLAY_PAUSE,
    KEY_SCENE_1,
    KEY_SCENE_2,
    KEY_SCENE_3,
    KEY_SCENE_4,
};

// Event types
enum KeyEventType {
    KEY_EVENT_PRESS = 0,
    KEY_EVENT_RELEASE,
    KEY_EVENT_CLICK,        // Short press + release
    KEY_EVENT_LONG_PRESS,   // Held > LONG_PRESS_MS
    KEY_EVENT_REPEAT,       // Auto-repeat while held
};

struct KeyEvent {
    KeyID key;
    KeyEventType type;
    uint32_t timestamp;
};

// Callback type for key events
typedef void (*KeyEventCallback)(KeyEvent event);

void keypad_init();
void keypad_loop();
void keypad_set_callback(KeyEventCallback cb);
bool keypad_any_pressed();
