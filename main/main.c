#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ina219.h>
#include <string.h>
#include <esp_log.h>
#include <assert.h>
#include <wifi.h>
#include <iot_servo.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"


#define I2C_PORT 0

#define I2C_SCL 22
#define I2C_SDA 21

//0 bottom left
//1 top left
//2 top right
//3 bottom right 


//w prawo plus w gore plus


// LDR1 33  bottom left      lewy gorny
// LDR2 32  top left         prawy gorny
// LDR3 35  top right        lewy dolny
// LDR4 34  bottom right     prawy dolny


#define LDR1 ADC_CHANNEL_4
#define LDR2 ADC_CHANNEL_5
#define LDR3 ADC_CHANNEL_6
#define LDR4 ADC_CHANNEL_7

#define ADC_SAMPLE_SIZE 64
//adc1 channel 4 5 6 7

static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static esp_adc_cal_characteristics_t *ldr_adc_chars;

#define SERVO_1 25
#define SERVO_2 26

#define LDR_DIFF_MOVE_TRESHOLD 100

const uint8_t INA219_1_ADDR = 0x40; //solar
const uint8_t INA219_2_ADDR = 0x44; //esp32

int servo1_angle = 90;
int servo2_angle = 179;

#define SHUNT_MOHM 100
#define MAX_CUR 1


servo_config_t servo_cfg = {
    .max_angle = 180,
    .min_width_us = 500,
    .max_width_us = 2500,
    .freq = 50,
    .timer_number = LEDC_TIMER_0,
    .channels = {
        .servo_pin = {
            SERVO_1,
            SERVO_2,
        },
        .ch = {
            LEDC_CHANNEL_0,
            LEDC_CHANNEL_1,
        },
    },
    .channel_number = 2,
} ;



void ina_measure(void *addr)
{
    TaskHandle_t cur = xTaskGetCurrentTaskHandle();
    char * name = pcTaskGetName(cur);
    ina219_t dev;
    memset(&dev, 0, sizeof(ina219_t));
    uint8_t *address = addr;
    assert(SHUNT_MOHM > 0);
    ESP_ERROR_CHECK(ina219_init_desc(&dev, *address, I2C_PORT, I2C_SDA, I2C_SCL));
    ESP_ERROR_CHECK(ina219_init(&dev));
    ESP_ERROR_CHECK(ina219_configure(&dev, INA219_BUS_RANGE_16V, INA219_GAIN_0_125,
            INA219_RES_12BIT_1S, INA219_RES_12BIT_1S, INA219_MODE_CONT_SHUNT_BUS));

    ESP_ERROR_CHECK(ina219_calibrate(&dev, (float)MAX_CUR, (float)SHUNT_MOHM / 1000.0f));

    float bus_voltage, current, power;

    while (1)
    {
        ESP_ERROR_CHECK(ina219_get_bus_voltage(&dev, &bus_voltage));
        ESP_ERROR_CHECK(ina219_get_current(&dev, &current));
        ESP_ERROR_CHECK(ina219_get_power(&dev, &power));


        if(strcmp(name, "INA219_1")==0) setINA219_1(bus_voltage, current*1000, power*1000); 
        if(strcmp(name, "INA219_2")==0) setINA219_2(bus_voltage, current*1000, power*1000); 
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void servo_loop(){
    while(true){
        for(int i = 0; i<90; i++){
            iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, i);
            iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 1, i);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

//servo 1 prawo lewo
//sevo 2 gora dol
//servo 2 po odwrocie dol gora


//wieksze od 0 ma isc w prawo
void adjustServos(int diff_vert, int diff_hor){
    if(abs(diff_vert)>=LDR_DIFF_MOVE_TRESHOLD){
        if(diff_vert>0 && servo2_angle>89) servo2_angle-=2;
        else if(diff_vert<0 && servo2_angle<179) servo2_angle+=2;
        iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 1, servo2_angle);
    }
    if(abs(diff_hor)>=LDR_DIFF_MOVE_TRESHOLD){
        if(diff_hor>0 && servo1_angle<179) servo1_angle+=2;
        else if(diff_hor<0 && servo1_angle>1) servo1_angle-=2;
        iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, servo1_angle);
    }
}


static void check_adc_efuse(void)
{
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("ADC eFuse Two Point: Supported\n");
    } else {
        printf("ADC eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("ADC eFuse Vref: Supported\n");
    } else {
        printf("ADC eFuse Vref: NOT supported\n");
    }
}


static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("ADC Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("ADC Characterized using eFuse Vref\n");
    } else {
        printf("ADC Characterized using Default Vref\n");
    }
}




