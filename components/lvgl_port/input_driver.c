/**
 * @file input_driver.c
 * @brief Input Driver for T-Embed (Rotary Encoder and Buttons)
 */

#include "lvgl_port.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "INPUT_DRV";

// T-Embed input pins (from hw_init.h)
extern const int TEMBED_ENCODER_A;
extern const int TEMBED_ENCODER_B;
extern const int TEMBED_ENCODER_KEY;
extern const int TEMBED_BUTTON_1;
extern const int TEMBED_BUTTON_2;

static lv_indev_drv_t s_indev_drv;
static QueueHandle_t s_input_queue = NULL;
static TaskHandle_t s_input_task_handle = NULL;

// Input state tracking
typedef struct {
    bool encoder_key_pressed;
    bool button1_pressed;
    bool button2_pressed;
    int16_t encoder_diff;
    uint32_t last_encoder_time;
    uint8_t encoder_a_last;
    uint8_t encoder_b_last;
} input_state_t;

static input_state_t s_input_state = {0};
static input_callback_t s_callback = NULL;
static void *s_callback_user_data = NULL;

// Encoder interrupt handler
static void IRAM_ATTR encoder_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    
    // Read current encoder state
    uint8_t a = gpio_get_level(TEMBED_ENCODER_A);
    uint8_t b = gpio_get_level(TEMBED_ENCODER_B);
    
    // Determine rotation direction using Gray code
    if (gpio_num == TEMBED_ENCODER_A) {
        if (a != s_input_state.encoder_a_last) {
            if (a == b) {
                s_input_state.encoder_diff++;
            } else {
                s_input_state.encoder_diff--;
            }
        }
        s_input_state.encoder_a_last = a;
    }
    
    s_input_state.last_encoder_time = xTaskGetTickCountFromISR();
}

// Button interrupt handler
static void IRAM_ATTR button_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    input_data_t input_data = {0};
    
    input_data.timestamp = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;
    input_data.type = INPUT_TYPE_BUTTON;
    
    // Determine which button and state
    bool pressed = !gpio_get_level(gpio_num); // Active low
    
    if (gpio_num == TEMBED_ENCODER_KEY) {
        input_data.key_id = 0;
        s_input_state.encoder_key_pressed = pressed;
    } else if (gpio_num == TEMBED_BUTTON_1) {
        input_data.key_id = 1;
        s_input_state.button1_pressed = pressed;
    } else if (gpio_num == TEMBED_BUTTON_2) {
        input_data.key_id = 2;
        s_input_state.button2_pressed = pressed;
    }
    
    input_data.event = pressed ? INPUT_EVENT_PRESS : INPUT_EVENT_RELEASE;
    
    // Send to queue (from ISR)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(s_input_queue, &input_data, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// Input processing task
static void input_task(void *pvParameters)
{
    input_data_t input_data;
    uint32_t last_encoder_check = 0;
    
    ESP_LOGI(TAG, "Input task started");
    
    while (1) {
        // Check for encoder rotation
        uint32_t current_time = xTaskGetTickCount();
        if (current_time - last_encoder_check > pdMS_TO_TICKS(10)) {
            last_encoder_check = current_time;
            
            if (s_input_state.encoder_diff != 0) {
                input_data.type = INPUT_TYPE_ENCODER;
                input_data.timestamp = current_time * portTICK_PERIOD_MS;
                input_data.key_id = 0;
                
                if (s_input_state.encoder_diff > 0) {
                    input_data.event = INPUT_EVENT_ENCODER_CW;
                    s_input_state.encoder_diff--;
                } else {
                    input_data.event = INPUT_EVENT_ENCODER_CCW;
                    s_input_state.encoder_diff++;
                }
                
                // Send encoder event to callback
                if (s_callback) {
                    s_callback(&input_data, s_callback_user_data);
                }
            }
        }
        
        // Process button events from queue
        if (xQueueReceive(s_input_queue, &input_data, pdMS_TO_TICKS(10))) {
            if (s_callback) {
                s_callback(&input_data, s_callback_user_data);
            }
        }
    }
}

