/**
 * ir_controller.h — IR Device Controller
 * 
 * Manages IR device selection and command routing
 */

#pragma once

#include <Arduino.h>

void ir_controller_init();
void ir_controller_set_device(int index);
int ir_controller_get_device();
void ir_controller_send_command(const char* command);
