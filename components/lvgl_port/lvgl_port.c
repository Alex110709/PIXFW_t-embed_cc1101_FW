/**
 * @file lvgl_port.c
 * @brief LVGL Port Implementation
 */

#include "lvgl_port.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include <string.h>

static const char *TAG = "LVGL_PORT";

// LVGL objects
static lv_disp_t *s_display = NULL;
static lv_indev_t *s_input_device = NULL;
static SemaphoreHandle_t s_lvgl_mutex = NULL;
static bool s_initialized = false;

// Input handling
static input_callback_t s_input_callback = NULL;
static void *s_input_user_data = NULL;
static lv_obj_t *s_notification_obj = NULL;
static lv_obj_t *s_loading_obj = NULL;

// Forward declarations
extern esp_err_t display_driver_init(lv_disp_t **disp);
extern esp_err_t input_driver_init(lv_indev_t **indev);
extern void display_driver_deinit(void);
extern void input_driver_deinit(void);

// LVGL tick callback
static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(portTICK_PERIOD_MS);
}

esp_err_t lvgl_port_init(void)
{
    if (s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Initializing LVGL port");

    // Create mutex for LVGL thread safety
    s_lvgl_mutex = xSemaphoreCreateMutex();
    if (!s_lvgl_mutex) {
        ESP_LOGE(TAG, "Failed to create LVGL mutex");
        return ESP_ERR_NO_MEM;
    }

    // Initialize LVGL
    lv_init();

    // Initialize display driver
    ESP_ERROR_CHECK(display_driver_init(&s_display));
    if (!s_display) {
        ESP_LOGE(TAG, "Failed to initialize display");
        return ESP_ERR_NOT_FOUND;
    }

    // Initialize input driver
    ESP_ERROR_CHECK(input_driver_init(&s_input_device));
    if (!s_input_device) {
        ESP_LOGE(TAG, "Failed to initialize input device");
        return ESP_ERR_NOT_FOUND;
    }

    // Create tick task for LVGL
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "lv_tick"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, portTICK_PERIOD_MS * 1000));

    // Set default theme
    lv_theme_t *theme = lv_theme_default_init(s_display, 
        lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED),
        true,
        LV_FONT_DEFAULT);
    lv_disp_set_theme(s_display, theme);

    s_initialized = true;
    ESP_LOGI(TAG, "LVGL port initialized successfully");

    return ESP_OK;
}

esp_err_t lvgl_port_deinit(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Deinitializing LVGL port");

    // Clean up display and input drivers
    display_driver_deinit();
    input_driver_deinit();

    // Deinitialize LVGL
    lv_deinit();

    // Delete mutex
    if (s_lvgl_mutex) {
        vSemaphoreDelete(s_lvgl_mutex);
        s_lvgl_mutex = NULL;
    }

    s_display = NULL;
    s_input_device = NULL;
    s_initialized = false;

    ESP_LOGI(TAG, "LVGL port deinitialized");
    return ESP_OK;
}

lv_disp_t* lvgl_port_get_display(void)
{
    return s_display;
}

lv_indev_t* lvgl_port_get_input_device(void)
{
    return s_input_device;
}

void lvgl_port_set_brightness(uint8_t brightness)
{
    extern void hw_set_backlight(uint8_t brightness);
    hw_set_backlight(brightness);
}

void lvgl_port_register_input_callback(input_callback_t callback, void *user_data)
{
    s_input_callback = callback;
    s_input_user_data = user_data;
}

void lvgl_port_lock(void)
{
    if (s_lvgl_mutex) {
        xSemaphoreTake(s_lvgl_mutex, portMAX_DELAY);
    }
}

void lvgl_port_unlock(void)
{
    if (s_lvgl_mutex) {
        xSemaphoreGive(s_lvgl_mutex);
    }
}

void lvgl_port_task(void)
{
    if (!s_initialized) {
        return;
    }

    lvgl_port_lock();
    lv_timer_handler();
    lvgl_port_unlock();
}

// UI Helper functions
lv_obj_t* lvgl_port_create_menu_screen(void)
{
    lvgl_port_lock();
    
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    
    // Create status bar
    lv_obj_t *status_bar = lvgl_port_create_status_bar(screen);
    
    // Create main menu list
    lv_obj_t *list = lv_list_create(screen);
    lv_obj_set_size(list, LCD_WIDTH, LCD_HEIGHT - 30);
    lv_obj_set_pos(list, 0, 30);
    
    // Add menu items
    lv_obj_t *btn;
    
    btn = lv_list_add_btn(list, LV_SYMBOL_WIFI, "RF Scanner");
    btn = lv_list_add_btn(list, LV_SYMBOL_EDIT, "Signal Generator");
    btn = lv_list_add_btn(list, LV_SYMBOL_FILE, "Apps");
    btn = lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "Settings");
    btn = lv_list_add_btn(list, LV_SYMBOL_LIST, "About");
    
    lvgl_port_unlock();
    
    return screen;
}

