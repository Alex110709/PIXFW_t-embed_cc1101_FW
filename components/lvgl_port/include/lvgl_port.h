/**
 * @file lvgl_port.h
 * @brief LVGL Port for LilyGO T-Embed CC1101
 */

#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Display configuration
#define LCD_WIDTH       170
#define LCD_HEIGHT      320
#define LCD_ROTATION    LV_DISP_ROT_90

// Input configuration
typedef enum {
    INPUT_TYPE_ENCODER,
    INPUT_TYPE_BUTTON
} input_type_t;

typedef enum {
    INPUT_EVENT_PRESS,
    INPUT_EVENT_RELEASE,
    INPUT_EVENT_ENCODER_CW,
    INPUT_EVENT_ENCODER_CCW
} input_event_t;

typedef struct {
    input_type_t type;
    input_event_t event;
    uint8_t key_id;
    uint32_t timestamp;
} input_data_t;

// Callback types
typedef void (*input_callback_t)(const input_data_t *input, void *user_data);

/**
 * @brief Initialize LVGL port
 * @return ESP_OK on success
 */
esp_err_t lvgl_port_init(void);

/**
 * @brief Deinitialize LVGL port
 * @return ESP_OK on success
 */
esp_err_t lvgl_port_deinit(void);

/**
 * @brief Get display object
 * @return Display object pointer
 */
lv_disp_t* lvgl_port_get_display(void);

/**
 * @brief Get input device object
 * @return Input device object pointer
 */
lv_indev_t* lvgl_port_get_input_device(void);

/**
 * @brief Set display brightness (0-255)
 * @param brightness Brightness level
 */
void lvgl_port_set_brightness(uint8_t brightness);

/**
 * @brief Register input callback
 * @param callback Callback function
 * @param user_data User data
 */
void lvgl_port_register_input_callback(input_callback_t callback, void *user_data);

/**
 * @brief Lock LVGL mutex (for thread safety)
 */
void lvgl_port_lock(void);

/**
 * @brief Unlock LVGL mutex
 */
void lvgl_port_unlock(void);

/**
 * @brief Update LVGL (should be called periodically)
 */
void lvgl_port_task(void);

// Helper functions for UI components
/**
 * @brief Create main menu screen
 * @return Screen object
 */
lv_obj_t* lvgl_port_create_menu_screen(void);

/**
 * @brief Create status bar
 * @param parent Parent object
 * @return Status bar object
 */
lv_obj_t* lvgl_port_create_status_bar(lv_obj_t *parent);

/**
 * @brief Update status bar
 * @param status_bar Status bar object
 * @param wifi_connected Wi-Fi connection status
 * @param battery_level Battery level (0-100)
 * @param time_str Time string
 */
void lvgl_port_update_status_bar(lv_obj_t *status_bar, bool wifi_connected, 
                                 uint8_t battery_level, const char *time_str);

/**
 * @brief Show notification
 * @param title Notification title
 * @param message Notification message
 * @param timeout_ms Timeout in milliseconds (0 = no timeout)
 */
void lvgl_port_show_notification(const char *title, const char *message, uint32_t timeout_ms);

/**
 * @brief Show loading screen
 * @param message Loading message
 */
void lvgl_port_show_loading(const char *message);

/**
 * @brief Hide loading screen
 */
void lvgl_port_hide_loading(void);

#ifdef __cplusplus
}
#endif

#endif // LVGL_PORT_H