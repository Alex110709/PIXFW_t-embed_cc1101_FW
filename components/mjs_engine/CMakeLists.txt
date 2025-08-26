idf_component_register(SRCS "mjs_engine.c"
                       "mjs_native_api.c"
                       "mjs_module_loader.c"
                       "mjs_console.c"
                       "mjs/mjs.c"
                       INCLUDE_DIRS "include" "mjs"
                       REQUIRES driver spiffs nvs_flash)