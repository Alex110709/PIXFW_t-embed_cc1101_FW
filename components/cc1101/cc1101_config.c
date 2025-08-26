/**
 * @file cc1101_config.c
 * @brief CC1101 Configuration Presets
 */

#include "cc1101.h"
#include "esp_log.h"

static const char *TAG = "CC1101_CFG";

// Configuration register values for different presets

// ASK/OOK 433MHz preset
static const uint8_t cc1101_preset_ask_ook_433[][2] = {
    {CC1101_IOCFG2,   0x0D}, // GDO2 configuration
    {CC1101_IOCFG1,   0x2E}, // GDO1 configuration  
    {CC1101_IOCFG0,   0x06}, // GDO0 configuration
    {CC1101_FIFOTHR,  0x47}, // FIFO threshold
    {CC1101_SYNC1,    0xD3}, // Sync word high byte
    {CC1101_SYNC0,    0x91}, // Sync word low byte
    {CC1101_PKTLEN,   0xFF}, // Packet length
    {CC1101_PKTCTRL1, 0x04}, // Packet automation control
    {CC1101_PKTCTRL0, 0x32}, // Packet automation control
    {CC1101_ADDR,     0x00}, // Device address
    {CC1101_CHANNR,   0x00}, // Channel number
    {CC1101_FSCTRL1,  0x06}, // Frequency synthesizer control
    {CC1101_FSCTRL0,  0x00}, // Frequency synthesizer control
    {CC1101_MDMCFG4,  0xF5}, // Modem configuration
    {CC1101_MDMCFG3,  0x83}, // Modem configuration
    {CC1101_MDMCFG2,  0x30}, // ASK/OOK, no Manchester, no preamble/sync
    {CC1101_MDMCFG1,  0x22}, // Modem configuration
    {CC1101_MDMCFG0,  0xF8}, // Modem configuration
    {CC1101_DEVIATN,  0x00}, // Modem deviation (not used in ASK/OOK)
    {CC1101_MCSM2,    0x07}, // Main Radio Control State Machine
    {CC1101_MCSM1,    0x30}, // Main Radio Control State Machine
    {CC1101_MCSM0,    0x18}, // Main Radio Control State Machine
    {CC1101_FOCCFG,   0x16}, // Frequency Offset Compensation
    {CC1101_BSCFG,    0x6C}, // Bit Synchronization Configuration
    {CC1101_AGCCTRL2, 0x43}, // AGC Control
    {CC1101_AGCCTRL1, 0x40}, // AGC Control
    {CC1101_AGCCTRL0, 0x91}, // AGC Control
    {CC1101_WOREVT1,  0x87}, // High byte Event 0 timeout
    {CC1101_WOREVT0,  0x6B}, // Low byte Event 0 timeout
    {CC1101_WORCTRL,  0xFB}, // Wake On Radio control
    {CC1101_FREND1,   0x56}, // Front end RX configuration
    {CC1101_FREND0,   0x10}, // Front end TX configuration
    {CC1101_FSCAL3,   0xE9}, // Frequency synthesizer calibration
    {CC1101_FSCAL2,   0x2A}, // Frequency synthesizer calibration
    {CC1101_FSCAL1,   0x00}, // Frequency synthesizer calibration
    {CC1101_FSCAL0,   0x1F}, // Frequency synthesizer calibration
    {CC1101_RCCTRL1,  0x41}, // RC oscillator configuration
    {CC1101_RCCTRL0,  0x00}, // RC oscillator configuration
    {0xFF, 0xFF} // End marker
};

