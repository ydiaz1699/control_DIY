/**
 * keypad_hal.cpp — Keypad Controller Implementation
 * 
 * TCA8418 I2C keypad controller (Rev5) or direct GPIO matrix (Rev1-4)
 * Supports multi-click, long press, and repeat events
 */

#include "keypad_hal.h"
#include "../config/pin_config.h"
#include "haptic_hal.h"
#include "imu_hal.h"

#if (HARDWARE_REV >= 5)
#include <Adafruit_TCA8418.h>
static Adafruit_TCA8418 keypad;
#endif

// Key state tracking
static KeyEvent pendingEvents[MAX_KEY_EVENTS];
static int numPendingEvents = 0;
static uint32_t keyPressTimestamp[NUM_KEYS] = {0};
static bool keyIsPressed[NUM_KEYS] = {false};
static bool longPressTriggered[NUM_KEYS] = {false};
static KeyEventCallback keyCallback = nullptr;

void keypad_init() {
    #if (HARDWARE_REV >= 5)
    if (!keypad.begin(KEYPAD_I2C_ADDR, &Wire)) {
        Serial.println("[KEYPAD] ERROR: TCA8418 not found!");
        return;
    }
    
    // Configure 5x5 matrix
    keypad.matrix(5, 5);
    keypad.enableInterrupts();
    
    // Configure interrupt pin
    pinMode(KEYPAD_INT_GPIO, INPUT_PULLUP);
    
    Serial.println("[KEYPAD] TCA8418 initialized (5x5 matrix)");
    #else
    // Direct GPIO setup for Rev1-4
    pinMode(SW_1_GPIO, INPUT_PULLUP);
    pinMode(SW_2_GPIO, INPUT_PULLUP);
    pinMode(SW_3_GPIO, INPUT_PULLUP);
    pinMode(SW_4_GPIO, INPUT_PULLUP);
    pinMode(SW_5_GPIO, INPUT_PULLUP);
    Serial.println("[KEYPAD] GPIO matrix initialized");
    #endif
}

void keypad_set_callback(KeyEventCallback cb) {
    keyCallback = cb;
}

// Map raw key ID to logical button name
static KeyID mapRawKey(uint8_t raw) {
    // TCA8418 key mapping (physical position to logical function)
    switch (raw) {
        case 1:  return KEY_UP;
        case 2:  return KEY_RIGHT;
        case 3:  return KEY_DOWN;
        case 4:  return KEY_LEFT;
        case 5:  return KEY_OK;
        case 6:  return KEY_BACK;
        case 7:  return KEY_HOME;
        case 8:  return KEY_MENU;
        case 9:  return KEY_VOL_UP;
        case 10: return KEY_VOL_DOWN;
        case 11: return KEY_CH_UP;
        case 12: return KEY_CH_DOWN;
        case 13: return KEY_POWER;
        case 14: return KEY_MUTE;
        case 15: return KEY_SOURCE;
        case 16: return KEY_PLAY_PAUSE;
        case 17: return KEY_SCENE_1;
        case 18: return KEY_SCENE_2;
        case 19: return KEY_SCENE_3;
        case 20: return KEY_SCENE_4;
        default: return KEY_NONE;
    }
}

void keypad_loop() {
    uint32_t now = millis();
    numPendingEvents = 0;
    
    #if (HARDWARE_REV >= 5)
    // Check TCA8418 for key events
    if (digitalRead(KEYPAD_INT_GPIO) == LOW) {
        while (keypad.available() > 0) {
            uint16_t evt = keypad.getEvent();
            uint8_t rawKey = evt & 0x7F;
            bool pressed = (evt & 0x80) != 0;
            
            KeyID key = mapRawKey(rawKey);
            if (key == KEY_NONE) continue;
            
            if (pressed) {
                keyIsPressed[key] = true;
                keyPressTimestamp[key] = now;
                longPressTriggered[key] = false;
                
                // Activity detected
                imu_set_activity_timestamp();
                haptic_pulse(HAPTIC_CLICK);
                
                // Emit press event
                if (numPendingEvents < MAX_KEY_EVENTS) {
                    pendingEvents[numPendingEvents++] = {key, KEY_EVENT_PRESS, now};
                }
            } else {
                if (keyIsPressed[key]) {
                    uint32_t duration = now - keyPressTimestamp[key];
                    
                    if (!longPressTriggered[key]) {
                        // Short press release = single click
                        if (numPendingEvents < MAX_KEY_EVENTS) {
                            pendingEvents[numPendingEvents++] = {key, KEY_EVENT_CLICK, now};
                        }
                    }
                    
                    if (numPendingEvents < MAX_KEY_EVENTS) {
                        pendingEvents[numPendingEvents++] = {key, KEY_EVENT_RELEASE, now};
                    }
                    
                    keyIsPressed[key] = false;
                }
            }
        }
        keypad.resetInterrupts();
    }
    #else
    // GPIO polling for Rev1-4 (simplified)
    static bool prevState[5] = {true, true, true, true, true};
    uint8_t pins[] = {SW_1_GPIO, SW_2_GPIO, SW_3_GPIO, SW_4_GPIO, SW_5_GPIO};
    KeyID keys[] = {KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_OK};
    
    for (int i = 0; i < 5; i++) {
        bool state = digitalRead(pins[i]);
        if (!state && prevState[i]) {
            // Button pressed
            imu_set_activity_timestamp();
            haptic_pulse(HAPTIC_CLICK);
            if (numPendingEvents < MAX_KEY_EVENTS) {
                pendingEvents[numPendingEvents++] = {keys[i], KEY_EVENT_CLICK, now};
            }
        }
        prevState[i] = state;
    }
    #endif
    
    // Check for long press (all revisions)
    for (int i = 0; i < NUM_KEYS; i++) {
        if (keyIsPressed[i] && !longPressTriggered[i]) {
            if (now - keyPressTimestamp[i] > LONG_PRESS_MS) {
                longPressTriggered[i] = true;
                haptic_pulse(HAPTIC_LONG_PRESS);
                if (numPendingEvents < MAX_KEY_EVENTS) {
                    pendingEvents[numPendingEvents++] = {(KeyID)i, KEY_EVENT_LONG_PRESS, now};
                }
            }
        }
    }
    
    // Dispatch events to callback
    if (keyCallback && numPendingEvents > 0) {
        for (int i = 0; i < numPendingEvents; i++) {
            keyCallback(pendingEvents[i]);
        }
    }
}

bool keypad_any_pressed() {
    for (int i = 0; i < NUM_KEYS; i++) {
        if (keyIsPressed[i]) return true;
    }
    return false;
}
