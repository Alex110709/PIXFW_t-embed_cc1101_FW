/**
 * @file js_gpio_api.c
 * @brief JavaScript GPIO API Implementation
 */

#include "js_api.h"
#include "mjs.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "JS_GPIO_API";

/**
 * gpio.setup(pin, mode)
 * Configure GPIO pin
 */
static mjs_val_t js_gpio_setup(struct mjs *mjs)
{
    double pin_num, mode_val;
    if (js_get_number_arg(mjs, 0, &pin_num) != ESP_OK) {
        return js_make_error(mjs, "Invalid pin parameter");
    }
    if (js_get_number_arg(mjs, 1, &mode_val) != ESP_OK) {
        return js_make_error(mjs, "Invalid mode parameter");
    }
    
    gpio_num_t pin = (gpio_num_t)pin_num;
    gpio_mode_t mode = (gpio_mode_t)mode_val;
    
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = mode,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to configure GPIO");
    }
    
    ESP_LOGI(TAG, "Configured GPIO %d with mode %d", pin, mode);
    return MJS_UNDEFINED;
}

/**
 * gpio.write(pin, level)
 * Write digital value to GPIO pin
 */
static mjs_val_t js_gpio_write(struct mjs *mjs)
{
    double pin_num;
    bool level;
    
    if (js_get_number_arg(mjs, 0, &pin_num) != ESP_OK) {
        return js_make_error(mjs, "Invalid pin parameter");
    }
    if (js_get_bool_arg(mjs, 1, &level) != ESP_OK) {
        return js_make_error(mjs, "Invalid level parameter");
    }
    
    gpio_num_t pin = (gpio_num_t)pin_num;
    esp_err_t ret = gpio_set_level(pin, level ? 1 : 0);
    
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to write GPIO");
    }
    
    ESP_LOGD(TAG, "Set GPIO %d to %s", pin, level ? "HIGH" : "LOW");
    return MJS_UNDEFINED;
}

/**
 * gpio.read(pin)
 * Read digital value from GPIO pin
 */
static mjs_val_t js_gpio_read(struct mjs *mjs)
{
    double pin_num;
    if (js_get_number_arg(mjs, 0, &pin_num) != ESP_OK) {
        return js_make_error(mjs, "Invalid pin parameter");
    }
    
    gpio_num_t pin = (gpio_num_t)pin_num;
    int level = gpio_get_level(pin);
    
    ESP_LOGD(TAG, "Read GPIO %d: %s", pin, level ? "HIGH" : "LOW");
    return mjs_mk_boolean(mjs, level != 0);
}

esp_err_t js_gpio_api_init(void)
{
    ESP_LOGI(TAG, "Initializing GPIO API");
    return ESP_OK;
}

esp_err_t js_gpio_api_register(js_context_t *ctx)
{
    if (!ctx || !ctx->mjs) {
        return ESP_ERR_INVALID_ARG;
    }
    
    struct mjs *mjs = ctx->mjs;
    
    mjs_set_ffi_func(mjs, "gpio.setup", js_gpio_setup);
    mjs_set_ffi_func(mjs, "gpio.write", js_gpio_write);
    mjs_set_ffi_func(mjs, "gpio.read", js_gpio_read);
    
    ESP_LOGI(TAG, "GPIO API functions registered");
    return ESP_OK;
}