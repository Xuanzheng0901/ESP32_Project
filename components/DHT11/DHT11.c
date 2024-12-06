#include <stdio.h>
#include "DHT11.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "OLED.h"

#define DHT_IO 0

static void DHT_Start(void);
static void DHT_Get_Data(DHT *a);
DHT aaa, *DHT_Structure;
extern TaskHandle_t led_handle, DHT_Handle;

void DHT_Sensor_Init(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT_OD;
    io_conf.pin_bit_mask = 1ULL<<DHT_IO;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

void DHT_TaskHandle(void *pvParameters)
{   
    OLED2_ShowIcon(3, 1, 5);//
    OLED2_ShowChar(3, 5, '.');
    OLED2_ShowChar(3, 7, 95 + ' ');
    OLED2_ShowIcon(3, 10, 4);//
    OLED2_ShowChar(3, 14, '.');
    OLED2_ShowChar(3, 16, '%');
    while(1)
    {
        DHT_Structure = Get_Temp_Humi();
        // if(DHT_Structure->temperature_h < 0)
        //     OLED2_ShowChar(3, 6-3, '-');
        // else OLED2_ShowChar(3, 6-3, ' ');
        OLED2_ShowNum(3, 3, DHT_Structure->temperature_h, 2);
        OLED2_ShowNum(3, 6, DHT_Structure->temperature_l, 1);
        OLED2_ShowNum(3, 12, DHT_Structure->humidity_h, 2);
        OLED2_ShowNum(3, 15, DHT_Structure->humidity_l, 1);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

DHT *Get_Temp_Humi(void)
{
    DHT_Start();
    esp_rom_delay_us(180);
    DHT_Get_Data(&aaa);
    return &aaa;
}

static void DHT_Start(void)
{
    gpio_set_level(DHT_IO, 0);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    gpio_set_level(DHT_IO, 1);
    esp_rom_delay_us(15);
}

static void DHT_Get_Data(DHT *a)
{
    uint8_t temp[40];
    uint16_t jishi = 0;
    vTaskSuspend(led_handle);
    for(int i = 0; i < 40; i++)
    {
        while(gpio_get_level(DHT_IO) == 0)
        {
            esp_rom_delay_us(1);
        }
        esp_rom_delay_us(40);
        temp[i] = gpio_get_level(DHT_IO);
        while(gpio_get_level(DHT_IO) == 1)
        {
            esp_rom_delay_us(1);
            jishi++;
            if(jishi > 1000)
            break;
        }
    }
    vTaskResume(led_handle);
    uint8_t flag = 1;
    if(temp[24] == 1)
    {
        flag = -1;
        temp[24] = 0;
    }

    uint8_t data[5];

    for(int i = 0; i < 5; i++)
    {
        uint8_t num = 0;
        for(int j = 0; j < 8; j++)
        {
            num = num * 2 + temp[i*8+j];
        }
        data[i] = num;
    }
    if(data[4] == data[0] + data[1] + data[2] + data[3])
    {
        data[2] *= flag;
        a->temperature_h = data[2];
        a->temperature_l = data[3];
        a->humidity_h = data[0];
        a->humidity_l = data[1];
    }
}