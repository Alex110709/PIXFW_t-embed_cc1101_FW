/**
 * @file app_sandbox.c
 * @brief App Sandbox Implementation
 */

#include "app_manager.h"
#include "js_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "APP_SANDBOX";

#define MAX_SANDBOXES 8

typedef struct {
    char app_id[32];
    js_context_t *js_context;
    uint32_t memory_limit;
    uint32_t time_limit;
    uint32_t start_time;
    bool active;
} sandbox_t;

static sandbox_t s_sandboxes[MAX_SANDBOXES];
static int s_sandbox_count = 0;

static sandbox_t* find_sandbox(const char *app_id)
{
    for (int i = 0; i < MAX_SANDBOXES; i++) {
        if (s_sandboxes[i].active && strcmp(s_sandboxes[i].app_id, app_id) == 0) {
            return &s_sandboxes[i];
        }
    }
    return NULL;
}

esp_err_t app_sandbox_create(const char *app_id, js_context_t **context)
{
    if (!app_id || !context) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Creating sandbox for app: %s", app_id);
    
    // Find free sandbox slot
    int slot = -1;
    for (int i = 0; i < MAX_SANDBOXES; i++) {
        if (!s_sandboxes[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        ESP_LOGE(TAG, "No free sandbox slots");
        return ESP_ERR_NO_MEM;
    }
    
    // Create JavaScript context with memory limit
    js_context_t *js_ctx = mjs_engine_create_context(65536); // 64KB limit
    if (!js_ctx) {
        ESP_LOGE(TAG, "Failed to create JavaScript context");
        return ESP_ERR_NO_MEM;
    }
    
    // Register API functions with permission checking
    esp_err_t ret = js_api_register_all(js_ctx);
    if (ret != ESP_OK) {
        mjs_engine_destroy_context(js_ctx);
        return ret;
    }
    
    // Initialize sandbox
    sandbox_t *sandbox = &s_sandboxes[slot];
    strcpy(sandbox->app_id, app_id);
    sandbox->js_context = js_ctx;
    sandbox->memory_limit = 65536;
    sandbox->time_limit = 5000; // 5 seconds
    sandbox->start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    sandbox->active = true;
    
    *context = js_ctx;
    s_sandbox_count++;
    
    ESP_LOGI(TAG, "Sandbox created for app: %s", app_id);
    return ESP_OK;
}

esp_err_t app_sandbox_destroy(const char *app_id)
{
    if (!app_id) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Destroying sandbox for app: %s", app_id);
    
    sandbox_t *sandbox = find_sandbox(app_id);
    if (!sandbox) {
        ESP_LOGW(TAG, "Sandbox not found for app: %s", app_id);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Destroy JavaScript context
    if (sandbox->js_context) {
        mjs_engine_destroy_context(sandbox->js_context);
    }
    
    // Clear sandbox
    memset(sandbox, 0, sizeof(sandbox_t));
    s_sandbox_count--;
    
    ESP_LOGI(TAG, "Sandbox destroyed for app: %s", app_id);
    return ESP_OK;
}

esp_err_t app_sandbox_set_limits(const char *app_id, uint32_t memory_limit, uint32_t time_limit)
{
    if (!app_id) {
        return ESP_ERR_INVALID_ARG;
    }
    
    sandbox_t *sandbox = find_sandbox(app_id);
    if (!sandbox) {
        return ESP_ERR_NOT_FOUND;
    }
    
    sandbox->memory_limit = memory_limit;
    sandbox->time_limit = time_limit;
    
    ESP_LOGI(TAG, "Updated limits for app %s: memory=%u, time=%u", 
             app_id, memory_limit, time_limit);
    
    return ESP_OK;
}

bool app_sandbox_check_access(const char *app_id, const char *resource)
{
    if (!app_id || !resource) {
        return false;
    }
    
    sandbox_t *sandbox = find_sandbox(app_id);
    if (!sandbox) {
        return false;
    }
    
    // Check execution time limit
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (current_time - sandbox->start_time > sandbox->time_limit) {
        ESP_LOGW(TAG, "App %s exceeded time limit", app_id);
        return false;
    }
    
    // Check resource access permissions
    // This is a simplified implementation
    if (strstr(resource, "/system/") && !app_manager_check_permission(app_id, APP_PERM_SYSTEM)) {
        ESP_LOGW(TAG, "App %s denied access to system resource: %s", app_id, resource);
        return false;
    }
    
    if (strstr(resource, "rf.") && !app_manager_check_permission(app_id, APP_PERM_RF_RECEIVE | APP_PERM_RF_TRANSMIT)) {
        ESP_LOGW(TAG, "App %s denied access to RF resource: %s", app_id, resource);
        return false;
    }
    
    return true;
}