// GFSK 433MHz preset
static const uint8_t cc1101_preset_gfsk_433[][2] = {
    {CC1101_IOCFG2,   0x29}, // GDO2 configuration
    {CC1101_IOCFG1,   0x2E}, // GDO1 configuration
    {CC1101_IOCFG0,   0x06}, // GDO0 configuration
    {CC1101_FIFOTHR,  0x47}, // FIFO threshold
    {CC1101_SYNC1,    0xD3}, // Sync word high byte
    {CC1101_SYNC0,    0x91}, // Sync word low byte
    {CC1101_PKTLEN,   0xFF}, // Packet length
    {CC1101_PKTCTRL1, 0x04}, // Packet automation control
    {CC1101_PKTCTRL0, 0x05}, // Packet automation control
    {CC1101_ADDR,     0x00}, // Device address
    {CC1101_CHANNR,   0x00}, // Channel number
    {CC1101_FSCTRL1,  0x08}, // Frequency synthesizer control
    {CC1101_FSCTRL0,  0x00}, // Frequency synthesizer control
    {CC1101_MDMCFG4,  0x5B}, // Modem configuration (data rate)
    {CC1101_MDMCFG3,  0xF8}, // Modem configuration (data rate)
    {CC1101_MDMCFG2,  0x13}, // GFSK, 30/32 sync word bits detected
    {CC1101_MDMCFG1,  0x22}, // Modem configuration
    {CC1101_MDMCFG0,  0xF8}, // Modem configuration
    {CC1101_DEVIATN,  0x47}, // Modem deviation setting (~47kHz)
    {CC1101_MCSM2,    0x07}, // Main Radio Control State Machine
    {CC1101_MCSM1,    0x30}, // Main Radio Control State Machine
    {CC1101_MCSM0,    0x18}, // Main Radio Control State Machine
    {CC1101_FOCCFG,   0x1D}, // Frequency Offset Compensation
    {CC1101_BSCFG,    0x1C}, // Bit Synchronization Configuration
    {CC1101_AGCCTRL2, 0xC7}, // AGC Control
    {CC1101_AGCCTRL1, 0x00}, // AGC Control
    {CC1101_AGCCTRL0, 0xB2}, // AGC Control
    {CC1101_WOREVT1,  0x87}, // High byte Event 0 timeout
    {CC1101_WOREVT0,  0x6B}, // Low byte Event 0 timeout
    {CC1101_WORCTRL,  0xFB}, // Wake On Radio control
    {CC1101_FREND1,   0xB6}, // Front end RX configuration
    {CC1101_FREND0,   0x10}, // Front end TX configuration
    {CC1101_FSCAL3,   0xEA}, // Frequency synthesizer calibration
    {CC1101_FSCAL2,   0x2A}, // Frequency synthesizer calibration
    {CC1101_FSCAL1,   0x00}, // Frequency synthesizer calibration
    {CC1101_FSCAL0,   0x1F}, // Frequency synthesizer calibration
    {CC1101_RCCTRL1,  0x41}, // RC oscillator configuration
    {CC1101_RCCTRL0,  0x00}, // RC oscillator configuration
    {0xFF, 0xFF} // End marker
};

// MSK 433MHz preset
static const uint8_t cc1101_preset_msk_433[][2] = {
    {CC1101_IOCFG2,   0x29}, // GDO2 configuration
    {CC1101_IOCFG1,   0x2E}, // GDO1 configuration
    {CC1101_IOCFG0,   0x06}, // GDO0 configuration
    {CC1101_FIFOTHR,  0x47}, // FIFO threshold
    {CC1101_SYNC1,    0xD3}, // Sync word high byte
    {CC1101_SYNC0,    0x91}, // Sync word low byte
    {CC1101_PKTLEN,   0xFF}, // Packet length
    {CC1101_PKTCTRL1, 0x04}, // Packet automation control
    {CC1101_PKTCTRL0, 0x05}, // Packet automation control
    {CC1101_ADDR,     0x00}, // Device address
    {CC1101_CHANNR,   0x00}, // Channel number
    {CC1101_FSCTRL1,  0x0A}, // Frequency synthesizer control
    {CC1101_FSCTRL0,  0x00}, // Frequency synthesizer control
    {CC1101_MDMCFG4,  0x7B}, // Modem configuration
    {CC1101_MDMCFG3,  0x83}, // Modem configuration
    {CC1101_MDMCFG2,  0x73}, // MSK, 30/32 sync word bits detected
    {CC1101_MDMCFG1,  0x22}, // Modem configuration
    {CC1101_MDMCFG0,  0xF8}, // Modem configuration
    {CC1101_DEVIATN,  0x00}, // Modem deviation (not used in MSK)
    {CC1101_MCSM2,    0x07}, // Main Radio Control State Machine
    {CC1101_MCSM1,    0x30}, // Main Radio Control State Machine
    {CC1101_MCSM0,    0x18}, // Main Radio Control State Machine
    {CC1101_FOCCFG,   0x1D}, // Frequency Offset Compensation
    {CC1101_BSCFG,    0x1C}, // Bit Synchronization Configuration
    {CC1101_AGCCTRL2, 0xC7}, // AGC Control
    {CC1101_AGCCTRL1, 0x00}, // AGC Control
    {CC1101_AGCCTRL0, 0xB0}, // AGC Control
    {CC1101_WOREVT1,  0x87}, // High byte Event 0 timeout
    {CC1101_WOREVT0,  0x6B}, // Low byte Event 0 timeout
    {CC1101_WORCTRL,  0xFB}, // Wake On Radio control
    {CC1101_FREND1,   0xB6}, // Front end RX configuration
    {CC1101_FREND0,   0x10}, // Front end TX configuration
    {CC1101_FSCAL3,   0xEA}, // Frequency synthesizer calibration
    {CC1101_FSCAL2,   0x2A}, // Frequency synthesizer calibration
    {CC1101_FSCAL1,   0x00}, // Frequency synthesizer calibration
    {CC1101_FSCAL0,   0x1F}, // Frequency synthesizer calibration
    {CC1101_RCCTRL1,  0x41}, // RC oscillator configuration
    {CC1101_RCCTRL0,  0x00}, // RC oscillator configuration
    {0xFF, 0xFF} // End marker
};

