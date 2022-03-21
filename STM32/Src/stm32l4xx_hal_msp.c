/**
  ******************************************************************************
  * File Name          : stm32l4xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
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

#include "main.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include "common.h"
#include "string.h"

extern void _Error_Handler(char *, int);

extern void wifi_Data_Deal(void);
extern void PMS7003_DATA_Refresh(void);

__IO ITStatus UartReady = RESET;

uint8_t usart1_RxBuffer[2];
uint8_t usart1_aRxData[USART1_RXDATASIZE];
unsigned char usart1_aRxData_Index = 0;
extern unsigned char PMS7003_Data_Ready;

uint8_t usart2_RxBuffer[2];
uint8_t usart2_aRxData[USART2_RXDATASIZE];
unsigned char usart2_aRxData_Index = 0;

unsigned char ZJUWLAN_connect_success = 0;
extern unsigned char RxCharBuf_wifi[16];
extern unsigned int RxCharBuf_wifi_index;
extern unsigned char Wifi_flag[16];
extern unsigned int Wifi_flag_index;
extern unsigned char wifi_receive_flag;

extern unsigned char wifi_init_stage;
extern unsigned char wifi_stage_change;
extern enum wifi_init_states;
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base) {
    if(htim_base->Instance==TIM3)
    {
        /* USER CODE BEGIN TIM3_MspInit 0 */

        /* USER CODE END TIM3_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();
        /* TIM3 interrupt Init */
        HAL_NVIC_SetPriority(TIM3_IRQn, 3, 3);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
        /* USER CODE BEGIN TIM3_MspInit 1 */

        /* USER CODE END TIM3_MspInit 1 */
    }

}

void HAL_UART_MspInit(UART_HandleTypeDef *huart) {

    GPIO_InitTypeDef GPIO_InitStruct;
    if (huart->Instance == USART1) {
        /* USER CODE BEGIN USART1_MspInit 0 */

        /* USER CODE END USART1_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 1, 0); //Todo:Change to  HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
        /* USER CODE BEGIN USART1_MspInit 1 */

        /* USER CODE END USART1_MspInit 1 */
    } else if (huart->Instance == USART2) {
        /* USER CODE BEGIN USART2_MspInit 0 */

        /* USER CODE END USART2_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX //(白)
        PA3     ------> USART2_RX //(黑)
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART2 interrupt Init */
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);// Todo: Change to HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspInit 1 */

        /* USER CODE END USART2_MspInit 1 */
    } else if(huart->Instance==USART3) {
        /* USER CODE BEGIN USART3_MspInit 0 */

        /* USER CODE END USART3_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_USART3_CLK_ENABLE();

        /**USART3 GPIO Configuration
        PC4     ------> USART3_TX
        PC5     ------> USART3_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART3 interrupt Init */
        HAL_NVIC_SetPriority(USART3_IRQn, 0, 0); //Todo:Change to  HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(USART3_IRQn);
        /* USER CODE BEGIN USART1_MspInit 1 */

        /* USER CODE END USART1_MspInit 1 */
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart) {

    if (huart->Instance == USART1) {
        /* USER CODE BEGIN USART1_MspDeInit 0 */

        /* USER CODE END USART1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10     ------> USART1_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

        /* USART1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(USART1_IRQn);
        /* USER CODE BEGIN USART1_MspDeInit 1 */

        /* USER CODE END USART1_MspDeInit 1 */
    } else if (huart->Instance == USART2) {
        /* USER CODE BEGIN USART2_MspDeInit 0 */

        /* USER CODE END USART2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);

        /* USART2 interrupt DeInit */
        HAL_NVIC_DisableIRQ(USART2_IRQn);
        /* USER CODE BEGIN USART2_MspDeInit 1 */

        /* USER CODE END USART2_MspDeInit 1 */
    } else if(huart->Instance==USART3) {
        /* USER CODE BEGIN USART3_MspDeInit 0 */

        /* USER CODE END USART3_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART3_CLK_DISABLE();

        /**USART3 GPIO Configuration
        PC4     ------> USART3_TX
        PC5     ------> USART3_RX
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_4|GPIO_PIN_5);

        HAL_NVIC_DisableIRQ(USART3_IRQn);
        /* USER CODE BEGIN USART3_MspDeInit 1 */

        /* USER CODE END USART3_MspDeInit 1 */
    }


}

/* USER CODE BEGIN 1 */

