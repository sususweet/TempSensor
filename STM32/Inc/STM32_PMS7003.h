//
// Created by tangyq on 2017/9/23.
//

#ifndef TEMPSENSOR_STM32_PMS7003_H
#define TEMPSENSOR_STM32_PMS7003_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "stm32l4xx_nucleo.h"

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    uint16_t Buffer_Len;
    uint16_t PM1_0_CF;
    uint16_t PM2_5_CF;
    uint16_t PM10_CF;
    uint16_t PM1_0;
    uint16_t PM2_5;
    uint16_t PM10;
    uint16_t Count0_3nm;
    uint16_t Count0_5nm;
    uint16_t Count1_0nm;
    uint16_t Count2_5nm;
    uint16_t Count5_0nm;
    uint16_t Count10nm;
}PM_Sensor_DataStruct;
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor USARTx/UARTx instance used and associated
   resources */
/* Definition for USARTx clock resources */

/* Size of Trasmission buffer */
#define TXBUFFERSIZE                      (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
#define RXBUFFERSIZE                      2

/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */

/* Private function prototypes -----------------------------------------------*/

static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
void MX_USART1_UART_Init(void);
void PMS7003_DATA_Refresh(void);
void PMS7003_UART_Send(void);
#endif //TEMPSENSOR_STM32_PMS7003_H
