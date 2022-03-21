/**
 * �ļ���:  STM32_LCD1602.c
 * ����:  LCD1602 ��������
 * ����:  tangyq
 * ��1�ţ�VSSΪ�ص�Դ
 * ��2�ţ�VDD��5V����Դ
 * ��3�ţ�V0ΪҺ����ʾ���Աȶȵ�����0.
 * ��4�ţ�RSΪ�Ĵ���ѡ�񣬸ߵ�ƽʱѡ�����ݼĴ������͵�ƽʱѡ��ָ��Ĵ�������R/S����STM32F0 Discovery���PC0��
 * ��5�ţ�R/WΪ��д�ź��ߣ��ߵ�ƽʱ���ж��������͵�ƽʱ����д��������R/W����STM32F0 Discovery���PC1��
 * ��6�ţ�E��Ϊʹ�ܶˣ���E���ɸߵ�ƽ����ɵ͵�ƽʱ��Һ��ģ��ִ�������E����STM32F0 Discovery���PC2��
 * ��7��14�ţ�D0��D7Ϊ8λ˫�������ߡ���D4��D7����STM32F0 Discovery���PB4-PB7��D0-D4���Խ�GND�������գ�
 * ��15�ţ������Դ����
 * ��16�ţ������Դ����
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
    LCD_RS_OUT_L;//ָ��

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

    while((LCD_D7_DATA)!=0);  //���æ��־

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

//��������  write_com
//�������ܣ�LCD1602дָ��
//ע�ͣ�
void write_com(unsigned char dat) {
    unsigned char j;
    LCD_E_OUT_L;
    LCD_RW_OUT_L;
    delay_us(40);
    LCD_RS_OUT_L;//ָ��
    for (j = 0; j < 2; j++) {
        //GPIOC->ODR |=(0x0001<<14);
        //GPIOB->ODR |= (dat & 0xf0);
        //GPIOB->ODR |= 0xff;
        lcd_data_out(dat);
        /*GPIOB->ODR &= 0x0f;
        GPIOB->ODR |= (dat & 0xf0);*/
        delay_us(50);
        //GPIO_Write(GPIOB, (dat & 0x00f0));
        LCD_E_OUT_H;//����
        delay_us(50);
        LCD_E_OUT_L;
        dat <<= 4;
        delay_us(500);
    }
}


//��������  write_data
//�������ܣ�LCD1602д����
//ע�ͣ�
void write_data(unsigned char dat) {
    unsigned char j;
    LCD_E_OUT_L;
    LCD_RW_OUT_L;
    delay_us(40);
    LCD_RS_OUT_H;//����
    for (j = 0; j < 2; j++) {
        lcd_data_out(dat);
        /*GPIOB->ODR &= 0x0f;
        GPIOB->ODR |= (dat & 0xf0);*/
        delay_us(50);
        //GPIOB->ODR |= 0xff;
        //GPIOB->ODR &= (dat & 0xf0);
        //GPIO_Write(GPIOB, (dat & 0x00f0));
        LCD_E_OUT_H;//����
        delay_us(50);
        LCD_E_OUT_L;
        dat <<= 4;
        delay_us(500);
    }
}

//��������lcd1602_write
//���ã������ݻ���ָ��д��LCD1602
//ע�ͣ�
void LCD1602_DATA_write(LCD1602_Write_TypeDef order, unsigned char dat) {
    WaitForEnable();
    if (order == Write_data)
        write_data(dat);
    else
        write_com(dat);
}

//��������  LCD1602_Clear_Screen
//�������ܣ�LCD1602����
//ע�ͣ�
void LCD1602_Clear_Screen(void) {
    LCD1602_DATA_write(Write_order, 0x01); //����
}


//��������  LCD1602_INIT
//�������ܣ�LCD1602��ʼ��
//ע�ͣ�    DATA������ָ���������
void LCD1602_INIT(void) {
    //GPIO_Init(GPIOD,GPIO_PIN_ALL,GPIO_MODE_OUT_PP_LOW_FAST);
    //GPIO_Init(GPIOC,GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7,GPIO_MODE_OUT_PP_LOW_FAST);
    //delay_ms(200);
    //lcd_data_out(0x20);
   // LCD_E_OUT_H;//����
    /*GPIOB->ODR = 0x20;
    //GPIO_Write(GPIOB, 0X20);*/

    //delay_us(50);
    //LCD_E_OUT_L;//4���ƣ��ڸ�λ������ͨѶʱ���������л������ǰ��ֽڵģ�֮����ʱ���볬��40US
    LCD1602_DATA_write(Write_order,0x32);
    LCD1602_DATA_write(Write_order,0x2c); //4λ��������ģʽ����ʾ2�����ݣ�5*10����ÿ�ַ�	0X38Ϊ8λ����ģʽ�� ��ʾ2�����ݣ�5*10����ÿ�ַ�
    /*LCD_E_OUT_H;
    delay_us(10);
    LCD_E_OUT_L;*/
    //LCD1602_DATA_write(Write_order,0x28);
    //delay_ms(5);
    //delay_us(10);
    LCD1602_Clear_Screen(); //����
    /*LCD1602_DATA_write(Write_order, 0x03);*/
    LCD1602_DATA_write(Write_order, 0x06); //д�����ݹ�����ƣ�д����������ʾ�����ƶ�
    LCD1602_DATA_write(Write_order, 0x0c); //����ʾ���޹��
    LCD1602_DATA_write(Write_order, 0x80);
}









void lcd1602_show_pic(unsigned char* dat)  //���趨�ִ���CGRAM
{
    unsigned char i;
    LCD1602_DATA_write(Write_order, 0x40);
    for(i=0;i<8;i++) {
        LCD1602_DATA_write(Write_data, dat[i]);
    }
}

//��������lcd1602_show_character
//���ã�  ���ƶ����꣬��ʾ�����ַ�
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

//��������lcd1602_show_number
//���ã�
//     ��(X,Y)����Ϊ��ʼ����ʾһ�����֣�������ֵ���ܴ���4294967295��0xffffffff����
//ע�ͣ�
//     �������ֵ���ʾ���ȣ���������ʾ���£������Զ����У� ��ʾ���ಿ��
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


//��������lcd1602_show_string
//���ã�
//     ��(X,Y)����Ϊ��ʼ����ʾһ���ַ���
//ע�ͣ�
//     �������ֵ���ʾ���ȣ���������ʾ���£������Զ����У� ��ʾ���ಿ�֣��ַ�����С��256
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


//��������lcd1602_show_number
//���ã�
//     ��(X,Y)����Ϊ��ʼ����ʾһ�����֣�������ֵ���ܴ���4294967295��0xffffffff����
//ע�ͣ�
//     �������ֵ���ʾ���ȣ���������ʾ���£������Զ����У� ��ʾ���ಿ��
unsigned char lcd1602_show_s32(unsigned char x, unsigned char y, short dat) {
    short pow = 1, instead;
    unsigned char increase = 0, lengh = 0;
    if (((dat & 0x80000000) == 0x80000000) && (dat != 0xffffffff))//����
    {
        instead = -dat;
        y = y % 2;
        lcd1602_show_character(x, y, 0x2D);//"-"
        x++;
    } else if ((dat & 0x80000000) != 0x80000000)//����
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
