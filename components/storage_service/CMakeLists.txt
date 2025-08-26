idf_component_register(SRCS "storage_service.c"
                       "fs_manager.c"
                       "config_manager.c"
                       INCLUDE_DIRS "include"
                       REQUIRES spiffs nvs_flash fatfs)