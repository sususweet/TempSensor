/**
  ******************************************************************************
  * @file    GPIO/GPIO_IOToggle/Src/main.c
  * @author  MCD Application Team
  * @version V1.8.0
  * @date    21-April-2017
  * @brief   This example describes how to configure and use GPIOs through
  *          the STM32L4xx HAL API.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "stdint.h"
#include "stm32l476xx.h"
#include "stm32l4xx.h"

#include "STM32_DHT12.h"
#include "STM32_LCD1602.h"
#include "STM32_PMS7003.h"
#include "STM32_ESP8266.h"
#include "common.h"
#include "main.h"
#define DEBUG
/** @addtogroup STM32L4xx_HAL_Examples
  * @{
  */

/** @addtogroup GPIO_IOToggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
enum lcd_show_states {
    TEMP_AND_HUMIDITY,
    PM2_5_AND_PM10
};
/* Private variables ---------------------------------------------------------*/
unsigned char line1[]="TEMP:          C";
unsigned char line2[]="HUMIDITY:      %";
unsigned char line3[]="PM2.5:     ug/m3";
unsigned char line4[]="PM10:      ug/m3";

unsigned char lcd_show_stage = TEMP_AND_HUMIDITY;
unsigned char lcd_show_count = 0;
unsigned char lcd_change_count = 0;
unsigned char wifi_refresh_count = 0;
unsigned char PMS7003_Data_Ready = 0;

static GPIO_InitTypeDef GPIO_InitStruct;
TIM_HandleTypeDef htim3;

float humidity;
float temperature;
unsigned char humidity_str[6];
unsigned char temperature_str[6];
unsigned char PM25_str[6];
unsigned char PM10_str[6];
extern PM_Sensor_DataStruct PM_Sensor_Data;
extern unsigned char wifi_init_stage;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Data_String_Update(void);
void LCD_Show_Update(void);
void LCD_Show_Stage_Change(void);
void MX_TIM3_Init(void);
/* Private functions ---------------------------------------------------------*/
/*void LCD_Display()					//显示函数
{
    unsigned char i;
    write_com(0x80);
    for(i=0;i<16;i++) {
        write_date(line1[i]);
        //delay1(205);
    }
    write_com(0x80+0x40); 		//更改数据指针，让字符换行
    for(i=0;i<16;i++) {
        write_date(t1[i]);
        //delay1(205);				//增加延时可以达到动态的效果
    }
}*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void) {
    /* This sample code shows how to use GPIO HAL API to toggle LED2 IOs
      in an infinite loop. */

    /* STM32L4xx HAL library initialization:
         - Configure the Flash prefetch
         - Systick timer is configured by default as source of time base, but user
           can eventually implement his proper time base source (a general purpose
           timer for example or other time source), keeping in mind that Time base
           duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
           handled in milliseconds basis.
         - Set NVIC Group Priority to 4
         - Low Level Initialization
       */

    HAL_Init();

    /* Configure the system clock to 80 MHz */
    SystemClock_Config();
    SET_BIT(PWR->CR1, PWR_CR1_LPR);

    delay_init();

    /* -1- Enable each GPIO Clock (to be able to program the configuration registers) */
    //LED2_GPIO_CLK_ENABLE();

    /* -2- Configure IOs in output push-pull mode to drive external LEDs */
    /*GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Pin = LED2_PIN;
    HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStruct);
*/

    delay_ms(2000);

    DHT12_Gpio_Init();
    DHT12_receive();
  
    LCD_Gpio_Init();

    //LCD_Init();
    LCD1602_INIT();//LCD1602_INIT
    lcd1602_show_string(0,0,line1);

    lcd1602_show_string(0,1,line2);
    
    MX_TIM3_Init();
    MX_USART3_UART_Init();
    
    MX_USART1_UART_Init();


    // PMS7003_UART_Receive();
    //ESP8266_UART_Send();
    wifi_ConnectServer();
    //PMS7003_UART_Send();
    //Data_String_Update();
    //wifi_sendData();
    /* -3- Toggle IOs in an infinite loop */
    while (1) {
        DHT12_receive();
        
        PMS7003_DATA_Refresh();

        if(wifi_refresh_count == 0){
            if (wifi_init_stage == WIFI_TO_SENSOR_DATA){
                wifi_sendData();
            }
        }

        if (lcd_show_count == 0){
            Data_String_Update();
            LCD_Show_Update();
        }

        if (lcd_change_count == 0){
            LCD_Show_Stage_Change();
        }

       // wifi_Data_Deal();

     //   DHT12_receive(); //TODO:111
        //HAL_Delay(1000);

    /*    if (lcd_show_count >= 100) {
            //LCD_Show_Stage_Change();

            lcd_show_count = 0;
        }*/
    /*    if (PMS7003_Data_Ready == 1){
            PMS7003_DATA_Refresh();
        }*/
//				lcd_show_count++;
       
      //  Data_String_Update();
      //  LCD_Show_Update();



     //   lcd_show_count++;
       
        //HAL_Delay(1000);
        /*HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
        
        HAL_Delay(100);
        delay_us(1);*/
    }
}

