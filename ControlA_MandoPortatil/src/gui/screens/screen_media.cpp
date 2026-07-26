/**
 * screen_media.cpp — Media Player Control Screen
 * 
 * Inspired by homeThing's Now Playing + Spotify-like UI
 */

#include "screen_media.h"
#include "../../app/media_player.h"
#include "../../hal/haptic_hal.h"
#include "../theme_engine.h"

static lv_obj_t *media_page = nullptr;
static lv_obj_t *lbl_title = nullptr;
static lv_obj_t *lbl_artist = nullptr;
static lv_obj_t *lbl_source = nullptr;
static lv_obj_t *slider_volume = nullptr;
static lv_obj_t *btn_play = nullptr;
static lv_obj_t *lbl_play_icon = nullptr;

static void btn_play_cb(lv_event_t *e) {
    media_player_toggle_play();
    haptic_pulse(HAPTIC_CLICK);
}

static void btn_prev_cb(lv_event_t *e) {
    media_player_prev();
    haptic_pulse(HAPTIC_CLICK);
}

static void btn_next_cb(lv_event_t *e) {
    media_player_next();
    haptic_pulse(HAPTIC_CLICK);
}

static void volume_slider_cb(lv_event_t *e) {
    lv_obj_t *slider = (lv_obj_t*)lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    media_player_set_volume(value);
}

