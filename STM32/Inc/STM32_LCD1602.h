//
// Created by tangyq on 2017/9/20.
//

#ifndef TEMPSENSOR_STM32_LCD1602_H
#define TEMPSENSOR_STM32_LCD1602_H

typedef enum {
    Write_data  =0X00,
    Write_order =0x01/*BIT0*/
}LCD1602_Write_TypeDef;

void LCD_Gpio_Init(void);
void LCD1602_INIT(void);
void LCD1602_Clear_Screen(void);
unsigned char lcd1602_show_number(unsigned char x, unsigned char y, unsigned short dat);
unsigned char lcd1602_show_string(unsigned char x, unsigned char y, unsigned char *dat);
#endif //TEMPSENSOR_STM32_LCD1602_H
