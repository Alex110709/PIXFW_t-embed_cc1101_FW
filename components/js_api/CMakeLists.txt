idf_component_register(SRCS "js_api.c"
                       "js_rf_api.c"
                       "js_gpio_api.c"
                       "js_ui_api.c"
                       "js_storage_api.c"
                       "js_notification_api.c"
                       "js_wifi_api.c"
                       INCLUDE_DIRS "include"
                       REQUIRES mjs_engine cc1101 lvgl_port nvs_flash spiffs network_service)