/**
 * screen_scenes.cpp — Scene Selection Screen
 * 
 * Grid of scene buttons with icons, swipe-to-navigate carousel
 */

#include "screen_scenes.h"
#include "../../config/user_config.h"
#include "../../app/scene_manager.h"
#include "../../hal/haptic_hal.h"
#include "../theme_engine.h"

static lv_obj_t *scene_page = nullptr;
static lv_obj_t *page_indicator = nullptr;
static lv_obj_t *scene_title_label = nullptr;

// Scene button click handler
static void scene_btn_event_cb(lv_event_t *e) {
    int scene_idx = (int)(intptr_t)lv_event_get_user_data(e);
    
    if (scene_idx >= 0 && scene_idx < NUM_SCENES) {
        scene_activate(scene_idx);
        haptic_pulse(HAPTIC_CONFIRM);
        
        // Visual feedback — flash the button
        lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x4CAF50), 0);
        // Reset after short delay via timer
    }
}

lv_obj_t* screen_scenes_create(lv_obj_t* parent) {
    ThemeColors colors = theme_get_colors();
    
    scene_page = lv_obj_create(parent);
    lv_obj_set_size(scene_page, 240, 296);
    lv_obj_align(scene_page, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(scene_page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(scene_page, 0, 0);
    lv_obj_set_style_pad_all(scene_page, 8, 0);
    lv_obj_clear_flag(scene_page, LV_OBJ_FLAG_SCROLLABLE);
    
    // Title
    scene_title_label = lv_label_create(scene_page);
    lv_label_set_text(scene_title_label, "Escenas");
    lv_obj_set_style_text_font(scene_title_label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(scene_title_label, lv_color_hex(colors.text_primary), 0);
    lv_obj_align(scene_title_label, LV_ALIGN_TOP_MID, 0, 4);
    
    // Grid of scene buttons (2 columns)
    lv_obj_t *grid = lv_obj_create(scene_page);
    lv_obj_set_size(grid, 224, 240);
    lv_obj_align(grid, LV_ALIGN_CENTER, 0, 12);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_pad_all(grid, 4, 0);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(grid, 8, 0);
    lv_obj_set_style_pad_column(grid, 8, 0);
    
    // Create scene buttons
    for (int i = 0; i < NUM_SCENES && i < MAX_SCENES; i++) {
        lv_obj_t *btn = lv_obj_create(grid);
        lv_obj_set_size(btn, 100, 68);
        lv_obj_set_style_bg_color(btn, lv_color_hex(colors.card_bg), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(colors.border), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_radius(btn, 12, 0);
        lv_obj_set_style_shadow_width(btn, 4, 0);
        lv_obj_set_style_shadow_opa(btn, LV_OPA_20, 0);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
        
        // Pressed style
        lv_obj_set_style_bg_color(btn, lv_color_hex(colors.accent), LV_STATE_PRESSED);
        
        // Icon
        lv_obj_t *icon = lv_label_create(btn);
        lv_label_set_text(icon, USER_SCENES[i].icon);
        lv_obj_set_style_text_font(icon, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_color(icon, lv_color_hex(colors.accent), 0);
        lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 4);
        
        // Name
        lv_obj_t *name = lv_label_create(btn);
        lv_label_set_text(name, USER_SCENES[i].name);
        lv_obj_set_style_text_font(name, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_color(name, lv_color_hex(colors.text_primary), 0);
        lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(name, LV_ALIGN_BOTTOM_MID, 0, -4);
        lv_obj_set_width(name, 90);
        
        // Click event
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(btn, scene_btn_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }
    
    // Page indicator dots
    page_indicator = lv_obj_create(scene_page);
    lv_obj_set_size(page_indicator, 100, 10);
    lv_obj_align(page_indicator, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_set_style_bg_opa(page_indicator, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(page_indicator, 0, 0);
    lv_obj_set_flex_flow(page_indicator, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(page_indicator, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(page_indicator, LV_OBJ_FLAG_SCROLLABLE);
    
    // Create dots for page indicator
    const char *screenNames[] = {"Scenes", "Media", "IR", "Smart", "Settings"};
    for (int i = 0; i < 5; i++) {
        lv_obj_t *dot = lv_obj_create(page_indicator);
        lv_obj_set_size(dot, (i == 0) ? 12 : 6, 6);
        lv_obj_set_style_bg_color(dot, lv_color_hex((i == 0) ? colors.accent : 0x555555), 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(dot, 3, 0);
        lv_obj_set_style_border_width(dot, 0, 0);
    }
    
    return scene_page;
}

void screen_scenes_refresh() {
    // Refresh scene states if needed
}
