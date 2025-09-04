/**
 * @file cc1101.c
 * @brief CC1101 Driver Implementation
 */

#include "cc1101.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <string.h>
#include <math.h>

static const char *TAG = "CC1101";

static cc1101_config_t s_config = {0};
static bool s_initialized = false;
static cc1101_rx_callback_t s_rx_callback = NULL;
static cc1101_tx_callback_t s_tx_callback = NULL;
static void *s_rx_user_data = NULL;
static void *s_tx_user_data = NULL;

// Forward declarations
extern esp_err_t cc1101_spi_init(spi_device_handle_t spi_device);
extern esp_err_t cc1101_spi_write_reg(uint8_t reg, uint8_t value);
extern esp_err_t cc1101_spi_read_reg(uint8_t reg, uint8_t *value);
extern esp_err_t cc1101_spi_write_burst_reg(uint8_t reg, const uint8_t *data, uint8_t length);
extern esp_err_t cc1101_spi_read_burst_reg(uint8_t reg, uint8_t *data, uint8_t length);
extern esp_err_t cc1101_spi_strobe(uint8_t strobe);
extern esp_err_t cc1101_spi_write_fifo(const uint8_t *data, uint8_t length);
extern esp_err_t cc1101_spi_read_fifo(uint8_t *data, uint8_t length);

// Configuration presets
extern esp_err_t cc1101_config_load_preset(const char *preset_name);

/**
 * @brief Calculate frequency registers from frequency in Hz
 */
static void cc1101_calc_freq_regs(uint32_t frequency, uint8_t *freq2, uint8_t *freq1, uint8_t *freq0)
{
    // CC1101 crystal frequency is 26 MHz
    // Frequency = (FREQ * XTAL) / 2^16
    uint32_t freq_word = (uint32_t)((uint64_t)frequency * 65536 / 26000000);
    
    *freq2 = (freq_word >> 16) & 0xFF;
    *freq1 = (freq_word >> 8) & 0xFF;
    *freq0 = freq_word & 0xFF;
}

/**
 * @brief Calculate data rate registers
 */
static void cc1101_calc_drate_regs(uint32_t data_rate, uint8_t *mdmcfg4, uint8_t *mdmcfg3)
{
    // Data rate = (256 + DRATE_M) * 2^DRATE_E * FXTAL / 2^28
    // Where FXTAL = 26 MHz
    
    uint8_t drate_e = 0;
    uint32_t temp = data_rate;
    
    // Find the exponent
    while (temp > 511) {
        temp >>= 1;
        drate_e++;
    }
    
    uint8_t drate_m = temp - 256;
    
    *mdmcfg4 = (*mdmcfg4 & 0xF0) | (drate_e & 0x0F);
    *mdmcfg3 = drate_m;
}

esp_err_t cc1101_init(const cc1101_config_t *config)
{
    if (!config || !config->spi_device) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Initializing CC1101");

    memcpy(&s_config, config, sizeof(cc1101_config_t));

    // Initialize SPI interface
    ESP_ERROR_CHECK(cc1101_spi_init(config->spi_device));

    // Configure GDO pins
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << config->pin_gdo0) | (1ULL << config->pin_gdo2),
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Reset the module
    ESP_ERROR_CHECK(cc1101_reset());

    // Check if module is present
    if (!cc1101_is_present()) {
        ESP_LOGE(TAG, "CC1101 module not detected");
        return ESP_ERR_NOT_FOUND;
    }

    ESP_LOGI(TAG, "CC1101 version: 0x%02X", cc1101_get_version());

    // Load default configuration based on frequency
    if (config->frequency_hz == CC1101_FREQ_433MHZ) {
        ESP_ERROR_CHECK(cc1101_load_preset_gfsk(config->frequency_hz));
    } else if (config->frequency_hz == CC1101_FREQ_315MHZ) {
        ESP_ERROR_CHECK(cc1101_load_preset_ask_ook(config->frequency_hz));
    } else {
        ESP_ERROR_CHECK(cc1101_load_preset_gfsk(config->frequency_hz));
    }

    // Apply custom configuration
    ESP_ERROR_CHECK(cc1101_set_frequency(config->frequency_hz));
    ESP_ERROR_CHECK(cc1101_set_modulation(config->modulation));
    
    if (config->data_rate > 0) {
        ESP_ERROR_CHECK(cc1101_set_data_rate(config->data_rate));
    }
    
    if (config->bandwidth > 0) {
        ESP_ERROR_CHECK(cc1101_set_bandwidth(config->bandwidth));
    }

    // Set sync word
    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_SYNC1, config->sync_word[0]));
    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_SYNC0, config->sync_word[1]));

    // Set device address
    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_ADDR, config->device_address));

    // Configure packet mode if enabled
    if (config->packet_mode) {
        ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_PKTLEN, config->packet_length));
        ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_PKTCTRL0, 0x05)); // Variable packet length, CRC enabled
    } else {
        ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_PKTCTRL0, 0x32)); // Infinite packet length
    }

    // Enter idle state
    ESP_ERROR_CHECK(cc1101_spi_strobe(CC1101_SIDLE));

    s_initialized = true;
    ESP_LOGI(TAG, "CC1101 initialized successfully");
    
    return ESP_OK;
}

