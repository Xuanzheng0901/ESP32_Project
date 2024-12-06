#ifndef __LED_H
#define __LED_H

void led(void *pvParameters);
void LED_Init(void);
void LED_Close(void);
void LED_Restart(void);
void LED_Warning(void);

#endif