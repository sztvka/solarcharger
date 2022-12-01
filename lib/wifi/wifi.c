#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <esp_http_server.h>



#include "cJSON.h"

#include "lwip/err.h"
#include "lwip/sys.h"


#include <config.h>

#include "wifi.h"

#include "esp_spiffs.h"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1



static int s_retry_num = 0;
int counter = 0;

struct INA219 {
    float voltage;
    float current;
    float wattage;
};

struct INA219 ina1;
struct INA219 ina2;

void setINA219_1(float voltage, float current, float wattage){
    ina1.voltage = voltage;
    ina1.current = current;
    ina1.wattage = wattage;
};

void setINA219_2(float voltage, float current, float wattage){
    ina2.voltage = voltage;
    ina2.current = current;
    ina2.wattage = wattage;
};


void spiffs_init(){
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = false
    };

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    // Note: esp_vfs_spiffs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }

}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *wifi_sta_conn = esp_netif_create_default_wifi_sta();
    if(DHCP==false){
        esp_netif_dhcpc_stop(wifi_sta_conn);

        esp_netif_ip_info_t ip_info;

        IP4_ADDR(&ip_info.ip, IP_1, IP_2, IP_3, IP_4);
        IP4_ADDR(&ip_info.gw, GATEWAY_1, GATEWAY_2, GATEWAY_3, GATEWAY_4);
        IP4_ADDR(&ip_info.netmask, MASK_1, MASK_2, MASK_3, MASK_4);

        esp_netif_set_ip_info(wifi_sta_conn, &ip_info);       
    }


    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
	     * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
          //  .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 ESP_WIFI_SSID, ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}







esp_err_t json_handler(httpd_req_t *req){
    cJSON *resp;
    resp = cJSON_CreateObject();

    cJSON *ina219_1;
    ina219_1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(ina219_1, "voltage", ina1.voltage);
    cJSON_AddNumberToObject(ina219_1, "current", ina1.current);
    cJSON_AddNumberToObject(ina219_1, "wattage", ina1.wattage);


    cJSON *ina219_2;
    ina219_2 = cJSON_CreateObject();
    cJSON_AddNumberToObject(ina219_2, "voltage", ina2.voltage);
    cJSON_AddNumberToObject(ina219_2, "current", ina2.current);
    cJSON_AddNumberToObject(ina219_2, "wattage", ina2.wattage);

    cJSON_AddItemToObject(resp, "ina1", ina219_1);
    cJSON_AddItemToObject(resp, "ina2", ina219_2);

    cJSON_AddNumberToObject(resp,"counter",counter);
    char * response = cJSON_Print(resp);
    httpd_resp_set_type(req, "applicaton/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t file_handler(httpd_req_t *req){
    char path[] = "/spiffs";
    char *buf = malloc(sizeof(path)+sizeof(req->uri));
    ESP_LOGI(TAG, "%d", strlen(req->uri));
    if(strlen(req->uri)==1) {
        free(buf);
        buf = "/spiffs/index.html";
    }
    else{
        sprintf(buf, "%s%s", path, req->uri);
    }

    ESP_LOGI(TAG, "%s", buf);
    FILE *file = fopen(buf, "r");
    if(file==NULL){
         httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get");
         return ESP_FAIL;
    }
    else{
        if(strstr(buf, ".ico")!=NULL) httpd_resp_set_type(req, "image/x-icon");
        else if(strstr(buf, ".css")!=NULL) httpd_resp_set_type(req, "text/css");       
        else if(strstr(buf, ".js")!=NULL) httpd_resp_set_type(req, "text/js");
        else httpd_resp_set_type(req, "text/html");
        char *chunk = malloc(PAGE_BUFF);
        size_t chunksize;
        do{
            chunksize = fread(chunk, 1, PAGE_BUFF, file);

        if (chunksize > 0) {
                if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                    fclose(file);
                    ESP_LOGE(TAG, "Request failed!");
                    httpd_resp_sendstr_chunk(req, NULL);
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get");
                return ESP_FAIL;
            }
            }

        
        }while(chunksize!=0);

        
        fclose(file);
        httpd_resp_send_chunk(req, NULL, 0);
        free(chunk);
        if(strlen(req->uri)!=1) free(buf);
        return ESP_OK;
    }

}






void http_start(void){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
   
    httpd_uri_t data = {
    .uri      = "/data",
    .method   = HTTP_GET,
    .handler  = json_handler,
    .user_ctx = NULL
    };

    httpd_uri_t html = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = file_handler,
    .user_ctx = NULL
    };

    httpd_uri_t js = {
    .uri      = "/main.js",
    .method   = HTTP_GET,
    .handler  = file_handler,
    .user_ctx = NULL
    };

    httpd_uri_t css = {
    .uri      = "/styles.css",
    .method   = HTTP_GET,
    .handler  = file_handler,
    .user_ctx = NULL
    };

    httpd_uri_t favicon = {
    .uri      = "/favicon.ico",
    .method   = HTTP_GET,
    .handler  = file_handler,
    .user_ctx = NULL
    };
    httpd_uri_t chart = {
    .uri      = "/chart.js",
    .method   = HTTP_GET,
    .handler  = file_handler,
    .user_ctx = NULL
    };


    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &data));
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &html));
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &js));
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &css));
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &favicon));
        ESP_ERROR_CHECK(httpd_register_uri_handler(server, &chart));
    }

}

