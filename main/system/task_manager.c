/**
 * @file task_manager.c
 * @brief Task Manager Implementation
 */

#include "task_manager.h"
#include "system_manager.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "TASK_MGR";

static task_info_t s_tasks[TASK_ID_MAX] = {
    [TASK_ID_UI] = {
        .name = "ui_task",
        .function = ui_task,
        .stack_size = TASK_STACK_SIZE_LARGE,
        .priority = TASK_PRIORITY_HIGH,
        .handle = NULL,
        .is_running = false
    },
    [TASK_ID_RF_SERVICE] = {
        .name = "rf_service_task", 
        .function = rf_service_task,
        .stack_size = TASK_STACK_SIZE_MEDIUM,
        .priority = TASK_PRIORITY_HIGH,
        .handle = NULL,
        .is_running = false
    },
    [TASK_ID_JS_ENGINE] = {
        .name = "js_engine_task",
        .function = js_engine_task,
        .stack_size = TASK_STACK_SIZE_LARGE,
        .priority = TASK_PRIORITY_NORMAL,
        .handle = NULL,
        .is_running = false
    },
    [TASK_ID_NETWORK] = {
        .name = "network_task",
        .function = network_task,
        .stack_size = TASK_STACK_SIZE_MEDIUM,
        .priority = TASK_PRIORITY_NORMAL,
        .handle = NULL,
        .is_running = false
    },
    [TASK_ID_APP_MANAGER] = {
        .name = "app_manager_task",
        .function = app_manager_task,
        .stack_size = TASK_STACK_SIZE_MEDIUM,
        .priority = TASK_PRIORITY_NORMAL,
        .handle = NULL,
        .is_running = false
    },
    [TASK_ID_INPUT_HANDLER] = {
        .name = "input_handler_task",
        .function = input_handler_task,
        .stack_size = TASK_STACK_SIZE_SMALL,
        .priority = TASK_PRIORITY_HIGH,
        .handle = NULL,
        .is_running = false
    }
};

esp_err_t task_manager_start(void)
{
    ESP_LOGI(TAG, "Starting system tasks");

    for (int i = 0; i < TASK_ID_MAX; i++) {
        task_info_t *task = &s_tasks[i];
        
        BaseType_t ret = xTaskCreate(
            task->function,
            task->name,
            task->stack_size,
            NULL,
            task->priority,
            &task->handle
        );

        if (ret != pdPASS) {
            ESP_LOGE(TAG, "Failed to create task %s", task->name);
            return ESP_ERR_NO_MEM;
        }

        task->is_running = true;
        ESP_LOGI(TAG, "Started task: %s", task->name);
    }

    ESP_LOGI(TAG, "All system tasks started successfully");
    return ESP_OK;
}

esp_err_t task_manager_stop(void)
{
    ESP_LOGI(TAG, "Stopping system tasks");

    for (int i = 0; i < TASK_ID_MAX; i++) {
        task_info_t *task = &s_tasks[i];
        
        if (task->handle && task->is_running) {
            vTaskDelete(task->handle);
            task->handle = NULL;
            task->is_running = false;
            ESP_LOGI(TAG, "Stopped task: %s", task->name);
        }
    }

    return ESP_OK;
}

TaskHandle_t task_manager_get_handle(task_id_t task_id)
{
    if (task_id >= TASK_ID_MAX) {
        return NULL;
    }
    return s_tasks[task_id].handle;
}

bool task_manager_is_running(task_id_t task_id)
{
    if (task_id >= TASK_ID_MAX) {
        return false;
    }
    return s_tasks[task_id].is_running;
}

esp_err_t task_manager_get_stats(TaskStatus_t *stats)
{
    if (!stats) {
        return ESP_ERR_INVALID_ARG;
    }

    UBaseType_t task_count = uxTaskGetNumberOfTasks();
    uxTaskGetSystemState(stats, task_count, NULL);

    return ESP_OK;
}

// Placeholder task implementations
void ui_task(void *pvParameters)
{
    ESP_LOGI(TAG, "UI task started");
    
    // Signal that UI is ready
    EventGroupHandle_t event_group = system_manager_get_event_group();
    if (event_group) {
        xEventGroupSetBits(event_group, SYSTEM_UI_READY_BIT);
    }

    while (1) {
        // TODO: Implement UI handling
        vTaskDelay(pdMS_TO_TICKS(50)); // 20 FPS
    }
}

void rf_service_task(void *pvParameters)
{
    ESP_LOGI(TAG, "RF service task started");
    
    // Signal that RF service is ready
    EventGroupHandle_t event_group = system_manager_get_event_group();
    if (event_group) {
        xEventGroupSetBits(event_group, SYSTEM_RF_READY_BIT);
    }

    while (1) {
        // TODO: Implement RF service
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void js_engine_task(void *pvParameters)
{
    ESP_LOGI(TAG, "JavaScript engine task started");
    
    // Signal that JS engine is ready
    EventGroupHandle_t event_group = system_manager_get_event_group();
    if (event_group) {
        xEventGroupSetBits(event_group, SYSTEM_JS_ENGINE_READY_BIT);
    }

    while (1) {
        // TODO: Implement JavaScript engine
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void network_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Network task started");

    while (1) {
        // TODO: Implement networking
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_manager_task(void *pvParameters)
{
    ESP_LOGI(TAG, "App manager task started");

    while (1) {
        // TODO: Implement app manager
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void input_handler_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Input handler task started");

    while (1) {
        // TODO: Implement input handling
        vTaskDelay(pdMS_TO_TICKS(20)); // 50 Hz
    }
}