/**
 * @file mjs_engine.c
 * @brief JavaScript Engine Implementation
 */

#include "mjs_engine.h"
#include "mjs.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *TAG = "MJS_ENGINE";

// Engine state
static bool s_initialized = false;
static SemaphoreHandle_t s_engine_mutex = NULL;
static js_context_t *s_contexts[8] = {0}; // Max 8 concurrent contexts
static uint8_t s_context_count = 0;

// Callbacks
static js_log_callback_t s_log_callback = NULL;
static js_error_callback_t s_error_callback = NULL;
static void *s_log_user_data = NULL;
static void *s_error_user_data = NULL;

// Default memory limits
#define DEFAULT_MEMORY_LIMIT 65536  // 64KB
#define DEFAULT_EXEC_TIME_LIMIT 5000 // 5 seconds

// MJS error handler
static void mjs_error_handler(struct mjs *mjs, const char *msg, void *user_data)
{
    js_context_t *ctx = (js_context_t *)user_data;
    
    ESP_LOGE(TAG, "JavaScript error in %s: %s", 
             ctx->filename ? ctx->filename : "unknown", msg);
    
    if (s_error_callback) {
        s_error_callback(msg, NULL, s_error_user_data);
    }
    
    ctx->is_running = false;
}

esp_err_t mjs_engine_init(void)
{
    if (s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Initializing JavaScript engine");
    
    // Create mutex for thread safety
    s_engine_mutex = xSemaphoreCreateMutex();
    if (!s_engine_mutex) {
        ESP_LOGE(TAG, "Failed to create engine mutex");
        return ESP_ERR_NO_MEM;
    }
    
    // Register built-in modules
    ESP_ERROR_CHECK(mjs_module_console_register());
    ESP_ERROR_CHECK(mjs_module_rf_register());
    ESP_ERROR_CHECK(mjs_module_gpio_register());
    ESP_ERROR_CHECK(mjs_module_ui_register());
    ESP_ERROR_CHECK(mjs_module_storage_register());
    ESP_ERROR_CHECK(mjs_module_notification_register());
    
    s_initialized = true;
    ESP_LOGI(TAG, "JavaScript engine initialized");
    
    return ESP_OK;
}

esp_err_t mjs_engine_deinit(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Deinitializing JavaScript engine");
    
    // Destroy all contexts
    xSemaphoreTake(s_engine_mutex, portMAX_DELAY);
    
    for (int i = 0; i < 8; i++) {
        if (s_contexts[i]) {
            mjs_engine_destroy_context(s_contexts[i]);
            s_contexts[i] = NULL;
        }
    }
    s_context_count = 0;
    
    xSemaphoreGive(s_engine_mutex);
    
    // Delete mutex
    vSemaphoreDelete(s_engine_mutex);
    s_engine_mutex = NULL;
    
    s_initialized = false;
    ESP_LOGI(TAG, "JavaScript engine deinitialized");
    
    return ESP_OK;
}

js_context_t* mjs_engine_create_context(uint32_t memory_limit)
{
    if (!s_initialized) {
        return NULL;
    }
    
    xSemaphoreTake(s_engine_mutex, portMAX_DELAY);
    
    // Check if we have space for new context
    if (s_context_count >= 8) {
        ESP_LOGE(TAG, "Maximum number of contexts reached");
        xSemaphoreGive(s_engine_mutex);
        return NULL;
    }
    
    // Find free slot
    int slot = -1;
    for (int i = 0; i < 8; i++) {
        if (!s_contexts[i]) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        ESP_LOGE(TAG, "No free context slots");
        xSemaphoreGive(s_engine_mutex);
        return NULL;
    }
    
    // Allocate context
    js_context_t *ctx = calloc(1, sizeof(js_context_t));
    if (!ctx) {
        ESP_LOGE(TAG, "Failed to allocate context");
        xSemaphoreGive(s_engine_mutex);
        return NULL;
    }
    
    // Create mJS instance
    ctx->mjs = mjs_create();
    if (!ctx->mjs) {
        ESP_LOGE(TAG, "Failed to create mJS instance");
        free(ctx);
        xSemaphoreGive(s_engine_mutex);
        return NULL;
    }
    
    // Set memory limit
    ctx->memory_limit = memory_limit > 0 ? memory_limit : DEFAULT_MEMORY_LIMIT;
    ctx->execution_time_limit_ms = DEFAULT_EXEC_TIME_LIMIT;
    ctx->is_running = false;
    
    // Set error handler
    mjs_set_error_handler(ctx->mjs, mjs_error_handler, ctx);
    
    // Register context
    s_contexts[slot] = ctx;
    s_context_count++;
    
    xSemaphoreGive(s_engine_mutex);
    
    ESP_LOGI(TAG, "Created JavaScript context (slot %d)", slot);
    return ctx;
}

void mjs_engine_destroy_context(js_context_t *ctx)
{
    if (!ctx || !s_initialized) {
        return;
    }
    
    xSemaphoreTake(s_engine_mutex, portMAX_DELAY);
    
    // Stop execution if running
    if (ctx->is_running) {
        ctx->is_running = false;
    }
    
    // Cleanup mJS instance
    if (ctx->mjs) {
        mjs_destroy(ctx->mjs);
    }
    
    // Free allocated memory
    if (ctx->filename) {
        free(ctx->filename);
    }
    if (ctx->code) {
        free(ctx->code);
    }
    
    // Remove from contexts array
    for (int i = 0; i < 8; i++) {
        if (s_contexts[i] == ctx) {
            s_contexts[i] = NULL;
            s_context_count--;
            break;
        }
    }
    
    free(ctx);
    
    xSemaphoreGive(s_engine_mutex);
    
    ESP_LOGI(TAG, "Destroyed JavaScript context");
}

esp_err_t mjs_engine_load_file(js_context_t *ctx, const char *filename)
{
    if (!ctx || !filename) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Loading JavaScript file: %s", filename);
    
    // Open file
    FILE *file = fopen(filename, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file: %s", filename);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0 || file_size > ctx->memory_limit / 2) {
        ESP_LOGE(TAG, "File too large or invalid: %ld bytes", file_size);
        fclose(file);
        return ESP_ERR_INVALID_SIZE;
    }
    
    // Allocate buffer
    char *code = malloc(file_size + 1);
    if (!code) {
        ESP_LOGE(TAG, "Failed to allocate memory for file");
        fclose(file);
        return ESP_ERR_NO_MEM;
    }
    
    // Read file
    size_t bytes_read = fread(code, 1, file_size, file);
    fclose(file);
    
    if (bytes_read != file_size) {
        ESP_LOGE(TAG, "Failed to read complete file");
        free(code);
        return ESP_ERR_INVALID_SIZE;
    }
    
    code[file_size] = '\0';
    
    // Store in context
    if (ctx->filename) {
        free(ctx->filename);
    }
    if (ctx->code) {
        free(ctx->code);
    }
    
    ctx->filename = strdup(filename);
    ctx->code = code;
    
    ESP_LOGI(TAG, "Loaded JavaScript file: %s (%ld bytes)", filename, file_size);
    return ESP_OK;
}

esp_err_t mjs_engine_load_string(js_context_t *ctx, const char *code, const char *filename)
{
    if (!ctx || !code) {
        return ESP_ERR_INVALID_ARG;
    }
    
    size_t code_len = strlen(code);
    if (code_len > ctx->memory_limit / 2) {
        ESP_LOGE(TAG, "Code too large: %zu bytes", code_len);
        return ESP_ERR_INVALID_SIZE;
    }
    
    // Store in context
    if (ctx->filename) {
        free(ctx->filename);
    }
    if (ctx->code) {
        free(ctx->code);
    }
    
    ctx->filename = filename ? strdup(filename) : strdup("string");
    ctx->code = strdup(code);
    
    ESP_LOGI(TAG, "Loaded JavaScript code (%zu bytes)", code_len);
    return ESP_OK;
}

js_exec_result_t mjs_engine_execute(js_context_t *ctx)
{
    if (!ctx || !ctx->code || !ctx->mjs) {
        return JS_EXEC_ERROR;
    }
    
    ESP_LOGI(TAG, "Executing JavaScript: %s", ctx->filename);
    
    ctx->is_running = true;
    uint32_t start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Execute JavaScript code
    mjs_val_t result = mjs_exec(ctx->mjs, ctx->code, NULL);
    
    uint32_t exec_time = (xTaskGetTickCount() * portTICK_PERIOD_MS) - start_time;
    ctx->is_running = false;
    
    // Check execution result
    if (mjs_is_error(result)) {
        const char *error_msg = mjs_get_error_message(ctx->mjs);
        ESP_LOGE(TAG, "JavaScript execution error: %s", error_msg);
        
        if (s_error_callback) {
            s_error_callback(error_msg, NULL, s_error_user_data);
        }
        
        return JS_EXEC_ERROR;
    }
    
    // Check for timeout
    if (exec_time > ctx->execution_time_limit_ms) {
        ESP_LOGW(TAG, "JavaScript execution timeout: %u ms", exec_time);
        return JS_EXEC_TIMEOUT;
    }
    
    ESP_LOGI(TAG, "JavaScript execution completed in %u ms", exec_time);
    return JS_EXEC_OK;
}

esp_err_t mjs_engine_stop(js_context_t *ctx)
{
    if (!ctx) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ctx->is_running = false;
    ESP_LOGI(TAG, "Stopped JavaScript execution");
    
    return ESP_OK;
}

bool mjs_engine_is_running(js_context_t *ctx)
{
    return ctx ? ctx->is_running : false;
}

void mjs_engine_get_stats(uint32_t *total_memory, uint32_t *free_memory, uint8_t *num_contexts)
{
    if (!s_initialized) {
        if (total_memory) *total_memory = 0;
        if (free_memory) *free_memory = 0;
        if (num_contexts) *num_contexts = 0;
        return;
    }
    
    xSemaphoreTake(s_engine_mutex, portMAX_DELAY);
    
    uint32_t total_used = 0;
    for (int i = 0; i < 8; i++) {
        if (s_contexts[i] && s_contexts[i]->mjs) {
            // Get memory usage from mJS (simplified)
            total_used += 4096; // Approximate per context
        }
    }
    
    if (total_memory) *total_memory = total_used;
    if (free_memory) *free_memory = esp_get_free_heap_size();
    if (num_contexts) *num_contexts = s_context_count;
    
    xSemaphoreGive(s_engine_mutex);
}

void mjs_engine_set_log_callback(js_log_callback_t callback, void *user_data)
{
    s_log_callback = callback;
    s_log_user_data = user_data;
}

void mjs_engine_set_error_callback(js_error_callback_t callback, void *user_data)
{
    s_error_callback = callback;
    s_error_user_data = user_data;
}

bool mjs_engine_is_running(js_context_t *ctx)
{
    return ctx ? ctx->is_running : false;
}