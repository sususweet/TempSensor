//
// Created by tangyq on 2017/9/23.
//


#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"

#include "string.h"
#include "common.h"
#include "STM32_PMS7003.h"

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* UART handler declaration */
UART_HandleTypeDef UartHandle;
UART_HandleTypeDef huart1;

extern uint8_t usart1_RxBuffer[2];
extern uint8_t usart1_aRxData[USART1_RXDATASIZE];
extern unsigned char usart1_aRxData_Index;
extern __IO ITStatus UartReady;

/* Buffer used for transmission */
uint8_t usart1_aTxBuffer[] = " ****UART_TwoBoards_ComIT****";

/* Buffer used for reception */

PM_Sensor_DataStruct PM_Sensor_Data;

/* Private functions ---------------------------------------------------------*/
void _Error_Handler(char * file, int line);

/* USART1 init function */
void MX_USART1_UART_Init(void)
{
  //  BSP_LED_Init(LED2);
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    //huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    //huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    //huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
    __HAL_UART_ENABLE(&huart1);
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    if (HAL_UART_Receive_IT(&huart1, (uint8_t *) usart1_RxBuffer, 1) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
}

/*
void PMS7003_UART_Init(void) {

    /* Configure LED2 */
  //  BSP_LED_Init(LED2);

    /*##-1- Configure the UART peripheral ######################################*/
    /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
    /* UART configured as follows:
        - Word Length = 8 Bits
        - Stop Bit = One Stop bit
        - Parity = None
        - BaudRate = 9600 baud
        - Hardware flow control disabled (RTS and CTS signals) */
  /*  UartHandle.Instance = USARTx;
    UartHandle.Init.BaudRate = 9600;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits = UART_STOPBITS_1;
    UartHandle.Init.Parity = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode = UART_MODE_TX_RX;

    /*if (HAL_UART_DeInit(&UartHandle) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }*/
 /*   if (HAL_UART_Init(&UartHandle) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
    __HAL_UART_ENABLE(&UartHandle);
    NVIC_SetPriority(USARTx_IRQn, 0);
    NVIC_EnableIRQ(USARTx_IRQn);
    if (HAL_UART_Receive_IT(&UartHandle, (uint8_t *) aRxBuffer, 1) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
}
*/
/**
* @brief This function handles UART interrupt request.
* @param None
* @retval None
* @Note This function is redefined in "main.h" and related to DMA stream
* used for USART data transmission
*/
/*void USART1_IRQHandler(void) {
    /*  uint8_t res[50];
      unsigned int i;//定义一个变量，在后面以为用
      HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_3);
      while(HAL_UART_GetState(&UartHandle) == HAL_UART_STATE_BUSY_RX) {}
      HAL_UART_Receive(&UartHandle,res,strlen(res),0xFFFF);
      HAL_UART_Transmit(&UartHandle,res,strlen(res),0xFFFF);


      HAL_UART_IRQHandler(&UartHandle);
      __HAL_UART_ENABLE_IT(&UartHandle, UART_IT_ERR);
      __HAL_UART_ENABLE_IT(&UartHandle, UART_IT_RXNE);
  */
   /* ;
    HAL_UART_IRQHandler(&UartHandle);
}*/

void PMS7003_UART_Receive() {
   /* if (HAL_UART_Receive_IT(&UartHandle, (uint8_t *) aRxBuffer, 1) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }*/
    /*##-4- Put UART peripheral in reception process ###########################*/
//    HAL_UART_Receive_IT(&UartHandle, (uint8_t *) aRxBuffer, RXBUFFERSIZE);
    /*##-5- Wait for the end of the transfer ###################################*/
    //while (UartReady != SET);
    /* Reset transmission flag */
    //UartReady = RESET;
}

void PMS7003_UART_Send() {
    /*##-2- Start the transmission process #####################################*/
    /* While the UART in reception process, user can transmit data through
       "usart1_aTxBuffer" buffer */
    if (HAL_UART_Transmit_IT(&huart1, (uint8_t *) usart1_aTxBuffer, (COUNTOF(usart1_aTxBuffer) - 1)) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    /*##-3- Wait for the end of the transfer ###################################*/
    while (UartReady != SET);

    /* Reset transmission flag */
    UartReady = RESET;
}

void PMS7003_DATA_Refresh() {
    uint16_t Buffer_Len;
    memset(&PM_Sensor_Data, 0, (sizeof(PM_Sensor_Data) - 2)); //PM_Sensor_Data.PM2_5_Old should not set to zero
    Buffer_Len = (uint16_t) ((usart1_aRxData[2] << 8) | usart1_aRxData[3]);

    if (Buffer_Len == 28) {     //PMS1003/5003/7003
        PM_Sensor_Data.Buffer_Len = 28;
        PM_Sensor_Data.PM1_0_CF = (uint16_t) ((usart1_aRxData[4] << 8) | usart1_aRxData[5]);
        PM_Sensor_Data.PM2_5_CF = (uint16_t) ((usart1_aRxData[6] << 8) | usart1_aRxData[7]);
        PM_Sensor_Data.PM10_CF = (uint16_t) ((usart1_aRxData[8] << 8) | usart1_aRxData[9]);
        PM_Sensor_Data.PM1_0 = (uint16_t) ((usart1_aRxData[10] << 8) | usart1_aRxData[11]);
        PM_Sensor_Data.PM2_5 = (uint16_t) ((usart1_aRxData[12] << 8) | usart1_aRxData[13]);
        PM_Sensor_Data.PM10 = (uint16_t) ((usart1_aRxData[14] << 8) | usart1_aRxData[15]);
        PM_Sensor_Data.Count0_3nm = (uint16_t) ((usart1_aRxData[16] << 8) | usart1_aRxData[17]);
        PM_Sensor_Data.Count0_5nm = (uint16_t) ((usart1_aRxData[18] << 8) | usart1_aRxData[19]);
        PM_Sensor_Data.Count1_0nm = (uint16_t) ((usart1_aRxData[20] << 8) | usart1_aRxData[21]);
        PM_Sensor_Data.Count2_5nm = (uint16_t) ((usart1_aRxData[22] << 8) | usart1_aRxData[23]);
        PM_Sensor_Data.Count5_0nm = (uint16_t) ((usart1_aRxData[24] << 8) | usart1_aRxData[25]);
        PM_Sensor_Data.Count10nm = (uint16_t) ((usart1_aRxData[26] << 8) | usart1_aRxData[27]);

    } else if (Buffer_Len == 20) {      // PMS3003
        PM_Sensor_Data.Buffer_Len = 20;
        PM_Sensor_Data.PM1_0_CF = (uint16_t) ((usart1_aRxData[4] << 8) | usart1_aRxData[5]);
        PM_Sensor_Data.PM2_5_CF = (uint16_t) ((usart1_aRxData[6] << 8) | usart1_aRxData[7]);
        PM_Sensor_Data.PM10_CF = (uint16_t) ((usart1_aRxData[8] << 8) | usart1_aRxData[9]);
        PM_Sensor_Data.PM1_0 = (uint16_t) ((usart1_aRxData[10] << 8) | usart1_aRxData[11]);
        PM_Sensor_Data.PM2_5 = (uint16_t) ((usart1_aRxData[12] << 8) | usart1_aRxData[13]);
        PM_Sensor_Data.PM10 = (uint16_t) ((usart1_aRxData[14] << 8) | usart1_aRxData[15]);
        PM_Sensor_Data.Count0_3nm = 0;
        PM_Sensor_Data.Count0_5nm = 0;
        PM_Sensor_Data.Count1_0nm = 0;
        PM_Sensor_Data.Count2_5nm = 0;
        PM_Sensor_Data.Count5_0nm = 0;
        PM_Sensor_Data.Count10nm = 0;
    }
}




/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
/*void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == USER_BUTTON_PIN) {
        UserButtonStatus = 1;
    }
}
*/
/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval 0  : pBuffer1 identical to pBuffer2
  *         >0 : pBuffer1 differs from pBuffer2
  */
static uint16_t Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint16_t BufferLength) {
    while (BufferLength--) {
        if ((*pBuffer1) != *pBuffer2) {
            return BufferLength;
        }
        pBuffer1++;
        pBuffer2++;
    }

    return 0;
}


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
