/**
 * @file test_spectrum_analyzer.c
 * @brief Unit tests for CC1101 Spectrum Analyzer functionality
 */

#include "cc1101.h"
#include "unity.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Mock SPI functions for testing
esp_err_t cc1101_spi_init(spi_device_handle_t spi_device) {
    return ESP_OK;
}

esp_err_t cc1101_spi_write_reg(uint8_t reg, uint8_t value) {
    return ESP_OK;
}

esp_err_t cc1101_spi_read_reg(uint8_t reg, uint8_t *value) {
    if (reg == CC1101_PARTNUM) {
        *value = 0x00;
    } else if (reg == CC1101_VERSION) {
        *value = 0x14;
    } else if (reg == CC1101_RSSI) {
        *value = 0x80; // -64 dBm
    } else {
        *value = 0x00;
    }
    return ESP_OK;
}

esp_err_t cc1101_spi_write_burst_reg(uint8_t reg, const uint8_t *data, uint8_t length) {
    return ESP_OK;
}

esp_err_t cc1101_spi_read_burst_reg(uint8_t reg, uint8_t *data, uint8_t length) {
    return ESP_OK;
}

esp_err_t cc1101_spi_strobe(uint8_t strobe) {
    return ESP_OK;
}

esp_err_t cc1101_spi_write_fifo(const uint8_t *data, uint8_t length) {
    return ESP_OK;
}

esp_err_t cc1101_spi_read_fifo(uint8_t *data, uint8_t length) {
    return ESP_OK;
}

esp_err_t cc1101_config_load_preset(const char *preset_name) {
    return ESP_OK;
}

// Test functions
void test_spectrum_analyzer_start(void) {
    // Initialize CC1101 with test configuration
    cc1101_config_t config = {
        .spi_device = NULL,
        .pin_gdo0 = 0,
        .pin_gdo2 = 0,
        .frequency_hz = CC1101_FREQ_433MHZ,
        .modulation = CC1101_MODULATION_ASK_OOK,
        .data_rate = 4800,
        .bandwidth = 58000,
        .sync_word = {0xAA, 0x55},
        .device_address = 0x00,
        .packet_mode = false,
        .packet_length = 0
    };
    
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_init(&config));
    
    // Test starting spectrum analysis
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_start_spectrum_analysis(433000000, 434000000, 100000));
    TEST_ASSERT_TRUE(cc1101_is_spectrum_analysis_running());
    
    // Test with invalid parameters
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, cc1101_start_spectrum_analysis(434000000, 433000000, 100000));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, cc1101_start_spectrum_analysis(433000000, 434000000, 0));
}

void test_spectrum_analyzer_stop(void) {
    // Test stopping spectrum analysis
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_stop_spectrum_analysis());
    TEST_ASSERT_FALSE(cc1101_is_spectrum_analysis_running());
}

void test_get_rssi_at_frequency(void) {
    // Test getting RSSI at specific frequency
    int16_t rssi = cc1101_get_rssi_at_frequency(433920000);
    TEST_ASSERT_EQUAL_INT16(-64, rssi);
}

void app_main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_spectrum_analyzer_start);
    RUN_TEST(test_spectrum_analyzer_stop);
    RUN_TEST(test_get_rssi_at_frequency);
    
    UNITY_END();
}