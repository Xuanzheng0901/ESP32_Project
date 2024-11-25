#include <stdio.h>
#include "HTTP.h"
#include "esp_http_client.h"
#include "string.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "OLED.h"

char TAG[5] = "HTTP";

char buffer[1024] = {0};

char font_buffer[32] = {0};
char weather_buffer[3][32] = {0};
uint8_t HTTP_Get_Data_Flag = 0;
esp_http_client_handle_t font_client, weather_client;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) 
    {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:

            memcpy(evt->user_data, evt->data, evt->data_len);
            ESP_LOGI(TAG, "evt->data:%s", (char*)evt->data);
            memset(evt->data, 0, 256);
            buffer[evt->data_len] = 0;
            HTTP_Get_Data_Flag = 1;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            break;
    };
    return ESP_OK;
}

void HTTP_Get_Font(char* string)
{
    static int http_get_flag;
    char url[256] = "http://182.92.11.87:5000/font/";
    strcat(url, string);
    if(http_get_flag == 0)
    {
        esp_http_client_config_t config = 
        {
            .url = url,
            .buffer_size = 1024,
            .event_handler = _http_event_handler,
            .user_data = buffer,
            .timeout_ms = 10000,
            .method = HTTP_METHOD_GET,
            
        };
        font_client = esp_http_client_init(&config);
        http_get_flag++;
    }
        
    else
    {
        esp_http_client_set_url(font_client, url);
    }
    
    esp_http_client_perform(font_client);

    ESP_LOGI(TAG, "buffer:%s", buffer);

    char* token = strtok(buffer, ", ");
    int i = 0;
    while(token != NULL)
    {
        font_buffer[i] = atoi(token);
        i++;
        token = strtok(NULL, ", ");
    }
}

void  HTTP_Get_Weather(char* string)
{
    static int http_get_flag;
    char url[256] = "http://182.92.11.87:5000/weather/";
    strcat(url, string);
    if(http_get_flag == 0)
    {
        esp_http_client_config_t config = 
        {
            .url = url,
            .buffer_size = 1024,
            .event_handler = _http_event_handler,
            .user_data = buffer,
            .timeout_ms = 10000,
            .method = HTTP_METHOD_GET,
            
        };
        weather_client = esp_http_client_init(&config);
    }
        
    else
    {
        esp_http_client_set_url(weather_client, url);
    }
    
    esp_http_client_perform(weather_client);

    ESP_LOGI(TAG, "buffer:%s", buffer);

    char* token = strtok(buffer, "@");
    int i = 0;
    while(token != NULL)
    {
        strcpy(weather_buffer[i], (char*)token);
        printf("%s\n", weather_buffer[i]);
        token = strtok(NULL, "@");
        i++;
    }
    HTTP_Get_Data_Flag = 0;
    OLED2_ShowString(1, 1, "              ");
    OLED2_ShowString(2, 6, "           ");
    OLED2_ShowString(3, 6, "           ");
    OLED2_NetString(1, 1, weather_buffer[0]);
    for(int i = 2 + 2 * (strlen(weather_buffer[0]) / 3); i <= 14; i++)
    {
        OLED2_ShowChar(1, i, ' ');
    }
    if(http_get_flag == 0)
    {
        OLED2_NetString(2, 1, "天气");
        OLED2_ShowChar(2, 5, ':');
    }
    OLED2_NetString(2, 6, weather_buffer[1]);
    for(int i = 6 + 2 * (strlen(weather_buffer[1]) / 3); i <= 16; i++)
    {
        OLED2_ShowChar(2, i, ' ');
    }
    if(http_get_flag == 0)
    {
        OLED2_NetString(3, 1, "温度:");
        OLED2_ShowChar(3, 5, ':');
    }
    OLED2_ShowString(3, 6, weather_buffer[2]);
    int temperature_len = strlen(weather_buffer[2]);
    OLED2_ShowIcon(3, 6+temperature_len, 1);
    OLED2_ShowChar(3, 7+temperature_len, 'C');
    for(int i = 8+temperature_len; i <= 16; i++)
    {
        OLED2_ShowChar(3, i, ' ');
    }
    http_get_flag++;
}