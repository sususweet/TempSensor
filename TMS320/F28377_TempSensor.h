//
// Created by tangyq on 2017/9/30.
//

#ifndef PM25_SCANNER_F28377_TEMPSENSOR_H
#define PM25_SCANNER_F28377_TEMPSENSOR_H
void DHT11_Gpio_Init();
unsigned char DHT11_rec_byte();
//接收40位的数据
void DHT11_receive();

#endif //PM25_SCANNER_F28377_TEMPSENSOR_H
