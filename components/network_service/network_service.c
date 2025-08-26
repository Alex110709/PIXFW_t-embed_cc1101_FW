/**
 * @file network_service.c
 * @brief Network Service Implementation
 */

#include "network_service.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "NET_SVC";

static bool s_initialized = false;
static network_config_t s_config = {0};
static wifi_event_callback_t s_wifi_callback = NULL;
static void *s_wifi_callback_data = NULL;

esp_err_t network_service_init(void)
{
    if (s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Initializing network service");
    
    // Initialize TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());
    
    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Initialize Wi-Fi manager
    ESP_ERROR_CHECK(wifi_manager_init());
    
    // Initialize web IDE
    ESP_ERROR_CHECK(web_ide_init());
    
    // Set default configuration
    strcpy(s_config.hostname, "t-embed-cc1101");
    s_config.mode = NETWORK_MODE_STATION;
    s_config.auto_connect = false;
    s_config.max_retry = 5;
    
    s_initialized = true;
    ESP_LOGI(TAG, "Network service initialized");
    
    return ESP_OK;
}

esp_err_t network_service_deinit(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Deinitializing network service");
    
    // Stop web services
    web_ide_stop();
    web_server_stop();
    
    // Disconnect Wi-Fi
    network_service_disconnect_wifi();
    
    s_initialized = false;
    ESP_LOGI(TAG, "Network service deinitialized");
    
    return ESP_OK;
}

esp_err_t network_service_set_config(const network_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&s_config, config, sizeof(network_config_t));
    
    // Save to NVS
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open("network", NVS_READWRITE, &nvs_handle);
    if (ret == ESP_OK) {
        nvs_set_blob(nvs_handle, "config", &s_config, sizeof(network_config_t));
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
    }
    
    ESP_LOGI(TAG, "Network configuration updated");
    return ESP_OK;
}

esp_err_t network_service_get_config(network_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &s_config, sizeof(network_config_t));
    return ESP_OK;
}

esp_err_t network_service_connect_wifi(const char *ssid, const char *password)
{
    if (!ssid) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Connecting to Wi-Fi: %s", ssid);
    
    // Update configuration
    strncpy(s_config.ssid, ssid, sizeof(s_config.ssid) - 1);
    if (password) {
        strncpy(s_config.password, password, sizeof(s_config.password) - 1);
    } else {
        s_config.password[0] = '\0';
    }
    
    return wifi_manager_connect(ssid, password);
}

esp_err_t network_service_disconnect_wifi(void)
{
    ESP_LOGI(TAG, "Disconnecting from Wi-Fi");
    return wifi_manager_disconnect();
}

esp_err_t network_service_start_ap(const char *ssid, const char *password)
{
    if (!ssid) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Starting AP mode: %s", ssid);
    
    esp_err_t ret = wifi_manager_start_ap(ssid, password);
    if (ret == ESP_OK) {
        // Start web IDE when AP mode is active
        web_ide_start();
    }
    
    return ret;
}

esp_err_t network_service_stop_ap(void)
{
    ESP_LOGI(TAG, "Stopping AP mode");
    
    // Stop web IDE
    web_ide_stop();
    
    return wifi_manager_disconnect();
}

esp_err_t network_service_scan_wifi(wifi_ap_info_t *ap_list, size_t max_aps, size_t *num_aps)
{
    if (!ap_list || !num_aps) {
        return ESP_ERR_INVALID_ARG;
    }
    
    wifi_ap_record_t *records = malloc(max_aps * sizeof(wifi_ap_record_t));
    if (!records) {
        return ESP_ERR_NO_MEM;
    }
    
    uint16_t count = max_aps;
    esp_err_t ret = wifi_manager_scan(records, &count);
    
    if (ret == ESP_OK) {
        for (int i = 0; i < count; i++) {
            strncpy(ap_list[i].ssid, (char*)records[i].ssid, MAX_SSID_LEN - 1);
            ap_list[i].ssid[MAX_SSID_LEN - 1] = '\0';
            ap_list[i].rssi = records[i].rssi;
            ap_list[i].auth_mode = records[i].authmode;
            ap_list[i].is_hidden = false; // Simplified
        }
        *num_aps = count;
    }
    
    free(records);
    return ret;
}

wifi_status_t network_service_get_wifi_status(void)
{
    return wifi_manager_get_status();
}

esp_err_t network_service_get_ip_address(char *ip_str, size_t max_len)
{
    if (!ip_str || max_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!netif) {
        netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    }
    
    if (!netif) {
        strcpy(ip_str, "0.0.0.0");
        return ESP_FAIL;
    }
    
    esp_netif_ip_info_t ip_info;
    esp_err_t ret = esp_netif_get_ip_info(netif, &ip_info);
    if (ret == ESP_OK) {
        snprintf(ip_str, max_len, IPSTR, IP2STR(&ip_info.ip));
    } else {
        strcpy(ip_str, "0.0.0.0");
    }
    
    return ret;
}

void network_service_set_wifi_callback(wifi_event_callback_t callback, void *user_data)
{
    s_wifi_callback = callback;
    s_wifi_callback_data = user_data;
}

// Internal callback for Wi-Fi events
void network_service_wifi_event(wifi_status_t status)
{
    if (s_wifi_callback) {
        s_wifi_callback(status, s_wifi_callback_data);
    }
    
    // Auto-start web IDE when connected in station mode
    if (status == WIFI_STATUS_CONNECTED && s_config.mode == NETWORK_MODE_STATION) {
        web_ide_start();
    }
}