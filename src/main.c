#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ina219.h>
#include <string.h>
#include <esp_log.h>
#include <assert.h>
#include "wifi.h"
#include "spiffs.h"

#define I2C_PORT 0

#define I2C_SCL 22
#define I2C_SDA 21

#define INA219_1_ADDR 0x40
#define INA219_2_ADDR 0x44

#define SHUNT_MOHM 100
#define MAX_CUR 1




void ina_measure(void *addr)
{
    ina219_t dev;
    memset(&dev, 0, sizeof(ina219_t));
    uint8_t* address;
    address = (uint8_t *)&addr;

    assert(SHUNT_MOHM > 0);
    ESP_ERROR_CHECK(ina219_init_desc(&dev, *address, I2C_PORT, I2C_SDA, I2C_SCL));
    ESP_LOGI(TAG, "Initializing INA219");
    ESP_ERROR_CHECK(ina219_init(&dev));

    ESP_LOGI(TAG, "Configuring INA219");
    ESP_ERROR_CHECK(ina219_configure(&dev, INA219_BUS_RANGE_16V, INA219_GAIN_0_125,
            INA219_RES_12BIT_1S, INA219_RES_12BIT_1S, INA219_MODE_CONT_SHUNT_BUS));

    ESP_LOGI(TAG, "Calibrating INA219");

    ESP_ERROR_CHECK(ina219_calibrate(&dev, (float)MAX_CUR, (float)SHUNT_MOHM / 1000.0f));

    float bus_voltage, shunt_voltage, current, power;

    ESP_LOGI(TAG, "Starting the loop");
    while (1)
    {
        ESP_ERROR_CHECK(ina219_get_bus_voltage(&dev, &bus_voltage));
        ESP_ERROR_CHECK(ina219_get_shunt_voltage(&dev, &shunt_voltage));
        ESP_ERROR_CHECK(ina219_get_current(&dev, &current));
        ESP_ERROR_CHECK(ina219_get_power(&dev, &power));
        /* Using float in printf() requires non-default configuration in
         * sdkconfig. See sdkconfig.defaults.esp32 and
         * sdkconfig.defaults.esp8266  */
        printf("VBUS: %.04f V, VSHUNT: %.04f mV, IBUS: %.04f mA, PBUS: %.04f mW\n",
                bus_voltage, shunt_voltage * 1000, current * 1000, power * 1000);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main()
{
    //ESP_ERROR_CHECK(i2cdev_init());
    wifi_init();
    setINA219_1(0.1,0.1,0.1);
    setINA219_2(0.2,0.2,0.2);
    spiffs_init();
    spiffs_test();
    //xTaskCreate(ina_measure, "INA219_1", configMINIMAL_STACK_SIZE * 8, (void*)INA219_1_ADDR, 5, NULL);
}
