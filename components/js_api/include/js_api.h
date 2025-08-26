/**
 * @file js_api.h
 * @brief JavaScript API Interface
 */

#ifndef JS_API_H
#define JS_API_H

#include "esp_err.h"
#include "mjs_engine.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize all JavaScript API modules
 * @return ESP_OK on success
 */
esp_err_t js_api_init(void);

/**
 * @brief Deinitialize all JavaScript API modules
 * @return ESP_OK on success
 */
esp_err_t js_api_deinit(void);

/**
 * @brief Register all API functions with mJS context
 * @param ctx JavaScript context
 * @return ESP_OK on success
 */
esp_err_t js_api_register_all(js_context_t *ctx);

// Module initialization functions
esp_err_t js_rf_api_init(void);
esp_err_t js_gpio_api_init(void);
esp_err_t js_ui_api_init(void);
esp_err_t js_storage_api_init(void);
esp_err_t js_notification_api_init(void);

// Module registration functions
esp_err_t js_rf_api_register(js_context_t *ctx);
esp_err_t js_gpio_api_register(js_context_t *ctx);
esp_err_t js_ui_api_register(js_context_t *ctx);
esp_err_t js_storage_api_register(js_context_t *ctx);
esp_err_t js_notification_api_register(js_context_t *ctx);

// Utility functions for type conversion
mjs_val_t js_make_error(struct mjs *mjs, const char *message);
mjs_val_t js_make_object(struct mjs *mjs);
esp_err_t js_get_string_arg(struct mjs *mjs, int arg_index, char *buffer, size_t buffer_size);
esp_err_t js_get_number_arg(struct mjs *mjs, int arg_index, double *value);
esp_err_t js_get_bool_arg(struct mjs *mjs, int arg_index, bool *value);

#ifdef __cplusplus
}
#endif

#endif // JS_API_H