#pragma once
#include <lvgl.h>

lv_obj_t* screen_ir_remote_create(lv_obj_t* parent);
void screen_ir_remote_set_device(int device_index);
