/**
 * 文件名:  STM32_LCD1602.c
 * 功能:  LCD1602 驱动程序
 * 作者:  tangyq
 * 第1脚：VSS为地电源
 * 第2脚：VDD接5V正电源
 * 第3脚：V0为液晶显示器对比度调整端0.
 * 第4脚：RS为寄存器选择，高电平时选择数据寄存器、低电平时选择指令寄存器。（R/S连接STM32F0 Discovery板的PC13）
 * 第5脚：R/W为读写信号线，高电平时进行读操作，低电平时进行写操作。（R/W连接STM32F0 Discovery板的PC14）
 * 第6脚：E端为使能端，当E端由高电平跳变成低电平时，液晶模块执行命令。（E连接STM32F0 Discovery板的PC15）
 * 第7～14脚：D0～D7为8位双向数据线。（D4～D7连接STM32F0 Discovery板的PB4-PB7，D0-D4可以接GND或者悬空）
 * 第15脚：背光电源正极
 * 第16脚：背光电源负极
 */

#include "common.h"
#include "stdint.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include "STM32_LCD1602_8.h"

unsigned char table[]="I LOVE MUC!";
unsigned char  table1[]="WWW.TCMUC.COM";

unsigned char  num;
#define LCD_RW_OUT_H HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,GPIO_PIN_SET);
#define LCD_RW_OUT_L HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,GPIO_PIN_RESET);
#define LCD_RS_OUT_H HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
#define LCD_RS_OUT_L HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);
#define LCD_E_OUT_H HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,GPIO_PIN_SET);
#define LCD_E_OUT_L HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,GPIO_PIN_RESET);

void LCD_Gpio_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
    LCD_RW_OUT_L;
    LCD_E_OUT_L;
    LCD_RS_OUT_L;//指令

}

void write_com(unsigned char dat) {
    LCD_RW_OUT_L;
    LCD_E_OUT_L;
    LCD_RS_OUT_L;
    delay_us(50);
    GPIOB->ODR = dat;
    delay_us(50);
    LCD_E_OUT_H;
    delay_us(50);
    LCD_E_OUT_L;
}

void write_data(unsigned char dat) {
    LCD_RW_OUT_L;
    LCD_E_OUT_L;
    LCD_RS_OUT_H;
    delay_us(50);
    GPIOB->ODR = dat;
    delay_us(50);
    LCD_E_OUT_H;
    delay_us(50);
    LCD_E_OUT_L;
}

//函数名：lcd1602_write
//作用：将数据或者指令写入LCD1602
//注释：
void LCD1602_DATA_write(LCD1602_Write_TypeDef order, unsigned char dat) {
    if (order == Write_data)
        write_data(dat);
    else
        write_com(dat);
}

//函数名：  LCD1602_Clear_Screen
//函数功能：LCD1602清屏
//注释：
void LCD1602_Clear_Screen(void) {
    LCD1602_DATA_write(Write_order, 0x01); //清屏
}


//函数名：  LCD1602_INIT
//函数功能：LCD1602初始化
//注释：    DATA可以是指令或者数据
void LCD1602_INIT(void) {
    //GPIO_Init(GPIOD,GPIO_PIN_ALL,GPIO_MODE_OUT_PP_LOW_FAST);
    //GPIO_Init(GPIOC,GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7,GPIO_MODE_OUT_PP_LOW_FAST);
    delay_ms(15);
    write_com(0x38);
    delay_ms(5);
    write_com(0x38);
    delay_ms(5);
    write_com(0x38);
    delay_ms(5);
    write_com(0x38);
    delay_ms(5);
    write_com(0x0c);
    write_com(0x06);
    write_com(0x01);

    write_com(0x80);
    for(num=0;num<11;num++) {
        write_data(table[num]);
        delay_us(5);
    }
}
