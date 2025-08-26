/**
 * @file mjs_module_loader.c
 * @brief Module Loader for JavaScript Engine
 */

#include "mjs_engine.h"
#include "mjs.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "MJS_MODULE";

// Module registry
static struct {
    char name[32];
    esp_err_t (*init_func)(void);
    bool initialized;
} s_modules[16];

static int s_module_count = 0;

/**
 * @brief Register a module
 */
static esp_err_t register_module(const char *name, esp_err_t (*init_func)(void))
{
    if (s_module_count >= 16) {
        ESP_LOGE(TAG, "Too many modules registered");
        return ESP_ERR_NO_MEM;
    }
    
    strncpy(s_modules[s_module_count].name, name, sizeof(s_modules[s_module_count].name) - 1);
    s_modules[s_module_count].name[sizeof(s_modules[s_module_count].name) - 1] = '\0';
    s_modules[s_module_count].init_func = init_func;
    s_modules[s_module_count].initialized = false;
    s_module_count++;
    
    ESP_LOGI(TAG, "Registered module: %s", name);
    return ESP_OK;
}

/**
 * @brief Initialize a module
 */
static esp_err_t init_module(const char *name)
{
    for (int i = 0; i < s_module_count; i++) {
        if (strcmp(s_modules[i].name, name) == 0) {
            if (!s_modules[i].initialized) {
                esp_err_t ret = s_modules[i].init_func();
                if (ret == ESP_OK) {
                    s_modules[i].initialized = true;
                    ESP_LOGI(TAG, "Initialized module: %s", name);
                } else {
                    ESP_LOGE(TAG, "Failed to initialize module: %s", name);
                }
                return ret;
            }
            return ESP_OK; // Already initialized
        }
    }
    
    ESP_LOGE(TAG, "Module not found: %s", name);
    return ESP_ERR_NOT_FOUND;
}

// Console module implementation
esp_err_t mjs_module_console_register(void)
{
    ESP_LOGI(TAG, "Registering console module");
    
    // Register console functions
    extern mjs_val_t native_console_log(struct mjs *mjs);
    mjs_engine_register_function("console.log", native_console_log);
    
    return ESP_OK;
}

// RF module implementation
static mjs_val_t rf_set_frequency(struct mjs *mjs)
{
    ESP_LOGI(TAG, "RF: setFrequency called");
    // TODO: Get frequency from arguments and call cc1101_set_frequency
    return MJS_UNDEFINED;
}

static mjs_val_t rf_start_receive(struct mjs *mjs)
{
    ESP_LOGI(TAG, "RF: startReceive called");
    // TODO: Call cc1101_start_receive
    return MJS_UNDEFINED;
}

static mjs_val_t rf_stop_receive(struct mjs *mjs)
{
    ESP_LOGI(TAG, "RF: stopReceive called");
    // TODO: Call cc1101_stop_receive
    return MJS_UNDEFINED;
}

static mjs_val_t rf_transmit(struct mjs *mjs)
{
    ESP_LOGI(TAG, "RF: transmit called");
    // TODO: Get data from arguments and call cc1101_transmit
    return MJS_UNDEFINED;
}

esp_err_t mjs_module_rf_register(void)
{
    ESP_LOGI(TAG, "Registering RF module");
    
    mjs_engine_register_function("rf.setFrequency", rf_set_frequency);
    mjs_engine_register_function("rf.startReceive", rf_start_receive);
    mjs_engine_register_function("rf.stopReceive", rf_stop_receive);
    mjs_engine_register_function("rf.transmit", rf_transmit);
    
    return register_module("rf", mjs_module_rf_register);
}

// GPIO module implementation
static mjs_val_t gpio_setup(struct mjs *mjs)
{
    ESP_LOGI(TAG, "GPIO: setup called");
    // TODO: Get pin and mode from arguments and configure GPIO
    return MJS_UNDEFINED;
}

static mjs_val_t gpio_write(struct mjs *mjs)
{
    ESP_LOGI(TAG, "GPIO: write called");
    // TODO: Get pin and value from arguments and set GPIO level
    return MJS_UNDEFINED;
}

static mjs_val_t gpio_read(struct mjs *mjs)
{
    ESP_LOGI(TAG, "GPIO: read called");
    // TODO: Get pin from arguments and return GPIO level
    return mjs_mk_boolean(mjs, false);
}

