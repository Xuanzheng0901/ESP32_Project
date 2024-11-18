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
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    if(evt->event_id == HTTP_EVENT_ON_DATA)
    {
        memcpy(evt->user_data, evt->data, evt->data_len);
        HTTP_Get_Data_Flag = 1;
    }
    return ESP_OK;
}

void HTTP_Get_Font(char* string)
{
    char url[256] = "http://182.92.11.87:5000/font/";
    strcat(url, string);
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = _http_event_handler,
        .user_data = buffer,        // Pass address of local buffer to get response
        .timeout_ms = 500,
        .method = HTTP_METHOD_GET,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_perform(client);
    ESP_LOGI(TAG, "%s\n", buffer);

    esp_http_client_cleanup(client);
    char* token = strtok(buffer, ", ");
    int i = 0;
    while(token != NULL)
    {
        font_buffer[i] = atoi(token);
        i++;
        token = strtok(NULL, ", ");
    }
}