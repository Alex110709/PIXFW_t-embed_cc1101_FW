/**
 * @file js_rf_api.c
 * @brief JavaScript RF API Implementation
 */

#include "js_api.h"
#include "cc1101.h"
#include "mjs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "JS_RF_API";

// JavaScript RF API functions

/**
 * rf.setFrequency(frequency)
 * Set RF frequency in Hz
 */
static mjs_val_t js_rf_set_frequency(struct mjs *mjs)
{
    double frequency;
    if (js_get_number_arg(mjs, 0, &frequency) != ESP_OK) {
        return js_make_error(mjs, "Invalid frequency parameter");
    }
    
    esp_err_t ret = cc1101_set_frequency((uint32_t)frequency);
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to set frequency");
    }
    
    ESP_LOGI(TAG, "Set frequency to %.0f Hz", frequency);
    return MJS_UNDEFINED;
}

/**
 * rf.getFrequency()
 * Get current RF frequency in Hz
 */
static mjs_val_t js_rf_get_frequency(struct mjs *mjs)
{
    uint32_t frequency = cc1101_get_frequency();
    return mjs_mk_number(mjs, (double)frequency);
}

/**
 * rf.setModulation(modulation)
 * Set modulation type ("ASK_OOK", "GFSK", "MSK", etc.)
 */
static mjs_val_t js_rf_set_modulation(struct mjs *mjs)
{
    char modulation_str[16];
    if (js_get_string_arg(mjs, 0, modulation_str, sizeof(modulation_str)) != ESP_OK) {
        return js_make_error(mjs, "Invalid modulation parameter");
    }
    
    cc1101_modulation_t modulation;
    if (strcmp(modulation_str, "ASK_OOK") == 0) {
        modulation = CC1101_MODULATION_ASK_OOK;
    } else if (strcmp(modulation_str, "GFSK") == 0) {
        modulation = CC1101_MODULATION_GFSK;
    } else if (strcmp(modulation_str, "MSK") == 0) {
        modulation = CC1101_MODULATION_MSK;
    } else if (strcmp(modulation_str, "2FSK") == 0) {
        modulation = CC1101_MODULATION_2FSK;
    } else {
        return js_make_error(mjs, "Unsupported modulation type");
    }
    
    esp_err_t ret = cc1101_set_modulation(modulation);
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to set modulation");
    }
    
    ESP_LOGI(TAG, "Set modulation to %s", modulation_str);
    return MJS_UNDEFINED;
}

/**
 * rf.startReceive()
 * Start receiving RF signals
 */
static mjs_val_t js_rf_start_receive(struct mjs *mjs)
{
    esp_err_t ret = cc1101_start_receive();
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to start receive");
    }
    
    ESP_LOGI(TAG, "Started RF receive mode");
    return MJS_UNDEFINED;
}

/**
 * rf.stopReceive()
 * Stop receiving RF signals
 */
static mjs_val_t js_rf_stop_receive(struct mjs *mjs)
{
    esp_err_t ret = cc1101_stop_receive();
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to stop receive");
    }
    
    ESP_LOGI(TAG, "Stopped RF receive mode");
    return MJS_UNDEFINED;
}

/**
 * rf.transmit(data)
 * Transmit RF data (data should be array of bytes)
 */
static mjs_val_t js_rf_transmit(struct mjs *mjs)
{
    // For now, transmit a test pattern
    uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78, 0xAB, 0xCD, 0xEF};
    
    esp_err_t ret = cc1101_transmit(test_data, sizeof(test_data));
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to transmit data");
    }
    
    ESP_LOGI(TAG, "Transmitted %d bytes", sizeof(test_data));
    return MJS_UNDEFINED;
}

/**
 * rf.readSignal()
 * Read received RF signal
 */
