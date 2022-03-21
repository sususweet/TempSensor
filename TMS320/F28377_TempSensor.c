/**
 * �ļ���:  F28377_TempSensor.c
 * ����:  �¶ȴ�������������
 * ����:  tangyq
 * GPIO 66--TEMP_SENSOR_DATA_IN
 *
 */
#include "F28x_Project.h"
#include "F28377_TempSensor.h"

#define DELAY_MS(x) DELAY_US(x*1000)

#define TEMP_SENSOR_DATA GpioDataRegs.GPCDAT.bit.GPIO66
#define TEMP_SENSOR_DATA_OUT EALLOW;\
                GpioCtrlRegs.GPCDIR.bit.GPIO66 = 1;\
                EDIS;
#define TEMP_SENSOR_DATA_IN EALLOW;\
                GpioCtrlRegs.GPCDIR.bit.GPIO66 = 0;\
                GpioCtrlRegs.GPCPUD.bit.GPIO66 = 0;\
                EDIS;

extern float humidity;
extern float temperature;

unsigned char Temp_Sensor_AnswerFlag = 0;   //���崫������Ӧ��־
unsigned char Temp_Sensor_ErrorFlag;        //�����ȡ�����������־

void DHT11_Gpio_Init() {


    /* ͨ��I/O����ѡ��Ĵ�������ͨ��I/O����*/
    //GpioCtrlRegs.GPAMUX1.all = 0x000000;   // ����GPIO0-GPIO15Ϊͨ��I/O��

    /* ����I/O��Ϊ�������*/

    EALLOW;
    GpioCtrlRegs.GPCMUX1.bit.GPIO66 = 0;
    EDIS;


    // ÿ������ڿ����в�ͬ�������޶�
    // a) ������ϵͳʱ�� SYSCLKOUTͬ��
    // b) ���뱻ָ���Ĳ��������޶�
    // c) �����첽 (��������������Ч)
    //GpioCtrlRegs.GPAQSEL1.all = 0x0000;    // GPIO0-GPIO15��ϵͳʱ��SYSCLKOUT ͬ��
    TEMP_SENSOR_DATA_OUT;
    TEMP_SENSOR_DATA = 1;
}

//����һ���ֽ�
unsigned char DHT11_rec_byte() {
    Uint16 j;
    unsigned char i, dat = 0;
    for (i = 0; i < 8; i++) {                        //�Ӹߵ������ν���8λ����

        j = 0;
        //while (!TEMP_SENSOR_DATA);
        while (!TEMP_SENSOR_DATA) {                 //�ȴ�50us�͵�ƽ��ȥ
            if (++j >= 500) {
                asm ("  NOP");
                break;
            }
        }

        DELAY_US(30);                               //��ʱ30us�������Ϊ��������Ϊ1������Ϊ0
        dat <<= 1;                                  //��λʹ��ȷ����8λ���ݣ�����Ϊ0ʱֱ����λ
        if (TEMP_SENSOR_DATA == 1) dat += 1;       //����Ϊ1ʱ��ʹdat��1����������1

        j = 0;
        //while (TEMP_SENSOR_DATA);
        while (TEMP_SENSOR_DATA){                   //�ȴ�����������
            if (++j >= 500){                      //��ֹ������ѭ��
                asm ("  NOP");
                break;
            }
        }
    }
    return dat;
}

//����40λ������
void DHT11_receive() {
    unsigned char Humi_H, Humi_L, Temp_H, Temp_L, Temp_CAL;
    //��������18ms
    Uint16 j;

    //TEMP_SENSOR_DATA_OUT;
    //DELAY_US(10);
    TEMP_SENSOR_DATA = 0;
    DELAY_MS(20);   //��ʱ18ms����
    TEMP_SENSOR_DATA = 1;
    TEMP_SENSOR_DATA_IN;


    //TEMP_SENSOR_DATA = 0;
    /*test = TEMP_SENSOR_DATA;
    while(test == 1){
        test = TEMP_SENSOR_DATA;
    };
    asm ("  NOP");
    //DELAY_US(10);

    test = TEMP_SENSOR_DATA;

    asm ("  NOP");*/

    Temp_Sensor_AnswerFlag = 0;    //��������Ӧ��־
    j = 0;
    while (TEMP_SENSOR_DATA){
        if (++j >= 500){                      //��ֹ������ѭ��
            asm ("  NOP");
            break;
        }
    }
    if (!TEMP_SENSOR_DATA) {

        Temp_Sensor_AnswerFlag = 1;    //�յ���ʼ�ź�

        //�жϴӻ��Ƿ񷢳� 80us �ĵ͵�ƽ��Ӧ�ź��Ƿ����
        j = 0;
        //while (!TEMP_SENSOR_DATA);
        while(!TEMP_SENSOR_DATA){
            if(++j>=500){ //��ֹ������ѭ��{
                Temp_Sensor_ErrorFlag = 1;
                break;
            }
        }

        //�жϴӻ��Ƿ񷢳� 80us �ĸߵ�ƽ���緢����������ݽ���״̬
        j = 0;
        //while (TEMP_SENSOR_DATA);
        while(TEMP_SENSOR_DATA){
            //��ֹ������ѭ��
            if(++j>=800) {
                Temp_Sensor_ErrorFlag = 1;
                break;
            }
        }

        Humi_H = DHT11_rec_byte();            //����ʪ�ȸ߰�λ
        Humi_L = DHT11_rec_byte();            //����ʪ�ȵͰ�λ
        Temp_H = DHT11_rec_byte();            //�����¶ȸ߰�λ
        Temp_L = DHT11_rec_byte();            //�����¶ȵͰ�λ
        Temp_CAL = DHT11_rec_byte();          //����У��λ

        DELAY_US(25);    //����

        //У��
        if ((Humi_H + Humi_L + Temp_H + Temp_L) == Temp_CAL) {
            /* RH=Humi_H;
             RL=Humi_L;
             TH=Temp_H;
             TL=Temp_L;*/

            humidity = Humi_H * 10 + Humi_L; //ʪ��

            if (Temp_L & 0X80){         //Ϊ���¶�
                temperature = 0 - (Temp_H * 10 + ((Temp_L & 0x7F)));
            } else{                     //Ϊ���¶�
                temperature = Temp_H * 10 + Temp_L;
            }
            //�ж������Ƿ񳬹����̣��¶ȣ�-20��~60�棬ʪ��20��RH~95��RH��
            if (humidity > 950) {
                humidity = 950;
            }
            if (humidity < 200) {
                humidity = 200;
            }
            if (temperature > 600) {
                temperature = 600;
            }
            if (temperature < -200) {
                temperature = -200;
            }
            temperature = temperature / 10;//����Ϊ�¶�ֵ
            humidity = humidity / 10; //����Ϊʪ��ֵ
        }
    }
    TEMP_SENSOR_DATA_OUT;
    TEMP_SENSOR_DATA = 1;
}
