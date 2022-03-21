//
// Created by tangyq on 2018/5/7.
//

#ifndef PM25_SCANNER_F28377_LCD12864_H
#define PM25_SCANNER_F28377_LCD12864_H

void LCD_Gpio_Init();
void LCD_init();                        //初始化函数
void LCD_WriteCommand(unsigned char command);        //写指令函数
void LCD_WriteData(unsigned char dat);               //写数据函数
void LCD_Show(unsigned char x, unsigned char y, unsigned char *str);
void LCD_Clear_Screen();
void LCD_Char_Display();
void LCD_Photo_Display(const unsigned char *Bmp);
void LCD_Position(unsigned char X, unsigned char Y);


#endif //PM25_SCANNER_F28377_LCD12864_H
