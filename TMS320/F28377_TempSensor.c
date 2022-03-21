/**
 * 文件名:  F28377_TempSensor.c
 * 功能:  温度传感器驱动程序
 * 作者:  tangyq
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

unsigned char Temp_Sensor_AnswerFlag = 0;   //定义传感器响应标志
unsigned char Temp_Sensor_ErrorFlag;        //定义读取传感器错误标志

void DHT11_Gpio_Init() {


    /* 通过I/O功能选择寄存器配置通用I/O功能*/
    //GpioCtrlRegs.GPAMUX1.all = 0x000000;   // 配置GPIO0-GPIO15为通用I/O口

    /* 设置I/O口为输出引脚*/

    EALLOW;
    GpioCtrlRegs.GPCMUX1.bit.GPIO66 = 0;
    EDIS;


    // 每个输入口可以有不同的输入限定
    // a) 输入与系统时钟 SYSCLKOUT同步
    // b) 输入被指定的采样窗口限定
    // c) 输入异步 (仅对外设输入有效)
    //GpioCtrlRegs.GPAQSEL1.all = 0x0000;    // GPIO0-GPIO15与系统时钟SYSCLKOUT 同步
    TEMP_SENSOR_DATA_OUT;
    TEMP_SENSOR_DATA = 1;
}

//接收一个字节
unsigned char DHT11_rec_byte() {
    Uint16 j;
    unsigned char i, dat = 0;
    for (i = 0; i < 8; i++) {                        //从高到低依次接收8位数据

        j = 0;
        //while (!TEMP_SENSOR_DATA);
        while (!TEMP_SENSOR_DATA) {                 //等待50us低电平过去
            if (++j >= 500) {
                asm ("  NOP");
                break;
            }
        }

        DELAY_US(30);                               //延时30us，如果还为高则数据为1，否则为0
        dat <<= 1;                                  //移位使正确接收8位数据，数据为0时直接移位
        if (TEMP_SENSOR_DATA == 1) dat += 1;       //数据为1时，使dat加1来接收数据1

        j = 0;
        //while (TEMP_SENSOR_DATA);
        while (TEMP_SENSOR_DATA){                   //等待数据线拉低
            if (++j >= 500){                      //防止进入死循环
                asm ("  NOP");
                break;
            }
        }
    }
    return dat;
}

//接收40位的数据
void DHT11_receive() {
    unsigned char Humi_H, Humi_L, Temp_H, Temp_L, Temp_CAL;
    //主机拉低18ms
    Uint16 j;

    //TEMP_SENSOR_DATA_OUT;
    //DELAY_US(10);
    TEMP_SENSOR_DATA = 0;
    DELAY_MS(20);   //延时18ms以上
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

    Temp_Sensor_AnswerFlag = 0;    //传感器响应标志
    j = 0;
    while (TEMP_SENSOR_DATA){
        if (++j >= 500){                      //防止进入死循环
            asm ("  NOP");
            break;
        }
    }
    if (!TEMP_SENSOR_DATA) {

        Temp_Sensor_AnswerFlag = 1;    //收到起始信号

        //判断从机是否发出 80us 的低电平响应信号是否结束
        j = 0;
        //while (!TEMP_SENSOR_DATA);
        while(!TEMP_SENSOR_DATA){
            if(++j>=500){ //防止进入死循环{
                Temp_Sensor_ErrorFlag = 1;
                break;
            }
        }

        //判断从机是否发出 80us 的高电平，如发出则进入数据接收状态
        j = 0;
        //while (TEMP_SENSOR_DATA);
        while(TEMP_SENSOR_DATA){
            //防止进入死循环
            if(++j>=800) {
                Temp_Sensor_ErrorFlag = 1;
                break;
            }
        }

        Humi_H = DHT11_rec_byte();            //接收湿度高八位
        Humi_L = DHT11_rec_byte();            //接收湿度低八位
        Temp_H = DHT11_rec_byte();            //接收温度高八位
        Temp_L = DHT11_rec_byte();            //接收温度低八位
        Temp_CAL = DHT11_rec_byte();          //接收校正位

        DELAY_US(25);    //结束

        //校正
        if ((Humi_H + Humi_L + Temp_H + Temp_L) == Temp_CAL) {
            /* RH=Humi_H;
             RL=Humi_L;
             TH=Temp_H;
             TL=Temp_L;*/

            humidity = Humi_H * 10 + Humi_L; //湿度

            if (Temp_L & 0X80){         //为负温度
                temperature = 0 - (Temp_H * 10 + ((Temp_L & 0x7F)));
            } else{                     //为正温度
                temperature = Temp_H * 10 + Temp_L;
            }
            //判断数据是否超过量程（温度：-20℃~60℃，湿度20％RH~95％RH）
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
            temperature = temperature / 10;//计算为温度值
            humidity = humidity / 10; //计算为湿度值
        }
    }
    TEMP_SENSOR_DATA_OUT;
    TEMP_SENSOR_DATA = 1;
}
