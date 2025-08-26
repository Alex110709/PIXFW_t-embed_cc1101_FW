/**
 * @file display_driver.c
 * @brief LCD Display Driver for T-Embed
 */

#include "lvgl_port.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "DISP_DRV";

// T-Embed LCD pins (from hw_init.h)
extern const int TEMBED_LCD_MOSI;
extern const int TEMBED_LCD_CLK;
extern const int TEMBED_LCD_CS;
extern const int TEMBED_LCD_DC;
extern const int TEMBED_LCD_RST;

static lv_disp_draw_buf_t s_disp_buf;
static lv_color_t *s_buf1 = NULL;
static lv_color_t *s_buf2 = NULL;
static lv_disp_drv_t s_disp_drv;
static spi_device_handle_t s_spi_device = NULL;

// ST7789 Commands
#define ST7789_SWRESET     0x01
#define ST7789_SLPOUT      0x11
#define ST7789_COLMOD      0x3A
#define ST7789_MADCTL      0x36
#define ST7789_CASET       0x2A
#define ST7789_RASET       0x2B
#define ST7789_RAMWR       0x2C
#define ST7789_DISPON      0x29
#define ST7789_INVON       0x21

// ST7789 initialization sequence
static const uint8_t st7789_init_sequence[] = {
    ST7789_SWRESET, 0,
    ST7789_SLPOUT, 0,
    ST7789_COLMOD, 1, 0x55,    // 16-bit color
    ST7789_MADCTL, 1, 0x00,    // Memory access control
    ST7789_INVON, 0,           // Invert colors
    ST7789_DISPON, 0,          // Display on
    0x00 // End
};

static void lcd_cmd(uint8_t cmd)
{
    gpio_set_level(TEMBED_LCD_DC, 0); // Command mode
    
    spi_transaction_t trans = {
        .length = 8,
        .tx_data = {cmd, 0, 0, 0},
        .flags = SPI_TRANS_USE_TXDATA
    };
    
    spi_device_transmit(s_spi_device, &trans);
}

static void lcd_data(uint8_t data)
{
    gpio_set_level(TEMBED_LCD_DC, 1); // Data mode
    
    spi_transaction_t trans = {
        .length = 8,
        .tx_data = {data, 0, 0, 0},
        .flags = SPI_TRANS_USE_TXDATA
    };
    
    spi_device_transmit(s_spi_device, &trans);
}

static void lcd_data_buf(const uint8_t *data, size_t len)
{
    if (len == 0) return;
    
    gpio_set_level(TEMBED_LCD_DC, 1); // Data mode
    
    spi_transaction_t trans = {
        .length = len * 8,
        .tx_buffer = data
    };
    
    spi_device_transmit(s_spi_device, &trans);
}

static void lcd_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    // Column address set
    lcd_cmd(ST7789_CASET);
    lcd_data(x0 >> 8);
    lcd_data(x0 & 0xFF);
    lcd_data(x1 >> 8);
    lcd_data(x1 & 0xFF);
    
    // Row address set
    lcd_cmd(ST7789_RASET);
    lcd_data(y0 >> 8);
    lcd_data(y0 & 0xFF);
    lcd_data(y1 >> 8);
    lcd_data(y1 & 0xFF);
    
    // Memory write
    lcd_cmd(ST7789_RAMWR);
}

static void lcd_init(void)
{
    ESP_LOGI(TAG, "Initializing LCD");
    
    // Reset LCD
    gpio_set_level(TEMBED_LCD_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TEMBED_LCD_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // Send initialization sequence
    const uint8_t *cmd = st7789_init_sequence;
    while (*cmd) {
        uint8_t command = *cmd++;
        uint8_t argc = *cmd++;
        
        lcd_cmd(command);
        for (int i = 0; i < argc; i++) {
            lcd_data(*cmd++);
        }
        
        // Some commands need delay
        if (command == ST7789_SWRESET || command == ST7789_SLPOUT) {
            vTaskDelay(pdMS_TO_TICKS(120));
        }
    }
    
    ESP_LOGI(TAG, "LCD initialized");
}

static void disp_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    int32_t x1 = area->x1;
    int32_t x2 = area->x2;
    int32_t y1 = area->y1;
    int32_t y2 = area->y2;
    
    // Set address window
    lcd_set_addr_window(x1, y1, x2, y2);
    
    // Calculate data size
    size_t size = (x2 - x1 + 1) * (y2 - y1 + 1) * 2; // 2 bytes per pixel (RGB565)
    
    // Send pixel data
    lcd_data_buf((uint8_t*)color_p, size);
    
    // Inform LVGL that flushing is done
    lv_disp_flush_ready(disp_drv);
}

esp_err_t display_driver_init(lv_disp_t **disp)
{
    if (!disp) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Initializing display driver");
    
    // Get SPI handle from hardware layer
    extern hw_handles_t* hw_get_handles(void);
    hw_handles_t *handles = hw_get_handles();
    if (!handles || !handles->lcd_spi) {
        ESP_LOGE(TAG, "LCD SPI handle not available");
        return ESP_ERR_INVALID_STATE;
    }
    
    s_spi_device = handles->lcd_spi;
    
    // Initialize LCD
    lcd_init();
    
    // Allocate display buffers
    size_t buf_size = LCD_WIDTH * LCD_HEIGHT / 10; // 1/10 of screen size
    
    s_buf1 = heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_DMA);
    if (!s_buf1) {
        ESP_LOGE(TAG, "Failed to allocate display buffer 1");
        return ESP_ERR_NO_MEM;
    }
    
    s_buf2 = heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_DMA);
    if (!s_buf2) {
        ESP_LOGE(TAG, "Failed to allocate display buffer 2");
        free(s_buf1);
        return ESP_ERR_NO_MEM;
    }
    
    // Initialize display buffer
    lv_disp_draw_buf_init(&s_disp_buf, s_buf1, s_buf2, buf_size);
    
    // Initialize display driver
    lv_disp_drv_init(&s_disp_drv);
    s_disp_drv.hor_res = LCD_WIDTH;
    s_disp_drv.ver_res = LCD_HEIGHT;
    s_disp_drv.flush_cb = disp_flush_cb;
    s_disp_drv.draw_buf = &s_disp_buf;
    s_disp_drv.rotated = LCD_ROTATION;
    
    // Register display driver
    *disp = lv_disp_drv_register(&s_disp_drv);
    if (!*disp) {
        ESP_LOGE(TAG, "Failed to register display driver");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Display driver initialized successfully");
    return ESP_OK;
}

void display_driver_deinit(void)
{
    ESP_LOGI(TAG, "Deinitializing display driver");
    
    // Free buffers
    if (s_buf1) {
        free(s_buf1);
        s_buf1 = NULL;
    }
    
    if (s_buf2) {
        free(s_buf2);
        s_buf2 = NULL;
    }
    
    s_spi_device = NULL;
    
    ESP_LOGI(TAG, "Display driver deinitialized");
}