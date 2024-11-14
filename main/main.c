#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "OLED.h"
#include "LED.h"
#include "DHT11.h"

TaskHandle_t led_handle = NULL, DHT_Handle = NULL;

void app_main(void)
{
    LED_Init();
    OLED_Init();
    DHT_Sensor_Init();
    
    
    xTaskCreate(led, "led", 4096, NULL, 5, &led_handle);
    xTaskCreate(DHT_TaskHandle, "DHT", 4096, NULL, 5, &DHT_Handle);

    while (1) 
    {
        vTaskDelay(100);
    }
}
