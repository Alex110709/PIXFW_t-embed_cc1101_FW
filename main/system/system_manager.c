/**
 * @file system_manager.c
 * @brief System Manager Implementation
 */

#include "system_manager.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "SYS_MGR";

static EventGroupHandle_t s_system_event_group = NULL;
static system_status_t s_system_status = {0};
static system_event_callback_t s_event_callback = NULL;
static void *s_callback_user_data = NULL;
static uint32_t s_boot_time = 0;

esp_err_t system_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing system manager");

    // Create event group
    s_system_event_group = xEventGroupCreate();
    if (!s_system_event_group) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }

    // Initialize system status
    s_system_status.state = SYSTEM_STATE_INITIALIZING;
    s_system_status.uptime_seconds = 0;
    s_system_status.wifi_connected = false;
    s_system_status.js_engine_running = false;
    
    s_boot_time = esp_log_timestamp();

    ESP_LOGI(TAG, "System manager initialized");
    return ESP_OK;
}

EventGroupHandle_t system_manager_get_event_group(void)
{
    return s_system_event_group;
}

esp_err_t system_manager_get_status(system_status_t *status)
{
    if (!status) {
        return ESP_ERR_INVALID_ARG;
    }

    // Update dynamic status
    s_system_status.uptime_seconds = (esp_log_timestamp() - s_boot_time) / 1000;
    s_system_status.free_heap = esp_get_free_heap_size();
    s_system_status.min_free_heap = esp_get_minimum_free_heap_size();

    memcpy(status, &s_system_status, sizeof(system_status_t));
    return ESP_OK;
}

esp_err_t system_manager_set_state(system_state_t state)
{
    system_state_t old_state = s_system_status.state;
    s_system_status.state = state;

    ESP_LOGI(TAG, "System state changed: %d -> %d", old_state, state);

    // Notify callback if registered
    if (s_event_callback) {
        s_event_callback(state, s_callback_user_data);
    }

    return ESP_OK;
}

void system_manager_heartbeat(void)
{
    // Update status
    s_system_status.uptime_seconds = (esp_log_timestamp() - s_boot_time) / 1000;
    s_system_status.free_heap = esp_get_free_heap_size();
    
    // Check if heap is getting low
    if (s_system_status.free_heap < 10240) { // Less than 10KB
        ESP_LOGW(TAG, "Low memory warning: %d bytes free", s_system_status.free_heap);
    }

    // Update system state based on event group
    EventBits_t bits = xEventGroupGetBits(s_system_event_group);
    
    s_system_status.wifi_connected = (bits & SYSTEM_WIFI_CONNECTED_BIT) != 0;
    s_system_status.js_engine_running = (bits & SYSTEM_JS_ENGINE_READY_BIT) != 0;

    // Set ready state if all subsystems are initialized
    if ((bits & (SYSTEM_JS_ENGINE_READY_BIT | SYSTEM_UI_READY_BIT | SYSTEM_RF_READY_BIT)) == 
        (SYSTEM_JS_ENGINE_READY_BIT | SYSTEM_UI_READY_BIT | SYSTEM_RF_READY_BIT)) {
        
        if (s_system_status.state == SYSTEM_STATE_INITIALIZING) {
            system_manager_set_state(SYSTEM_STATE_READY);
        }
    }
}

esp_err_t system_manager_register_callback(system_event_callback_t callback, void *user_data)
{
    s_event_callback = callback;
    s_callback_user_data = user_data;
    return ESP_OK;
}