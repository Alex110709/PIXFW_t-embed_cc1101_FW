/**
 * @file app_manager.h
 * @brief JavaScript App Manager and Sandbox System
 */

#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include "esp_err.h"
#include "mjs_engine.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_INSTALLED_APPS 16
#define MAX_APP_NAME_LEN 32
#define MAX_APP_PATH_LEN 128

// App states
typedef enum {
    APP_STATE_STOPPED,
    APP_STATE_RUNNING,
    APP_STATE_PAUSED,
    APP_STATE_ERROR
} app_state_t;

// App info structure
typedef struct {
    char id[32];
    char name[MAX_APP_NAME_LEN];
    char version[16];
    char author[32];
    char entry_point[64];
    char install_path[MAX_APP_PATH_LEN];
    app_state_t state;
    js_context_t *js_context;
    uint32_t memory_usage;
    uint32_t cpu_time;
    bool is_system_app;
    uint32_t permissions;
} app_info_t;

// Permission flags
#define APP_PERM_RF_RECEIVE     (1 << 0)
#define APP_PERM_RF_TRANSMIT    (1 << 1)
#define APP_PERM_GPIO_READ      (1 << 2)
#define APP_PERM_GPIO_WRITE     (1 << 3)
#define APP_PERM_STORAGE_READ   (1 << 4)
#define APP_PERM_STORAGE_WRITE  (1 << 5)
#define APP_PERM_UI_CREATE      (1 << 6)
#define APP_PERM_NETWORK        (1 << 7)
#define APP_PERM_SYSTEM         (1 << 8)

/**
 * @brief Initialize app manager
 * @return ESP_OK on success
 */
esp_err_t app_manager_init(void);

/**
 * @brief Deinitialize app manager
 * @return ESP_OK on success
 */
esp_err_t app_manager_deinit(void);

/**
 * @brief Install app from package
 * @param package_path Path to app package
 * @param app_id Output app ID
 * @return ESP_OK on success
 */
esp_err_t app_manager_install(const char *package_path, char *app_id);

/**
 * @brief Uninstall app
 * @param app_id App ID to uninstall
 * @return ESP_OK on success
 */
esp_err_t app_manager_uninstall(const char *app_id);

/**
 * @brief Start app
 * @param app_id App ID to start
 * @return ESP_OK on success
 */
esp_err_t app_manager_start_app(const char *app_id);

/**
 * @brief Stop app
 * @param app_id App ID to stop
 * @return ESP_OK on success
 */
esp_err_t app_manager_stop_app(const char *app_id);

/**
 * @brief Get list of installed apps
 * @param apps Array to store app info
 * @param max_apps Maximum number of apps to return
 * @param num_apps Output number of apps found
 * @return ESP_OK on success
 */
esp_err_t app_manager_list_apps(app_info_t *apps, size_t max_apps, size_t *num_apps);

/**
 * @brief Get app info by ID
 * @param app_id App ID
 * @param app_info Output app info
 * @return ESP_OK on success
 */
esp_err_t app_manager_get_app_info(const char *app_id, app_info_t *app_info);

/**
 * @brief Check if app has permission
 * @param app_id App ID
 * @param permission Permission flag
 * @return true if permitted
 */
bool app_manager_check_permission(const char *app_id, uint32_t permission);

/**
 * @brief Set app permissions
 * @param app_id App ID
 * @param permissions Permission flags
 * @return ESP_OK on success
 */
esp_err_t app_manager_set_permissions(const char *app_id, uint32_t permissions);

/**
 * @brief Get currently running app
 * @return App ID or NULL if none
 */
const char* app_manager_get_current_app(void);

/**
 * @brief Pause app execution
 * @param app_id App ID to pause
 * @return ESP_OK on success
 */
esp_err_t app_manager_pause_app(const char *app_id);

/**
 * @brief Resume app execution
 * @param app_id App ID to resume
 * @return ESP_OK on success
 */
esp_err_t app_manager_resume_app(const char *app_id);

// App installer functions
esp_err_t app_installer_extract_package(const char *package_path, const char *extract_path);
esp_err_t app_installer_validate_manifest(const char *manifest_path);
esp_err_t app_installer_copy_files(const char *src_path, const char *dst_path);

// Sandbox functions
esp_err_t app_sandbox_create(const char *app_id, js_context_t **context);
esp_err_t app_sandbox_destroy(const char *app_id);
esp_err_t app_sandbox_set_limits(const char *app_id, uint32_t memory_limit, uint32_t time_limit);
bool app_sandbox_check_access(const char *app_id, const char *resource);

// Permission system functions
esp_err_t app_permissions_load(const char *app_id, uint32_t *permissions);
esp_err_t app_permissions_save(const char *app_id, uint32_t permissions);
uint32_t app_permissions_parse_string(const char *permissions_str);
const char* app_permissions_to_string(uint32_t permissions);

#ifdef __cplusplus
}
#endif

#endif // APP_MANAGER_H