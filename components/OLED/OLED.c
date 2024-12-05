#include "OLED_Font.h"
#include <stdarg.h>
#include <string.h>
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "HTTP.h"
#include "stdio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#define SCL 18
#define SDA 19

extern char font_buffer[32], HTTP_Get_Data_Flag;

static void OLED_W_SCL(uint8_t a)
{
	gpio_set_level(SCL, a);
}

static void OLED_W_SDA(uint8_t a)
{
	gpio_set_level(SDA, a);
}

static void OLED_I2C_Init()
{
	gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    io_conf.pin_bit_mask = 1ULL<<18;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = 1ULL<<19;
	gpio_config(&io_conf);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

static void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

static void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

static void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

static void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命令
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}
static void OLED2_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x7A);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命令
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}

static void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}
static void OLED2_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x7A);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}
/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
static void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}
static void OLED2_SetCursor(uint8_t Y, uint8_t X)
{
	OLED2_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED2_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED2_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}
void OLED2_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED2_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED2_WriteData(0x00);
		}
	}
}

void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8[Char - ' '][i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8[Char - ' '][i + 8]);		//显示下半部分内容
	}
}

void OLED2_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED2_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		OLED2_WriteData(OLED_F8[Char - ' '][i]);			//显示上半部分内容
	}
	OLED2_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		OLED2_WriteData(OLED_F8[Char - ' '][i + 8]);		//显示下半部分内容
	}
}

void OLED_ShowIcon(uint8_t Line, uint8_t Column, unsigned char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 16; i++)
	{
		OLED_WriteData(OLED_F16[Char][i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 16; i++)
	{
		OLED_WriteData(OLED_F16[Char][i + 16]);		//显示下半部分内容
	}
}

void OLED_ShowNetIcon(uint8_t Line, uint8_t Column, char *Font)
{
	char encoded_hanzi[10] = {0};
	char temp_hex[3] = {0};
	for(int i = 0; i < 3; i++)
	{
		snprintf(temp_hex, 3, "%X", Font[i]);
		strncat(encoded_hanzi, "\%", 2);
		strncat(encoded_hanzi, temp_hex, 3);
	}
	HTTP_Get_Font(encoded_hanzi);
	while(HTTP_Get_Data_Flag == 0)
	{
		vTaskDelay(2);
	}
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	int i = 0;
	for (i = 0; i < 16; i++)
	{
		OLED_WriteData(font_buffer[i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 16; i++)
	{
		OLED_WriteData(font_buffer[i+16]);		//显示下半部分内容
	}
	memset(font_buffer, 0, 32 * sizeof(uint8_t));
	HTTP_Get_Data_Flag = 0;
}
void OLED2_ShowIcon(uint8_t Line, uint8_t Column, unsigned char Char)
{      	
	uint8_t i;
	OLED2_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 16; i++)
	{
		OLED2_WriteData(OLED_F16[Char][i]);			//显示上半部分内容
	}
	OLED2_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 16; i++)
	{
		OLED2_WriteData(OLED_F16[Char][i + 16]);		//显示下半部分内容
	}
}
void OLED2_ShowNetIcon(uint8_t Line, uint8_t Column, char *Font)
{
	char encoded_hanzi[10] = {0};
	char temp_hex[3] = {0};
	for(int i = 0; i < 3; i++)
	{
		snprintf(temp_hex, 3, "%X", Font[i]);
		strncat(encoded_hanzi, "\%", 2);
		strncat(encoded_hanzi, temp_hex, 3);
	}
	HTTP_Get_Font(encoded_hanzi);
	while(HTTP_Get_Data_Flag == 0)
	{
		vTaskDelay(2);
	}
	OLED2_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	int i = 0;
	for (i = 0; i < 16; i++)
	{
		OLED2_WriteData(font_buffer[i]);			//显示上半部分内容
	}
	OLED2_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 16; i++)
	{
		OLED2_WriteData(font_buffer[i+16]);		//显示下半部分内容
	}
	memset(font_buffer, 0, 32 * sizeof(uint8_t));
	HTTP_Get_Data_Flag = 0;
}

void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}
void OLED2_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED2_ShowChar(Line, Column + i, String[i]);
	}
}

