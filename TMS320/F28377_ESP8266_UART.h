//
// Created by tangyq on 2018/5/7.
//

#ifndef PM25_SCANNER_F28377_ESP8266_UART_H
#define PM25_SCANNER_F28377_ESP8266_UART_H

//
// Function Prototypes
//

interrupt void scibTxFifoIsr(void);
interrupt void scibRxFifoIsr(void);
void scib_msg(char *msg);
void scib_xmit(int a);

void wifi_UART_Init();
void wifi_ConnectServer(void);
void wifi_sendData(void);

#endif //PM25_SCANNER_F28377_ESP8266_UART_H
