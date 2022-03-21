/**
 * 文件名:  STM32_LCD1602.c
 * 功能:  LCD1602 驱动程序
 * 作者:  tangyq
 * 第1脚：VSS为地电源
 * 第2脚：VDD接5V正电源
 * 第3脚：V0为液晶显示器对比度调整端0.
 * 第4脚：RS为寄存器选择，高电平时选择数据寄存器、低电平时选择指令寄存器。（R/S连接STM32F0 Discovery板的PC0）
 * 第5脚：R/W为读写信号线，高电平时进行读操作，低电平时进行写操作。（R/W连接STM32F0 Discovery板的PC1）
 * 第6脚：E端为使能端，当E端由高电平跳变成低电平时，液晶模块执行命令。（E连接STM32F0 Discovery板的PC2）
 * 第7～14脚：D0～D7为8位双向数据线。（D4～D7连接STM32F0 Discovery板的PB4-PB7，D0-D4可以接GND或者悬空）
 * 第15脚：背光电源正极
 * 第16脚：背光电源负极
 */

#include "common.h"
#include "stdint.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include "STM32_LCD1602.h"

#define LCD_D7_IN GPIO_InitStruct.Pin = GPIO_PIN_7;\
                            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;\
                            GPIO_InitStruct.Pull = GPIO_NOPULL;\
                            GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;\
                            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

#define LCD_D7_OUT GPIO_InitStruct.Pin = GPIO_PIN_7;\
                            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;\
                            GPIO_InitStruct.Pull = GPIO_NOPULL;\
                            GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;\
                            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

#define LCD_D7_DATA HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_7)

#define LCD_RS_OUT_H HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET);
#define LCD_RS_OUT_L HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_RESET);
#define LCD_RW_OUT_H HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);
#define LCD_RW_OUT_L HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET);
#define LCD_E_OUT_H HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_SET);
#define LCD_E_OUT_L HAL_GPIO_WritePin(GPIOC,GPIO_PIN_2,GPIO_PIN_RESET);

#define LCD_D7_OUT_H HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET);
#define LCD_D7_OUT_L HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_RESET);
#define LCD_D6_OUT_H HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET);
#define LCD_D6_OUT_L HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET);
#define LCD_D5_OUT_H HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
#define LCD_D5_OUT_L HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
#define LCD_D4_OUT_H HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
#define LCD_D4_OUT_L HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);


void LCD_Gpio_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    LCD_RW_OUT_L;
    LCD_E_OUT_L;
    LCD_RS_OUT_L;//指令

}
void WaitForEnable(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    LCD_D7_IN;
    LCD_RS_OUT_L;
    LCD_RW_OUT_H;

    __NOP();
    LCD_E_OUT_H;
    __NOP();
    __NOP();

    while((LCD_D7_DATA)!=0);  //检测忙标志

    LCD_E_OUT_L;

    LCD_D7_OUT;
}


void lcd_data_out(unsigned char data){
    if (data & 0x80){
        LCD_D7_OUT_H;
    }
    else {
        LCD_D7_OUT_L;
    }
    if ((data << 1) & 0x80){
        LCD_D6_OUT_H;
    }
    else {
        LCD_D6_OUT_L;
    }
    if ((data << 2) & 0x80){
        LCD_D5_OUT_H;
    }
    else {
        LCD_D5_OUT_L;
    }
    if ((data << 3) & 0x80){
        LCD_D4_OUT_H;
    }
    else {
        LCD_D4_OUT_L;
    }
}

//函数名：  write_com
//函数功能：LCD1602写指令
//注释：
void write_com(unsigned char dat) {
    unsigned char j;
    LCD_E_OUT_L;
    LCD_RW_OUT_L;
    delay_us(40);
    LCD_RS_OUT_L;//指令
    for (j = 0; j < 2; j++) {
        //GPIOC->ODR |=(0x0001<<14);
        //GPIOB->ODR |= (dat & 0xf0);
        //GPIOB->ODR |= 0xff;
        lcd_data_out(dat);
        /*GPIOB->ODR &= 0x0f;
        GPIOB->ODR |= (dat & 0xf0);*/
        delay_us(50);
        //GPIO_Write(GPIOB, (dat & 0x00f0));
        LCD_E_OUT_H;//允许
        delay_us(50);
        LCD_E_OUT_L;
        dat <<= 4;
        delay_us(500);
    }
}


//函数名：  write_data
//函数功能：LCD1602写数据
//注释：
void write_data(unsigned char dat) {
    unsigned char j;
    LCD_E_OUT_L;
    LCD_RW_OUT_L;
    delay_us(40);
    LCD_RS_OUT_H;//数据
    for (j = 0; j < 2; j++) {
        lcd_data_out(dat);
        /*GPIOB->ODR &= 0x0f;
        GPIOB->ODR |= (dat & 0xf0);*/
        delay_us(50);
        //GPIOB->ODR |= 0xff;
        //GPIOB->ODR &= (dat & 0xf0);
        //GPIO_Write(GPIOB, (dat & 0x00f0));
        LCD_E_OUT_H;//允许
        delay_us(50);
        LCD_E_OUT_L;
        dat <<= 4;
        delay_us(500);
    }
}

