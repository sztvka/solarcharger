// This file will be set as a config file while building. In order to reset the config, remove config.h at lib/config

static const char *TAG = "SolarCharger";
char *sites[] = {"/", "/main.js", "/styles.css", "/favicon.ico", "/chart.js", "/bootstrap.min.js", "/bootstrap-grid.min.css"};
#define ESP_WIFI_SSID      ""
#define ESP_WIFI_PASS      ""
#define ESP_MAXIMUM_RETRY  3
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#define DHCP true
#define PAGE_BUFF 2048 //safe value
#if (DHCP==false)
    #define IP_1 192
    #define IP_2 168
    #define IP_3 0
    #define IP_4 116
    #define GATEWAY_1 192
    #define GATEWAY_2 168
    #define GATEWAY_3 0
    #define GATEWAY_4 1
    #define MASK_1 255
    #define MASK_2 255
    #define MASK_3 255
    #define MASK_4 0
#endif