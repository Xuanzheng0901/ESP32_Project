#ifndef __DHT_11_H
#define __DHT_11_H

typedef struct 
{
    int8_t temperature_h;
    int8_t temperature_l;
    uint8_t humidity_h;
    uint8_t humidity_l;
} DHT;

void DHT_Sensor_Init(void);
DHT *Get_Temp_Humi(void);
void DHT_TaskHandle(void *pvParameters);
void Temperature_Warning(void *pvParameters);

#endif