/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle.
  * @note   This example shows a simple way to report end of IT Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle) {
    /* Set transmission flag: transfer complete */
    UartReady = SET;
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        if (usart1_aRxData_Index >= USART1_RXDATASIZE) usart1_aRxData_Index = 0;

        HAL_UART_Receive_IT(huart, (uint8_t *) (&usart1_RxBuffer), 1);

        if (usart1_RxBuffer[0] == 0x42) {
            PMS7003_Data_Ready = 0;
            usart1_aRxData_Index = 0;
            usart1_aRxData[usart1_aRxData_Index++] = usart1_RxBuffer[0];
        } else if (usart1_RxBuffer[0] == 0x4D) {
            usart1_aRxData[usart1_aRxData_Index++] = usart1_RxBuffer[0];
        } else if (usart1_aRxData_Index >= 31) {
            PMS7003_DATA_Refresh();
            PMS7003_Data_Ready = 1;
            usart1_aRxData_Index = 0;
        } else {
            usart1_aRxData[usart1_aRxData_Index++] = usart1_RxBuffer[0];
        }

    } else if (huart->Instance == USART3) {
        char equalFlag = 1;
        if (usart2_aRxData_Index >= USART2_RXDATASIZE) usart2_aRxData_Index = 0;

        HAL_UART_Receive_IT(huart, (uint8_t *) (&usart2_RxBuffer), 1);

        if (usart2_RxBuffer[0] == '\r') {
            usart2_aRxData[usart2_aRxData_Index++] = '\0';
        } else if (strstr(usart2_aRxData, "+++") && wifi_init_stage == WIFI_TO_END_ZJUWLAN_SEND) {
            wifi_init_stage = WIFI_TO_END_ZJUWLAN_TCP;
            wifi_stage_change = 1;
        } else if (usart2_RxBuffer[0] == '\n') {
            usart2_aRxData_Index = 0;
            if (strcmp(usart2_aRxData, "WIFI GOT IP") == 0) {
                wifi_init_stage = WIFI_TO_ZJUWLAN_LOGIN;
                wifi_stage_change = 1;
            } else if (strstr(usart2_aRxData, "Content-Length")) {
                if (usart2_aRxData[16] == '5'){
                    ZJUWLAN_connect_success = 1;
                }else{
                    ZJUWLAN_connect_success = 0;
                }
            }/*else if (strcmp(usart2_aRxData, ">") == 0 ){
                if (wifi_init_stage == WIFI_TO_ZJUWLAN_SEND){
                    wifi_init_stage = WIFI_TO_ZJUWLAN_DATA;
                }else if (wifi_init_stage == WIFI_TO_SENSOR_SEND){
                    wifi_init_stage = WIFI_TO_SENSOR_DATA;
                }
                wifi_stage_change = 1;
            }*/
            else if (strstr(usart2_aRxData,"HTTP/1.1 404 Not Found")
                     && wifi_init_stage == WIFI_TO_SENSOR_DATA){
                wifi_init_stage = WIFI_TO_ZJUWLAN_RELOGIN;
            }
            else if(strstr(usart2_aRxData, "</html>") && wifi_init_stage == WIFI_TO_ZJUWLAN_DATA){
                if (ZJUWLAN_connect_success == 1) {
                    wifi_init_stage = WIFI_TO_END_ZJUWLAN_TCP;
                }
                wifi_stage_change = 1;
            }else{
                equalFlag = (char) strcmp(usart2_aRxData, "OK");
                if (equalFlag == 0){
                    if (wifi_init_stage == WIFI_TO_ZJUWLAN_LOGIN){
                        wifi_init_stage = WIFI_TO_ZJUWLAN_CIPMODE;
                    }else if (wifi_init_stage == WIFI_TO_ZJUWLAN_CIPMODE){
                        wifi_init_stage = WIFI_TO_ZJUWLAN_SEND;
                    }else if (wifi_init_stage == WIFI_TO_ZJUWLAN_SEND){
                        wifi_init_stage = WIFI_TO_ZJUWLAN_DATA;
                    }else if (wifi_init_stage == WIFI_TO_END_ZJUWLAN_TCP){
                        wifi_init_stage = WIFI_TO_START_SENSOR_TCP;
                    }else if (wifi_init_stage == WIFI_TO_START_SENSOR_TCP){
                        wifi_init_stage = WIFI_TO_SENSOR_SEND;
                    }else if (wifi_init_stage == WIFI_TO_SENSOR_SEND){
                        wifi_init_stage = WIFI_TO_SENSOR_DATA;
                    }else if (wifi_init_stage == WIFI_TO_ZJUWLAN_RELOGIN){
                        wifi_init_stage = WIFI_TO_ZJUWLAN_LOGIN;
                    }

                    wifi_stage_change = 1;
                }
            }
        }  else {
            usart2_aRxData[usart2_aRxData_Index++] = usart2_RxBuffer[0];
        }
        /*if (wifi_receive_flag){
            if(usart2_RxBuffer[0] == 0x0d){                  //将每一个回车符前面的一个字符串记录下来，用作判断的标志
                if(RxCharBuf_wifi_index == 0){          //RxCharBuf_wifi即为数组A, wifi_flag即为数组B， RxBufWr_wifi为数组A的写入指针, flagWr为数组B的写入指针
                    Wifi_flag[Wifi_flag_index] = RxCharBuf_wifi[15];
                }else{
                    Wifi_flag[Wifi_flag_index] = RxCharBuf_wifi[RxCharBuf_wifi_index-1];
                }
                Wifi_flag_index++;
                Wifi_flag_index &= 0x0f;                //数组长度为16,当写入指针为达到16时 自动归零
                if(Wifi_flag_index != 0)
                    Wifi_flag[Wifi_flag_index] = 0x00; //让接收的字符串求出的长度为正确的

            }else if(usart2_RxBuffer[0] != 0x0a) {
                RxCharBuf_wifi[RxCharBuf_wifi_index] = usart2_RxBuffer[0];
                RxCharBuf_wifi_index++;
                RxCharBuf_wifi_index &= 0x0f;            //16->0
                if(RxCharBuf_wifi_index != 0)
                    RxCharBuf_wifi[RxCharBuf_wifi_index] = 0x00; //让接收的字符串求出的长度为正确的（加个0x00结尾）
            }
        }*/
        //usart2_aRxData[usart2_aRxData_Index++] = usart2_RxBuffer[0];
    }


    UartReady = SET;
    // HAL_UART_Receive_IT(UartHandle, (uint8_t *)&aRxBuffer, 1) ;
    /* Set transmission flag: transfer complete */
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle) {
    //_Error_Handler(__FILE__, __LINE__);
}


/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