lv_obj_t* lvgl_port_create_status_bar(lv_obj_t *parent)
{
    lv_obj_t *status_bar = lv_obj_create(parent);
    lv_obj_set_size(status_bar, LCD_WIDTH, 30);
    lv_obj_set_pos(status_bar, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_set_style_radius(status_bar, 0, 0);
    
    // Time label
    lv_obj_t *time_label = lv_label_create(status_bar);
    lv_label_set_text(time_label, "00:00");
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_align(time_label, LV_ALIGN_LEFT_MID, 5, 0);
    
    // Wi-Fi icon
    lv_obj_t *wifi_icon = lv_label_create(status_bar);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_icon, lv_color_white(), 0);
    lv_obj_align(wifi_icon, LV_ALIGN_RIGHT_MID, -40, 0);
    
    // Battery icon
    lv_obj_t *battery_icon = lv_label_create(status_bar);
    lv_label_set_text(battery_icon, LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_color(battery_icon, lv_color_white(), 0);
    lv_obj_align(battery_icon, LV_ALIGN_RIGHT_MID, -5, 0);
    
    return status_bar;
}

void lvgl_port_update_status_bar(lv_obj_t *status_bar, bool wifi_connected, 
                                 uint8_t battery_level, const char *time_str)
{
    if (!status_bar) return;
    
    lvgl_port_lock();
    
    // Update time
    lv_obj_t *time_label = lv_obj_get_child(status_bar, 0);
    if (time_label && time_str) {
        lv_label_set_text(time_label, time_str);
    }
    
    // Update Wi-Fi icon
    lv_obj_t *wifi_icon = lv_obj_get_child(status_bar, 1);
    if (wifi_icon) {
        lv_obj_set_style_opa(wifi_icon, wifi_connected ? LV_OPA_COVER : LV_OPA_30, 0);
    }
    
    // Update battery icon
    lv_obj_t *battery_icon = lv_obj_get_child(status_bar, 2);
    if (battery_icon) {
        const char *battery_symbol;
        if (battery_level > 75) {
            battery_symbol = LV_SYMBOL_BATTERY_FULL;
        } else if (battery_level > 50) {
            battery_symbol = LV_SYMBOL_BATTERY_3;
        } else if (battery_level > 25) {
            battery_symbol = LV_SYMBOL_BATTERY_2;
        } else if (battery_level > 10) {
            battery_symbol = LV_SYMBOL_BATTERY_1;
        } else {
            battery_symbol = LV_SYMBOL_BATTERY_EMPTY;
        }
        lv_label_set_text(battery_icon, battery_symbol);
    }
    
    lvgl_port_unlock();
}

static void notification_close_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_obj_del(obj);
    s_notification_obj = NULL;
}

void lvgl_port_show_notification(const char *title, const char *message, uint32_t timeout_ms)
{
    if (!s_initialized) return;
    
    lvgl_port_lock();
    
    // Remove existing notification
    if (s_notification_obj) {
        lv_obj_del(s_notification_obj);
        s_notification_obj = NULL;
    }
    
    // Create notification popup
    s_notification_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_notification_obj, LCD_WIDTH - 20, 80);
    lv_obj_center(s_notification_obj);
    lv_obj_set_style_bg_color(s_notification_obj, lv_color_hex(0x4CAF50), 0);
    lv_obj_set_style_border_width(s_notification_obj, 2, 0);
    lv_obj_set_style_border_color(s_notification_obj, lv_color_white(), 0);
    
    // Title
    if (title) {
        lv_obj_t *title_label = lv_label_create(s_notification_obj);
        lv_label_set_text(title_label, title);
        lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
        lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);
        lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 5);
    }
    
    // Message
    if (message) {
        lv_obj_t *msg_label = lv_label_create(s_notification_obj);
        lv_label_set_text(msg_label, message);
        lv_obj_set_style_text_color(msg_label, lv_color_white(), 0);
        lv_obj_align(msg_label, LV_ALIGN_CENTER, 0, title ? 10 : 0);
    }
    
    // Auto close timer
    if (timeout_ms > 0) {
        lv_obj_add_event_cb(s_notification_obj, notification_close_cb, LV_EVENT_DELETE, NULL);
        
        // Create timer to auto close
        static lv_timer_t *close_timer = NULL;
        if (close_timer) {
            lv_timer_del(close_timer);
        }
        close_timer = lv_timer_create([](lv_timer_t *timer) {
            if (s_notification_obj) {
                lv_obj_del(s_notification_obj);
                s_notification_obj = NULL;
            }
            lv_timer_del(timer);
        }, timeout_ms, NULL);
    }
    
    lvgl_port_unlock();
}

void lvgl_port_show_loading(const char *message)
{
    if (!s_initialized) return;
    
    lvgl_port_lock();
    
    // Remove existing loading screen
    if (s_loading_obj) {
        lv_obj_del(s_loading_obj);
    }
    
    // Create loading screen
    s_loading_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(s_loading_obj, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(s_loading_obj);
    lv_obj_set_style_bg_color(s_loading_obj, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(s_loading_obj, LV_OPA_80, 0);
    
    // Spinner
    lv_obj_t *spinner = lv_spinner_create(s_loading_obj, 1000, 60);
    lv_obj_set_size(spinner, 40, 40);
    lv_obj_center(spinner);
    
    // Message
    if (message) {
        lv_obj_t *msg_label = lv_label_create(s_loading_obj);
        lv_label_set_text(msg_label, message);
        lv_obj_set_style_text_color(msg_label, lv_color_white(), 0);
        lv_obj_align_to(msg_label, spinner, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    }
    
    lvgl_port_unlock();
}

void lvgl_port_hide_loading(void)
{
    if (!s_initialized) return;
    
    lvgl_port_lock();
    
    if (s_loading_obj) {
        lv_obj_del(s_loading_obj);
        s_loading_obj = NULL;
    }
    
    lvgl_port_unlock();
}