esp_err_t cc1101_deinit(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // Enter power down mode
    cc1101_spi_strobe(CC1101_SPWD);
    
    s_initialized = false;
    s_rx_callback = NULL;
    s_tx_callback = NULL;
    
    ESP_LOGI(TAG, "CC1101 deinitialized");
    return ESP_OK;
}

esp_err_t cc1101_reset(void)
{
    ESP_LOGI(TAG, "Resetting CC1101");
    
    // Send reset strobe
    ESP_ERROR_CHECK(cc1101_spi_strobe(CC1101_SRES));
    
    // Wait for reset to complete
    vTaskDelay(pdMS_TO_TICKS(10));
    
    return ESP_OK;
}

esp_err_t cc1101_set_frequency(uint32_t frequency_hz)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t freq2, freq1, freq0;
    cc1101_calc_freq_regs(frequency_hz, &freq2, &freq1, &freq0);

    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_FREQ2, freq2));
    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_FREQ1, freq1));
    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_FREQ0, freq0));

    s_config.frequency_hz = frequency_hz;
    
    ESP_LOGI(TAG, "Frequency set to %u Hz", frequency_hz);
    return ESP_OK;
}

uint32_t cc1101_get_frequency(void)
{
    return s_config.frequency_hz;
}

esp_err_t cc1101_set_modulation(cc1101_modulation_t modulation)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t mdmcfg2;
    ESP_ERROR_CHECK(cc1101_spi_read_reg(CC1101_MDMCFG2, &mdmcfg2));
    
    mdmcfg2 = (mdmcfg2 & 0x8F) | ((modulation & 0x07) << 4);
    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_MDMCFG2, mdmcfg2));

    s_config.modulation = modulation;
    return ESP_OK;
}

esp_err_t cc1101_set_data_rate(uint32_t data_rate)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t mdmcfg4, mdmcfg3;
    ESP_ERROR_CHECK(cc1101_spi_read_reg(CC1101_MDMCFG4, &mdmcfg4));
    
    cc1101_calc_drate_regs(data_rate, &mdmcfg4, &mdmcfg3);
    
    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_MDMCFG4, mdmcfg4));
    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_MDMCFG3, mdmcfg3));

    s_config.data_rate = data_rate;
    return ESP_OK;
}

esp_err_t cc1101_start_receive(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // Flush RX FIFO
    ESP_ERROR_CHECK(cc1101_spi_strobe(CC1101_SFRX));
    
    // Enter RX mode
    ESP_ERROR_CHECK(cc1101_spi_strobe(CC1101_SRX));
    
    ESP_LOGI(TAG, "Entered RX mode");
    return ESP_OK;
}

esp_err_t cc1101_stop_receive(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // Enter idle mode
    ESP_ERROR_CHECK(cc1101_spi_strobe(CC1101_SIDLE));
    
    ESP_LOGI(TAG, "Exited RX mode");
    return ESP_OK;
}