void Data_String_Update() {
    temperature_str[0] = (unsigned char) ('0' + (unsigned char) temperature / 10);
    temperature_str[1] = (unsigned char) ('0' + (unsigned char) temperature % 10);
    temperature_str[2] = '.';
    temperature_str[3] = (unsigned char) ('0' + (unsigned char) (temperature * 10) % 10);
    temperature_str[4] = '\0';

    humidity_str[0] = (unsigned char) ('0' + (unsigned char) humidity / 10);
    humidity_str[1] = (unsigned char) ('0' + (unsigned char) humidity % 10);
    humidity_str[2] = '.';
    humidity_str[3] = (unsigned char) ('0' + (unsigned char) (humidity * 10) % 10);
    humidity_str[4] = '\0';

    uint16_t PM25_Value = PM_Sensor_Data.PM2_5;
    uint16_t PM10_Value = PM_Sensor_Data.PM10;
    if (PM25_Value > 0 && PM10_Value > 0) {
        PM25_str[0] = (unsigned char) ('0' + (unsigned char) PM25_Value / 100);
        PM25_str[1] = (unsigned char) ('0' + (unsigned char) PM25_Value % 100 / 10);
        PM25_str[2] = (unsigned char) ('0' + (unsigned char) PM25_Value % 100 % 10);
        PM25_str[3] = '\0';
        /*PM25_str[3] = '.';
        PM25_str[4] = (unsigned char) ('0' + (unsigned char) (PM25_Value * 10) % 10);
        PM25_str[5] = '\0';*/

        PM10_str[0] = (unsigned char) ('0' + (unsigned char) PM10_Value / 100);
        PM10_str[1] = (unsigned char) ('0' + (unsigned char) PM10_Value % 100 / 10);
        PM10_str[2] = (unsigned char) ('0' + (unsigned char) PM10_Value % 100 % 10);
        PM10_str[3] = '\0';
        /*PM10_str[3] = '.';
        PM10_str[4] = (unsigned char) ('0' + (unsigned char) (PM10_Value * 10) % 10);
        PM10_str[5] = '\0';*/
    }
}

void LCD_Show_Stage_Change(){
    switch (lcd_show_stage){
        case TEMP_AND_HUMIDITY:{
            lcd_show_stage = PM2_5_AND_PM10;
            lcd1602_show_string(0, 0, line3);
            lcd1602_show_string(0, 1, line4);
            break;
        }
        case PM2_5_AND_PM10:{
            lcd_show_stage = TEMP_AND_HUMIDITY;
            lcd1602_show_string(0, 0, line1);
            lcd1602_show_string(0, 1, line2);
            break;
        }
        default:
            break;
    }
}

void LCD_Show_Update(){
    switch (lcd_show_stage){
        case TEMP_AND_HUMIDITY:{
            if (temperature >= 0){
                lcd1602_show_string(10, 0, temperature_str);
            }
            if (humidity >= 0){
                lcd1602_show_string(10, 1, humidity_str);
            }
            break;
        }
        case PM2_5_AND_PM10:{
            //lcd1602_show_number(7, 0, PM_Sensor_Data.PM2_5);
            lcd1602_show_string(7, 0, PM25_str);
            //lcd1602_show_number(7, 1, PM_Sensor_Data.PM10);
            lcd1602_show_string(7, 1, PM10_str);
            break;
        }
        default:
            break;
    }
}
/*100ms进中断*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

    if(htim==&htim3) {
			
        lcd_show_count++;
        lcd_change_count++;
        wifi_refresh_count++;
        // DHT12_receive();
        /*if (PMS7003_Data_Ready == 1){
            PMS7003_DATA_Refresh();
        }*/
        if (lcd_show_count >= 15){
            lcd_show_count = 0;
            //Data_String_Update();
            //LCD_Show_Update();
        }

        if (lcd_change_count >= 50){
            lcd_change_count = 0;
           // LCD_Show_Stage_Change();
        }

        if (wifi_refresh_count >= 150){
            wifi_refresh_count = 0;
            /* if (wifi_init_stage == WIFI_TO_SENSOR_DATA){
                wifi_sendData();
            }*/
        }


        /*lcd_twinkle_num++;

        if (lcd_twinkle_num >= LCD_REFRESH) {   //500MS
            lcd_twinkle_num = 0;
            LCD_Show_Update();
        }*/

        // HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,(GPIO_PinState)!HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0));
    }
}
/* TIM3 init function */
void MX_TIM3_Init(void) {

    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 8000-1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 100;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /*##-2- Start the TIM Base generation in interrupt mode ####################*/
    /* Start Channel1 */
    if (HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
    {
        /* Starting Error */
        _Error_Handler(__FILE__, __LINE__);
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
    HAL_NVIC_SetPriority(TIM3_IRQn, 3, 3);
    //NVIC_SetPriority(TIM3_IRQn, 1);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
}
#ifdef DEBUG
void SystemClock_Config(void) {

    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLN = 16;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV8;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        // Initialization Error
        while (1);
    }

    /**Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        // Initialization Error
        while (1);
    }

    /**Configure the main internal regulator output voltage
    */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
        // Initialization Error
        while (1);
    }

    /**Configure the Systick interrupt time
    */
    //HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    /**Configure the Systick
    */
   // HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    //HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
#endif

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
/*
void SystemClock_Config(void) {
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    // MSI is enabled after System reset, activate PLL with MSI as source
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 40;

    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLP = 7;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        // Initialization Error
        while (1);
    }

    // Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
    // clocks dividers
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
                                   RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        // Initialization Error
        while (1);
    }
}
*/

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
