/**
 * �ļ���:  STM32_DHT12.c
 * ����:  �¶ȴ�������������
 * ����:  tangyq
 * PA0 -- TEMP_SENSOR_DATA_IN
 *
 */

#include "common.h"
#include "stdint.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"

#include "STM32_DHT12.h"

void LCD_GPIO_Cogfig(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO Ports Clock Enable */
            __GPIOC_CLK_ENABLE();
            __GPIOA_CLK_ENABLE();
            __GPIOB_CLK_ENABLE();

    /*Configure GPIO pins : PC0 PC1 */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : PA1 PA4 */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pin : PB0 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
#define TEMP_SENSOR_DATA_IN GPIO_InitStruct.Pin = GPIO_PIN_0;\
                            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;\
                            GPIO_InitStruct.Pull = GPIO_PULLUP;\
                            GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;\
                            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

#define TEMP_SENSOR_DATA_OUT_H HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET);
#define TEMP_SENSOR_DATA_OUT_L HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_RESET);

#define TEMP_SENSOR_DATA_OUT GPIO_InitStruct.Pin = GPIO_PIN_0;\
                            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;\
                            GPIO_InitStruct.Pull = GPIO_PULLUP;\
                            GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;\
                            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

#define TEMP_SENSOR_DATA HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)

extern float humidity;
extern float temperature;

unsigned char Temp_Sensor_AnswerFlag = 0;   //���崫������Ӧ��־
unsigned char Temp_Sensor_ErrorFlag;        //�����ȡ�����������־

void DHT12_Gpio_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    //__GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();           //����GPIOAʱ��

    TEMP_SENSOR_DATA_OUT;
    TEMP_SENSOR_DATA_OUT_H;
    //TEMP_SENSOR_DATA_OUT_L;
}

//����һ���ֽ�
unsigned char DHT12_rec_byte(void) {
    uint16_t j;
    unsigned char i, dat = 0;
    for (i = 0; i < 8; i++) {                        //�Ӹߵ������ν���8λ����

        j = 0;
        //while (!TEMP_SENSOR_DATA);
        while (!TEMP_SENSOR_DATA) {                 //�ȴ�50us�͵�ƽ��ȥ
            if (++j >= 500) {
                //asm ("  NOP");
                break;
            }
        }

        delay_us(30);                               //��ʱ30us�������Ϊ��������Ϊ1������Ϊ0
        dat <<= 1;                                  //��λʹ��ȷ����8λ���ݣ�����Ϊ0ʱֱ����λ
        if (TEMP_SENSOR_DATA == 1) dat += 1;       //����Ϊ1ʱ��ʹdat��1����������1

        j = 0;
        //while (TEMP_SENSOR_DATA);
        while (TEMP_SENSOR_DATA){                   //�ȴ�����������
            if (++j >= 500){                      //��ֹ������ѭ��
                //asm ("  NOP");
                break;
            }
        }
    }
    return dat;
}

//����40λ������
void DHT12_receive(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    unsigned char Humi_H, Humi_L, Temp_H, Temp_L, Temp_CAL;
    //��������18ms
    uint16_t j;
  
    //TEMP_SENSOR_DATA_OUT;
    //DELAY_US(10);
    TEMP_SENSOR_DATA_OUT_L;
    delay_ms(20);   //��ʱ18ms����
    //TEMP_SENSOR_DATA_OUT_H;
    TEMP_SENSOR_DATA_IN;


    //TEMP_SENSOR_DATA = 0;
    /*test = TEMP_SENSOR_DATA;
    while(test == 1){
        test = TEMP_SENSOR_DATA;
    };
    asm ("  NOP");
    //DELAY_US(10);

    test = TEMP_SENSOR_DATA;

    asm ("  NOP");*/

    Temp_Sensor_AnswerFlag = 0;    //��������Ӧ��־
    j = 0;
    while (TEMP_SENSOR_DATA){
        if (++j >= 500){                      //��ֹ������ѭ��
            //asm ("  NOP");
            break;
        }
    }
    if (!TEMP_SENSOR_DATA) {

        Temp_Sensor_AnswerFlag = 1;    //�յ���ʼ�ź�

        //�жϴӻ��Ƿ񷢳� 80us �ĵ͵�ƽ��Ӧ�ź��Ƿ����
        j = 0;
        //while (!TEMP_SENSOR_DATA);
        while(!TEMP_SENSOR_DATA){
            if(++j>=500){ //��ֹ������ѭ��{
                Temp_Sensor_ErrorFlag = 1;
                break;
            }
        }

        //�жϴӻ��Ƿ񷢳� 80us �ĸߵ�ƽ���緢����������ݽ���״̬
        //j = 0;
        //while (TEMP_SENSOR_DATA);
        while(TEMP_SENSOR_DATA){
            //��ֹ������ѭ��
            if(++j>=800) {
                Temp_Sensor_ErrorFlag = 1;
                break;
            }
        }

        Humi_H = DHT12_rec_byte();            //����ʪ�ȸ߰�λ
        Humi_L = DHT12_rec_byte();            //����ʪ�ȵͰ�λ
        Temp_H = DHT12_rec_byte();            //�����¶ȸ߰�λ
        Temp_L = DHT12_rec_byte();            //�����¶ȵͰ�λ
        Temp_CAL = DHT12_rec_byte();          //����У��λ

        delay_us(25);    //����

        //У��
        if ((Humi_H + Humi_L + Temp_H + Temp_L) == Temp_CAL) {
            /* RH=Humi_H;
             RL=Humi_L;
             TH=Temp_H;
             TL=Temp_L;*/

            humidity = Humi_H * 10 + Humi_L; //ʪ��

            if (Temp_L & 0X80){         //Ϊ���¶�
                temperature = 0 - (Temp_H * 10 + ((Temp_L & 0x7F)));
            } else{                     //Ϊ���¶�
                temperature = Temp_H * 10 + Temp_L;
            }
            //�ж������Ƿ񳬹����̣��¶ȣ�-20��~60�棬ʪ��20��RH~95��RH��
            if (humidity > 950) {
                humidity = 950;
            }
            if (humidity < 200) {
                humidity = 200;
            }
            if (temperature > 600) {
                temperature = 600;
            }
            if (temperature < -200) {
                temperature = -200;
            }
            temperature = temperature / 10;//����Ϊ�¶�ֵ
            humidity = humidity / 10; //����Ϊʪ��ֵ
        }
    }
    TEMP_SENSOR_DATA_OUT;
    TEMP_SENSOR_DATA_OUT_H;
    
}
