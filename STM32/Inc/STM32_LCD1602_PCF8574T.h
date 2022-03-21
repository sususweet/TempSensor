//
// Created by tangyq on 2017/9/19.
//

#ifndef TEMPSENSOR_STM32_LCD1602_PCF8574T_H
#define TEMPSENSOR_STM32_LCD1602_PCF8574T_H

void LCD_Gpio_Init(void);
void LCD_Init(void);
void write_com(unsigned char com);
void write_date(unsigned char date);

#endif //TEMPSENSOR_STM32_LCD1602_PCF8574T_H
