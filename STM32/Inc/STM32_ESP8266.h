//
// Created by tangyq on 2017/9/23.
//

#ifndef TEMPSENSOR_STM32_ESP8266_H
#define TEMPSENSOR_STM32_ESP8266_H

/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_nucleo.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor USARTx/UARTx instance used and associated
   resources */

/* Size of Trasmission buffer */
#define TXBUFFERSIZE                      (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
#define RXBUFFERSIZE                      2

/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */

/* Private function prototypes -----------------------------------------------*/

static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
void MX_USART3_UART_Init(void);
void wifi_ConnectServer(void);
void wifi_sendData(void);
void ESP8266_UART_Send(char *msg);
void wifi_Data_Deal(void);
#endif //TEMPSENSOR_STM32_ESP8266_H
