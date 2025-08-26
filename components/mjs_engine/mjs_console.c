/**
 * @file mjs_console.c
 * @brief JavaScript Console Implementation
 */

#include "mjs_engine.h"
#include "esp_log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const char *TAG = "JS_CONSOLE";

static js_log_callback_t s_log_callback = NULL;
static void *s_log_user_data = NULL;

void mjs_console_log(const char *level, const char *fmt, ...)
{
    if (!level || !fmt) {
        return;
    }
    
    char buffer[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    // Print to ESP-IDF log system
    if (strcmp(level, "ERROR") == 0) {
        ESP_LOGE(TAG, "%s", buffer);
    } else if (strcmp(level, "WARN") == 0) {
        ESP_LOGW(TAG, "%s", buffer);
    } else if (strcmp(level, "INFO") == 0) {
        ESP_LOGI(TAG, "%s", buffer);
    } else if (strcmp(level, "DEBUG") == 0) {
        ESP_LOGD(TAG, "%s", buffer);
    } else {
        ESP_LOGI(TAG, "[%s] %s", level, buffer);
    }
    
    // Call user callback if registered
    if (s_log_callback) {
        s_log_callback(level, buffer, s_log_user_data);
    }
}

void mjs_console_print(const char *str)
{
    if (str) {
        printf("JS: %s\n", str);
        
        if (s_log_callback) {
            s_log_callback("LOG", str, s_log_user_data);
        }
    }
}

static void console_set_log_callback(js_log_callback_t callback, void *user_data)
{
    s_log_callback = callback;
    s_log_user_data = user_data;
}

/**
 * @brief Console module initialization
 */
esp_err_t console_module_init(void)
{
    ESP_LOGI(TAG, "Console module initialized");
    return ESP_OK;
}

// Export console functions for module registration
void mjs_console_set_callback(js_log_callback_t callback, void *user_data)
{
    console_set_log_callback(callback, user_data);
}