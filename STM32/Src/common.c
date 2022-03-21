
#include <main.h>
#include "stm32l4xx_hal.h"
#include "core_cm4.h"
#include "common.h"
#include "stdint.h"

static uint8_t fac_us = 0;
static uint16_t fac_ms = 0;

#define SYSCLK 8

void delay_init(void) {
    SysTick->CTRL &= 0xfffffffb;//控制寄存器，选择外部时钟即系统时钟的八分之一（HCLK/8；72M/8=9M）
    fac_us = SYSCLK / 8;    //为系统时钟的1/8
    fac_ms = (uint16_t) fac_us * 1000;//代表每个ms需要的systick时钟数
}


void delay_us(uint32_t nus)//延时us
{
    uint32_t temp;
    SysTick->LOAD = nus * fac_us;
    SysTick->VAL = 0x00;
    SysTick->CTRL = 0x01;
    do {
        temp = SysTick->CTRL;
    } while (temp & 0x01 && !(temp & (1 << 16)));
    SysTick->CTRL = 0x00;       //关闭计数器
    SysTick->VAL = 0X00;
}

void delay_ms(uint32_t nms)//延时ms
{
    for (;nms>0;nms--){
        delay_us(1000);
    }
    /*uint32_t temp;
    SysTick->LOAD = (uint32_t) nms * fac_ms;
    SysTick->VAL = 0x00;
    SysTick->CTRL = 0x01;
    do {
        temp = SysTick->CTRL;
    } while (temp & 0x01 && !(temp & (1 << 16)));
    SysTick->CTRL = 0x00;       //关闭计数器
    SysTick->VAL = 0X00;*/
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line) {
    /* Turn LED2 on */
    BSP_LED_On(LED2);
	return;
    while (1) {
        /* Error if LED2 is slowly blinking (1 sec. period) */
        BSP_LED_Toggle(LED2);
        HAL_Delay(1000);
    }
}
