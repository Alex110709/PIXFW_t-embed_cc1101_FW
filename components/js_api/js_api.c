/**
 * @file js_api.c
 * @brief JavaScript API Main Implementation
 */

#include "js_api.h"
#include "mjs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "JS_API";

static bool s_initialized = false;

esp_err_t js_api_init(void)
{
    if (s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Initializing JavaScript API modules");
    
    // Initialize all API modules
    ESP_ERROR_CHECK(js_rf_api_init());
    ESP_ERROR_CHECK(js_gpio_api_init());
    ESP_ERROR_CHECK(js_ui_api_init());
    ESP_ERROR_CHECK(js_storage_api_init());
    ESP_ERROR_CHECK(js_notification_api_init());
    
    s_initialized = true;
    ESP_LOGI(TAG, "JavaScript API modules initialized");
    
    return ESP_OK;
}

esp_err_t js_api_deinit(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Deinitializing JavaScript API modules");
    
    s_initialized = false;
    
    return ESP_OK;
}

esp_err_t js_api_register_all(js_context_t *ctx)
{
    if (!ctx || !s_initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Registering all API functions");
    
    // Register all API modules with the context
    ESP_ERROR_CHECK(js_rf_api_register(ctx));
    ESP_ERROR_CHECK(js_gpio_api_register(ctx));
    ESP_ERROR_CHECK(js_ui_api_register(ctx));
    ESP_ERROR_CHECK(js_storage_api_register(ctx));
    ESP_ERROR_CHECK(js_notification_api_register(ctx));
    
    ESP_LOGI(TAG, "All API functions registered");
    
    return ESP_OK;
}

// Utility functions for JavaScript type conversion
mjs_val_t js_make_error(struct mjs *mjs, const char *message)
{
    if (!mjs || !message) {
        return MJS_NULL;
    }
    
    // Create an error object (simplified)
    mjs_val_t error_obj = mjs_mk_string(mjs, message, -1);
    return error_obj;
}

mjs_val_t js_make_object(struct mjs *mjs)
{
    if (!mjs) {
        return MJS_NULL;
    }
    
    // Create a simple object (simplified)
    return mjs_mk_number(mjs, 0); // Placeholder
}

esp_err_t js_get_string_arg(struct mjs *mjs, int arg_index, char *buffer, size_t buffer_size)
{
    if (!mjs || !buffer || buffer_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Simplified argument retrieval
    // In a real implementation, this would get the argument from mJS stack
    snprintf(buffer, buffer_size, "arg_%d", arg_index);
    
    return ESP_OK;
}

esp_err_t js_get_number_arg(struct mjs *mjs, int arg_index, double *value)
{
    if (!mjs || !value) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Simplified argument retrieval
    *value = (double)arg_index;
    
    return ESP_OK;
}

esp_err_t js_get_bool_arg(struct mjs *mjs, int arg_index, bool *value)
{
    if (!mjs || !value) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Simplified argument retrieval
    *value = (arg_index % 2 == 0);
    
    return ESP_OK;
}