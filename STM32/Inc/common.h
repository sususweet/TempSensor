//
// Created by tangyq on 2017/9/17.
//

#ifndef TEMPSENSOR_COMMON_H
#define TEMPSENSOR_COMMON_H

#include "stdint.h"

#define USART1_RXDATASIZE 40
#define USART2_RXDATASIZE 80

enum wifi_init_states {
    WIFI_TO_CONNECTED,
    WIFI_TO_ZJUWLAN_RELOGIN,
    WIFI_TO_ZJUWLAN_LOGIN,
    WIFI_TO_ZJUWLAN_CIPMODE,
    WIFI_TO_ZJUWLAN_SEND,
    WIFI_TO_ZJUWLAN_DATA,
    WIFI_TO_END_ZJUWLAN_SEND,
    WIFI_TO_END_ZJUWLAN_TCP,
    WIFI_TO_START_SENSOR_TCP,
    WIFI_TO_SENSOR_SEND,
    WIFI_TO_SENSOR_DATA
};

void delay_init(void);
void delay_us(uint32_t nus);
void delay_ms(uint32_t nms);
void _Error_Handler(char * file, int line);

#endif //TEMPSENSOR_COMMON_H
