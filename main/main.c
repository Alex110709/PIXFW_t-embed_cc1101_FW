/**
 * @file main.c
 * @brief LilyGO T-Embed CC1101 JavaScript App Firmware Main
 * 
 * This firmware provides Flipper Zero style JavaScript app development
 * environment for LilyGO T-Embed CC1101 device.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"

#include "system/system_manager.h"
#include "system/hw_init.h"
#include "system/task_manager.h"

static const char *TAG = "MAIN";

/**
 * @brief Initialize SPIFFS file system
 */
static esp_err_t init_spiffs(void)
{
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    // Initialize apps partition
    esp_vfs_spiffs_conf_t apps_conf = {
        .base_path = "/apps",
        .partition_label = "apps",
        .max_files = 10,
        .format_if_mount_failed = true
    };

    ret = esp_vfs_spiffs_register(&apps_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize apps filesystem (%s)", esp_err_to_name(ret));
        return ret;
    }

    // Initialize web files partition
    esp_vfs_spiffs_conf_t www_conf = {
        .base_path = "/www",
        .partition_label = "www",
        .max_files = 5,
        .format_if_mount_failed = true
    };

    ret = esp_vfs_spiffs_register(&www_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize www filesystem (%s)", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

/**
 * @brief Initialize NVS (Non-Volatile Storage)
 */
static esp_err_t init_nvs(void)
{
    ESP_LOGI(TAG, "Initializing NVS");
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    return ret;
}

void app_main(void)
{
    ESP_LOGI(TAG, "T-Embed CC1101 JavaScript App Firmware Starting...");
    ESP_LOGI(TAG, "Version: 1.0.0");
    ESP_LOGI(TAG, "Build Date: %s %s", __DATE__, __TIME__);

    // Initialize NVS
    ESP_ERROR_CHECK(init_nvs());

    // Initialize SPIFFS file systems
    ESP_ERROR_CHECK(init_spiffs());

    // Initialize hardware
    ESP_LOGI(TAG, "Initializing hardware...");
    ESP_ERROR_CHECK(hw_init());

    // Initialize system manager
    ESP_LOGI(TAG, "Initializing system manager...");
    ESP_ERROR_CHECK(system_manager_init());

    // Start task manager
    ESP_LOGI(TAG, "Starting task manager...");
    ESP_ERROR_CHECK(task_manager_start());

    ESP_LOGI(TAG, "Firmware initialization complete");
    ESP_LOGI(TAG, "Ready for JavaScript apps!");

    // Main loop - keep the main task running
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // System health check and monitoring can be added here
        system_manager_heartbeat();
    }
}