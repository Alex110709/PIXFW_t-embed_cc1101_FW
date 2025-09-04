/**
 * @file test_cc1101_full.c
 * @brief Full integration tests for CC1101 driver with all Bruce firmware features
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

// Test initialization and basic functionality
void test_cc1101_initialization(void) {
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
    TEST_ASSERT_TRUE(cc1101_is_present());
    TEST_ASSERT_EQUAL_UINT32(CC1101_FREQ_433MHZ, cc1101_get_frequency());
}

// Test frequency setting
void test_frequency_setting(void) {
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_set_frequency(CC1101_FREQ_868MHZ));
    TEST_ASSERT_EQUAL_UINT32(CC1101_FREQ_868MHZ, cc1101_get_frequency());
    
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_set_frequency(CC1101_FREQ_915MHZ));
    TEST_ASSERT_EQUAL_UINT32(CC1101_FREQ_915MHZ, cc1101_get_frequency());
}

// Test modulation setting
void test_modulation_setting(void) {
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_set_modulation(CC1101_MODULATION_GFSK));
    // Note: We can't directly verify modulation setting without more complex mocking
    
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_set_modulation(CC1101_MODULATION_MSK));
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_set_modulation(CC1101_MODULATION_2FSK));
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_set_modulation(CC1101_MODULATION_ASK_OOK));
}

// Test receive mode
void test_receive_mode(void) {
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_start_receive());
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_stop_receive());
}

// Test transmit functionality
void test_transmit_functionality(void) {
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_transmit(test_data, sizeof(test_data)));
}

// Test signal reading
void test_signal_reading(void) {
    cc1101_signal_t signal;
    // This will return ESP_ERR_NOT_FOUND since we're not actually receiving signals
    TEST_ASSERT_EQUAL(ESP_ERR_NOT_FOUND, cc1101_read_signal(&signal));
}

// Test RSSI functionality
void test_rssi_functionality(void) {
    int16_t rssi = cc1101_get_rssi();
    TEST_ASSERT_EQUAL_INT16(-64, rssi);
}

// Test jammer functionality
void test_jammer_functionality(void) {
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_start_jammer(CC1101_FREQ_433MHZ));
    TEST_ASSERT_TRUE(cc1101_is_jammer_running());
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_stop_jammer());
    TEST_ASSERT_FALSE(cc1101_is_jammer_running());
}

// Test spectrum analyzer functionality
void test_spectrum_analyzer_functionality(void) {
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_start_spectrum_analysis(433000000, 434000000, 100000));
    TEST_ASSERT_TRUE(cc1101_is_spectrum_analysis_running());
    
    int16_t rssi = cc1101_get_rssi_at_frequency(433920000);
    TEST_ASSERT_EQUAL_INT16(-64, rssi);
    
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_stop_spectrum_analysis());
    TEST_ASSERT_FALSE(cc1101_is_spectrum_analysis_running());
}

// Test state management
void test_state_management(void) {
    cc1101_state_t state = cc1101_get_state();
    // State will depend on current operation, but should be a valid state
    TEST_ASSERT_TRUE(state >= CC1101_STATE_IDLE && state <= CC1101_STATE_TXFIFO_UNDERFLOW);
}

// Test deinitialization
void test_cc1101_deinitialization(void) {
    TEST_ASSERT_EQUAL(ESP_OK, cc1101_deinit());
}

void app_main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_cc1101_initialization);
    RUN_TEST(test_frequency_setting);
    RUN_TEST(test_modulation_setting);
    RUN_TEST(test_receive_mode);
    RUN_TEST(test_transmit_functionality);
    RUN_TEST(test_signal_reading);
    RUN_TEST(test_rssi_functionality);
    RUN_TEST(test_jammer_functionality);
    RUN_TEST(test_spectrum_analyzer_functionality);
    RUN_TEST(test_state_management);
    RUN_TEST(test_cc1101_deinitialization);
    
    UNITY_END();
}