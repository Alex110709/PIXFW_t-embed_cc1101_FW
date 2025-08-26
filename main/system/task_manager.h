/**
 * @file task_manager.h
 * @brief Task Manager for T-Embed CC1101 Firmware
 */

#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

// Task priorities
#define TASK_PRIORITY_CRITICAL  25
#define TASK_PRIORITY_HIGH      20
#define TASK_PRIORITY_NORMAL    15
#define TASK_PRIORITY_LOW       10
#define TASK_PRIORITY_IDLE      5

// Task stack sizes
#define TASK_STACK_SIZE_LARGE   8192
#define TASK_STACK_SIZE_MEDIUM  4096
#define TASK_STACK_SIZE_SMALL   2048

typedef enum {
    TASK_ID_UI,
    TASK_ID_RF_SERVICE,
    TASK_ID_JS_ENGINE,
    TASK_ID_NETWORK,
    TASK_ID_APP_MANAGER,
    TASK_ID_INPUT_HANDLER,
    TASK_ID_MAX
} task_id_t;

typedef struct {
    const char *name;
    TaskFunction_t function;
    uint32_t stack_size;
    UBaseType_t priority;
    TaskHandle_t handle;
    bool is_running;
} task_info_t;

/**
 * @brief Initialize and start all system tasks
 * @return ESP_OK on success
 */
esp_err_t task_manager_start(void);

/**
 * @brief Stop all tasks
 * @return ESP_OK on success
 */
esp_err_t task_manager_stop(void);

/**
 * @brief Get task handle by ID
 * @param task_id Task ID
 * @return Task handle or NULL if not found
 */
TaskHandle_t task_manager_get_handle(task_id_t task_id);

/**
 * @brief Check if task is running
 * @param task_id Task ID
 * @return true if running, false otherwise
 */
bool task_manager_is_running(task_id_t task_id);

/**
 * @brief Get task statistics
 * @param stats Array to store task stats (must be at least TASK_ID_MAX size)
 * @return ESP_OK on success
 */
esp_err_t task_manager_get_stats(TaskStatus_t *stats);

// Task functions (implemented in separate files)
void ui_task(void *pvParameters);
void rf_service_task(void *pvParameters);
void js_engine_task(void *pvParameters);
void network_task(void *pvParameters);
void app_manager_task(void *pvParameters);
void input_handler_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // TASK_MANAGER_H