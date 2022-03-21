/**
 * 文件名:  STM32_DHT12.c
 * 功能:  温度传感器驱动程序
 * 作者:  tangyq
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

unsigned char Temp_Sensor_AnswerFlag = 0;   //定义传感器响应标志
unsigned char Temp_Sensor_ErrorFlag;        //定义读取传感器错误标志

void DHT12_Gpio_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    //__GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟

    TEMP_SENSOR_DATA_OUT;
    TEMP_SENSOR_DATA_OUT_H;
    //TEMP_SENSOR_DATA_OUT_L;
}

//接收一个字节
unsigned char DHT12_rec_byte(void) {
    uint16_t j;
    unsigned char i, dat = 0;
    for (i = 0; i < 8; i++) {                        //从高到低依次接收8位数据

        j = 0;
        //while (!TEMP_SENSOR_DATA);
        while (!TEMP_SENSOR_DATA) {                 //等待50us低电平过去
            if (++j >= 500) {
                //asm ("  NOP");
                break;
            }
        }

        delay_us(30);                               //延时30us，如果还为高则数据为1，否则为0
        dat <<= 1;                                  //移位使正确接收8位数据，数据为0时直接移位
        if (TEMP_SENSOR_DATA == 1) dat += 1;       //数据为1时，使dat加1来接收数据1

        j = 0;
        //while (TEMP_SENSOR_DATA);
        while (TEMP_SENSOR_DATA){                   //等待数据线拉低
            if (++j >= 500){                      //防止进入死循环
                //asm ("  NOP");
                break;
            }
        }
    }
    return dat;
}

//接收40位的数据
void DHT12_receive(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    unsigned char Humi_H, Humi_L, Temp_H, Temp_L, Temp_CAL;
    //主机拉低18ms
    uint16_t j;
  
    //TEMP_SENSOR_DATA_OUT;
    //DELAY_US(10);
    TEMP_SENSOR_DATA_OUT_L;
    delay_ms(20);   //延时18ms以上
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

    Temp_Sensor_AnswerFlag = 0;    //传感器响应标志
    j = 0;
    while (TEMP_SENSOR_DATA){
        if (++j >= 500){                      //防止进入死循环
            //asm ("  NOP");
            break;
        }
    }
    if (!TEMP_SENSOR_DATA) {

        Temp_Sensor_AnswerFlag = 1;    //收到起始信号

        //判断从机是否发出 80us 的低电平响应信号是否结束
        j = 0;
        //while (!TEMP_SENSOR_DATA);
        while(!TEMP_SENSOR_DATA){
            if(++j>=500){ //防止进入死循环{
                Temp_Sensor_ErrorFlag = 1;
                break;
            }
        }

        //判断从机是否发出 80us 的高电平，如发出则进入数据接收状态
        //j = 0;
        //while (TEMP_SENSOR_DATA);
        while(TEMP_SENSOR_DATA){
            //防止进入死循环
            if(++j>=800) {
                Temp_Sensor_ErrorFlag = 1;
                break;
            }
        }

        Humi_H = DHT12_rec_byte();            //接收湿度高八位
        Humi_L = DHT12_rec_byte();            //接收湿度低八位
        Temp_H = DHT12_rec_byte();            //接收温度高八位
        Temp_L = DHT12_rec_byte();            //接收温度低八位
        Temp_CAL = DHT12_rec_byte();          //接收校正位

        delay_us(25);    //结束

        //校正
        if ((Humi_H + Humi_L + Temp_H + Temp_L) == Temp_CAL) {
            /* RH=Humi_H;
             RL=Humi_L;
             TH=Temp_H;
             TL=Temp_L;*/

            humidity = Humi_H * 10 + Humi_L; //湿度

            if (Temp_L & 0X80){         //为负温度
                temperature = 0 - (Temp_H * 10 + ((Temp_L & 0x7F)));
            } else{                     //为正温度
                temperature = Temp_H * 10 + Temp_L;
            }
            //判断数据是否超过量程（温度：-20℃~60℃，湿度20％RH~95％RH）
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
            temperature = temperature / 10;//计算为温度值
            humidity = humidity / 10; //计算为湿度值
        }
    }
    TEMP_SENSOR_DATA_OUT;
    TEMP_SENSOR_DATA_OUT_H;
    
}
