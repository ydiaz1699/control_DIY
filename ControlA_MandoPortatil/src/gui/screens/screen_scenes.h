/**
 * screen_scenes.h — Scene Selection Screen
 * 
 * Carousel of quick-access scenes with icons and animations
 * Inspired by OLED Remote carousel + OMOTE scene selection
 */

#pragma once

#include <lvgl.h>

lv_obj_t* screen_scenes_create(lv_obj_t* parent);
void screen_scenes_refresh();
