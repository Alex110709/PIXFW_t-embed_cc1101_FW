/**
 * @file lvgl_helpers.c
 * @brief LVGL Helper Functions
 */

#include "lvgl_port.h"
#include "esp_log.h"

static const char *TAG = "LVGL_HELPERS";

// Theme colors
#define THEME_PRIMARY   lv_color_hex(0x2196F3)  // Blue
#define THEME_SECONDARY lv_color_hex(0x4CAF50)  // Green
#define THEME_ACCENT    lv_color_hex(0xFF9800)  // Orange
#define THEME_ERROR     lv_color_hex(0xF44336)  // Red
#define THEME_SUCCESS   lv_color_hex(0x4CAF50)  // Green
#define THEME_WARNING   lv_color_hex(0xFF9800)  // Orange

/**
 * @brief Create a styled button
 */
lv_obj_t* lvgl_create_button(lv_obj_t *parent, const char *text, lv_coord_t width, lv_coord_t height)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, width, height);
    
    // Style the button
    lv_obj_set_style_bg_color(btn, THEME_PRIMARY, 0);
    lv_obj_set_style_bg_color(btn, lv_color_darken(THEME_PRIMARY, LV_OPA_20), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, 8, 0);
    
    // Add label if text provided
    if (text) {
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_center(label);
    }
    
    return btn;
}

/**
 * @brief Create a styled list
 */
lv_obj_t* lvgl_create_list(lv_obj_t *parent)
{
    lv_obj_t *list = lv_list_create(parent);
    
    // Style the list
    lv_obj_set_style_bg_color(list, lv_color_hex(0x1E1E1E), 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_radius(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    
    return list;
}

/**
 * @brief Create a styled list button
 */
lv_obj_t* lvgl_create_list_button(lv_obj_t *list, const char *icon, const char *text)
{
    lv_obj_t *btn = lv_list_add_btn(list, icon, text);
    
    // Style the list button
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x2E2E2E), 0);
    lv_obj_set_style_bg_color(btn, THEME_PRIMARY, LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x3E3E3E), LV_STATE_FOCUSED);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, 0, 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    
    return btn;
}

/**
 * @brief Create a modal dialog
 */
lv_obj_t* lvgl_create_modal(lv_obj_t *parent, const char *title)
{
    // Background overlay
    lv_obj_t *bg = lv_obj_create(parent);
    lv_obj_set_size(bg, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_set_style_bg_color(bg, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_70, 0);
    lv_obj_set_style_border_width(bg, 0, 0);
    
    // Modal container
    lv_obj_t *modal = lv_obj_create(bg);
    lv_obj_set_size(modal, LCD_WIDTH - 40, LCD_HEIGHT - 80);
    lv_obj_center(modal);
    lv_obj_set_style_bg_color(modal, lv_color_hex(0x2E2E2E), 0);
    lv_obj_set_style_border_color(modal, THEME_PRIMARY, 0);
    lv_obj_set_style_border_width(modal, 2, 0);
    lv_obj_set_style_radius(modal, 8, 0);
    
    // Title bar
    if (title) {
        lv_obj_t *title_bar = lv_obj_create(modal);
        lv_obj_set_size(title_bar, lv_pct(100), 40);
        lv_obj_set_pos(title_bar, 0, 0);
        lv_obj_set_style_bg_color(title_bar, THEME_PRIMARY, 0);
        lv_obj_set_style_border_width(title_bar, 0, 0);
        lv_obj_set_style_radius(title_bar, 6, 0);
        lv_obj_set_style_radius(title_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        
        lv_obj_t *title_label = lv_label_create(title_bar);
        lv_label_set_text(title_label, title);
        lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
        lv_obj_center(title_label);
    }
    
    return modal;
}

/**
 * @brief Create a progress bar
 */
lv_obj_t* lvgl_create_progress_bar(lv_obj_t *parent, lv_coord_t width)
{
    lv_obj_t *bar = lv_bar_create(parent);
    lv_obj_set_size(bar, width, 20);
    
    // Style the progress bar
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x4E4E4E), 0);
    lv_obj_set_style_bg_color(bar, THEME_SUCCESS, LV_PART_INDICATOR);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 10, 0);
    
    return bar;
}

/**
 * @brief Create a toggle switch
 */
