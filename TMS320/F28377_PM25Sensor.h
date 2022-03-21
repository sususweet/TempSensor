//
// Created by tangyq on 2018/5/7.
//

#ifndef PM25_SCANNER_F28377_PM25SENSOR_H
#define PM25_SCANNER_F28377_PM25SENSOR_H

void Sensor_Gpio_Init();
void Sensor_PWM_Init();
float Sensor_Get_PM25();
void ConfigureADC(void);
void SetupADCSoftware(void);
Uint16 ReadADC();

#endif //PM25_SCANNER_F28377_PM25SENSOR_H
