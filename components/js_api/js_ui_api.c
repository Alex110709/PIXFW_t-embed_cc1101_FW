/**
 * @file js_ui_api.c
 * @brief JavaScript UI API Implementation
 */

#include "js_api.h"
#include "lvgl_port.h" 
#include "mjs.h"
#include "esp_log.h"

static const char *TAG = "JS_UI_API";

/**
 * ui.createScreen()
 * Create new LVGL screen
 */
static mjs_val_t js_ui_create_screen(struct mjs *mjs)
{
    lvgl_port_lock();
    lv_obj_t *screen = lv_obj_create(NULL);
    lvgl_port_unlock();
    
    if (!screen) {
        return js_make_error(mjs, "Failed to create screen");
    }
    
    ESP_LOGI(TAG, "Created screen object");
    return mjs_mk_number(mjs, (double)(uintptr_t)screen);
}

/**
 * ui.createButton(parent, text, x, y, width, height)
 * Create button widget
 */
static mjs_val_t js_ui_create_button(struct mjs *mjs)
{
    char text[64];
    double parent_ptr, x, y, width, height;
    
    if (js_get_number_arg(mjs, 0, &parent_ptr) != ESP_OK) {
        return js_make_error(mjs, "Invalid parent parameter");
    }
    if (js_get_string_arg(mjs, 1, text, sizeof(text)) != ESP_OK) {
        return js_make_error(mjs, "Invalid text parameter");
    }
    
    lv_obj_t *parent = (lv_obj_t *)(uintptr_t)parent_ptr;
    
    lvgl_port_lock();
    lv_obj_t *btn = lv_btn_create(parent);
    if (btn) {
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, text);
        lv_obj_center(label);
    }
    lvgl_port_unlock();
    
    ESP_LOGI(TAG, "Created button: %s", text);
    return mjs_mk_number(mjs, (double)(uintptr_t)btn);
}

/**
 * ui.createLabel(parent, text)
 * Create label widget
 */
static mjs_val_t js_ui_create_label(struct mjs *mjs)
{
    char text[128];
    double parent_ptr;
    
    if (js_get_number_arg(mjs, 0, &parent_ptr) != ESP_OK) {
        return js_make_error(mjs, "Invalid parent parameter");
    }
    if (js_get_string_arg(mjs, 1, text, sizeof(text)) != ESP_OK) {
        return js_make_error(mjs, "Invalid text parameter");
    }
    
    lv_obj_t *parent = (lv_obj_t *)(uintptr_t)parent_ptr;
    
    lvgl_port_lock();
    lv_obj_t *label = lv_label_create(parent);
    if (label) {
        lv_label_set_text(label, text);
    }
    lvgl_port_unlock();
    
    ESP_LOGI(TAG, "Created label: %s", text);
    return mjs_mk_number(mjs, (double)(uintptr_t)label);
}

/**
 * ui.showNotification(title, message, timeout)
 * Show notification popup
 */
static mjs_val_t js_ui_show_notification(struct mjs *mjs)
{
    char title[64], message[128];
    double timeout;
    
    if (js_get_string_arg(mjs, 0, title, sizeof(title)) != ESP_OK) {
        return js_make_error(mjs, "Invalid title parameter");
    }
    if (js_get_string_arg(mjs, 1, message, sizeof(message)) != ESP_OK) {
        return js_make_error(mjs, "Invalid message parameter");
    }
    if (js_get_number_arg(mjs, 2, &timeout) != ESP_OK) {
        timeout = 3000; // Default 3 seconds
    }
    
    lvgl_port_show_notification(title, message, (uint32_t)timeout);
    
    ESP_LOGI(TAG, "Showed notification: %s - %s", title, message);
    return MJS_UNDEFINED;
}

esp_err_t js_ui_api_init(void)
{
    ESP_LOGI(TAG, "Initializing UI API");
    return ESP_OK;
}

esp_err_t js_ui_api_register(js_context_t *ctx)
{
    if (!ctx || !ctx->mjs) {
        return ESP_ERR_INVALID_ARG;
    }
    
    struct mjs *mjs = ctx->mjs;
    
    mjs_set_ffi_func(mjs, "ui.createScreen", js_ui_create_screen);
    mjs_set_ffi_func(mjs, "ui.createButton", js_ui_create_button);
    mjs_set_ffi_func(mjs, "ui.createLabel", js_ui_create_label);
    mjs_set_ffi_func(mjs, "ui.showNotification", js_ui_show_notification);
    
    ESP_LOGI(TAG, "UI API functions registered");
    return ESP_OK;
}