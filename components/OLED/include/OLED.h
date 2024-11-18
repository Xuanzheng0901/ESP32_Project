#ifndef __OLED_H
#define __OLED_H

#define SCL 18
#define SDA 19

void OLED_Init(void);
void OLED2_Init(void);
void OLED_Clear(void);
void OLED2_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ClearLine(unsigned char x);
void OLED_ShowIcon(uint8_t Line, uint8_t Column, unsigned char Char);
void OLED_String(int Line, int Column, int Count,int header, ...);
void OLED_ShowNetIcon(uint8_t Line, uint8_t Column, char *Font);
void OLED_NetString(uint8_t Line, uint8_t Column, char* font);

void OLED2_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED2_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED2_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED2_ClearLine(unsigned char x);
void OLED2_ShowIcon(uint8_t Line, uint8_t Column, unsigned char Char);
void OLED2_String(int Line, int Column, int Count,int header, ...);
void OLED2_ShowNetIcon(uint8_t Line, uint8_t Column, char *Font);
void OLED2_NetString(uint8_t Line, uint8_t Column, char* font);
uint8_t GetNumLength(uint32_t a);
#endif
