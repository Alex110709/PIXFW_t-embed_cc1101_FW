/**
 * @file system_manager.h
 * @brief System Manager for T-Embed CC1101 Firmware
 */

#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

// System events
#define SYSTEM_WIFI_CONNECTED_BIT   BIT0
#define SYSTEM_JS_ENGINE_READY_BIT  BIT1
#define SYSTEM_UI_READY_BIT         BIT2
#define SYSTEM_RF_READY_BIT         BIT3

typedef enum {
    SYSTEM_STATE_BOOTING,
    SYSTEM_STATE_INITIALIZING,
    SYSTEM_STATE_READY,
    SYSTEM_STATE_RUNNING_APP,
    SYSTEM_STATE_ERROR
} system_state_t;

typedef struct {
    system_state_t state;
    uint32_t uptime_seconds;
    uint32_t free_heap;
    uint32_t min_free_heap;
    uint8_t cpu_usage;
    bool wifi_connected;
    bool js_engine_running;
} system_status_t;

/**
 * @brief Initialize system manager
 * @return ESP_OK on success
 */
esp_err_t system_manager_init(void);

/**
 * @brief Get system event group
 * @return Event group handle
 */
EventGroupHandle_t system_manager_get_event_group(void);

/**
 * @brief Get current system status
 * @param status Pointer to status structure
 * @return ESP_OK on success
 */
esp_err_t system_manager_get_status(system_status_t *status);

/**
 * @brief Set system state
 * @param state New system state
 * @return ESP_OK on success
 */
esp_err_t system_manager_set_state(system_state_t state);

/**
 * @brief System heartbeat (called periodically)
 */
void system_manager_heartbeat(void);

/**
 * @brief Register system event callback
 * @param callback Callback function
 * @return ESP_OK on success
 */
typedef void (*system_event_callback_t)(system_state_t state, void *user_data);
esp_err_t system_manager_register_callback(system_event_callback_t callback, void *user_data);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_MANAGER_H