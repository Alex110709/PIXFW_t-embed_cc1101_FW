/**
 * @file cc1101_spi.c
 * @brief CC1101 SPI Communication Implementation
 */

#include "cc1101.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CC1101_SPI";

static spi_device_handle_t s_spi_device = NULL;

esp_err_t cc1101_spi_init(spi_device_handle_t spi_device)
{
    if (!spi_device) {
        return ESP_ERR_INVALID_ARG;
    }

    s_spi_device = spi_device;
    ESP_LOGI(TAG, "CC1101 SPI interface initialized");
    
    return ESP_OK;
}

esp_err_t cc1101_spi_write_reg(uint8_t reg, uint8_t value)
{
    if (!s_spi_device) {
        return ESP_ERR_INVALID_STATE;
    }

    spi_transaction_t trans = {
        .length = 16, // 8 bits address + 8 bits data
        .tx_data = {reg, value, 0, 0},
        .flags = SPI_TRANS_USE_TXDATA
    };

    esp_err_t ret = spi_device_transmit(s_spi_device, &trans);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write register 0x%02X", reg);
    }

    return ret;
}

esp_err_t cc1101_spi_read_reg(uint8_t reg, uint8_t *value)
{
    if (!s_spi_device || !value) {
        return ESP_ERR_INVALID_ARG;
    }

    // Set read bit (bit 7)
    uint8_t cmd = reg | 0x80;
    
    spi_transaction_t trans = {
        .length = 16,
        .rxlength = 8,
        .tx_data = {cmd, 0x00, 0, 0},
        .flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA
    };

    esp_err_t ret = spi_device_transmit(s_spi_device, &trans);
    if (ret == ESP_OK) {
        *value = trans.rx_data[1];
    } else {
        ESP_LOGE(TAG, "Failed to read register 0x%02X", reg);
    }

    return ret;
}

esp_err_t cc1101_spi_write_burst_reg(uint8_t reg, const uint8_t *data, uint8_t length)
{
    if (!s_spi_device || !data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Set burst bit (bit 6)
    uint8_t cmd = reg | 0x40;
    
    spi_transaction_t trans = {
        .length = (1 + length) * 8,
        .tx_buffer = NULL,
        .rx_buffer = NULL
    };

    // Prepare TX buffer
    uint8_t *tx_buf = malloc(1 + length);
    if (!tx_buf) {
        return ESP_ERR_NO_MEM;
    }

    tx_buf[0] = cmd;
    memcpy(&tx_buf[1], data, length);
    trans.tx_buffer = tx_buf;

    esp_err_t ret = spi_device_transmit(s_spi_device, &trans);
    
    free(tx_buf);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write burst register 0x%02X", reg);
    }

    return ret;
}

esp_err_t cc1101_spi_read_burst_reg(uint8_t reg, uint8_t *data, uint8_t length)
{
    if (!s_spi_device || !data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Set read bit (bit 7) and burst bit (bit 6)
    uint8_t cmd = reg | 0xC0;
    
    spi_transaction_t trans = {
        .length = (1 + length) * 8,
        .rxlength = length * 8,
        .tx_buffer = NULL,
        .rx_buffer = NULL
    };

    // Prepare TX buffer (command + dummy bytes)
    uint8_t *tx_buf = malloc(1 + length);
    uint8_t *rx_buf = malloc(1 + length);
    
    if (!tx_buf || !rx_buf) {
        free(tx_buf);
        free(rx_buf);
        return ESP_ERR_NO_MEM;
    }

    tx_buf[0] = cmd;
    memset(&tx_buf[1], 0x00, length); // Dummy bytes
    
    trans.tx_buffer = tx_buf;
    trans.rx_buffer = rx_buf;

    esp_err_t ret = spi_device_transmit(s_spi_device, &trans);
    
    if (ret == ESP_OK) {
        memcpy(data, &rx_buf[1], length);
    } else {
        ESP_LOGE(TAG, "Failed to read burst register 0x%02X", reg);
    }

    free(tx_buf);
    free(rx_buf);

    return ret;
}

esp_err_t cc1101_spi_strobe(uint8_t strobe)
{
    if (!s_spi_device) {
        return ESP_ERR_INVALID_STATE;
    }

    spi_transaction_t trans = {
        .length = 8,
        .tx_data = {strobe, 0, 0, 0},
        .flags = SPI_TRANS_USE_TXDATA
    };

    esp_err_t ret = spi_device_transmit(s_spi_device, &trans);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send strobe 0x%02X", strobe);
    }

    return ret;
}

esp_err_t cc1101_spi_write_fifo(const uint8_t *data, uint8_t length)
{
    if (!s_spi_device || !data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // FIFO write with burst bit
    uint8_t cmd = CC1101_TXFIFO | 0x40;
    
    spi_transaction_t trans = {
        .length = (1 + length) * 8,
        .tx_buffer = NULL
    };

    uint8_t *tx_buf = malloc(1 + length);
    if (!tx_buf) {
        return ESP_ERR_NO_MEM;
    }

    tx_buf[0] = cmd;
    memcpy(&tx_buf[1], data, length);
    trans.tx_buffer = tx_buf;

    esp_err_t ret = spi_device_transmit(s_spi_device, &trans);
    
    free(tx_buf);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write FIFO");
    }

    return ret;
}

esp_err_t cc1101_spi_read_fifo(uint8_t *data, uint8_t length)
{
    if (!s_spi_device || !data || length == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // FIFO read with read and burst bits
    uint8_t cmd = CC1101_RXFIFO | 0xC0;
    
    spi_transaction_t trans = {
        .length = (1 + length) * 8,
        .rxlength = length * 8,
        .tx_buffer = NULL,
        .rx_buffer = NULL
    };

    uint8_t *tx_buf = malloc(1 + length);
    uint8_t *rx_buf = malloc(1 + length);
    
    if (!tx_buf || !rx_buf) {
        free(tx_buf);
        free(rx_buf);
        return ESP_ERR_NO_MEM;
    }

    tx_buf[0] = cmd;
    memset(&tx_buf[1], 0x00, length);
    
    trans.tx_buffer = tx_buf;
    trans.rx_buffer = rx_buf;

    esp_err_t ret = spi_device_transmit(s_spi_device, &trans);
    
    if (ret == ESP_OK) {
        memcpy(data, &rx_buf[1], length);
    } else {
        ESP_LOGE(TAG, "Failed to read FIFO");
    }

    free(tx_buf);
    free(rx_buf);

    return ret;
}