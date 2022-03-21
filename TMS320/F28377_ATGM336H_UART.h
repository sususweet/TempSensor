//
// Created by tangyq on 2018/5/7.
//

#ifndef PM25_SCANNER_F28377_ATGM336H_UART_H
#define PM25_SCANNER_F28377_ATGM336H_UART_H

//
// Function Prototypes
//
void GPS_UART_Init();

interrupt void scicTxFifoIsr(void);
interrupt void scicRxFifoIsr(void);
void scic_msg(char *msg);
void scic_xmit(int a);

void GPS_scic_init(void);

#endif //PM25_SCANNER_F28377_ATGM336H_UART_H