void ldr_setup(){
    check_adc_efuse();

    adc1_config_width(width);
    adc1_config_channel_atten(LDR1, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(LDR2, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(LDR3, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(LDR4, ADC_ATTEN_DB_11);
    ldr_adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, width, 1100, ldr_adc_chars);
    print_char_val_type(val_type); 
}

void ldr_read(){
    //has to be moved here due to api not being thread-safe
    iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 0, servo1_angle);
    iot_servo_write_angle(LEDC_LOW_SPEED_MODE, 1, servo2_angle);
    while(1){
        uint32_t ldr_raw_reading[4] = {0};
        uint32_t ldr_voltage[4] = {0};
        for (int i = 0; i<ADC_SAMPLE_SIZE; i++){
            ldr_raw_reading[0] += adc1_get_raw((adc1_channel_t)LDR1);
            ldr_raw_reading[1] += adc1_get_raw((adc1_channel_t)LDR2);
            ldr_raw_reading[2] += adc1_get_raw((adc1_channel_t)LDR3);
            ldr_raw_reading[3] += adc1_get_raw((adc1_channel_t)LDR4);
        };
        
        for(int i = 0; i<4; i++){
            ldr_raw_reading[i] /= ADC_SAMPLE_SIZE;
            ldr_voltage[i] = esp_adc_cal_raw_to_voltage(ldr_raw_reading[i], ldr_adc_chars);
        };


    int avg_top = (ldr_voltage[0] + ldr_voltage[1]) / 2;
    int avg_bot = (ldr_voltage[2] + ldr_voltage[3]) / 2;
    int avg_left = (ldr_voltage[0] + ldr_voltage[2]) / 2;
    int avg_right = (ldr_voltage[1] + ldr_voltage[3]) / 2;



    int diff_vertical = avg_top-avg_bot; //  positive: top is brighter   negative: bottom is brighter
    int diff_horizontal = avg_left-avg_right; //positive left is brighter   negative right is brighter


   
    adjustServos(diff_vertical, diff_horizontal);
    vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void app_main()
{
    ESP_ERROR_CHECK(i2cdev_init());

    
    wifi_init();
    setINA219_1(0.1,0.1,0.1);
    setINA219_2(0.2,0.2,0.2);
    xTaskCreate(ina_measure, "INA219_1", configMINIMAL_STACK_SIZE * 8, (void*)&INA219_1_ADDR, 5, NULL);
    xTaskCreate(ina_measure, "INA219_2", configMINIMAL_STACK_SIZE * 8, (void*)&INA219_2_ADDR, 5, NULL);
    iot_servo_init(LEDC_LOW_SPEED_MODE, &servo_cfg);
//    xTaskCreate(servo_loop, "servo", configMINIMAL_STACK_SIZE*4, NULL, 4, NULL);
    ldr_setup();

    xTaskCreate(ldr_read, "ADC", configMINIMAL_STACK_SIZE*8, NULL, 4, NULL);

  //  xTaskCreate(servo, "ServoHandle", configMINIMAL_STACK_SIZE * 5, NULL, 4, NULL);
   // printf("Free memory left: %u", (unsigned int)esp_get_free_heap_size());

}
