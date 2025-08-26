/**
 * @file app_permissions.c
 * @brief App Permissions System Implementation
 */

#include "app_manager.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "APP_PERMS";
static const char *NVS_NAMESPACE = "app_perms";

// Permission name mappings
static const struct {
    const char *name;
    uint32_t flag;
} permission_map[] = {
    {"rf.receive", APP_PERM_RF_RECEIVE},
    {"rf.transmit", APP_PERM_RF_TRANSMIT},
    {"gpio.read", APP_PERM_GPIO_READ},
    {"gpio.write", APP_PERM_GPIO_WRITE},
    {"storage.read", APP_PERM_STORAGE_READ},
    {"storage.write", APP_PERM_STORAGE_WRITE},
    {"ui.create", APP_PERM_UI_CREATE},
    {"network", APP_PERM_NETWORK},
    {"system", APP_PERM_SYSTEM},
};

#define PERMISSION_COUNT (sizeof(permission_map) / sizeof(permission_map[0]))

esp_err_t app_permissions_load(const char *app_id, uint32_t *permissions)
{
    if (!app_id || !permissions) {
        return ESP_ERR_INVALID_ARG;
    }
    
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGD(TAG, "Failed to open NVS namespace: %s", esp_err_to_name(ret));
        *permissions = 0;
        return ESP_OK; // Default to no permissions
    }
    
    ret = nvs_get_u32(nvs_handle, app_id, permissions);
    nvs_close(nvs_handle);
    
    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGD(TAG, "No permissions found for app: %s", app_id);
        *permissions = 0;
        return ESP_OK;
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to load permissions for app %s: %s", app_id, esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Loaded permissions for app %s: 0x%08x", app_id, *permissions);
    return ESP_OK;
}

esp_err_t app_permissions_save(const char *app_id, uint32_t permissions)
{
    if (!app_id) {
        return ESP_ERR_INVALID_ARG;
    }
    
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = nvs_set_u32(nvs_handle, app_id, permissions);
    if (ret != ESP_OK) {
        nvs_close(nvs_handle);
        ESP_LOGE(TAG, "Failed to save permissions for app %s: %s", app_id, esp_err_to_name(ret));
        return ret;
    }
    
    ret = nvs_commit(nvs_handle);
    nvs_close(nvs_handle);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit permissions for app %s: %s", app_id, esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "Saved permissions for app %s: 0x%08x", app_id, permissions);
    return ESP_OK;
}

uint32_t app_permissions_parse_string(const char *permissions_str)
{
    if (!permissions_str) {
        return 0;
    }
    
    uint32_t permissions = 0;
    char *str_copy = strdup(permissions_str);
    if (!str_copy) {
        ESP_LOGE(TAG, "Failed to allocate memory for permissions string");
        return 0;
    }
    
    char *token = strtok(str_copy, ",");
    while (token) {
        // Remove leading/trailing whitespace
        while (*token == ' ' || *token == '\t') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
            *end = '\0';
            end--;
        }
        
        // Find matching permission
        for (size_t i = 0; i < PERMISSION_COUNT; i++) {
            if (strcmp(token, permission_map[i].name) == 0) {
                permissions |= permission_map[i].flag;
                ESP_LOGD(TAG, "Parsed permission: %s", token);
                break;
            }
        }
        
        token = strtok(NULL, ",");
    }
    
    free(str_copy);
    
    ESP_LOGI(TAG, "Parsed permissions string '%s' to 0x%08x", permissions_str, permissions);
    return permissions;
}

const char* app_permissions_to_string(uint32_t permissions)
{
    static char perm_str[256];
    perm_str[0] = '\0';
    
    bool first = true;
    for (size_t i = 0; i < PERMISSION_COUNT; i++) {
        if (permissions & permission_map[i].flag) {
            if (!first) {
                strcat(perm_str, ",");
            }
            strcat(perm_str, permission_map[i].name);
            first = false;
        }
    }
    
    return perm_str;
}

bool app_permissions_check(const char *app_id, uint32_t required_permission)
{
    if (!app_id) {
        return false;
    }
    
    uint32_t app_permissions = 0;
    esp_err_t ret = app_permissions_load(app_id, &app_permissions);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to load permissions for app: %s", app_id);
        return false;
    }
    
    bool has_permission = (app_permissions & required_permission) != 0;
    
    if (!has_permission) {
        ESP_LOGW(TAG, "App %s denied permission 0x%08x (has: 0x%08x)", 
                 app_id, required_permission, app_permissions);
    }
    
    return has_permission;
}

esp_err_t app_permissions_grant(const char *app_id, uint32_t permission)
{
    if (!app_id) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint32_t current_permissions = 0;
    esp_err_t ret = app_permissions_load(app_id, &current_permissions);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to load current permissions, starting with none");
        current_permissions = 0;
    }
    
    current_permissions |= permission;
    
    ret = app_permissions_save(app_id, current_permissions);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save updated permissions");
        return ret;
    }
    
    ESP_LOGI(TAG, "Granted permission 0x%08x to app %s", permission, app_id);
    return ESP_OK;
}

esp_err_t app_permissions_revoke(const char *app_id, uint32_t permission)
{
    if (!app_id) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint32_t current_permissions = 0;
    esp_err_t ret = app_permissions_load(app_id, &current_permissions);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to load current permissions");
        return ret;
    }
    
    current_permissions &= ~permission;
    
    ret = app_permissions_save(app_id, current_permissions);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save updated permissions");
        return ret;
    }
    
    ESP_LOGI(TAG, "Revoked permission 0x%08x from app %s", permission, app_id);
    return ESP_OK;
}