void OLED_String(int Line, int Column, int Count, int header, ...)
{
	int i;
	va_list String;
	va_start(String, header);
	for(i = 0; i < Count; i++)
	{
		OLED_ShowIcon(Line, Column+i*2, header);
		header=va_arg(String, int);
	}
	va_end(String);
}
void OLED2_String(int Line, int Column, int Count, int header, ...)
{
	int i;
	va_list String;
	va_start(String, header);
	for(i = 0; i < Count; i++)
	{
		OLED2_ShowIcon(Line, Column+i*2, header);
		header=va_arg(String, int);
	}
	va_end(String);
}

void OLED_NetString(uint8_t Line, uint8_t Column, char* font)
{
	int num =  strlen(font) / 3;
	char font_temp[4] = {0};
	for(int i = 0; i < num; i++)
	{
		vTaskDelay(3);
		font_temp[0] = font[i*3+0];
		font_temp[1] = font[i*3+1];
		font_temp[2] = font[i*3+2]; 
		OLED_ShowNetIcon(Line, Column+i*2, font_temp);
	}
}
void OLED2_NetString(uint8_t Line, uint8_t Column, char* font)
{
	int num =  strlen(font) / 3;
	char font_temp[4] = {0};
	for(int i = 0; i < num; i++)
	{
		vTaskDelay(3);
		font_temp[0] = font[i*3+0];
		font_temp[1] = font[i*3+1];
		font_temp[2] = font[i*3+2]; 
		OLED2_ShowNetIcon(Line, Column+i*2, font_temp);
	}
}

uint8_t GetNumLength(uint32_t a)
{
	uint8_t i;
	for(i = 1; i < 16; i++)
	{
		if(a / 10 != 0)
			a /= 10;
		else 
			return i;
	}
	return 16;
}

void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	char num[Length+1];
	memset(num, 0, sizeof(num));
	sprintf(num, "%ld", Number);
	OLED_ShowString(Line, Column, num);
}

void OLED2_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	char num[Length+1];
	memset(num, 0, sizeof(num));
	OLED_ShowString(Line, Column, num);
}

void OLED_ClearLine(unsigned char x)
{
	OLED_ShowString(x, 1, "                ");
}
void OLED2_ClearLine(unsigned char x)
{
	OLED2_ShowString(x, 1, "                ");
}

void OLED2_ShowBigNum(uint8_t Line, uint8_t Column, uint32_t Number)
{
	uint8_t i;
	OLED2_SetCursor((Line - 1) * 2, (Column - 1) * 12);		//设置光标位置在上半部分
	for (i = 0; i < 12; i++)
	{
		OLED2_WriteData(OLED_F32[Number*2][i]);			//显示上半部分内容
	}
	OLED2_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 12);	//设置光标位置在下半部分
	for (i = 0; i < 12; i++)
	{
		OLED2_WriteData(OLED_F32[Number * 2 + 1][i]);		//显示下半部分内容
	}
}


void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA0);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC0);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xAF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏

	//OLED_ShowChar(1, 5-2, ':');
	//OLED_ShowChar(2, 5-2, ':');
    OLED_ShowIcon(1, 11-3, 1);
    OLED_ShowChar(1, 12-3, 'C');
    OLED_ShowChar(2, 12-3, '%');
    OLED_ShowChar(1, 9-3, '.');
    OLED_ShowChar(2, 9-3, '.');
	OLED_ShowChar(3, 9, ':');
    OLED_ShowString(3, 14, "ppm");
}

void OLED2_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//端口初始化
	
	OLED2_WriteCommand(0xAE);	//关闭显示
	
	OLED2_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED2_WriteCommand(0x80);
	
	OLED2_WriteCommand(0xA8);	//设置多路复用率
	OLED2_WriteCommand(0x3F);
	
	OLED2_WriteCommand(0xD3);	//设置显示偏移
	OLED2_WriteCommand(0x00);
	
	OLED2_WriteCommand(0x40);	//设置显示开始行
	
	OLED2_WriteCommand(0xA0);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED2_WriteCommand(0xC0);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED2_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED2_WriteCommand(0x12);
	
	OLED2_WriteCommand(0x81);	//设置对比度控制
	OLED2_WriteCommand(0xFF);

	OLED2_WriteCommand(0xD9);	//设置预充电周期
	OLED2_WriteCommand(0xF1);

	OLED2_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED2_WriteCommand(0x30);

	OLED2_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED2_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED2_WriteCommand(0x8D);	//设置充电泵
	OLED2_WriteCommand(0x14);

	OLED2_WriteCommand(0xAF);	//开启显示
		
	OLED2_Clear();				//OLED清屏
}
