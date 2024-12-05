#include <stdio.h>

#ifndef __HTTP_H
#define __HTTP_H

void HTTP_Get_Font(char* string);
void HTTP_Get_Weather(char* string);
void HTTP_Time_Init(void);
void Time_Update(void *pvParameters);

#endif