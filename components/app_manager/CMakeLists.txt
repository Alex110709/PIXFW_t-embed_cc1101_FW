idf_component_register(SRCS "app_manager.c"
                       "app_installer.c"
                       "app_sandbox.c"
                       "app_permissions.c"
                       INCLUDE_DIRS "include"
                       REQUIRES mjs_engine js_api spiffs nvs_flash)