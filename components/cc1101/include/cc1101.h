/**
 * @file cc1101.h
 * @brief CC1101 Sub-GHz Transceiver Driver
 * 
 * Driver for Texas Instruments CC1101 Sub-GHz RF transceiver
 * Supporting frequencies: 315MHz, 433MHz, 868MHz, 915MHz
 */

#ifndef CC1101_H
#define CC1101_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// CC1101 Register addresses
#define CC1101_IOCFG2       0x00    // GDO2 output pin configuration
#define CC1101_IOCFG1       0x01    // GDO1 output pin configuration
#define CC1101_IOCFG0       0x02    // GDO0 output pin configuration
#define CC1101_FIFOTHR      0x03    // RX FIFO and TX FIFO thresholds
#define CC1101_SYNC1        0x04    // Sync word, high byte
#define CC1101_SYNC0        0x05    // Sync word, low byte
#define CC1101_PKTLEN       0x06    // Packet length
#define CC1101_PKTCTRL1     0x07    // Packet automation control
#define CC1101_PKTCTRL0     0x08    // Packet automation control
#define CC1101_ADDR         0x09    // Device address
#define CC1101_CHANNR       0x0A    // Channel number
#define CC1101_FSCTRL1      0x0B    // Frequency synthesizer control
#define CC1101_FSCTRL0      0x0C    // Frequency synthesizer control
#define CC1101_FREQ2        0x0D    // Frequency control word, high byte
#define CC1101_FREQ1        0x0E    // Frequency control word, middle byte
#define CC1101_FREQ0        0x0F    // Frequency control word, low byte
#define CC1101_MDMCFG4      0x10    // Modem configuration
#define CC1101_MDMCFG3      0x11    // Modem configuration
#define CC1101_MDMCFG2      0x12    // Modem configuration
#define CC1101_MDMCFG1      0x13    // Modem configuration
#define CC1101_MDMCFG0      0x14    // Modem configuration
#define CC1101_DEVIATN      0x15    // Modem deviation setting
#define CC1101_MCSM2        0x16    // Main Radio Control State Machine configuration
#define CC1101_MCSM1        0x17    // Main Radio Control State Machine configuration
#define CC1101_MCSM0        0x18    // Main Radio Control State Machine configuration
#define CC1101_FOCCFG       0x19    // Frequency Offset Compensation configuration
#define CC1101_BSCFG        0x1A    // Bit Synchronization configuration
#define CC1101_AGCCTRL2     0x1B    // AGC control
#define CC1101_AGCCTRL1     0x1C    // AGC control
#define CC1101_AGCCTRL0     0x1D    // AGC control
#define CC1101_WOREVT1      0x1E    // High byte Event 0 timeout
#define CC1101_WOREVT0      0x1F    // Low byte Event 0 timeout
#define CC1101_WORCTRL      0x20    // Wake On Radio control
#define CC1101_FREND1       0x21    // Front end RX configuration
#define CC1101_FREND0       0x22    // Front end TX configuration
#define CC1101_FSCAL3       0x23    // Frequency synthesizer calibration
#define CC1101_FSCAL2       0x24    // Frequency synthesizer calibration
#define CC1101_FSCAL1       0x25    // Frequency synthesizer calibration
#define CC1101_FSCAL0       0x26    // Frequency synthesizer calibration
#define CC1101_RCCTRL1      0x27    // RC oscillator configuration
#define CC1101_RCCTRL0      0x28    // RC oscillator configuration

// Command strobes
#define CC1101_SRES         0x30    // Reset chip
#define CC1101_SFSTXON      0x31    // Enable and calibrate frequency synthesizer
#define CC1101_SXOFF        0x32    // Turn off crystal oscillator
#define CC1101_SCAL         0x33    // Calibrate frequency synthesizer and turn it off
#define CC1101_SRX          0x34    // Enable RX
#define CC1101_STX          0x35    // Enable TX
#define CC1101_SIDLE        0x36    // Exit RX / TX, turn off frequency synthesizer
#define CC1101_SWOR         0x38    // Start automatic RX polling sequence
#define CC1101_SPWD         0x39    // Enter power down mode when CSn goes high
#define CC1101_SFRX         0x3A    // Flush the RX FIFO buffer
#define CC1101_SFTX         0x3B    // Flush the TX FIFO buffer
#define CC1101_SWORRST      0x3C    // Reset real time clock
#define CC1101_SNOP         0x3D    // No operation

// Status registers
#define CC1101_PARTNUM      0x30
#define CC1101_VERSION      0x31
#define CC1101_FREQEST      0x32
#define CC1101_LQI          0x33
#define CC1101_RSSI         0x34
#define CC1101_MARCSTATE    0x35
#define CC1101_WORTIME1     0x36
#define CC1101_WORTIME0     0x37
#define CC1101_PKTSTATUS    0x38
#define CC1101_VCO_VC_DAC   0x39
#define CC1101_TXBYTES      0x3A
#define CC1101_RXBYTES      0x3B

// FIFO access
#define CC1101_TXFIFO       0x3F
#define CC1101_RXFIFO       0x3F

