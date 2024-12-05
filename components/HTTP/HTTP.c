#include <stdio.h>
#include "HTTP.h"
#include "esp_http_client.h"
#include "string.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "OLED.h"
#include <time.h>

char TAG[5] = "HTTP";

char buffer[1024] = {0};
char Week[][4] = {"SUN", "MON", "TUE", "WED", "THR", "FRI", "SAT"};
char font_buffer[32] = {0};
time_t Time = 0;
char weather_buffer[3][32] = {0};
uint8_t HTTP_Get_Data_Flag = 0;
esp_http_client_handle_t font_client, weather_client;
struct tm *tm_s;
extern TaskHandle_t Time_Task_Handle;
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    if(evt->event_id == HTTP_EVENT_ON_DATA) 
    {
        memcpy(evt->user_data, evt->data, evt->data_len);
        //ESP_LOGI(TAG, "evt->data:%s", (char*)evt->data);
        memset(evt->data, 0, 256);
        buffer[evt->data_len] = 0;
        HTTP_Get_Data_Flag = 1;
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
        esp_http_client_set_url(font_client, url);
    
    esp_http_client_perform(font_client);

    //ESP_LOGI(TAG, "buffer:%s", buffer);

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
        esp_http_client_set_url(weather_client, url);

    vTaskSuspend(Time_Task_Handle);
    esp_http_client_perform(weather_client);

    if(esp_http_client_get_status_code(weather_client) != 200)
        return;

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
    OLED2_ClearLine(3);
    OLED2_ShowString(4, 6, "           ");
    OLED2_NetString(3, 1, weather_buffer[0]);
    for(int i = 2 + 2 * (strlen(weather_buffer[0]) / 3); i <= 14; i++)
    {
        OLED2_ShowChar(3, i, ' ');
    }
    OLED2_NetString(3, 2 * (strlen(weather_buffer[0]) / 3) + 2, weather_buffer[1]);
    for(int i = 6 + 2 * (strlen(weather_buffer[1]) / 3); i <= 16; i++)
    {
        OLED2_ShowChar(4, i, ' ');
    }
    // if(http_get_flag == 0)
    // {
    //     OLED2_NetString(4, 1, "温度:");
    //     OLED2_ShowChar(4, 5, ':');
    // }
    OLED2_ShowString(4, 1, weather_buffer[2]);
    int temperature_len = strlen(weather_buffer[2]);
    OLED2_ShowIcon(4, 1+temperature_len, 1);
    OLED2_ShowChar(4, 2+temperature_len, 'C');
    for(int i = 3+temperature_len; i <= 16; i++)
    {
        OLED2_ShowChar(4, i, ' ');
    } 
    vTaskResume(Time_Task_Handle);
    http_get_flag++;
}

void HTTP_Time_Init(void)
{
    esp_http_client_config_t config = 
    {
        .url = "http://182.92.11.87:5000/time",
        .buffer_size = 1024,
        .event_handler = _http_event_handler,
        .user_data = buffer,
        .timeout_ms = 10000,
        .method = HTTP_METHOD_GET,
    };
    esp_http_client_handle_t http_time_handle = esp_http_client_init(&config);
    esp_http_client_perform(http_time_handle);
    Time = atoll(buffer) + 28800;
    esp_http_client_close(http_time_handle);
    esp_http_client_cleanup(http_time_handle);
}

void Time_Update(void *pvParameters)
{
    while(1)
    {
        tm_s = gmtime(&Time);
        tm_s->tm_year += 1900;
        tm_s->tm_mon += 1;
        char ftime[30];
        sprintf(ftime, "%2d-%02d %s", tm_s->tm_mon, tm_s->tm_mday, Week[tm_s->tm_wday]);
        OLED2_ShowString(2, 1, ftime);
        OLED2_ShowBigNum(1, 1, tm_s->tm_hour / 10);
        OLED2_ShowBigNum(1, 2, tm_s->tm_hour % 10);
        OLED2_ShowBigNum(1, 3, 10);
        OLED2_ShowBigNum(1, 4, tm_s->tm_min / 10);
        OLED2_ShowBigNum(1, 5, tm_s->tm_min % 10);
        vTaskDelay(3000); 
        Time+=30;
    }
        
}