#include <stdio.h>
#include "LED.h"
#include "driver/ledc.h"
#include "led_strip.h"

static led_strip_handle_t led_strip;
extern TaskHandle_t led_handle;

uint8_t R = 255, G = 0, B = 0, color = 1, led_enable = 1;

void led(void *pvParameters)
{
    while(1)
    {
        while(!led_enable)
            vTaskDelay(10);
        //1红(255,0,0) 2黄(255,255,0) 3绿(0,255,0) 
        //4青(0,255,255) 5蓝(0,0,255) 6紫(255,0,255)
        if(color == 1)
        {
            G++;
            if(G == 255)
                color++;
        }
        else if(color == 2)
        {
            R--;
            if(R == 0)
                color++;
        }
        else if(color == 3)
        {
            B++;
            if(B == 255)
                color++;
        }
        else if(color == 4)
        {
            G--;
            if(G == 0)
                color++;
        }
        else if(color == 5)
        {
            R++;
            if(R == 255)
                color++;
        }
        else if(color == 6)
        {
            B--;
            if(B == 0)
                color = 1;
        }
        led_strip_set_pixel(led_strip, 0, R, G, B);
        led_strip_refresh(led_strip);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void LED_Init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = 8,
        .max_leds = 1,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
}

void LED_Close(void)
{
    led_enable = 0;
    vTaskDelay(2);
    led_strip_set_pixel(led_strip, 0, 0, 0, 0);
    led_strip_refresh(led_strip);
}

void LED_Restart(void)
{
    led_enable = 1;
}

void LED_Warning(void)
{
    LED_Close();
    led_strip_set_pixel(led_strip, 0, 255, 0, 0);
    led_strip_refresh(led_strip);
}