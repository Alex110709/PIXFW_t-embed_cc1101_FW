/**
 * @file hw_init.h
 * @brief Hardware Initialization for T-Embed CC1101
 */

#ifndef HW_INIT_H
#define HW_INIT_H

#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C" {
#endif

// T-Embed Hardware Pin Definitions
#define TEMBED_LCD_MOSI         35
#define TEMBED_LCD_CLK          36
#define TEMBED_LCD_CS           37
#define TEMBED_LCD_DC           4
#define TEMBED_LCD_RST          5
#define TEMBED_LCD_BL           38

#define TEMBED_ENCODER_A        1
#define TEMBED_ENCODER_B        2
#define TEMBED_ENCODER_KEY      0

#define TEMBED_BUTTON_1         21
#define TEMBED_BUTTON_2         14

#define TEMBED_POWER_ON         46
#define TEMBED_BATTERY_ADC      3

// CC1101 Pin Definitions
#define CC1101_SPI_HOST         SPI2_HOST
#define CC1101_MOSI             11
#define CC1101_MISO             13
#define CC1101_CLK              12
#define CC1101_CS               10
#define CC1101_GDO0             6
#define CC1101_GDO2             7

// I2C for potential expansion
#define I2C_MASTER_SCL_IO       9
#define I2C_MASTER_SDA_IO       8

typedef struct {
    spi_device_handle_t lcd_spi;
    spi_device_handle_t cc1101_spi;
} hw_handles_t;

/**
 * @brief Initialize all hardware components
 * @return ESP_OK on success
 */
esp_err_t hw_init(void);

/**
 * @brief Get hardware handles
 * @return Pointer to hardware handles structure
 */
hw_handles_t* hw_get_handles(void);

/**
 * @brief Initialize GPIO pins
 * @return ESP_OK on success
 */
esp_err_t hw_init_gpio(void);

/**
 * @brief Initialize SPI buses
 * @return ESP_OK on success
 */
esp_err_t hw_init_spi(void);

/**
 * @brief Initialize I2C bus
 * @return ESP_OK on success
 */
esp_err_t hw_init_i2c(void);

/**
 * @brief Initialize ADC for battery monitoring
 * @return ESP_OK on success
 */
esp_err_t hw_init_adc(void);

/**
 * @brief Get battery voltage in millivolts
 * @return Battery voltage in mV
 */
uint32_t hw_get_battery_voltage(void);

/**
 * @brief Control backlight brightness (0-255)
 * @param brightness Brightness level
 */
void hw_set_backlight(uint8_t brightness);

/**
 * @brief Control power state
 * @param power_on true to keep power on, false to allow sleep
 */
void hw_set_power_hold(bool power_on);

#ifdef __cplusplus
}
#endif

#endif // HW_INIT_H