//函数名：lcd1602_write
//作用：将数据或者指令写入LCD1602
//注释：
void LCD1602_DATA_write(LCD1602_Write_TypeDef order, unsigned char dat) {
    WaitForEnable();
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
    //delay_ms(200);
    //lcd_data_out(0x20);
   // LCD_E_OUT_H;//允许
    /*GPIOB->ODR = 0x20;
    //GPIO_Write(GPIOB, 0X20);*/

    //delay_us(50);
    //LCD_E_OUT_L;//4线制，在该位四线制通讯时，数据线切换必须是半字节的，之后延时必须超过40US
    LCD1602_DATA_write(Write_order,0x32);
    LCD1602_DATA_write(Write_order,0x2c); //4位数据总线模式，显示2行数据，5*10点阵每字符	0X38为8位数据模式， 显示2行数据，5*10点阵每字符
    /*LCD_E_OUT_H;
    delay_us(10);
    LCD_E_OUT_L;*/
    //LCD1602_DATA_write(Write_order,0x28);
    //delay_ms(5);
    //delay_us(10);
    LCD1602_Clear_Screen(); //清屏
    /*LCD1602_DATA_write(Write_order, 0x03);*/
    LCD1602_DATA_write(Write_order, 0x06); //写入数据光标右移，写入新数据显示屏不移动
    LCD1602_DATA_write(Write_order, 0x0c); //开显示，无光标
    LCD1602_DATA_write(Write_order, 0x80);
}









void lcd1602_show_pic(unsigned char* dat)  //把设定字存入CGRAM
{
    unsigned char i;
    LCD1602_DATA_write(Write_order, 0x40);
    for(i=0;i<8;i++) {
        LCD1602_DATA_write(Write_data, dat[i]);
    }
}

//函数名：lcd1602_show_character
//作用：  在制定坐标，显示单个字符
void lcd1602_show_character(unsigned char x, unsigned char y, unsigned char dat) {
    unsigned char address;
    x = x % 16;
    y = y % 2;
    if (y == 1)
        address = 0xc0 + x;
    else
        address = 0x80 + x;
    LCD1602_DATA_write(Write_order, address);
    LCD1602_DATA_write(Write_data, dat);
}

//函数名：lcd1602_show_number
//作用：
//     以(X,Y)坐标为起始，显示一个数字（此数字值不能大于4294967295（0xffffffff））
//注释：
//     返回数字的显示长度，若改行显示不下，函数自动换行， 显示其余部分
unsigned char lcd1602_show_number(unsigned char x, unsigned char y, unsigned short dat) {
    unsigned short pow = 1, instead;
    unsigned char increase = 0, lengh = 0;
    instead = dat;
    while (instead != 0) {
        instead = instead / 10;
        increase++;
    }
    lengh = increase;
    if (increase == 0) {
        lcd1602_show_character(x, y, 0x30);
        return 1;
    } else {
        for (; increase > 1; increase--)
            pow = pow * 10;
        while (pow != 0) {
            instead = dat / pow;
            lcd1602_show_character(x, y, (0x30 + instead));
            x++;
            if (((x % 16) == 0) && (x != 0)) {
                y++;
                y = y % 2;
                x = x % 16;
            }
            dat = dat % pow;
            pow = pow / 10;
        }
    }
    return lengh;
}


//函数名：lcd1602_show_string
//作用：
//     以(X,Y)坐标为起始，显示一个字符串
//注释：
//     返回数字的显示长度，若改行显示不下，函数自动换行， 显示其余部分，字符长度小于256
unsigned char lcd1602_show_string(unsigned char x, unsigned char y, unsigned char *dat) {
    unsigned char lengh = 0;
    while (dat[lengh] != '\0') {
        if (((x % 16) == 0) && (x != 0)) {
            y++;
            y = y % 2;
            x = x % 16;
        }
        lcd1602_show_character(x, y, dat[lengh]);
        x++;
        lengh++;
    }
    return (lengh);
}


//函数名：lcd1602_show_number
//作用：
//     以(X,Y)坐标为起始，显示一个数字（此数字值不能大于4294967295（0xffffffff））
//注释：
//     返回数字的显示长度，若改行显示不下，函数自动换行， 显示其余部分
unsigned char lcd1602_show_s32(unsigned char x, unsigned char y, short dat) {
    short pow = 1, instead;
    unsigned char increase = 0, lengh = 0;
    if (((dat & 0x80000000) == 0x80000000) && (dat != 0xffffffff))//负数
    {
        instead = -dat;
        y = y % 2;
        lcd1602_show_character(x, y, 0x2D);//"-"
        x++;
    } else if ((dat & 0x80000000) != 0x80000000)//正数
    {
        instead = dat;
        y = y % 2;
        lcd1602_show_character(x, y, 0X2B);//"+"
        x++;
    } else//0
    {
        instead = 0;
    }
    dat = instead;
    while (instead != 0) {
        instead = instead / 10;
        increase++;
    }
    lengh = increase;
    if (increase == 0) {
        lcd1602_show_character(x, y, 0x30);
        return 1;
    } else {
        for (; increase > 1; increase--)
            pow = pow * 10;
        while (pow != 0) {
            instead = dat / pow;
            lcd1602_show_character(x, y, (0x30 + instead));
            x++;
            if (((x % 16) == 0) && (x != 0)) {
                y++;
                y = y % 2;
                //x=x%16;
            }
            dat = dat % pow;
            pow = pow / 10;
        }
    }
    return lengh + 1;
}
