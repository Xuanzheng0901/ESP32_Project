#include <stdio.h>
#include "HTTP.h"
#include "esp_http_client.h"
#include "string.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

char TAG[5] = "HTTP";

char buffer[1024] = {0};

uint8_t font_buffer[32] = {0};
uint8_t HTTP_Get_Data_Flag = 0;
esp_http_client_handle_t client;

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
            .event_handler = _http_event_handler,
            .user_data = buffer,
            .timeout_ms = 10000,
            .method = HTTP_METHOD_GET,
            
        };
        client = esp_http_client_init(&config);
        http_get_flag++;
    }
        
    else
    {
        esp_http_client_set_url(client, url);
    }
    
    esp_http_client_perform(client);

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