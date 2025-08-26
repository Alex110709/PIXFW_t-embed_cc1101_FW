/**
 * @file js_notification_api.c
 * @brief JavaScript Notification API Implementation
 */

#include "js_api.h"
#include "lvgl_port.h"
#include "mjs.h"
#include "esp_log.h"
#include "driver/ledc.h"

static const char *TAG = "JS_NOTIFY_API";

/**
 * notify.show(title, message, timeout)
 * Show notification popup
 */
static mjs_val_t js_notify_show(struct mjs *mjs)
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

/**
 * notify.led(color, duration)
 * Control LED notification
 */
static mjs_val_t js_notify_led(struct mjs *mjs)
{
    char color[16];
    double duration;
    
    if (js_get_string_arg(mjs, 0, color, sizeof(color)) != ESP_OK) {
        return js_make_error(mjs, "Invalid color parameter");
    }
    if (js_get_number_arg(mjs, 1, &duration) != ESP_OK) {
        duration = 1000; // Default 1 second
    }
    
    // For T-Embed, we can use backlight as LED indicator
    uint8_t brightness = 255;
    if (strcmp(color, "red") == 0) {
        brightness = 128; // Dim for red effect
    } else if (strcmp(color, "blue") == 0) {
        brightness = 192; // Medium for blue effect
    } else if (strcmp(color, "green") == 0) {
        brightness = 255; // Full for green effect
    }
    
    lvgl_port_set_brightness(brightness);
    
    // Schedule to restore original brightness after duration
    // (simplified - in real implementation would use timer)
    
    ESP_LOGI(TAG, "LED notification: %s for %.0f ms", color, duration);
    return MJS_UNDEFINED;
}

/**
 * notify.beep(frequency, duration)
 * Generate beep sound
 */
static mjs_val_t js_notify_beep(struct mjs *mjs)
{
    double frequency, duration;
    
    if (js_get_number_arg(mjs, 0, &frequency) != ESP_OK) {
        frequency = 1000; // Default 1kHz
    }
    if (js_get_number_arg(mjs, 1, &duration) != ESP_OK) {
        duration = 200; // Default 200ms
    }
    
    // T-Embed doesn't have built-in speaker, but we can simulate with PWM
    // This is a placeholder implementation
    ESP_LOGI(TAG, "Beep: %.0f Hz for %.0f ms", frequency, duration);
    
    return MJS_UNDEFINED;
}

/**
 * notify.vibrate(duration)
 * Vibrate device (if supported)
 */
static mjs_val_t js_notify_vibrate(struct mjs *mjs)
{
    double duration;
    
    if (js_get_number_arg(mjs, 0, &duration) != ESP_OK) {
        duration = 500; // Default 500ms
    }
    
    // T-Embed doesn't have vibration motor, but we can simulate with visual feedback
    lvgl_port_show_notification("Vibrate", "Vibration simulation", 500);
    
    ESP_LOGI(TAG, "Vibrate for %.0f ms", duration);
    return MJS_UNDEFINED;
}

/**
 * notify.flash(times, interval)
 * Flash backlight
 */
static mjs_val_t js_notify_flash(struct mjs *mjs)
{
    double times, interval;
    
    if (js_get_number_arg(mjs, 0, &times) != ESP_OK) {
        times = 3; // Default 3 times
    }
    if (js_get_number_arg(mjs, 1, &interval) != ESP_OK) {
        interval = 200; // Default 200ms interval
    }
    
    // Flash backlight by quickly changing brightness
    // (simplified implementation)
    for (int i = 0; i < (int)times; i++) {
        lvgl_port_set_brightness(0);   // Off
        vTaskDelay(pdMS_TO_TICKS((int)interval / 2));
        lvgl_port_set_brightness(255); // On
        vTaskDelay(pdMS_TO_TICKS((int)interval / 2));
    }
    
    ESP_LOGI(TAG, "Flash %.0f times with %.0f ms interval", times, interval);
    return MJS_UNDEFINED;
}

esp_err_t js_notification_api_init(void)
{
    ESP_LOGI(TAG, "Initializing Notification API");
    return ESP_OK;
}

esp_err_t js_notification_api_register(js_context_t *ctx)
{
    if (!ctx || !ctx->mjs) {
        return ESP_ERR_INVALID_ARG;
    }
    
    struct mjs *mjs = ctx->mjs;
    
    mjs_set_ffi_func(mjs, "notify.show", js_notify_show);
    mjs_set_ffi_func(mjs, "notify.led", js_notify_led);
    mjs_set_ffi_func(mjs, "notify.beep", js_notify_beep);
    mjs_set_ffi_func(mjs, "notify.vibrate", js_notify_vibrate);
    mjs_set_ffi_func(mjs, "notify.flash", js_notify_flash);
    
    ESP_LOGI(TAG, "Notification API functions registered");
    return ESP_OK;
}