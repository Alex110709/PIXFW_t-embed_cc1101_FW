/**
 * @file js_wifi_api.c
 * @brief JavaScript Wi-Fi API Implementation
 */

#include "js_api.h"
#include "network_service.h"
#include "mjs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "JS_WIFI_API";

// JavaScript Wi-Fi API functions

/**
 * wifi.connect(ssid, password)
 * Connect to Wi-Fi network
 */
static mjs_val_t js_wifi_connect(struct mjs *mjs)
{
    char ssid[33], password[65];
    
    if (js_get_string_arg(mjs, 0, ssid, sizeof(ssid)) != ESP_OK) {
        return js_make_error(mjs, "Invalid SSID parameter");
    }
    
    // Password is optional
    if (js_get_string_arg(mjs, 1, password, sizeof(password)) != ESP_OK) {
        password[0] = '\0';
    }
    
    esp_err_t ret = network_service_connect_wifi(ssid, password[0] ? password : NULL);
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to connect to Wi-Fi");
    }
    
    ESP_LOGI(TAG, "Connecting to Wi-Fi: %s", ssid);
    return MJS_UNDEFINED;
}

/**
 * wifi.disconnect()
 * Disconnect from Wi-Fi network
 */
static mjs_val_t js_wifi_disconnect(struct mjs *mjs)
{
    esp_err_t ret = network_service_disconnect_wifi();
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to disconnect from Wi-Fi");
    }
    
    ESP_LOGI(TAG, "Disconnected from Wi-Fi");
    return MJS_UNDEFINED;
}

/**
 * wifi.startAP(ssid, password)
 * Start Wi-Fi access point
 */
static mjs_val_t js_wifi_start_ap(struct mjs *mjs)
{
    char ssid[33], password[65];
    
    if (js_get_string_arg(mjs, 0, ssid, sizeof(ssid)) != ESP_OK) {
        return js_make_error(mjs, "Invalid SSID parameter");
    }
    
    // Password is optional
    if (js_get_string_arg(mjs, 1, password, sizeof(password)) != ESP_OK) {
        password[0] = '\0';
    }
    
    esp_err_t ret = network_service_start_ap(ssid, password[0] ? password : NULL);
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to start Wi-Fi AP");
    }
    
    ESP_LOGI(TAG, "Started Wi-Fi AP: %s", ssid);
    return MJS_UNDEFINED;
}

/**
 * wifi.stopAP()
 * Stop Wi-Fi access point
 */
static mjs_val_t js_wifi_stop_ap(struct mjs *mjs)
{
    esp_err_t ret = network_service_stop_ap();
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to stop Wi-Fi AP");
    }
    
    ESP_LOGI(TAG, "Stopped Wi-Fi AP");
    return MJS_UNDEFINED;
}

/**
 * wifi.scan()
 * Scan for Wi-Fi networks
 */
static mjs_val_t js_wifi_scan(struct mjs *mjs)
{
    wifi_ap_info_t ap_list[10];
    size_t num_aps = 0;
    
    esp_err_t ret = network_service_scan_wifi(ap_list, 10, &num_aps);
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to scan for Wi-Fi networks");
    }
    
    // Create JavaScript array with scan results
    mjs_val_t ap_array = mjs_mk_array(mjs);
    
    for (int i = 0; i < num_aps && i < 10; i++) {
        mjs_val_t ap_obj = mjs_mk_object(mjs);
        
        // Set SSID
        mjs_val_t ssid_val = mjs_mk_string(mjs, ap_list[i].ssid, -1);
        mjs_set(mjs, ap_obj, "ssid", ~0, ssid_val);
        
        // Set RSSI
        mjs_val_t rssi_val = mjs_mk_number(mjs, ap_list[i].rssi);
        mjs_set(mjs, ap_obj, "rssi", ~0, rssi_val);
        
        // Set auth mode
        mjs_val_t auth_val = mjs_mk_number(mjs, ap_list[i].auth_mode);
        mjs_set(mjs, ap_obj, "authMode", ~0, auth_val);
        
        // Add to array
        mjs_array_push(mjs, ap_array, ap_obj);
    }
    
    ESP_LOGI(TAG, "Scanned %d Wi-Fi networks", (int)num_aps);
    return ap_array;
}

/**
 * wifi.getStatus()
 * Get current Wi-Fi status
 */
static mjs_val_t js_wifi_get_status(struct mjs *mjs)
{
    wifi_status_t status = network_service_get_wifi_status();
    return mjs_mk_number(mjs, (double)status);
}

/**
 * wifi.getIPAddress()
 * Get current IP address
 */
static mjs_val_t js_wifi_get_ip_address(struct mjs *mjs)
{
    char ip_str[16] = {0};
    esp_err_t ret = network_service_get_ip_address(ip_str, sizeof(ip_str));
    
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to get IP address");
    }
    
    return mjs_mk_string(mjs, ip_str, -1);
}

esp_err_t js_wifi_api_init(void)
{
    ESP_LOGI(TAG, "Initializing Wi-Fi API");
    return ESP_OK;
}

esp_err_t js_wifi_api_register(js_context_t *ctx)
{
    if (!ctx || !ctx->mjs) {
        return ESP_ERR_INVALID_ARG;
    }
    
    struct mjs *mjs = ctx->mjs;
    
    // Register Wi-Fi API functions
    mjs_set_ffi_func(mjs, "wifi.connect", js_wifi_connect);
    mjs_set_ffi_func(mjs, "wifi.disconnect", js_wifi_disconnect);
    mjs_set_ffi_func(mjs, "wifi.startAP", js_wifi_start_ap);
    mjs_set_ffi_func(mjs, "wifi.stopAP", js_wifi_stop_ap);
    mjs_set_ffi_func(mjs, "wifi.scan", js_wifi_scan);
    mjs_set_ffi_func(mjs, "wifi.getStatus", js_wifi_get_status);
    mjs_set_ffi_func(mjs, "wifi.getIPAddress", js_wifi_get_ip_address);
    
    ESP_LOGI(TAG, "Wi-Fi API functions registered");
    return ESP_OK;
}