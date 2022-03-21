/**
 * 文件名:  STM32_LCD1602_PCF8574T.c
 * 功能:  LCD1602 驱动程序
 * 作者:  tangyq
 * PA1 -- SDA
 * PA4 -- SCL
 * 
 */

#include "common.h"
#include "stdint.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include "STM32_LCD1602_PCF8574T.h"

#define LCD_SDA_IN GPIO_InitStruct.Pin = GPIO_PIN_1;\
                            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;\
                            GPIO_InitStruct.Pull = GPIO_PULLUP;\
                            GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;\
                            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#define LCD_SDA_OUT_H HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);
#define LCD_SDA_OUT_L HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);
#define LCD_SDA_OUT GPIO_InitStruct.Pin = GPIO_PIN_1;\
                            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;\
                            GPIO_InitStruct.Pull = GPIO_PULLUP;\
                            GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;\
                            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#define LCD_SDA_DATA HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1)

#define LCD_SCL_OUT    GPIO_InitStruct.Pin = GPIO_PIN_4;\
                            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;\
                            GPIO_InitStruct.Pull = GPIO_PULLUP;\
                            GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;\
                            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#define LCD_SCL_OUT_H HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
#define LCD_SCL_OUT_L HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);

unsigned char addr = 0x00;

void LCD_Gpio_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    //__GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟
    LCD_SDA_OUT;
    LCD_SCL_OUT;
    LCD_SDA_OUT_H;
    LCD_SCL_OUT_H;
}

/*************************************************************
*函数名称：LCD_I2C_Start
*函数功能：I2C开始信号
*输入参数：
*输出参数：
*备注：时钟线高时，数据线由高到低的跳变，表示I2C开始信号
**************************************************************/
void LCD_I2C_Start() {
    LCD_SDA_OUT_H;
    LCD_SCL_OUT_H;
    delay_us(4);
    LCD_SDA_OUT_L;
    delay_us(4);
    LCD_SCL_OUT_L;
}

/*************************************************************
*函数名称：I2CStop
*函数功能：I2C停止信号
*输入参数：
*输出参数：
*备注：时钟线高时，数据线由低到高的跳变，表示I2C停止信号
**************************************************************/
void LCD_I2C_Stop() {
    LCD_SCL_OUT_L;
    LCD_SDA_OUT_L;
    delay_us(4);
    LCD_SCL_OUT_H;
    LCD_SDA_OUT_H;
    delay_us(4);
}

/*************************************************************
*函数名称：LCD_I2C_Response
*函数功能：I2C从机设备应答查询
*输入参数：
*输出参数：
*备           注：
**************************************************************/
unsigned char LCD_I2C_Response() {
    unsigned int TimeOut = 0;
    GPIO_InitTypeDef GPIO_InitStruct;

    LCD_SDA_OUT_H;
    delay_us(1);
    LCD_SCL_OUT_H;
    delay_us(1);

    LCD_SDA_IN;
    //while(LCD_SDA_DATA);
    

   // while(LCD_SDA_DATA);

    while(LCD_SDA_DATA) {
        TimeOut ++;
        if(TimeOut > 250) {
            //LCD_SDA_OUT;
            LCD_I2C_Stop();
            return 1;
        }
    }
    LCD_SDA_OUT;
    LCD_SCL_OUT_L;
    return 0;


}

/*************************************************************
*函数名称：I2CWriteByte
*函数功能：I2C写一字节数据
*输入参数：
*输出参数：
*备           注：
**************************************************************/
unsigned char LCD_I2C_Write_byte(unsigned char date) {
    unsigned char i;
    GPIO_InitTypeDef GPIO_InitStruct;
    LCD_SDA_OUT;
    LCD_SCL_OUT_L;
    for (i = 0; i < 8; i++) {
        //判断发送位
        if ((date & 0x80) >> 7) {
            LCD_SDA_OUT_H;
        } else {
            LCD_SDA_OUT_L;
        }
        date <<= 1;
        delay_us(2);
        LCD_SCL_OUT_H;
        delay_us(2);
        LCD_SCL_OUT_L;
        delay_us(2);
    }
   // LCD_I2C_Response();
   /* if(RESET == LCD_I2C_Response()) {
        return RESET;
    }else{
        return SET;
    }
*/
   // while (LCD_SDA_DATA);
    
   // delay_us(2);
}

void delay1(unsigned char x) {
    unsigned char a, b;
    for (a = x; a > 0; a--)
        for (b = 200; b > 0; b--);
}

void write_add(unsigned char date1) {
    LCD_I2C_Start();
    unsigned char temp;
    //LCD_I2C_Write_byte(0x2f);

    temp = addr;
    //8574T 地址+写入		 （8574 地址+写入0x4e）
    while(!LCD_I2C_Write_byte(temp)){
        addr++;
        if (addr == 0xff) {
            __NOP();
            break;
        }
        temp = addr;
        delay_ms(1);
    };
    //LCD_I2C_Response();
    LCD_I2C_Write_byte(date1);
    //LCD_I2C_Response();
    LCD_I2C_Stop();
}

void write_com(unsigned char com)        //写命令函数
{
    unsigned char com1, com2;
    com1 = com | 0x0f;
    write_add(com1 & 0xfc);
    delay_us(400);
    //delay1(2);
    write_add(com1 & 0xf8);
    com2 = com << 4;
    com2 = com2 | 0x0f;
    write_add(com2 & 0xfc);
    delay_us(400);
    //delay1(2);
    write_add(com2 & 0xf8);

}

void write_date(unsigned char date)        //写数据函数
{
    unsigned char date1, date2;
    date1 = date | 0x0f;
    write_add(date1 & 0xfd);
    delay_us(400);
    //delay1(2);
    write_add(date1 & 0xf9);
    date2 = date << 4;
    date2 = date2 | 0x0f;
    write_add(date2 & 0xfd);
    //delay1(2);
    delay_us(400);
    write_add(date2 & 0xf9);

}
/***
	send one frame by iic
***/
void i2cAction(unsigned int status)
{
    LCD_I2C_Start();
    LCD_I2C_Write_byte(addr);
	  while(LCD_I2C_Response()){
        addr++;
        if (addr == 0xff) {
             __NOP();
            break;
        }
				LCD_I2C_Write_byte(addr);
        delay_ms(10);
    };
    LCD_I2C_Response();
    LCD_I2C_Write_byte(status);
    LCD_I2C_Response();
    LCD_I2C_Stop();
}
//初始化函数
void LCD_Init(void) {
    LCD_Gpio_Init();
    i2cAction(0);
    write_add(0x08);            //默认开始状态为关使能端，见时序图  选择状态为 写
    write_com(0x0f);
    write_com(0x28);            //显示模式设置 0x28中高位2，设置4线。
    write_add(0x0c);
    write_add(0x08);            //使能4线
    write_com(0x28);            //显示模式设置，为0x28。
    write_com(0x01);            //显示清屏，将上次的内容清除，默认为0x01.
    write_com(0x0c);            //显示功能设置0x0f为开显示，显示光标，光标闪烁；0x0c为开显示，不显光标，光标不闪
    write_com(0x06);            //设置光标状态默认0x06,为读一个字符光标加1.
}