// Modulation types
typedef enum {
    CC1101_MODULATION_2FSK = 0,
    CC1101_MODULATION_GFSK = 1,
    CC1101_MODULATION_ASK_OOK = 3,
    CC1101_MODULATION_4FSK = 4,
    CC1101_MODULATION_MSK = 7
} cc1101_modulation_t;

// Radio states
typedef enum {
    CC1101_STATE_IDLE = 0,
    CC1101_STATE_RX = 1,
    CC1101_STATE_TX = 2,
    CC1101_STATE_FSTXON = 3,
    CC1101_STATE_CALIBRATE = 4,
    CC1101_STATE_SETTLING = 5,
    CC1101_STATE_RXFIFO_OVERFLOW = 6,
    CC1101_STATE_TXFIFO_UNDERFLOW = 7
} cc1101_state_t;

// Configuration structure
typedef struct {
    spi_device_handle_t spi_device;
    int pin_gdo0;
    int pin_gdo2;
    uint32_t frequency_hz;
    cc1101_modulation_t modulation;
    uint32_t data_rate;
    uint32_t bandwidth;
    uint8_t sync_word[2];
    uint8_t device_address;
    bool packet_mode;
    uint8_t packet_length;
} cc1101_config_t;

// Signal data structure
typedef struct {
    uint32_t frequency;
    int16_t rssi;
    uint8_t lqi;
    uint8_t length;
    uint8_t data[64];
    uint32_t timestamp;
} cc1101_signal_t;

// Callback types
typedef void (*cc1101_rx_callback_t)(const cc1101_signal_t *signal, void *user_data);
typedef void (*cc1101_tx_callback_t)(bool success, void *user_data);

/**
 * @brief Initialize CC1101 module
 * @param config Configuration structure
 * @return ESP_OK on success
 */
esp_err_t cc1101_init(const cc1101_config_t *config);

/**
 * @brief Deinitialize CC1101 module
 * @return ESP_OK on success
 */
esp_err_t cc1101_deinit(void);

/**
 * @brief Reset CC1101 module
 * @return ESP_OK on success
 */
esp_err_t cc1101_reset(void);

/**
 * @brief Set frequency in Hz
 * @param frequency_hz Frequency in Hz
 * @return ESP_OK on success
 */
esp_err_t cc1101_set_frequency(uint32_t frequency_hz);

/**
 * @brief Get current frequency in Hz
 * @return Current frequency in Hz
 */
uint32_t cc1101_get_frequency(void);

/**
 * @brief Set modulation type
 * @param modulation Modulation type
 * @return ESP_OK on success
 */
esp_err_t cc1101_set_modulation(cc1101_modulation_t modulation);

/**
 * @brief Set data rate
 * @param data_rate Data rate in bps
 * @return ESP_OK on success
 */
esp_err_t cc1101_set_data_rate(uint32_t data_rate);

/**
 * @brief Set bandwidth
 * @param bandwidth Bandwidth in Hz
 * @return ESP_OK on success
 */
esp_err_t cc1101_set_bandwidth(uint32_t bandwidth);

/**
 * @brief Enter receive mode
 * @return ESP_OK on success
 */
esp_err_t cc1101_start_receive(void);

/**
 * @brief Exit receive mode
 * @return ESP_OK on success
 */
esp_err_t cc1101_stop_receive(void);

/**
 * @brief Transmit data
 * @param data Data to transmit
 * @param length Data length
 * @return ESP_OK on success
 */
esp_err_t cc1101_transmit(const uint8_t *data, uint8_t length);

/**
 * @brief Read received data
 * @param signal Pointer to signal structure
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if no data
 */
esp_err_t cc1101_read_signal(cc1101_signal_t *signal);

/**
 * @brief Get current RSSI
 * @return RSSI in dBm
 */
int16_t cc1101_get_rssi(void);

/**
 * @brief Get current radio state
 * @return Current radio state
 */
cc1101_state_t cc1101_get_state(void);

/**
 * @brief Set RX callback
 * @param callback Callback function
 * @param user_data User data
 */
void cc1101_set_rx_callback(cc1101_rx_callback_t callback, void *user_data);

/**
 * @brief Set TX callback
 * @param callback Callback function
 * @param user_data User data
 */
void cc1101_set_tx_callback(cc1101_tx_callback_t callback, void *user_data);

/**
 * @brief Get CC1101 version
 * @return Version byte
 */
uint8_t cc1101_get_version(void);

/**
 * @brief Check if module is present and responding
 * @return true if module is present
 */
bool cc1101_is_present(void);

// Pre-defined frequency configurations
#define CC1101_FREQ_315MHZ  315000000
#define CC1101_FREQ_433MHZ  433920000
#define CC1101_FREQ_868MHZ  868300000
#define CC1101_FREQ_915MHZ  915000000

// Pre-defined configurations for common protocols
esp_err_t cc1101_load_preset_ask_ook(uint32_t frequency);
esp_err_t cc1101_load_preset_gfsk(uint32_t frequency);
esp_err_t cc1101_load_preset_msk(uint32_t frequency);

#ifdef __cplusplus
}
#endif

#endif // CC1101_H