static mjs_val_t js_rf_read_signal(struct mjs *mjs)
{
    cc1101_signal_t signal;
    esp_err_t ret = cc1101_read_signal(&signal);
    
    if (ret == ESP_ERR_NOT_FOUND) {
        return MJS_NULL; // No signal available
    }
    
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to read signal");
    }
    
    // Create JavaScript object with signal data (simplified)
    // In real implementation, this would create a proper object with properties
    ESP_LOGI(TAG, "Read signal: freq=%u, rssi=%d, length=%d", 
             signal.frequency, signal.rssi, signal.length);
    
    return mjs_mk_number(mjs, signal.rssi);
}

/**
 * rf.getRssi()
 * Get current RSSI in dBm
 */
static mjs_val_t js_rf_get_rssi(struct mjs *mjs)
{
    int16_t rssi = cc1101_get_rssi();
    return mjs_mk_number(mjs, (double)rssi);
}

/**
 * rf.isPresent()
 * Check if CC1101 module is present and responding
 */
static mjs_val_t js_rf_is_present(struct mjs *mjs)
{
    bool present = cc1101_is_present();
    return mjs_mk_boolean(mjs, present);
}

/**
 * rf.loadPreset(preset)
 * Load predefined configuration preset
 */
static mjs_val_t js_rf_load_preset(struct mjs *mjs)
{
    char preset_name[32];
    if (js_get_string_arg(mjs, 0, preset_name, sizeof(preset_name)) != ESP_OK) {
        return js_make_error(mjs, "Invalid preset parameter");
    }
    
    esp_err_t ret = ESP_OK;
    
    if (strcmp(preset_name, "ask_ook_433") == 0) {
        ret = cc1101_load_preset_ask_ook(CC1101_FREQ_433MHZ);
    } else if (strcmp(preset_name, "gfsk_433") == 0) {
        ret = cc1101_load_preset_gfsk(CC1101_FREQ_433MHZ);
    } else if (strcmp(preset_name, "msk_433") == 0) {
        ret = cc1101_load_preset_msk(CC1101_FREQ_433MHZ);
    } else if (strcmp(preset_name, "ask_ook_315") == 0) {
        ret = cc1101_load_preset_ask_ook(CC1101_FREQ_315MHZ);
    } else if (strcmp(preset_name, "gfsk_315") == 0) {
        ret = cc1101_load_preset_gfsk(CC1101_FREQ_315MHZ);
    } else {
        return js_make_error(mjs, "Unknown preset");
    }
    
    if (ret != ESP_OK) {
        return js_make_error(mjs, "Failed to load preset");
    }
    
    ESP_LOGI(TAG, "Loaded preset: %s", preset_name);
    return MJS_UNDEFINED;
}

esp_err_t js_rf_api_init(void)
{
    ESP_LOGI(TAG, "Initializing RF API");
    return ESP_OK;
}

esp_err_t js_rf_api_register(js_context_t *ctx)
{
    if (!ctx || !ctx->mjs) {
        return ESP_ERR_INVALID_ARG;
    }
    
    struct mjs *mjs = ctx->mjs;
    
    // Register RF API functions
    mjs_set_ffi_func(mjs, "rf.setFrequency", js_rf_set_frequency);
    mjs_set_ffi_func(mjs, "rf.getFrequency", js_rf_get_frequency);
    mjs_set_ffi_func(mjs, "rf.setModulation", js_rf_set_modulation);
    mjs_set_ffi_func(mjs, "rf.startReceive", js_rf_start_receive);
    mjs_set_ffi_func(mjs, "rf.stopReceive", js_rf_stop_receive);
    mjs_set_ffi_func(mjs, "rf.transmit", js_rf_transmit);
    mjs_set_ffi_func(mjs, "rf.readSignal", js_rf_read_signal);
    mjs_set_ffi_func(mjs, "rf.getRssi", js_rf_get_rssi);
    mjs_set_ffi_func(mjs, "rf.isPresent", js_rf_is_present);
    mjs_set_ffi_func(mjs, "rf.loadPreset", js_rf_load_preset);
    
    ESP_LOGI(TAG, "RF API functions registered");
    return ESP_OK;
}