static esp_err_t cc1101_apply_preset(const uint8_t preset[][2])
{
    extern esp_err_t cc1101_spi_write_reg(uint8_t reg, uint8_t value);
    
    for (int i = 0; preset[i][0] != 0xFF; i++) {
        esp_err_t ret = cc1101_spi_write_reg(preset[i][0], preset[i][1]);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to write register 0x%02X", preset[i][0]);
            return ret;
        }
    }
    
    return ESP_OK;
}

esp_err_t cc1101_load_preset_ask_ook(uint32_t frequency)
{
    ESP_LOGI(TAG, "Loading ASK/OOK preset for %u Hz", frequency);
    
    esp_err_t ret = cc1101_apply_preset(cc1101_preset_ask_ook_433);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Set the specific frequency
    return cc1101_set_frequency(frequency);
}

esp_err_t cc1101_load_preset_gfsk(uint32_t frequency)
{
    ESP_LOGI(TAG, "Loading GFSK preset for %u Hz", frequency);
    
    esp_err_t ret = cc1101_apply_preset(cc1101_preset_gfsk_433);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Set the specific frequency
    return cc1101_set_frequency(frequency);
}

esp_err_t cc1101_load_preset_msk(uint32_t frequency)
{
    ESP_LOGI(TAG, "Loading MSK preset for %u Hz", frequency);
    
    esp_err_t ret = cc1101_apply_preset(cc1101_preset_msk_433);
    if (ret != ESP_OK) {
        return ret;
    }
    
    // Set the specific frequency
    return cc1101_set_frequency(frequency);
}

esp_err_t cc1101_config_load_preset(const char *preset_name)
{
    if (!preset_name) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (strcmp(preset_name, "ask_ook_433") == 0) {
        return cc1101_load_preset_ask_ook(CC1101_FREQ_433MHZ);
    } else if (strcmp(preset_name, "gfsk_433") == 0) {
        return cc1101_load_preset_gfsk(CC1101_FREQ_433MHZ);
    } else if (strcmp(preset_name, "msk_433") == 0) {
        return cc1101_load_preset_msk(CC1101_FREQ_433MHZ);
    } else if (strcmp(preset_name, "ask_ook_315") == 0) {
        return cc1101_load_preset_ask_ook(CC1101_FREQ_315MHZ);
    } else if (strcmp(preset_name, "gfsk_315") == 0) {
        return cc1101_load_preset_gfsk(CC1101_FREQ_315MHZ);
    } else if (strcmp(preset_name, "ask_ook_868") == 0) {
        return cc1101_load_preset_ask_ook(CC1101_FREQ_868MHZ);
    } else if (strcmp(preset_name, "gfsk_868") == 0) {
        return cc1101_load_preset_gfsk(CC1101_FREQ_868MHZ);
    } else if (strcmp(preset_name, "ask_ook_915") == 0) {
        return cc1101_load_preset_ask_ook(CC1101_FREQ_915MHZ);
    } else if (strcmp(preset_name, "gfsk_915") == 0) {
        return cc1101_load_preset_gfsk(CC1101_FREQ_915MHZ);
    } else {
        ESP_LOGE(TAG, "Unknown preset: %s", preset_name);
        return ESP_ERR_NOT_FOUND;
    }
}