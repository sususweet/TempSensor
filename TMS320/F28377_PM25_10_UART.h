//
// Created by tangyq on 2018/5/7.
//

#ifndef PM25_SCANNER_F28377_PM25_10_UART_H
#define PM25_SCANNER_F28377_PM25_10_UART_H

//
// Function Prototypes
//
#define MAX_FRAME_LEN 64
typedef unsigned char   uint8_t;

interrupt void sciaTxFifoIsr(void);
interrupt void sciaRxFifoIsr(void);
void scia_msg(char *msg);
void scia_xmit(int a);

void PM25_UART_Init();

#endif //PM25_SCANNER_F28377_PM25_10_UART_H
