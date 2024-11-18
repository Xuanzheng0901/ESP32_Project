#include "MQ-SmokeSensor.h"
#include "esp_adc/adc_oneshot.h"
#include "OLED.h"
#include "FreeRTOS/FreeRTOS.h"
#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t cali_handle = NULL;
int adc_raw = 0, adc_voltage = 0;

const adc_oneshot_unit_init_cfg_t adc_init_config = 
{
    .unit_id = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
};

void Get_Smoke_Conc(void *pvParameters)
{
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
            OLED_ShowNum(3, 14-len, conc, len);
            for(int i = 0; i < 4-len; i++)
            {
                OLED_ShowChar(3, 10+i, ' ');
            }
        }
        else 
        {
            OLED_String(3, 10, 2, 15, 16);
            OLED_ShowString(3, 14, "  ");
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
}
