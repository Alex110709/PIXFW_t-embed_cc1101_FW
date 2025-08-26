/**
 * @file mjs_native_api.c
 * @brief Native API Bridge for JavaScript Engine
 */

#include "mjs_engine.h"
#include "mjs.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "MJS_API";

// Global engine reference for native functions
static struct {
    mjs_func_ptr_t functions[32];
    char names[32][32];
    int count;
} s_native_functions = {0};

/**
 * @brief Register native function helper
 */
static esp_err_t register_native_function(const char *name, mjs_func_ptr_t func)
{
    if (s_native_functions.count >= 32) {
        ESP_LOGE(TAG, "Too many native functions registered");
        return ESP_ERR_NO_MEM;
    }
    
    strncpy(s_native_functions.names[s_native_functions.count], name, 31);
    s_native_functions.names[s_native_functions.count][31] = '\0';
    s_native_functions.functions[s_native_functions.count] = func;
    s_native_functions.count++;
    
    ESP_LOGI(TAG, "Registered native function: %s", name);
    return ESP_OK;
}

/**
 * @brief Native console.log implementation
 */
static mjs_val_t native_console_log(struct mjs *mjs)
{
    // Get arguments (simplified - just print them)
    printf("JS Console: Hello from native function\n");
    return MJS_UNDEFINED;
}

/**
 * @brief Native setTimeout implementation
 */
static mjs_val_t native_set_timeout(struct mjs *mjs)
{
    // Simplified setTimeout - just return a timer ID
    static uint32_t timer_id = 1;
    return mjs_mk_number(mjs, timer_id++);
}

/**
 * @brief Native clearTimeout implementation
 */
static mjs_val_t native_clear_timeout(struct mjs *mjs)
{
    // Simplified clearTimeout
    return MJS_UNDEFINED;
}

/**
 * @brief Native print implementation
 */
static mjs_val_t native_print(struct mjs *mjs)
{
    printf("JS Print: Called native print function\n");
    return MJS_UNDEFINED;
}

esp_err_t mjs_engine_register_function(const char *name, void *func)
{
    if (!name || !func) {
        return ESP_ERR_INVALID_ARG;
    }
    
    return register_native_function(name, (mjs_func_ptr_t)func);
}

esp_err_t mjs_engine_register_object(const char *name, void *obj)
{
    if (!name || !obj) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Simplified object registration
    ESP_LOGI(TAG, "Registered native object: %s", name);
    return ESP_OK;
}

/**
 * @brief Register all native functions with mJS instance
 */
void mjs_register_native_functions(struct mjs *mjs)
{
    if (!mjs) return;
    
    // Register built-in functions
    mjs_set_ffi_func(mjs, "console.log", native_console_log);
    mjs_set_ffi_func(mjs, "setTimeout", native_set_timeout);
    mjs_set_ffi_func(mjs, "clearTimeout", native_clear_timeout);
    mjs_set_ffi_func(mjs, "print", native_print);
    
    // Register user-defined functions
    for (int i = 0; i < s_native_functions.count; i++) {
        mjs_set_ffi_func(mjs, s_native_functions.names[i], s_native_functions.functions[i]);
    }
    
    ESP_LOGI(TAG, "Registered %d native functions", s_native_functions.count + 4);
}

// Manifest loading implementation
esp_err_t mjs_engine_load_manifest(const char *manifest_path, js_app_manifest_t *manifest)
{
    if (!manifest_path || !manifest) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Loading app manifest: %s", manifest_path);
    
    FILE *file = fopen(manifest_path, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open manifest file: %s", manifest_path);
        return ESP_ERR_NOT_FOUND;
    }
    
    char line[256];
    memset(manifest, 0, sizeof(js_app_manifest_t));
    
    // Simple JSON-like parsing (very basic)
    while (fgets(line, sizeof(line), file)) {
        char *key_start = strstr(line, "\"");
        if (!key_start) continue;
        
        key_start++; // Skip opening quote
        char *key_end = strstr(key_start, "\"");
        if (!key_end) continue;
        
        *key_end = '\0';
        char *value_start = strstr(key_end + 1, ":");
        if (!value_start) continue;
        
        value_start++;
        while (*value_start == ' ' || *value_start == '\t') value_start++;
        
        if (*value_start == '\"') {
            // String value
            value_start++;
            char *value_end = strstr(value_start, "\"");
            if (value_end) {
                *value_end = '\0';
                
                if (strcmp(key_start, "name") == 0) {
                    strncpy(manifest->name, value_start, sizeof(manifest->name) - 1);
                } else if (strcmp(key_start, "version") == 0) {
                    strncpy(manifest->version, value_start, sizeof(manifest->version) - 1);
                } else if (strcmp(key_start, "author") == 0) {
                    strncpy(manifest->author, value_start, sizeof(manifest->author) - 1);
                } else if (strcmp(key_start, "description") == 0) {
                    strncpy(manifest->description, value_start, sizeof(manifest->description) - 1);
                } else if (strcmp(key_start, "entry_point") == 0) {
                    strncpy(manifest->entry_point, value_start, sizeof(manifest->entry_point) - 1);
                } else if (strcmp(key_start, "permissions") == 0) {
                    strncpy(manifest->permissions, value_start, sizeof(manifest->permissions) - 1);
                }
            }
        } else if (isdigit(*value_start)) {
            // Numeric value
            int num_value = atoi(value_start);
            if (strcmp(key_start, "memory_limit") == 0) {
                manifest->memory_limit = num_value;
            }
        } else if (strncmp(value_start, "true", 4) == 0) {
            // Boolean true
            if (strcmp(key_start, "has_icon") == 0) {
                manifest->has_icon = true;
            }
        }
    }
    
    fclose(file);
    
    // Set defaults if not specified
    if (strlen(manifest->name) == 0) {
        strcpy(manifest->name, "Unknown App");
    }
    if (strlen(manifest->version) == 0) {
        strcpy(manifest->version, "1.0.0");
    }
    if (manifest->memory_limit == 0) {
        manifest->memory_limit = DEFAULT_MEMORY_LIMIT;
    }
    
    ESP_LOGI(TAG, "Loaded manifest: %s v%s by %s", 
             manifest->name, manifest->version, manifest->author);
    
    return ESP_OK;
}

// Permission checking
bool mjs_engine_check_permission(js_context_t *ctx, const char *permission)
{
    if (!ctx || !permission) {
        return false;
    }
    
    // For now, allow all permissions (simplified)
    // In a real implementation, this would check against the app's manifest permissions
    ESP_LOGD(TAG, "Permission requested: %s (granted)", permission);
    return true;
}