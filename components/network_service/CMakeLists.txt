idf_component_register(SRCS "network_service.c"
                       "wifi_manager.c" 
                       "web_server.c"
                       "web_ide.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_wifi esp_http_server esp_netif lwip json)