// LVGL input reading callback
static void indev_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    static bool encoder_key_last = false;
    
    // Handle encoder key as the main input for LVGL
    data->key = LV_KEY_ENTER;
    data->state = s_input_state.encoder_key_pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    
    // Handle encoder rotation for navigation
    if (s_input_state.encoder_diff > 0) {
        data->key = LV_KEY_RIGHT;
        data->state = LV_INDEV_STATE_PRESSED;
        s_input_state.encoder_diff--;
    } else if (s_input_state.encoder_diff < 0) {
        data->key = LV_KEY_LEFT;
        data->state = LV_INDEV_STATE_PRESSED;
        s_input_state.encoder_diff++;
    }
    
    // Continue reading if there's more encoder movement
    data->continue_reading = (s_input_state.encoder_diff != 0);
}

esp_err_t input_driver_init(lv_indev_t **indev)
{
    if (!indev) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Initializing input driver");
    
    // Create input queue
    s_input_queue = xQueueCreate(10, sizeof(input_data_t));
    if (!s_input_queue) {
        ESP_LOGE(TAG, "Failed to create input queue");
        return ESP_ERR_NO_MEM;
    }
    
    // Configure encoder interrupts
    gpio_config_t encoder_cfg = {
        .pin_bit_mask = (1ULL << TEMBED_ENCODER_A) | (1ULL << TEMBED_ENCODER_B),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&encoder_cfg));
    
    // Configure button interrupts
    gpio_config_t button_cfg = {
        .pin_bit_mask = (1ULL << TEMBED_ENCODER_KEY) | 
                       (1ULL << TEMBED_BUTTON_1) | 
                       (1ULL << TEMBED_BUTTON_2),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&button_cfg));
    
    // Install GPIO ISR service
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    
    // Add ISR handlers
    ESP_ERROR_CHECK(gpio_isr_handler_add(TEMBED_ENCODER_A, encoder_isr_handler, (void*)TEMBED_ENCODER_A));
    ESP_ERROR_CHECK(gpio_isr_handler_add(TEMBED_ENCODER_B, encoder_isr_handler, (void*)TEMBED_ENCODER_B));
    ESP_ERROR_CHECK(gpio_isr_handler_add(TEMBED_ENCODER_KEY, button_isr_handler, (void*)TEMBED_ENCODER_KEY));
    ESP_ERROR_CHECK(gpio_isr_handler_add(TEMBED_BUTTON_1, button_isr_handler, (void*)TEMBED_BUTTON_1));
    ESP_ERROR_CHECK(gpio_isr_handler_add(TEMBED_BUTTON_2, button_isr_handler, (void*)TEMBED_BUTTON_2));
    
    // Initialize encoder state
    s_input_state.encoder_a_last = gpio_get_level(TEMBED_ENCODER_A);
    s_input_state.encoder_b_last = gpio_get_level(TEMBED_ENCODER_B);
    
    // Create input processing task
    BaseType_t ret = xTaskCreate(input_task, "input_task", 2048, NULL, 10, &s_input_task_handle);
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create input task");
        return ESP_ERR_NO_MEM;
    }
    
    // Initialize LVGL input device driver
    lv_indev_drv_init(&s_indev_drv);
    s_indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    s_indev_drv.read_cb = indev_read_cb;
    
    // Register input device
    *indev = lv_indev_drv_register(&s_indev_drv);
    if (!*indev) {
        ESP_LOGE(TAG, "Failed to register input device");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Input driver initialized successfully");
    return ESP_OK;
}

void input_driver_deinit(void)
{
    ESP_LOGI(TAG, "Deinitializing input driver");
    
    // Remove ISR handlers
    gpio_isr_handler_remove(TEMBED_ENCODER_A);
    gpio_isr_handler_remove(TEMBED_ENCODER_B);
    gpio_isr_handler_remove(TEMBED_ENCODER_KEY);
    gpio_isr_handler_remove(TEMBED_BUTTON_1);
    gpio_isr_handler_remove(TEMBED_BUTTON_2);
    
    // Delete task
    if (s_input_task_handle) {
        vTaskDelete(s_input_task_handle);
        s_input_task_handle = NULL;
    }
    
    // Delete queue
    if (s_input_queue) {
        vQueueDelete(s_input_queue);
        s_input_queue = NULL;
    }
    
    // Clear callback
    s_callback = NULL;
    s_callback_user_data = NULL;
    
    ESP_LOGI(TAG, "Input driver deinitialized");
}

void input_driver_register_callback(input_callback_t callback, void *user_data)
{
    s_callback = callback;
    s_callback_user_data = user_data;
}