void cntup(void* params){
    while(1){
    counter++;
    vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void wifi_init(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    http_start();
    spiffs_init();
    xTaskCreate(cntup,"cntTask",configMINIMAL_STACK_SIZE*3, NULL, 5, NULL);
}




/*
LEGACY HANDLERS 


esp_err_t html_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    char *response = return_html(PAGE_BUFF);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    free(response);
    return ESP_OK;
}

esp_err_t js_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/javascript");
    char *response = return_js(PAGE_BUFF);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    free(response);
    return ESP_OK;
}

esp_err_t css_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/css");
    char *response = return_css(PAGE_BUFF);
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    free(response);
    return ESP_OK;
}



esp_err_t favicon_handler(httpd_req_t *req){
    char path[] = "/spiffs";
    char *buf = malloc(sizeof(path)+sizeof(req->uri));
    if(strlen(req->uri)==1) {
        buf = "/spiffs/index.html";
    }
    else{
        
        //char buf[sizeof(path)+sizeof(req->uri)];
        sprintf(buf, "%s%s", path, req->uri);
    }

    ESP_LOGI(TAG, "%s", buf);
    FILE *file = fopen(buf, "r");
    if(file==NULL){
         httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get");
         return ESP_FAIL;
    }
    else{
        if(strstr(buf, ".ico")!=NULL) httpd_resp_set_type(req, "image/x-icon");
        else if(strstr(buf, ".css")!=NULL) httpd_resp_set_type(req, "text/css");       
        else if(strstr(buf, ".js")!=NULL) httpd_resp_set_type(req, "text/js");
        else httpd_resp_set_type(req, "text/html");
        char *chunk = malloc(PAGE_BUFF);
        size_t chunksize;
        do{
            chunksize = fread(chunk, 1, PAGE_BUFF, file);

        if (chunksize > 0) {
                if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                    fclose(file);
                    ESP_LOGE(TAG, "Request failed!");
                    httpd_resp_sendstr_chunk(req, NULL);
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get");
                return ESP_FAIL;
            }
            }

        
        }while(chunksize!=0);

        
        fclose(file);
        httpd_resp_send_chunk(req, NULL, 0);
        free(chunk);
        free(buf);
        return ESP_OK;
    }

}


esp_err_t html_handler(httpd_req_t *req){
    ESP_LOGI(TAG, "%d", strlen(req->uri));
    FILE *file = fopen("/spiffs/index.html", "r");
    if(file==NULL){
         httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get index.html");
         return ESP_FAIL;
    }
    else{
        httpd_resp_set_type(req, "text/html");
        char *chunk = malloc(PAGE_BUFF);
        size_t chunksize;
        do{
            chunksize = fread(chunk, 1, PAGE_BUFF, file);

        if (chunksize > 0) {
                if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                    fclose(file);
                    ESP_LOGE(TAG, "Request failed!");
                    httpd_resp_sendstr_chunk(req, NULL);
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get index.html");
                return ESP_FAIL;
            }
            }

        
        }while(chunksize!=0);

        
        fclose(file);
        httpd_resp_send_chunk(req, NULL, 0);
        free(chunk);
        return ESP_OK;
    }

}

esp_err_t js_handler(httpd_req_t *req){
    FILE *file = fopen("/spiffs/main.js", "r");
    if(file==NULL){
         httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get main.js");
         return ESP_FAIL;
    }
    else{
        httpd_resp_set_type(req, "text/js");
        char *chunk = malloc(PAGE_BUFF);
        size_t chunksize;
        do{
            chunksize = fread(chunk, 1, PAGE_BUFF, file);

        if (chunksize > 0) {
                if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                    fclose(file);
                    ESP_LOGE(TAG, "Request failed!");
                    httpd_resp_sendstr_chunk(req, NULL);
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get main.js");
                return ESP_FAIL;
            }
            }

        
        }while(chunksize!=0);

        
        fclose(file);
        httpd_resp_send_chunk(req, NULL, 0);
        free(chunk);
        return ESP_OK;
    }

}

esp_err_t css_handler(httpd_req_t *req){
    FILE *file = fopen("/spiffs/styles.css", "r");
    if(file==NULL){
         httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get styles.css");
         return ESP_FAIL;
    }
    else{
        httpd_resp_set_type(req, "text/css");
        char *chunk = malloc(PAGE_BUFF);
        size_t chunksize;
        do{
            chunksize = fread(chunk, 1, PAGE_BUFF, file);

        if (chunksize > 0) {
                if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                    fclose(file);
                    ESP_LOGE(TAG, "Request failed!");
                    httpd_resp_sendstr_chunk(req, NULL);
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get styles.css");
                return ESP_FAIL;
            }
            }

        
        }while(chunksize!=0);

        
        fclose(file);
        httpd_resp_send_chunk(req, NULL, 0);
        free(chunk);
        return ESP_OK;
    }

}

esp_err_t chartjs_handler(httpd_req_t *req){
    FILE *file = fopen("/spiffs/chart.js", "r");
    if(file==NULL){
         httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get chart.js");
         return ESP_FAIL;
    }
    else{
        httpd_resp_set_type(req, "text/js");
        char *chunk = malloc(PAGE_BUFF);
        size_t chunksize;
        do{
            chunksize = fread(chunk, 1, PAGE_BUFF, file);

        if (chunksize > 0) {
                if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                    fclose(file);
                    ESP_LOGE(TAG, "Request failed!");
                    httpd_resp_sendstr_chunk(req, NULL);
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get chart.js");
                return ESP_FAIL;
            }
            }

        
        }while(chunksize!=0);

        
        fclose(file);
        httpd_resp_send_chunk(req, NULL, 0);
        free(chunk);
        return ESP_OK;
    }

}




*/