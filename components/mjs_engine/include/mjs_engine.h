/**
 * @file mjs_engine.h
 * @brief JavaScript Engine for T-Embed CC1101 Apps
 */

#ifndef MJS_ENGINE_H
#define MJS_ENGINE_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
struct mjs;
typedef struct mjs mjs_t;

// JavaScript execution context
typedef struct {
    mjs_t *mjs;
    char *filename;
    char *code;
    bool is_running;
    uint32_t memory_limit;
    uint32_t execution_time_limit_ms;
    void *user_data;
} js_context_t;

// JavaScript app manifest
typedef struct {
    char name[32];
    char version[16];
    char author[32];
    char description[128];
    char entry_point[64];
    char permissions[256];
    uint32_t memory_limit;
    bool has_icon;
} js_app_manifest_t;

// Execution result
typedef enum {
    JS_EXEC_OK,
    JS_EXEC_ERROR,
    JS_EXEC_TIMEOUT,
    JS_EXEC_OUT_OF_MEMORY,
    JS_EXEC_PERMISSION_DENIED
} js_exec_result_t;

// Callback types
typedef void (*js_log_callback_t)(const char *level, const char *message, void *user_data);
typedef void (*js_error_callback_t)(const char *error, const char *stack, void *user_data);

/**
 * @brief Initialize JavaScript engine
 * @return ESP_OK on success
 */
esp_err_t mjs_engine_init(void);

/**
 * @brief Deinitialize JavaScript engine
 * @return ESP_OK on success
 */
esp_err_t mjs_engine_deinit(void);

/**
 * @brief Create new JavaScript context
 * @param memory_limit Memory limit in bytes (0 = default)
 * @return Context pointer or NULL on failure
 */
js_context_t* mjs_engine_create_context(uint32_t memory_limit);

/**
 * @brief Destroy JavaScript context
 * @param ctx Context to destroy
 */
void mjs_engine_destroy_context(js_context_t *ctx);

/**
 * @brief Load JavaScript file
 * @param ctx JavaScript context
 * @param filename File path to load
 * @return ESP_OK on success
 */
esp_err_t mjs_engine_load_file(js_context_t *ctx, const char *filename);

/**
 * @brief Load JavaScript code from string
 * @param ctx JavaScript context
 * @param code JavaScript code string
 * @param filename Optional filename for debugging
 * @return ESP_OK on success
 */
esp_err_t mjs_engine_load_string(js_context_t *ctx, const char *code, const char *filename);

/**
 * @brief Execute loaded JavaScript
 * @param ctx JavaScript context
 * @return Execution result
 */
js_exec_result_t mjs_engine_execute(js_context_t *ctx);

/**
 * @brief Stop JavaScript execution
 * @param ctx JavaScript context
 * @return ESP_OK on success
 */
esp_err_t mjs_engine_stop(js_context_t *ctx);

/**
 * @brief Check if context is running
 * @param ctx JavaScript context
 * @return true if running
 */
bool mjs_engine_is_running(js_context_t *ctx);

/**
 * @brief Get engine statistics
 * @param total_memory Total allocated memory
 * @param free_memory Free memory
 * @param num_contexts Number of active contexts
 */
void mjs_engine_get_stats(uint32_t *total_memory, uint32_t *free_memory, uint8_t *num_contexts);

/**
 * @brief Set log callback
 * @param callback Log callback function
 * @param user_data User data for callback
 */
void mjs_engine_set_log_callback(js_log_callback_t callback, void *user_data);

/**
 * @brief Set error callback
 * @param callback Error callback function
 * @param user_data User data for callback
 */
void mjs_engine_set_error_callback(js_error_callback_t callback, void *user_data);

/**
 * @brief Load app manifest from file
 * @param manifest_path Path to manifest.json
 * @param manifest Manifest structure to fill
 * @return ESP_OK on success
 */
esp_err_t mjs_engine_load_manifest(const char *manifest_path, js_app_manifest_t *manifest);

/**
 * @brief Validate app permissions
 * @param ctx JavaScript context
 * @param permission Permission string to check
 * @return true if permission granted
 */
bool mjs_engine_check_permission(js_context_t *ctx, const char *permission);

/**
 * @brief Register native function
 * @param name Function name
 * @param func Native function pointer
 * @return ESP_OK on success
 */
esp_err_t mjs_engine_register_function(const char *name, void *func);

/**
 * @brief Register native object
 * @param name Object name
 * @param obj Object data
 * @return ESP_OK on success
 */
esp_err_t mjs_engine_register_object(const char *name, void *obj);

// Built-in JavaScript modules
esp_err_t mjs_module_rf_register(void);
esp_err_t mjs_module_gpio_register(void);
esp_err_t mjs_module_ui_register(void);
esp_err_t mjs_module_storage_register(void);
esp_err_t mjs_module_notification_register(void);
esp_err_t mjs_module_console_register(void);

// Console functions
void mjs_console_log(const char *level, const char *fmt, ...);
void mjs_console_print(const char *str);

#ifdef __cplusplus
}
#endif

#endif // MJS_ENGINE_H