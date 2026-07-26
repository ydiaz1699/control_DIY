/**
 * ir_controller.cpp — IR Device Controller Implementation
 */

#include "ir_controller.h"
#include "../config/user_config.h"
#include "../hal/ir_hal.h"
#include "../hal/haptic_hal.h"

static int activeDeviceIndex = 0;

void ir_controller_init() {
    activeDeviceIndex = 0;
    Serial.printf("[IR_CTRL] Initialized with %d devices\n", NUM_IR_DEVICES);
}

void ir_controller_set_device(int index) {
    if (index >= 0 && index < NUM_IR_DEVICES) {
        activeDeviceIndex = index;
        Serial.printf("[IR_CTRL] Active device: %s\n", IR_DEVICES[index].name);
    }
}

int ir_controller_get_device() {
    return activeDeviceIndex;
}

void ir_controller_send_command(const char* command) {
    if (activeDeviceIndex < 0 || activeDeviceIndex >= NUM_IR_DEVICES) return;
    
    if (strcmp(command, "power") == 0) {
        ir_send_power(activeDeviceIndex);
    } else if (strcmp(command, "vol_up") == 0) {
        ir_send_volume_up(activeDeviceIndex);
    } else if (strcmp(command, "vol_down") == 0) {
        ir_send_volume_down(activeDeviceIndex);
    } else if (strcmp(command, "mute") == 0) {
        ir_send_mute(activeDeviceIndex);
    } else if (strcmp(command, "ch_up") == 0) {
        ir_send_channel_up(activeDeviceIndex);
    } else if (strcmp(command, "ch_down") == 0) {
        ir_send_channel_down(activeDeviceIndex);
    }
    
    haptic_pulse(HAPTIC_CLICK);
}
