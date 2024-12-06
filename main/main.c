#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "OLED.h"
#include "LED.h"
#include "DHT11.h"
#include "WIFI.h"
#include "MQ-SmokeSensor.h"
#include "HTTP.h"
#include "BlueTooth.h"

TaskHandle_t led_handle = NULL, DHT_Handle = NULL, MQ2_Handle = NULL, Time_Task_Handle = NULL, Smoke_Warning_Handle = NULL;

extern uint8_t WIFI_STATU;

void app_main(void)
{
    LED_Init();
    OLED_Init();
    OLED2_Init();
    DHT_Sensor_Init();
    MQ2_Init();
    
    wifi_init_sta();


    xTaskCreate(led, "led", 4096, NULL, 5, &led_handle);
    xTaskCreate(DHT_TaskHandle, "DHT", 4096, NULL, 5, &DHT_Handle);
    xTaskCreate(Get_Smoke_Conc, "MQ2", 4096, NULL, 5, &MQ2_Handle);
    xTaskCreate(Smoke_Warning, "Warning", 4096, NULL, 5, &Smoke_Warning_Handle);
    
    while(!WIFI_STATU)
    {
        vTaskDelay(10);
    }
    
    HTTP_Time_Init();
    xTaskCreate(Time_Update, "Time", 4096, NULL, 4, &Time_Task_Handle);
    HTTP_Get_Weather("450323");
    BT_Init();
    
    while (1) 
    {
        vTaskDelay(100);
    }
}
