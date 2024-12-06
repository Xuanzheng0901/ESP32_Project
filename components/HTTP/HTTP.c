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
char font_buffer[32] = {0};
char weather_buffer[3][32] = {0};

esp_http_client_handle_t font_client, weather_client, Yiyan_Handle;

uint8_t HTTP_Get_Data_Flag = 0, Show_Sec = 1;

time_t Time = 0;
struct tm *tm_s;
static struct tm Current_Time = 
{
    .tm_hour = 25,
    .tm_mday = 32,
    .tm_mon = 13,
    .tm_min = 60,
    .tm_sec = 60,
    .tm_wday = 7,
    .tm_yday = 367,
    .tm_year = 0,
};

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

    //vTaskSuspend(Time_Task_Handle);
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
    OLED_ClearLine(1);
    OLED_ClearLine(2);
    OLED_NetString(1, 1, weather_buffer[0]);
    for(int i = 2 + 2 * (strlen(weather_buffer[0]) / 3); i <= 16; i++)
    {
        OLED_ShowChar(1, i, ' ');
    }
    OLED_NetString(2, 1, weather_buffer[1]);
    for(int i = 2 * (strlen(weather_buffer[1]) / 3) + 1; i <= 16; i++)
    {
        OLED_ShowChar(2, i, ' ');
    }
    OLED_ShowString(2,  2 * (strlen(weather_buffer[1]) / 3) + 2, weather_buffer[2]);

    int temperature_len = strlen(weather_buffer[2]);
    OLED_ShowIcon(2,  2 * (strlen(weather_buffer[1]) / 3) + 2 + temperature_len, 1);
    OLED_ShowChar(2, 2 * (strlen(weather_buffer[1]) / 3) + 3 + temperature_len, 'C');
    for(int i = 2 * (strlen(weather_buffer[1]) / 3) + 4 + temperature_len; i <= 16; i++)
    {
        OLED_ShowChar(2, i, ' ');
    } 
    //vTaskResume(Time_Task_Handle);
    http_get_flag++;
}

void HTTP_Get_Yiyan(void)
{
    start:
    int jishu_flag = 0;
    char yiyan[2][22] = {0};
    char url[] = "http://182.92.11.87:5000/yiyan";
    esp_http_client_config_t config = 
    {
        .url = url,
        .buffer_size = 1024,
        .event_handler = _http_event_handler,
        .user_data = buffer,
        .timeout_ms = 10000,
        .method = HTTP_METHOD_GET,
    };
    Yiyan_Handle = esp_http_client_init(&config);
    esp_http_client_perform(Yiyan_Handle);

    if(esp_http_client_get_status_code(Yiyan_Handle) != 200)
        return;

    int len = strlen(buffer);
    ESP_LOGI(TAG, "length:%d,buffer:%s", len, buffer);
    if(len > 48)
    {
        goto start;
    }
    if(len % 2 == 1)
        jishu_flag = 1;
    len /= 2;
    len -= jishu_flag;
        
    strncpy(yiyan[0], buffer, len);
    strncpy(yiyan[1], &buffer[len], len + 3*jishu_flag);
    OLED_NetString(3, 1, yiyan[0]);
    OLED_NetString(4, 1, yiyan[1]);
    //ESP_ERROR_CHECK(esp_http_client_close(Yiyan_Handle));
    //ESP_ERROR_CHECK(esp_http_client_cleanup(Yiyan_Handle));
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
    OLED2_ClearLine(2);
    OLED2_ShowBigNum(1, 3, 10);
    OLED2_ShowBigNum(1, 6, 10);

    while(1)
    {
        tm_s = gmtime(&Time);
        tm_s->tm_year += 1900;
        tm_s->tm_mon += 1;
        if(Current_Time.tm_mday != tm_s->tm_mday)
        {
            Current_Time.tm_year = tm_s->tm_year;
            Current_Time.tm_mon = tm_s->tm_mon;
            Current_Time.tm_wday = tm_s->tm_wday;
            Current_Time.tm_mday = tm_s->tm_mday;
            char ftime[40];
            sprintf(ftime, "%d-%2d-%02d", Current_Time.tm_year, Current_Time.tm_mon, Current_Time.tm_mday);
            
            OLED2_ShowString(2, 1, ftime);
            OLED2_String(2, 12, 2, 21, Current_Time.tm_wday + 22);
        }
        if(Current_Time.tm_hour != tm_s->tm_hour)
        {
            Current_Time.tm_hour = tm_s->tm_hour;
            OLED2_ShowBigNum(1, 1, Current_Time.tm_hour / 10);
            OLED2_ShowBigNum(1, 2, Current_Time.tm_hour % 10);
        }
        if(Current_Time.tm_min != tm_s->tm_min)
        {
            Current_Time.tm_min = tm_s->tm_min;
            OLED2_ShowBigNum(1, 4, Current_Time.tm_min / 10);
            OLED2_ShowBigNum(1, 5, Current_Time.tm_min % 10);
        }

        if((Current_Time.tm_sec != tm_s->tm_sec) && Show_Sec)
        {
            Current_Time.tm_sec = tm_s->tm_sec;
            OLED2_ShowBigNum(1, 7, Current_Time.tm_sec / 10);
            OLED2_ShowBigNum(1, 8, Current_Time.tm_sec % 10);
        }
        // vTaskDelay(1000); 
        // vTaskDelay(1000); 
        vTaskDelay(100); 
        Time+=1;
    }
        
}