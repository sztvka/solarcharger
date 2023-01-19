idf_component_register(
        SRCS wifi.c
        INCLUDE_DIRS .
        REQUIRES config esp_wifi esp_event nvs_flash esp_http_server spiffs json esp_netif
)