//
// Created by tangyq on 2018/5/7.
//

#ifndef PM25_SCANNER_F28377_LCD12864_H
#define PM25_SCANNER_F28377_LCD12864_H

void LCD_Gpio_Init();
void LCD_init();                        //��ʼ������
void LCD_WriteCommand(unsigned char command);        //дָ���
void LCD_WriteData(unsigned char dat);               //д���ݺ���
void LCD_Show(unsigned char x, unsigned char y, unsigned char *str);
void LCD_Clear_Screen();
void LCD_Char_Display();
void LCD_Photo_Display(const unsigned char *Bmp);
void LCD_Position(unsigned char X, unsigned char Y);


#endif //PM25_SCANNER_F28377_LCD12864_H