lv_obj_t* screen_media_create(lv_obj_t* parent) {
    ThemeColors colors = theme_get_colors();
    
    media_page = lv_obj_create(parent);
    lv_obj_set_size(media_page, 240, 296);
    lv_obj_align(media_page, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(media_page, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(media_page, 0, 0);
    lv_obj_set_style_pad_all(media_page, 12, 0);
    lv_obj_clear_flag(media_page, LV_OBJ_FLAG_SCROLLABLE);
    
    // Title: "Now Playing"
    lv_obj_t *header = lv_label_create(media_page);
    lv_label_set_text(header, "Reproduciendo");
    lv_obj_set_style_text_font(header, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(header, lv_color_hex(colors.text_secondary), 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    
    // Album art placeholder (colored circle)
    lv_obj_t *art = lv_obj_create(media_page);
    lv_obj_set_size(art, 100, 100);
    lv_obj_align(art, LV_ALIGN_TOP_MID, 0, 24);
    lv_obj_set_style_bg_color(art, lv_color_hex(colors.accent), 0);
    lv_obj_set_style_bg_opa(art, LV_OPA_30, 0);
    lv_obj_set_style_radius(art, 50, 0);
    lv_obj_set_style_border_color(art, lv_color_hex(colors.accent), 0);
    lv_obj_set_style_border_width(art, 2, 0);
    lv_obj_clear_flag(art, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t *music_icon = lv_label_create(art);
    lv_label_set_text(music_icon, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_font(music_icon, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(music_icon, lv_color_hex(colors.accent), 0);
    lv_obj_center(music_icon);
    
    // Track title
    lbl_title = lv_label_create(media_page);
    lv_label_set_text(lbl_title, "Sin reproducción");
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(colors.text_primary), 0);
    lv_obj_set_style_text_align(lbl_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lbl_title, 216);
    lv_label_set_long_mode(lbl_title, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 132);
    
    // Artist
    lbl_artist = lv_label_create(media_page);
    lv_label_set_text(lbl_artist, "");
    lv_obj_set_style_text_font(lbl_artist, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_artist, lv_color_hex(colors.text_secondary), 0);
    lv_obj_set_style_text_align(lbl_artist, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lbl_artist, 216);
    lv_obj_align(lbl_artist, LV_ALIGN_TOP_MID, 0, 154);
    
    // Source label
    lbl_source = lv_label_create(media_page);
    lv_label_set_text(lbl_source, "");
    lv_obj_set_style_text_font(lbl_source, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_source, lv_color_hex(colors.accent), 0);
    lv_obj_align(lbl_source, LV_ALIGN_TOP_MID, 0, 170);
    
    // Transport controls (Prev / Play / Next)
    lv_obj_t *controls = lv_obj_create(media_page);
    lv_obj_set_size(controls, 200, 50);
    lv_obj_align(controls, LV_ALIGN_TOP_MID, 0, 185);
    lv_obj_set_style_bg_opa(controls, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(controls, 0, 0);
    lv_obj_set_flex_flow(controls, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(controls, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(controls, LV_OBJ_FLAG_SCROLLABLE);
    
    // Prev button
    lv_obj_t *btn_prev = lv_btn_create(controls);
    lv_obj_set_size(btn_prev, 44, 44);
    lv_obj_set_style_radius(btn_prev, 22, 0);
    lv_obj_set_style_bg_color(btn_prev, lv_color_hex(colors.card_bg), 0);
    lv_obj_t *prev_icon = lv_label_create(btn_prev);
    lv_label_set_text(prev_icon, LV_SYMBOL_PREV);
    lv_obj_center(prev_icon);
    lv_obj_add_event_cb(btn_prev, btn_prev_cb, LV_EVENT_CLICKED, nullptr);
    
    // Play/Pause button (larger)
    btn_play = lv_btn_create(controls);
    lv_obj_set_size(btn_play, 52, 52);
    lv_obj_set_style_radius(btn_play, 26, 0);
    lv_obj_set_style_bg_color(btn_play, lv_color_hex(colors.accent), 0);
    lbl_play_icon = lv_label_create(btn_play);
    lv_label_set_text(lbl_play_icon, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_font(lbl_play_icon, &lv_font_montserrat_20, 0);
    lv_obj_center(lbl_play_icon);
    lv_obj_add_event_cb(btn_play, btn_play_cb, LV_EVENT_CLICKED, nullptr);
    
    // Next button
    lv_obj_t *btn_next = lv_btn_create(controls);
    lv_obj_set_size(btn_next, 44, 44);
    lv_obj_set_style_radius(btn_next, 22, 0);
    lv_obj_set_style_bg_color(btn_next, lv_color_hex(colors.card_bg), 0);
    lv_obj_t *next_icon = lv_label_create(btn_next);
    lv_label_set_text(next_icon, LV_SYMBOL_NEXT);
    lv_obj_center(next_icon);
    lv_obj_add_event_cb(btn_next, btn_next_cb, LV_EVENT_CLICKED, nullptr);
    
    // Volume slider
    slider_volume = lv_slider_create(media_page);
    lv_obj_set_size(slider_volume, 180, 10);
    lv_obj_align(slider_volume, LV_ALIGN_TOP_MID, 0, 250);
    lv_slider_set_range(slider_volume, 0, 100);
    lv_slider_set_value(slider_volume, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider_volume, lv_color_hex(colors.bg_secondary), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider_volume, lv_color_hex(colors.accent), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider_volume, lv_color_hex(colors.accent), LV_PART_KNOB);
    lv_obj_add_event_cb(slider_volume, volume_slider_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    
    // Volume icons
    lv_obj_t *vol_low = lv_label_create(media_page);
    lv_label_set_text(vol_low, LV_SYMBOL_VOLUME_MID);
    lv_obj_set_style_text_color(vol_low, lv_color_hex(colors.text_secondary), 0);
    lv_obj_align(vol_low, LV_ALIGN_TOP_LEFT, 6, 247);
    
    lv_obj_t *vol_high = lv_label_create(media_page);
    lv_label_set_text(vol_high, LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_style_text_color(vol_high, lv_color_hex(colors.text_secondary), 0);
    lv_obj_align(vol_high, LV_ALIGN_TOP_RIGHT, -6, 247);
    
    return media_page;
}

void screen_media_update_state(const char* title, const char* artist, 
                               const char* source, int volume, bool playing) {
    if (lbl_title) lv_label_set_text(lbl_title, title ? title : "Sin reproducción");
    if (lbl_artist) lv_label_set_text(lbl_artist, artist ? artist : "");
    if (lbl_source) lv_label_set_text(lbl_source, source ? source : "");
    if (slider_volume) lv_slider_set_value(slider_volume, volume, LV_ANIM_ON);
    if (lbl_play_icon) lv_label_set_text(lbl_play_icon, playing ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);
}
