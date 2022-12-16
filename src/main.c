#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ina219.h>
#include <string.h>
#include <esp_log.h>
#include <assert.h>
#include <wifi.h>

#define I2C_PORT 0

#define I2C_SCL 22
#define I2C_SDA 21

const uint8_t INA219_1_ADDR = 0x40; //solar
const uint8_t INA219_2_ADDR = 0x44; //esp32


#define SHUNT_MOHM 100
#define MAX_CUR 1




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

void app_main()
{
    ESP_ERROR_CHECK(i2cdev_init());


    wifi_init();
    setINA219_1(0.1,0.1,0.1);
    setINA219_2(0.2,0.2,0.2);
    xTaskCreate(ina_measure, "INA219_1", configMINIMAL_STACK_SIZE * 8, (void*)&INA219_1_ADDR, 5, NULL);
    xTaskCreate(ina_measure, "INA219_2", configMINIMAL_STACK_SIZE * 8, (void*)&INA219_2_ADDR, 5, NULL);
    printf("Free memory left: %u", esp_get_free_heap_size());

}