esp_err_t mjs_module_gpio_register(void)
{
    ESP_LOGI(TAG, "Registering GPIO module");
    
    mjs_engine_register_function("gpio.setup", gpio_setup);
    mjs_engine_register_function("gpio.write", gpio_write);
    mjs_engine_register_function("gpio.read", gpio_read);
    
    return register_module("gpio", mjs_module_gpio_register);
}

// UI module implementation
static mjs_val_t ui_create_screen(struct mjs *mjs)
{
    ESP_LOGI(TAG, "UI: createScreen called");
    // TODO: Create LVGL screen and return handle
    return mjs_mk_number(mjs, 1); // Return dummy screen ID
}

static mjs_val_t ui_create_button(struct mjs *mjs)
{
    ESP_LOGI(TAG, "UI: createButton called");
    // TODO: Create LVGL button and return handle
    return mjs_mk_number(mjs, 2); // Return dummy button ID
}

static mjs_val_t ui_create_label(struct mjs *mjs)
{
    ESP_LOGI(TAG, "UI: createLabel called");
    // TODO: Create LVGL label and return handle
    return mjs_mk_number(mjs, 3); // Return dummy label ID
}

esp_err_t mjs_module_ui_register(void)
{
    ESP_LOGI(TAG, "Registering UI module");
    
    mjs_engine_register_function("ui.createScreen", ui_create_screen);
    mjs_engine_register_function("ui.createButton", ui_create_button);
    mjs_engine_register_function("ui.createLabel", ui_create_label);
    
    return register_module("ui", mjs_module_ui_register);
}

// Storage module implementation
static mjs_val_t storage_write_text(struct mjs *mjs)
{
    ESP_LOGI(TAG, "Storage: writeText called");
    // TODO: Get filename and content from arguments and write file
    return MJS_UNDEFINED;
}

static mjs_val_t storage_read_text(struct mjs *mjs)
{
    ESP_LOGI(TAG, "Storage: readText called");
    // TODO: Get filename from arguments and return file content
    return mjs_mk_string(mjs, "file content", -1);
}

static mjs_val_t storage_set_config(struct mjs *mjs)
{
    ESP_LOGI(TAG, "Storage: setConfig called");
    // TODO: Save configuration value to NVS
    return MJS_UNDEFINED;
}

static mjs_val_t storage_get_config(struct mjs *mjs)
{
    ESP_LOGI(TAG, "Storage: getConfig called");
    // TODO: Load configuration value from NVS
    return mjs_mk_string(mjs, "config value", -1);
}

esp_err_t mjs_module_storage_register(void)
{
    ESP_LOGI(TAG, "Registering Storage module");
    
    mjs_engine_register_function("storage.writeText", storage_write_text);
    mjs_engine_register_function("storage.readText", storage_read_text);
    mjs_engine_register_function("storage.setConfig", storage_set_config);
    mjs_engine_register_function("storage.getConfig", storage_get_config);
    
    return register_module("storage", mjs_module_storage_register);
}

// Notification module implementation
static mjs_val_t notify_show(struct mjs *mjs)
{
    ESP_LOGI(TAG, "Notification: show called");
    // TODO: Show notification using LVGL
    return MJS_UNDEFINED;
}

static mjs_val_t notify_led(struct mjs *mjs)
{
    ESP_LOGI(TAG, "Notification: led called");
    // TODO: Control LED
    return MJS_UNDEFINED;
}

static mjs_val_t notify_beep(struct mjs *mjs)
{
    ESP_LOGI(TAG, "Notification: beep called");
    // TODO: Generate beep sound
    return MJS_UNDEFINED;
}

esp_err_t mjs_module_notification_register(void)
{
    ESP_LOGI(TAG, "Registering Notification module");
    
    mjs_engine_register_function("notify.show", notify_show);
    mjs_engine_register_function("notify.led", notify_led);
    mjs_engine_register_function("notify.beep", notify_beep);
    
    return register_module("notification", mjs_module_notification_register);
}

/**
 * @brief Initialize all registered modules
 */
esp_err_t mjs_module_loader_init_all(void)
{
    ESP_LOGI(TAG, "Initializing all modules");
    
    for (int i = 0; i < s_module_count; i++) {
        if (!s_modules[i].initialized) {
            esp_err_t ret = s_modules[i].init_func();
            if (ret == ESP_OK) {
                s_modules[i].initialized = true;
                ESP_LOGI(TAG, "Initialized module: %s", s_modules[i].name);
            } else {
                ESP_LOGW(TAG, "Failed to initialize module: %s", s_modules[i].name);
            }
        }
    }
    
    return ESP_OK;
}