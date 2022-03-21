//
// Created by tangyq on 2017/9/17.
//

#ifndef TEMPSENSOR_STM32_DHT12_H
#define TEMPSENSOR_STM32_DHT12_H

void DHT12_Gpio_Init(void);
unsigned char DHT12_rec_byte(void);
//接收40位的数据
void DHT12_receive(void);

#endif //TEMPSENSOR_STM32_DHT12_H
