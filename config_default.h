// This file will be set as a config file while building. In order to reset the config, remove config.h at lib/config

static const char *TAG = "SolarCharger";
#define ESP_WIFI_SSID      ""
#define ESP_WIFI_PASS      ""
#define ESP_MAXIMUM_RETRY  3
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#define DHCP true
#define PAGE_BUFF 2048 //safe value
#if (DHCP==false)
    #define IP "0.0.0.0"
    #define MASK "0.0.0.0"
    #define GATEWAY "0.0.0.0"
#endif