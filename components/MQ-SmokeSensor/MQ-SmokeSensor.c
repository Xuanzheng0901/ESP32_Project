#include "MQ-SmokeSensor.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "OLED.h"
#include "FreeRTOS/FreeRTOS.h"
#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "LED.h"

adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t cali_handle = NULL;
int adc_raw = 0, adc_voltage = 0;
extern TaskHandle_t MQ2_Handle;

const adc_oneshot_unit_init_cfg_t adc_init_config = 
{
    .unit_id = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
};

static void Beep(uint8_t flag)
{
    if(flag)
        ledc_timer_resume(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0);
    else
        ledc_timer_pause(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0);
}

void Get_Smoke_Conc(void *pvParameters)
{
    
	OLED2_ShowChar(4, 9, ':');
    OLED2_ShowString(4, 14, "ppm");
    while(1)
    {
        adc_oneshot_read(adc1_handle, ADC_CHANNEL_2, &adc_raw);

        adc_cali_raw_to_voltage(cali_handle, adc_raw, &adc_voltage);

        int true_voltage = ((double)adc_voltage) / (0.8f * (1- ((double)adc_voltage) / 4130.0f) + 1.0f);

        //printf("calied voltage:%d\n真实电压: %d\n", adc_voltage, true_voltage);
        int conc = (int)((double)true_voltage * (10000.0f/3300.0f));
        uint8_t len = GetNumLength(conc);
        if(len <= 4)
        {
            OLED2_ShowNum(4, 14-len, conc, len);
            for(int i = 0; i < 4-len; i++)
            {
                OLED2_ShowChar(4, 10+i, ' ');
            }
        }
        else 
        {
            OLED2_String(4, 10, 2, 15, 16);
            OLED2_ShowString(4, 14, "  ");
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void ADC_CALI(void)
{
    adc_cali_curve_fitting_config_t cali_config = 
    {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = 12,
        .chan = 2
    };
    adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
}

void Beep_Init(void)
{
    ledc_timer_config_t pwm_config = 
    {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = 0,
        .freq_hz = 700,
        .duty_resolution = 8,
        .clk_cfg = LEDC_USE_XTAL_CLK
    };
    ledc_timer_config(&pwm_config);
    ledc_timer_pause(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0);
    ledc_channel_config_t channel_config = 
    {
        .channel = 0,
        .duty = 128,
        .gpio_num = 4,
        .hpoint = 0,
        .flags.output_invert = 1,
        .timer_sel = LEDC_TIMER_0,
        .speed_mode = LEDC_LOW_SPEED_MODE,
    };
    ledc_channel_config(&channel_config);
}


void MQ2_Init(void)
{
    adc_oneshot_new_unit(&adc_init_config, &adc1_handle);
    adc_oneshot_chan_cfg_t adc1_chan_config = 
    {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = 12,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_2, &adc1_chan_config);
    ADC_CALI();
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL<<3;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    Beep_Init();
}

void Smoke_Warning(void *pvParameters)
{
    while(1)
    {
        if(gpio_get_level(3) == 0)
        {
            OLED2_String(4, 10, 2, 2, 3);
            vTaskSuspend(MQ2_Handle);
            Beep(1);
            while(gpio_get_level(3) == 0)
            {
                vTaskDelay(100);
                LED_Close();
                vTaskDelay(100);
                LED_Warning();
            }
            Beep(0);
            OLED2_ShowString(4, 10, "    ");
            vTaskResume(MQ2_Handle);
            LED_Restart();
        }
        vTaskDelay(20);
    }
    
}