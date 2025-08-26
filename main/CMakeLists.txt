idf_component_register(SRCS "main.c"
                       "system/system_manager.c"
                       "system/hw_init.c"
                       "system/task_manager.c"
                       INCLUDE_DIRS "."
                       REQUIRES 
                       cc1101 
                       mjs_engine 
                       lvgl_port 
                       app_manager 
                       js_api
                       nvs_flash
                       spiffs
                       esp_wifi
                       esp_http_server
                       driver
                       spi_flash)