esp_err_t cc1101_transmit(const uint8_t *data, uint8_t length)
{
    if (!s_initialized || !data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Enter idle mode
    ESP_ERROR_CHECK(cc1101_spi_strobe(CC1101_SIDLE));
    
    // Flush TX FIFO
    ESP_ERROR_CHECK(cc1101_spi_strobe(CC1101_SFTX));
    
    // Write data to FIFO
    if (s_config.packet_mode) {
        ESP_ERROR_CHECK(cc1101_spi_write_fifo(&length, 1)); // Write length first
    }
    ESP_ERROR_CHECK(cc1101_spi_write_fifo(data, length));
    
    // Enter TX mode
    ESP_ERROR_CHECK(cc1101_spi_strobe(CC1101_STX));
    
    ESP_LOGI(TAG, "Transmitting %d bytes", length);
    return ESP_OK;
}

esp_err_t cc1101_read_signal(cc1101_signal_t *signal)
{
    if (!s_initialized || !signal) {
        return ESP_ERR_INVALID_ARG;
    }

    // Check if data is available
    uint8_t rxbytes;
    ESP_ERROR_CHECK(cc1101_spi_read_reg(CC1101_RXBYTES, &rxbytes));
    
    if ((rxbytes & 0x7F) == 0) {
        return ESP_ERR_NOT_FOUND; // No data available
    }

    // Read packet length if in packet mode
    uint8_t length = s_config.packet_length;
    if (s_config.packet_mode) {
        ESP_ERROR_CHECK(cc1101_spi_read_fifo(&length, 1));
        if (length > sizeof(signal->data)) {
            length = sizeof(signal->data);
        }
    } else {
        length = (rxbytes & 0x7F) > sizeof(signal->data) ? sizeof(signal->data) : (rxbytes & 0x7F);
    }

    // Read data
    ESP_ERROR_CHECK(cc1101_spi_read_fifo(signal->data, length));
    
    // Read RSSI and LQI
    uint8_t rssi_raw, lqi;
    ESP_ERROR_CHECK(cc1101_spi_read_fifo(&rssi_raw, 1));
    ESP_ERROR_CHECK(cc1101_spi_read_fifo(&lqi, 1));

    // Fill signal structure
    signal->frequency = s_config.frequency_hz;
    signal->rssi = (rssi_raw >= 128) ? (rssi_raw - 256) / 2 - 74 : rssi_raw / 2 - 74;
    signal->lqi = lqi & 0x7F;
    signal->length = length;
    signal->timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;

    return ESP_OK;
}

int16_t cc1101_get_rssi(void)
{
    if (!s_initialized) {
        return -128;
    }

    uint8_t rssi_raw;
    if (cc1101_spi_read_reg(CC1101_RSSI, &rssi_raw) != ESP_OK) {
        return -128;
    }

    return (rssi_raw >= 128) ? (rssi_raw - 256) / 2 - 74 : rssi_raw / 2 - 74;
}

cc1101_state_t cc1101_get_state(void)
{
    if (!s_initialized) {
        return CC1101_STATE_IDLE;
    }

    uint8_t marcstate;
    if (cc1101_spi_read_reg(CC1101_MARCSTATE, &marcstate) != ESP_OK) {
        return CC1101_STATE_IDLE;
    }

    return (cc1101_state_t)(marcstate & 0x1F);
}

void cc1101_set_rx_callback(cc1101_rx_callback_t callback, void *user_data)
{
    s_rx_callback = callback;
    s_rx_user_data = user_data;
}

void cc1101_set_tx_callback(cc1101_tx_callback_t callback, void *user_data)
{
    s_tx_callback = callback;
    s_tx_user_data = user_data;
}

uint8_t cc1101_get_version(void)
{
    if (!s_initialized) {
        return 0;
    }

    uint8_t version;
    if (cc1101_spi_read_reg(CC1101_VERSION, &version) != ESP_OK) {
        return 0;
    }

    return version;
}

bool cc1101_is_present(void)
{
    uint8_t partnum, version;
    
    if (cc1101_spi_read_reg(CC1101_PARTNUM, &partnum) != ESP_OK) {
        return false;
    }
    
    if (cc1101_spi_read_reg(CC1101_VERSION, &version) != ESP_OK) {
        return false;
    }

    // CC1101 part number should be 0x00
    // Version should be 0x14 for CC1101
    return (partnum == 0x00 && version == 0x14);
}

// Spectrum Analyzer functions

static bool s_spectrum_analysis_running = false;
static uint32_t s_spectrum_start_freq = 0;
static uint32_t s_spectrum_stop_freq = 0;
static uint32_t s_spectrum_step_size = 0;

esp_err_t cc1101_start_spectrum_analysis(uint32_t start_frequency, uint32_t stop_frequency, uint32_t step_size)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (start_frequency > stop_frequency || step_size == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    s_spectrum_start_freq = start_frequency;
    s_spectrum_stop_freq = stop_frequency;
    s_spectrum_step_size = step_size;
    s_spectrum_analysis_running = true;

    ESP_LOGI(TAG, "Started spectrum analysis: %u Hz to %u Hz, step %u Hz", 
             start_frequency, stop_frequency, step_size);
    
    return ESP_OK;
}

esp_err_t cc1101_stop_spectrum_analysis(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    s_spectrum_analysis_running = false;
    
    ESP_LOGI(TAG, "Stopped spectrum analysis");
    return ESP_OK;
}

bool cc1101_is_spectrum_analysis_running(void)
{
    return s_spectrum_analysis_running;
}

int16_t cc1101_get_rssi_at_frequency(uint32_t frequency)
{
    if (!s_initialized) {
        return -128;
    }

    // Set the frequency
    uint8_t freq2, freq1, freq0;
    cc1101_calc_freq_regs(frequency, &freq2, &freq1, &freq0);
    
    // Store current frequency to restore later
    uint32_t current_freq = s_config.frequency_hz;
    
    // Set new frequency
    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_FREQ2, freq2));
    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_FREQ1, freq1));
    ESP_ERROR_CHECK(cc1101_spi_write_reg(CC1101_FREQ0, freq0));
    
    // Wait a bit for frequency to settle
    vTaskDelay(pdMS_TO_TICKS(2));
    
    // Get RSSI
    int16_t rssi = cc1101_get_rssi();
    
    // Restore original frequency
    cc1101_calc_freq_regs(current_freq, &freq2, &freq1, &freq0);
    cc1101_spi_write_reg(CC1101_FREQ2, freq2);
    cc1101_spi_write_reg(CC1101_FREQ1, freq1);
    cc1101_spi_write_reg(CC1101_FREQ0, freq0);
    
    return rssi;
}