lv_obj_t* lvgl_create_switch(lv_obj_t *parent)
{
    lv_obj_t *sw = lv_switch_create(parent);
    
    // Style the switch
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x4E4E4E), 0);
    lv_obj_set_style_bg_color(sw, THEME_SUCCESS, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw, lv_color_white(), LV_PART_KNOB);
    
    return sw;
}

/**
 * @brief Create a text area with label
 */
lv_obj_t* lvgl_create_text_area(lv_obj_t *parent, const char *label_text, const char *placeholder)
{
    // Container
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 5, 0);
    
    // Label
    if (label_text) {
        lv_obj_t *label = lv_label_create(container);
        lv_label_set_text(label, label_text);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
    }
    
    // Text area
    lv_obj_t *ta = lv_textarea_create(container);
    lv_obj_set_size(ta, lv_pct(100), 40);
    lv_obj_set_style_bg_color(ta, lv_color_hex(0x1E1E1E), 0);
    lv_obj_set_style_border_color(ta, THEME_PRIMARY, 0);
    lv_obj_set_style_border_width(ta, 1, 0);
    lv_obj_set_style_radius(ta, 4, 0);
    lv_obj_set_style_text_color(ta, lv_color_white(), 0);
    
    if (placeholder) {
        lv_textarea_set_placeholder_text(ta, placeholder);
    }
    
    return ta;
}

/**
 * @brief Show error message
 */
void lvgl_show_error(const char *message)
{
    lvgl_port_show_notification("Error", message, 3000);
}

/**
 * @brief Show success message
 */
void lvgl_show_success(const char *message)
{
    lvgl_port_show_notification("Success", message, 2000);
}

/**
 * @brief Show warning message
 */
void lvgl_show_warning(const char *message)
{
    lvgl_port_show_notification("Warning", message, 3000);
}

/**
 * @brief Show info message
 */
void lvgl_show_info(const char *message)
{
    lvgl_port_show_notification("Info", message, 2000);
}

/**
 * @brief Format frequency for display
 */
void lvgl_format_frequency(uint32_t freq_hz, char *buffer, size_t buffer_size)
{
    if (freq_hz >= 1000000) {
        // MHz
        uint32_t mhz = freq_hz / 1000000;
        uint32_t khz = (freq_hz % 1000000) / 1000;
        if (khz == 0) {
            snprintf(buffer, buffer_size, "%u MHz", mhz);
        } else {
            snprintf(buffer, buffer_size, "%u.%03u MHz", mhz, khz);
        }
    } else if (freq_hz >= 1000) {
        // kHz
        uint32_t khz = freq_hz / 1000;
        uint32_t hz = freq_hz % 1000;
        if (hz == 0) {
            snprintf(buffer, buffer_size, "%u kHz", khz);
        } else {
            snprintf(buffer, buffer_size, "%u.%03u kHz", khz, hz);
        }
    } else {
        // Hz
        snprintf(buffer, buffer_size, "%u Hz", freq_hz);
    }
}

/**
 * @brief Format RSSI for display
 */
void lvgl_format_rssi(int16_t rssi_dbm, char *buffer, size_t buffer_size)
{
    snprintf(buffer, buffer_size, "%d dBm", rssi_dbm);
}

/**
 * @brief Format data rate for display
 */
void lvgl_format_data_rate(uint32_t rate_bps, char *buffer, size_t buffer_size)
{
    if (rate_bps >= 1000000) {
        // Mbps
        uint32_t mbps = rate_bps / 1000000;
        uint32_t kbps = (rate_bps % 1000000) / 1000;
        if (kbps == 0) {
            snprintf(buffer, buffer_size, "%u Mbps", mbps);
        } else {
            snprintf(buffer, buffer_size, "%u.%03u Mbps", mbps, kbps);
        }
    } else if (rate_bps >= 1000) {
        // kbps
        uint32_t kbps = rate_bps / 1000;
        uint32_t bps = rate_bps % 1000;
        if (bps == 0) {
            snprintf(buffer, buffer_size, "%u kbps", kbps);
        } else {
            snprintf(buffer, buffer_size, "%u.%03u kbps", kbps, bps);
        }
    } else {
        // bps
        snprintf(buffer, buffer_size, "%u bps", rate_bps);
    }
}