/**
 * @file js_storage_api.c
 * @brief JavaScript Storage API Implementation
 */

#include "js_api.h"
#include "mjs.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "JS_STORAGE_API";
static const char *NVS_NAMESPACE = "js_apps";

/**
 * storage.writeText(filename, content)
 * Write text to file
 */
static mjs_val_t js_storage_write_text(struct mjs *mjs)
{
    char filename[128], content[512];
    
    if (js_get_string_arg(mjs, 0, filename, sizeof(filename)) != ESP_OK) {
        return js_make_error(mjs, "Invalid filename parameter");
    }
    if (js_get_string_arg(mjs, 1, content, sizeof(content)) != ESP_OK) {
        return js_make_error(mjs, "Invalid content parameter");
    }
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        return js_make_error(mjs, "Failed to open file for writing");
    }
    
    size_t written = fwrite(content, 1, strlen(content), file);
    fclose(file);
    
    if (written != strlen(content)) {
        return js_make_error(mjs, "Failed to write complete content");
    }
    
    ESP_LOGI(TAG, "Wrote %zu bytes to %s", strlen(content), filename);
    return MJS_UNDEFINED;
}

/**
 * storage.readText(filename)
 * Read text from file
 */
static mjs_val_t js_storage_read_text(struct mjs *mjs)
{
    char filename[128];
    
    if (js_get_string_arg(mjs, 0, filename, sizeof(filename)) != ESP_OK) {
        return js_make_error(mjs, "Invalid filename parameter");
    }
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        return js_make_error(mjs, "Failed to open file for reading");
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0 || file_size > 1024) {
        fclose(file);
        return js_make_error(mjs, "File too large or empty");
    }
    
    char *content = malloc(file_size + 1);
    if (!content) {
        fclose(file);
        return js_make_error(mjs, "Out of memory");
    }
    
    size_t read_size = fread(content, 1, file_size, file);
    fclose(file);
    
    if (read_size != file_size) {
        free(content);
        return js_make_error(mjs, "Failed to read complete file");
    }
    
    content[file_size] = '\0';
    mjs_val_t result = mjs_mk_string(mjs, content, file_size);
    
    free(content);
    ESP_LOGI(TAG, "Read %ld bytes from %s", file_size, filename);
    
    return result;
}

/**
 * storage.setConfig(key, value)
 * Save configuration value to NVS
 */
static mjs_val_t js_storage_set_config(struct mjs *mjs)
{
    char key[64], value[256];
    
    if (js_get_string_arg(mjs, 0, key, sizeof(key)) != ESP_OK) {
        return js_make_error(mjs, "Invalid key parameter");
    }
    if (js_get_string_arg(mjs, 1, value, sizeof(value)) != ESP_OK) {
        return js_make_error(mjs, "Invalid value parameter");
    }
    
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to open NVS");
    }
    
    ret = nvs_set_str(nvs_handle, key, value);
    if (ret != ESP_OK) {
        nvs_close(nvs_handle);
        return js_make_error(mjs, "Failed to save config");
    }
    
    ret = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to commit config");
    }
    
    ESP_LOGI(TAG, "Saved config: %s = %s", key, value);
    return MJS_UNDEFINED;
}

/**
 * storage.getConfig(key, default_value)
 * Load configuration value from NVS
 */
static mjs_val_t js_storage_get_config(struct mjs *mjs)
{
    char key[64], default_value[256], value[256];
    
    if (js_get_string_arg(mjs, 0, key, sizeof(key)) != ESP_OK) {
        return js_make_error(mjs, "Invalid key parameter");
    }
    if (js_get_string_arg(mjs, 1, default_value, sizeof(default_value)) != ESP_OK) {
        strcpy(default_value, ""); // Empty default
    }
    
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        return mjs_mk_string(mjs, default_value, -1);
    }
    
    size_t required_size = sizeof(value);
    ret = nvs_get_str(nvs_handle, key, value, &required_size);
    nvs_close(nvs_handle);
    
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "Config not found: %s, using default", key);
        return mjs_mk_string(mjs, default_value, -1);
    }
    
    ESP_LOGI(TAG, "Loaded config: %s = %s", key, value);
    return mjs_mk_string(mjs, value, -1);
}

/**
 * storage.deleteFile(filename)
 * Delete file
 */
static mjs_val_t js_storage_delete_file(struct mjs *mjs)
{
    char filename[128];
    
    if (js_get_string_arg(mjs, 0, filename, sizeof(filename)) != ESP_OK) {
        return js_make_error(mjs, "Invalid filename parameter");
    }
    
    int ret = remove(filename);
    if (ret != 0) {
        return js_make_error(mjs, "Failed to delete file");
    }
    
    ESP_LOGI(TAG, "Deleted file: %s", filename);
    return MJS_UNDEFINED;
}

esp_err_t js_storage_api_init(void)
{
    ESP_LOGI(TAG, "Initializing Storage API");
    return ESP_OK;
}

esp_err_t js_storage_api_register(js_context_t *ctx)
{
    if (!ctx || !ctx->mjs) {
        return ESP_ERR_INVALID_ARG;
    }
    
    struct mjs *mjs = ctx->mjs;
    
    mjs_set_ffi_func(mjs, "storage.writeText", js_storage_write_text);
    mjs_set_ffi_func(mjs, "storage.readText", js_storage_read_text);
    mjs_set_ffi_func(mjs, "storage.setConfig", js_storage_set_config);
    mjs_set_ffi_func(mjs, "storage.getConfig", js_storage_get_config);
    mjs_set_ffi_func(mjs, "storage.deleteFile", js_storage_delete_file);
    
    ESP_LOGI(TAG, "Storage API functions registered");
    return ESP_OK;
}