/**
 * @file wifi_manager.c
 * @brief Wi-Fi Manager Implementation
 */

#include "network_service.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "WIFI_MGR";

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static wifi_status_t s_wifi_status = WIFI_STATUS_DISCONNECTED;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Forward declaration
extern void network_service_wifi_event(wifi_status_t status);

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "Wi-Fi station started");
                esp_wifi_connect();
                s_wifi_status = WIFI_STATUS_CONNECTING;
                network_service_wifi_event(s_wifi_status);
                break;
                
            case WIFI_EVENT_STA_DISCONNECTED:
                if (s_retry_num < 5) {
                    esp_wifi_connect();
                    s_retry_num++;
                    ESP_LOGI(TAG, "Retry to connect to AP (%d/5)", s_retry_num);
                } else {
                    xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                    s_wifi_status = WIFI_STATUS_FAILED;
                    network_service_wifi_event(s_wifi_status);
                    ESP_LOGI(TAG, "Failed to connect to AP");
                }
                break;
                
            case WIFI_EVENT_AP_STACONNECTED:
                {
                    wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
                    ESP_LOGI(TAG, "Station "MACSTR" joined, AID=%d",
                             MAC2STR(event->mac), event->aid);
                }
                break;
                
            case WIFI_EVENT_AP_STADISCONNECTED:
                {
                    wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
                    ESP_LOGI(TAG, "Station "MACSTR" left, AID=%d",
                             MAC2STR(event->mac), event->aid);
                }
                break;
                
            default:
                break;
        }
    } else if (event_base == IP_EVENT) {
        switch (event_id) {
            case IP_EVENT_STA_GOT_IP:
                {
                    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                    ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
                    s_retry_num = 0;
                    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                    s_wifi_status = WIFI_STATUS_CONNECTED;
                    network_service_wifi_event(s_wifi_status);
                }
                break;
                
            default:
                break;
        }
    }
}

esp_err_t wifi_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Wi-Fi manager");
    
    s_wifi_event_group = xEventGroupCreate();
    
    // Create default Wi-Fi station and AP interfaces
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    
    ESP_LOGI(TAG, "Wi-Fi manager initialized");
    return ESP_OK;
}

esp_err_t wifi_manager_connect(const char *ssid, const char *password)
{
    if (!ssid) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Connecting to Wi-Fi: %s", ssid);
    
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    if (password) {
        strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    }
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Wait for connection
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP SSID:%s", ssid);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", ssid);
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "Unexpected event");
        return ESP_FAIL;
    }
}

esp_err_t wifi_manager_disconnect(void)
{
    ESP_LOGI(TAG, "Disconnecting Wi-Fi");
    
    esp_err_t ret = esp_wifi_disconnect();
    if (ret == ESP_OK) {
        ret = esp_wifi_stop();
        s_wifi_status = WIFI_STATUS_DISCONNECTED;
        network_service_wifi_event(s_wifi_status);
    }
    
    return ret;
}

esp_err_t wifi_manager_start_ap(const char *ssid, const char *password)
{
    if (!ssid) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Starting AP: %s", ssid);
    
    wifi_config_t wifi_config = {
        .ap = {
            .channel = 1,
            .max_connection = 4,
            .authmode = password ? WIFI_AUTH_WPA_WPA2_PSK : WIFI_AUTH_OPEN
        },
    };
    
    strncpy((char*)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid) - 1);
    wifi_config.ap.ssid_len = strlen(ssid);
    
    if (password) {
        strncpy((char*)wifi_config.ap.password, password, sizeof(wifi_config.ap.password) - 1);
    }
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    s_wifi_status = WIFI_STATUS_AP_MODE;
    network_service_wifi_event(s_wifi_status);
    
    ESP_LOGI(TAG, "Wi-Fi AP started. SSID:%s channel:%d",
             ssid, wifi_config.ap.channel);
    
    return ESP_OK;
}

esp_err_t wifi_manager_scan(wifi_ap_record_t *ap_records, uint16_t *ap_count)
{
    if (!ap_records || !ap_count) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Starting Wi-Fi scan");
    
    // Start scan
    esp_wifi_scan_start(NULL, true);
    
    // Get scan results
    esp_err_t ret = esp_wifi_scan_get_ap_records(ap_count, ap_records);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Scan completed. Found %d APs", *ap_count);
    }
    
    return ret;
}

wifi_status_t wifi_manager_get_status(void)
{
    return s_wifi_status;
}