/**
 * @file network_service.h
 * @brief Network Service for T-Embed CC1101
 */

#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SSID_LEN 32
#define MAX_PASSWORD_LEN 64
#define MAX_HOSTNAME_LEN 32

// Network modes
typedef enum {
    NETWORK_MODE_STATION,
    NETWORK_MODE_AP,
    NETWORK_MODE_STATION_AP
} network_mode_t;

// Wi-Fi connection status
typedef enum {
    WIFI_STATUS_DISCONNECTED,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED,
    WIFI_STATUS_FAILED,
    WIFI_STATUS_AP_MODE
} wifi_status_t;

// Network configuration
typedef struct {
    char ssid[MAX_SSID_LEN];
    char password[MAX_PASSWORD_LEN];
    char hostname[MAX_HOSTNAME_LEN];
    network_mode_t mode;
    bool auto_connect;
    uint8_t max_retry;
} network_config_t;

// Wi-Fi scan result
typedef struct {
    char ssid[MAX_SSID_LEN];
    int8_t rssi;
    wifi_auth_mode_t auth_mode;
    bool is_hidden;
} wifi_ap_info_t;

// Callback types
typedef void (*wifi_event_callback_t)(wifi_status_t status, void *user_data);
typedef void (*web_request_callback_t)(httpd_req_t *req, void *user_data);

/**
 * @brief Initialize network service
 * @return ESP_OK on success
 */
esp_err_t network_service_init(void);

/**
 * @brief Deinitialize network service
 * @return ESP_OK on success
 */
esp_err_t network_service_deinit(void);

/**
 * @brief Set network configuration
 * @param config Network configuration
 * @return ESP_OK on success
 */
esp_err_t network_service_set_config(const network_config_t *config);

/**
 * @brief Get network configuration
 * @param config Output network configuration
 * @return ESP_OK on success
 */
esp_err_t network_service_get_config(network_config_t *config);

/**
 * @brief Connect to Wi-Fi
 * @param ssid SSID to connect
 * @param password Password
 * @return ESP_OK on success
 */
esp_err_t network_service_connect_wifi(const char *ssid, const char *password);

/**
 * @brief Disconnect from Wi-Fi
 * @return ESP_OK on success
 */
esp_err_t network_service_disconnect_wifi(void);

/**
 * @brief Start Wi-Fi AP mode
 * @param ssid AP SSID
 * @param password AP password
 * @return ESP_OK on success
 */
esp_err_t network_service_start_ap(const char *ssid, const char *password);

/**
 * @brief Stop Wi-Fi AP mode
 * @return ESP_OK on success
 */
esp_err_t network_service_stop_ap(void);

/**
 * @brief Scan for Wi-Fi networks
 * @param ap_list Output AP list
 * @param max_aps Maximum APs to scan
 * @param num_aps Output number of APs found
 * @return ESP_OK on success
 */
esp_err_t network_service_scan_wifi(wifi_ap_info_t *ap_list, size_t max_aps, size_t *num_aps);

/**
 * @brief Get current Wi-Fi status
 * @return Current Wi-Fi status
 */
wifi_status_t network_service_get_wifi_status(void);

/**
 * @brief Get IP address
 * @param ip_str Output IP string
 * @param max_len Maximum string length
 * @return ESP_OK on success
 */
esp_err_t network_service_get_ip_address(char *ip_str, size_t max_len);

/**
 * @brief Set Wi-Fi event callback
 * @param callback Callback function
 * @param user_data User data
 */
void network_service_set_wifi_callback(wifi_event_callback_t callback, void *user_data);

// Web server functions
/**
 * @brief Start web server
 * @param port Server port
 * @return ESP_OK on success
 */
esp_err_t web_server_start(uint16_t port);

/**
 * @brief Stop web server
 * @return ESP_OK on success
 */
esp_err_t web_server_stop(void);

/**
 * @brief Register web handler
 * @param uri URI pattern
 * @param method HTTP method
 * @param handler Handler function
 * @param user_ctx User context
 * @return ESP_OK on success
 */
esp_err_t web_server_register_handler(const char *uri, httpd_method_t method, 
                                      httpd_uri_func_t handler, void *user_ctx);

// Web IDE functions
/**
 * @brief Initialize web IDE
 * @return ESP_OK on success
 */
esp_err_t web_ide_init(void);

/**
 * @brief Start web IDE server
 * @return ESP_OK on success
 */
esp_err_t web_ide_start(void);

/**
 * @brief Stop web IDE server
 * @return ESP_OK on success
 */
esp_err_t web_ide_stop(void);

// Wi-Fi manager functions
esp_err_t wifi_manager_init(void);
esp_err_t wifi_manager_connect(const char *ssid, const char *password);
esp_err_t wifi_manager_disconnect(void);
esp_err_t wifi_manager_start_ap(const char *ssid, const char *password);
esp_err_t wifi_manager_scan(wifi_ap_record_t *ap_records, uint16_t *ap_count);
wifi_status_t wifi_manager_get_status(void);

#ifdef __cplusplus
}
#endif

#endif // NETWORK_SERVICE_H