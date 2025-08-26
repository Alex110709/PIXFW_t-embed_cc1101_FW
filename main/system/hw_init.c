/**
 * @file hw_init.c
 * @brief Hardware Initialization Implementation
 */

#include "hw_init.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG = "HW_INIT";

static hw_handles_t s_hw_handles = {0};
static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t s_adc_cali_handle = NULL;

esp_err_t hw_init_gpio(void)
{
    ESP_LOGI(TAG, "Initializing GPIO");

    // Configure power hold pin
    gpio_config_t pwr_cfg = {
        .pin_bit_mask = (1ULL << TEMBED_POWER_ON),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&pwr_cfg));
    gpio_set_level(TEMBED_POWER_ON, 1); // Keep power on

    // Configure button pins
    gpio_config_t btn_cfg = {
        .pin_bit_mask = (1ULL << TEMBED_ENCODER_KEY) | 
                       (1ULL << TEMBED_BUTTON_1) | 
                       (1ULL << TEMBED_BUTTON_2),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&btn_cfg));

    // Configure encoder pins
    gpio_config_t enc_cfg = {
        .pin_bit_mask = (1ULL << TEMBED_ENCODER_A) | (1ULL << TEMBED_ENCODER_B),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&enc_cfg));

    // Configure LCD control pins
    gpio_config_t lcd_cfg = {
        .pin_bit_mask = (1ULL << TEMBED_LCD_DC) | (1ULL << TEMBED_LCD_RST),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&lcd_cfg));

    // Configure CC1101 GDO pins
    gpio_config_t cc1101_cfg = {
        .pin_bit_mask = (1ULL << CC1101_GDO0) | (1ULL << CC1101_GDO2),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cc1101_cfg));

    return ESP_OK;
}

esp_err_t hw_init_spi(void)
{
    ESP_LOGI(TAG, "Initializing SPI buses");

    // LCD SPI Bus (SPI3)
    spi_bus_config_t lcd_bus_cfg = {
        .mosi_io_num = TEMBED_LCD_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = TEMBED_LCD_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4092
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &lcd_bus_cfg, SPI_DMA_CH_AUTO));

    // LCD Device
    spi_device_interface_config_t lcd_dev_cfg = {
        .clock_speed_hz = 40 * 1000 * 1000, // 40 MHz
        .mode = 0,
        .spics_io_num = TEMBED_LCD_CS,
        .queue_size = 1,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &lcd_dev_cfg, &s_hw_handles.lcd_spi));

    // CC1101 SPI Bus (SPI2)
    spi_bus_config_t cc1101_bus_cfg = {
        .mosi_io_num = CC1101_MOSI,
        .miso_io_num = CC1101_MISO,
        .sclk_io_num = CC1101_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 64
    };
    ESP_ERROR_CHECK(spi_bus_initialize(CC1101_SPI_HOST, &cc1101_bus_cfg, SPI_DMA_DISABLED));

    // CC1101 Device
    spi_device_interface_config_t cc1101_dev_cfg = {
        .clock_speed_hz = 8 * 1000 * 1000, // 8 MHz
        .mode = 0,
        .spics_io_num = CC1101_CS,
        .queue_size = 1,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(CC1101_SPI_HOST, &cc1101_dev_cfg, &s_hw_handles.cc1101_spi));

    return ESP_OK;
}

esp_err_t hw_init_i2c(void)
{
    ESP_LOGI(TAG, "Initializing I2C");

    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000, // 100 kHz
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_cfg));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

    return ESP_OK;
}

esp_err_t hw_init_adc(void)
{
    ESP_LOGI(TAG, "Initializing ADC for battery monitoring");

    // ADC1 Init
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &s_adc_handle));

    // ADC1 Config
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_11,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, ADC_CHANNEL_2, &config)); // GPIO3

    // ADC Calibration
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &s_adc_cali_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "ADC calibration curve fitting scheme initialized");
    }
#endif

    return ESP_OK;
}

static esp_err_t hw_init_backlight(void)
{
    ESP_LOGI(TAG, "Initializing backlight PWM");

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = TEMBED_LCD_BL,
        .duty = 128, // 50% brightness initially
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    return ESP_OK;
}

esp_err_t hw_init(void)
{
    ESP_LOGI(TAG, "Starting hardware initialization");

    ESP_ERROR_CHECK(hw_init_gpio());
    ESP_ERROR_CHECK(hw_init_spi());
    ESP_ERROR_CHECK(hw_init_i2c());
    ESP_ERROR_CHECK(hw_init_adc());
    ESP_ERROR_CHECK(hw_init_backlight());

    ESP_LOGI(TAG, "Hardware initialization complete");
    return ESP_OK;
}

hw_handles_t* hw_get_handles(void)
{
    return &s_hw_handles;
}

uint32_t hw_get_battery_voltage(void)
{
    if (!s_adc_handle) {
        return 0;
    }

    int adc_raw;
    esp_err_t ret = adc_oneshot_read(s_adc_handle, ADC_CHANNEL_2, &adc_raw);
    if (ret != ESP_OK) {
        return 0;
    }

    int voltage_mv = 0;
    if (s_adc_cali_handle) {
        adc_cali_raw_to_voltage(s_adc_cali_handle, adc_raw, &voltage_mv);
        // T-Embed has a voltage divider (1:1), so multiply by 2
        voltage_mv *= 2;
    }

    return voltage_mv;
}

void hw_set_backlight(uint8_t brightness)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void hw_set_power_hold(bool power_on)
{
    gpio_set_level(TEMBED_POWER_ON, power_on ? 1 : 0);
}