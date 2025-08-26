/**
 * @file app_manager.c
 * @brief App Manager Implementation
 */

#include "app_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "APP_MGR";

// App registry
static app_info_t s_installed_apps[MAX_INSTALLED_APPS];
static size_t s_num_installed_apps = 0;
static SemaphoreHandle_t s_app_mutex = NULL;
static char s_current_app_id[32] = {0};

static bool s_initialized = false;

esp_err_t app_manager_init(void)
{
    if (s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Initializing app manager");
    
    // Create mutex for thread safety
    s_app_mutex = xSemaphoreCreateMutex();
    if (!s_app_mutex) {
        ESP_LOGE(TAG, "Failed to create app mutex");
        return ESP_ERR_NO_MEM;
    }
    
    // Clear app registry
    memset(s_installed_apps, 0, sizeof(s_installed_apps));
    s_num_installed_apps = 0;
    
    // Load installed apps from storage
    // TODO: Implement persistent app registry
    
    s_initialized = true;
    ESP_LOGI(TAG, "App manager initialized");
    
    return ESP_OK;
}

esp_err_t app_manager_deinit(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Deinitializing app manager");
    
    // Stop all running apps
    for (size_t i = 0; i < s_num_installed_apps; i++) {
        if (s_installed_apps[i].state == APP_STATE_RUNNING) {
            app_manager_stop_app(s_installed_apps[i].id);
        }
    }
    
    // Delete mutex
    if (s_app_mutex) {
        vSemaphoreDelete(s_app_mutex);
        s_app_mutex = NULL;
    }
    
    s_initialized = false;
    ESP_LOGI(TAG, "App manager deinitialized");
    
    return ESP_OK;
}

esp_err_t app_manager_install(const char *package_path, char *app_id)
{
    if (!package_path || !app_id) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Installing app from: %s", package_path);
    
    xSemaphoreTake(s_app_mutex, portMAX_DELAY);
    
    if (s_num_installed_apps >= MAX_INSTALLED_APPS) {
        xSemaphoreGive(s_app_mutex);
        ESP_LOGE(TAG, "Maximum number of apps reached");
        return ESP_ERR_NO_MEM;
    }
    
    // Generate unique app ID
    snprintf(app_id, 32, "app_%08x", esp_random());
    
    // Create app install directory
    char install_path[MAX_APP_PATH_LEN];
    snprintf(install_path, sizeof(install_path), "/apps/%s", app_id);
    
    // Extract package (simplified)
    esp_err_t ret = app_installer_extract_package(package_path, install_path);
    if (ret != ESP_OK) {
        xSemaphoreGive(s_app_mutex);
        return ret;
    }
    
    // Load and validate manifest
    char manifest_path[MAX_APP_PATH_LEN];
    snprintf(manifest_path, sizeof(manifest_path), "%s/manifest.json", install_path);
    
    js_app_manifest_t manifest;
    ret = mjs_engine_load_manifest(manifest_path, &manifest);
    if (ret != ESP_OK) {
        xSemaphoreGive(s_app_mutex);
        return ret;
    }
    
    // Create app info
    app_info_t *app = &s_installed_apps[s_num_installed_apps];
    strcpy(app->id, app_id);
    strcpy(app->name, manifest.name);
    strcpy(app->version, manifest.version);
    strcpy(app->author, manifest.author);
    strcpy(app->entry_point, manifest.entry_point);
    strcpy(app->install_path, install_path);
    app->state = APP_STATE_STOPPED;
    app->js_context = NULL;
    app->memory_usage = 0;
    app->cpu_time = 0;
    app->is_system_app = false;
    
    // Parse and set permissions
    app->permissions = app_permissions_parse_string(manifest.permissions);
    
    s_num_installed_apps++;
    
    xSemaphoreGive(s_app_mutex);
    
    ESP_LOGI(TAG, "Installed app: %s (%s) by %s", app->name, app_id, app->author);
    
    return ESP_OK;
}

esp_err_t app_manager_start_app(const char *app_id)
{
    if (!app_id) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Starting app: %s", app_id);
    
    xSemaphoreTake(s_app_mutex, portMAX_DELAY);
    
    // Find app
    app_info_t *app = NULL;
    for (size_t i = 0; i < s_num_installed_apps; i++) {
        if (strcmp(s_installed_apps[i].id, app_id) == 0) {
            app = &s_installed_apps[i];
            break;
        }
    }
    
    if (!app) {
        xSemaphoreGive(s_app_mutex);
        ESP_LOGE(TAG, "App not found: %s", app_id);
        return ESP_ERR_NOT_FOUND;
    }
    
    if (app->state == APP_STATE_RUNNING) {
        xSemaphoreGive(s_app_mutex);
        ESP_LOGW(TAG, "App already running: %s", app_id);
        return ESP_OK;
    }
    
    // Create sandbox environment
    esp_err_t ret = app_sandbox_create(app_id, &app->js_context);
    if (ret != ESP_OK) {
        xSemaphoreGive(s_app_mutex);
        ESP_LOGE(TAG, "Failed to create sandbox for app: %s", app_id);
        return ret;
    }
    
    // Load and execute app
    char entry_file[MAX_APP_PATH_LEN];
    snprintf(entry_file, sizeof(entry_file), "%s/%s", app->install_path, app->entry_point);
    
    ret = mjs_engine_load_file(app->js_context, entry_file);
    if (ret != ESP_OK) {
        app_sandbox_destroy(app_id);
        app->js_context = NULL;
        xSemaphoreGive(s_app_mutex);
        ESP_LOGE(TAG, "Failed to load app file: %s", entry_file);
        return ret;
    }
    
    js_exec_result_t exec_result = mjs_engine_execute(app->js_context);
    if (exec_result != JS_EXEC_OK) {
        app_sandbox_destroy(app_id);
        app->js_context = NULL;
        xSemaphoreGive(s_app_mutex);
        ESP_LOGE(TAG, "Failed to execute app: %s", app_id);
        return ESP_FAIL;
    }
    
    app->state = APP_STATE_RUNNING;
    strcpy(s_current_app_id, app_id);
    
    xSemaphoreGive(s_app_mutex);
    
    ESP_LOGI(TAG, "Started app: %s", app->name);
    
    return ESP_OK;
}

esp_err_t app_manager_stop_app(const char *app_id)
{
    if (!app_id) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Stopping app: %s", app_id);
    
    xSemaphoreTake(s_app_mutex, portMAX_DELAY);
    
    // Find app
    app_info_t *app = NULL;
    for (size_t i = 0; i < s_num_installed_apps; i++) {
        if (strcmp(s_installed_apps[i].id, app_id) == 0) {
            app = &s_installed_apps[i];
            break;
        }
    }
    
    if (!app) {
        xSemaphoreGive(s_app_mutex);
        ESP_LOGE(TAG, "App not found: %s", app_id);
        return ESP_ERR_NOT_FOUND;
    }
    
    if (app->state != APP_STATE_RUNNING) {
        xSemaphoreGive(s_app_mutex);
        ESP_LOGW(TAG, "App not running: %s", app_id);
        return ESP_OK;
    }
    
    // Stop JavaScript execution
    if (app->js_context) {
        mjs_engine_stop(app->js_context);
        app_sandbox_destroy(app_id);
        app->js_context = NULL;
    }
    
    app->state = APP_STATE_STOPPED;
    
    // Clear current app if it's this one
    if (strcmp(s_current_app_id, app_id) == 0) {
        s_current_app_id[0] = '\0';
    }
    
    xSemaphoreGive(s_app_mutex);
    
    ESP_LOGI(TAG, "Stopped app: %s", app->name);
    
    return ESP_OK;
}

esp_err_t app_manager_list_apps(app_info_t *apps, size_t max_apps, size_t *num_apps)
{
    if (!apps || !num_apps) {
        return ESP_ERR_INVALID_ARG;
    }
    
    xSemaphoreTake(s_app_mutex, portMAX_DELAY);
    
    size_t count = s_num_installed_apps > max_apps ? max_apps : s_num_installed_apps;
    memcpy(apps, s_installed_apps, count * sizeof(app_info_t));
    *num_apps = count;
    
    xSemaphoreGive(s_app_mutex);
    
    return ESP_OK;
}

bool app_manager_check_permission(const char *app_id, uint32_t permission)
{
    if (!app_id) {
        return false;
    }
    
    xSemaphoreTake(s_app_mutex, portMAX_DELAY);
    
    for (size_t i = 0; i < s_num_installed_apps; i++) {
        if (strcmp(s_installed_apps[i].id, app_id) == 0) {
            bool has_permission = (s_installed_apps[i].permissions & permission) != 0;
            xSemaphoreGive(s_app_mutex);
            return has_permission;
        }
    }
    
    xSemaphoreGive(s_app_mutex);
    return false;
}

const char* app_manager_get_current_app(void)
{
    return s_current_app_id[0] ? s_current_app_id : NULL;
}