/**
 * main.c
 */
#include "stdio.h"
#include "string.h"

#include "F28x_Project.h"
#include "F28377_LCD12864.h"
#include "F28377_Keyboard.h"
#include "F28377_PM25Sensor.h"
#include "F28377_TempSensor.h"
#include "F28377_ESP8266_UART.h"
#include "F28377_Common.h"
#include "F28377_ATGM336H_UART.h"
#include "nmea0183.h"
#include "F28377_PM25_10_UART.h"

#ifndef _FLASH
#define _FLASH
#endif

#define LCD_REFRESH 100    /*LCD刷新周期 1s*/
#define N 20
#define DELAY_MS(x) DELAY_US(x*1000)
//16个字符/行
const unsigned char Screen_Startup[4][17]={
        {"  空气质量检测  "},
        {"                "},
        {"  系统启动中    "},
        {"                "},
};
const unsigned char Screen_Main[4][17]={
        {"温度00.0℃      "},
        {"湿度00.0%       "},
        {"PM1 0:000ug/m3  "},
        {"PM2.5:000ug/m3  "},
};

const unsigned char Screen_Wifi_GPS_Status[2][17]={
        {"禁用"},
        {"启用"}
};
const unsigned char air_quality[6][10]={
        {"优      "},
        {"良      "},
        {"轻度污染"},
        {"中度污染"},
        {"重度污染"},
        {"严重污染"}
};

extern uint16_t PM25_Value;
extern uint16_t PM10_Value;
float PM25_Array[2];

float PM25_Array_buf[N];
char i = 0;
unsigned char PM25_filter_ready_flag = 0;

float PM25_Sum = 0;
unsigned int PM25_index = 0;
unsigned char PM25_time_flag = 0;
unsigned char PM25_ready_flag = 0;

float humidity = 0;
float temperature = 0;
Nmea_msg gpsData;

unsigned char PM25_str[6];
unsigned char PM10_str[6];
unsigned char humidity_str[6];
unsigned char temperature_str[6];
unsigned char longitude_str[10] = "0\0";
unsigned char latitude_str[10] = "0\0";

unsigned char system_ready_flag = 0;
unsigned char wifi_connect_flag = 0;

void InitTMS(void);
void LCD_Show_Startup();
void LCD_Show_Main();
void LCD_Show_Update();
void two_second_filter();
float low_filter(float low_buf[]);
float smooth_filter();
float filter();
float mid_filter();
void MemCopy(Uint16 *SourceAddr, Uint16 *SourceEndAddr, Uint16 *DestAddr);
__interrupt void cpu_timer0_isr(void);

void main(void) {
    InitTMS();

    //
    // Step 2. Initialize GPIO:
    // This example function is found in the F2837xS_Gpio.c file and
    // illustrates how to set the GPIO to it's default state.
    //
    InitGpio();
    LCD_Gpio_Init();
    LCD_init();
    //
    // Step 3. Clear all __interrupts and initialize PIE vector table:
    //
    DINT;

    // Initialize the PIE control registers to their default state.
    // The default state is all PIE __interrupts disabled and flags
    // are cleared.
    // This function is found in the F2837xS_PieCtrl.c file.
    // 初始化中断寄存器
    InitPieCtrl();

    //
    // Disable CPU __interrupts and clear all CPU __interrupt flags:
    //
    IER = 0x0000;
    IFR = 0x0000;

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the __interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in F2837xS_DefaultIsr.c.
    // This function is found in F2837xS_PieVect.c.
    // 初始化PIE矢量表
    InitPieVectTable();

    //
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    //
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

    //
    // Step 4. Initialize the Device Peripheral. This function can be
    //         found in F2837xS_CpuTimers.c
    // 初始化CPU定时器
    InitCpuTimers();   // For this example, only initialize the Cpu Timers

    //
    // Configure CPU-Timer 0 to __interrupt every 500 milliseconds:
    // 60MHz CPU Freq, 50 millisecond Period (in uSeconds)
    //
    //ConfigCpuTimer(&CpuTimer0, 10, 10000000);  //1s

    /*Configure CPU-Timer 0 to __interrupt every 10 milliseconds:*/
    //ConfigCpuTimer(&CpuTimer0, 10, 5000000);
    ConfigCpuTimer(&CpuTimer0, CPU_FREQ/1000000, 10000);
    //
    // To ensure precise timing, use write-only instructions to write to the entire
    // register. Therefore, if any of the configuration bits are changed in
    // ConfigCpuTimer and InitCpuTimers (in F2837xS_cputimervars.h), the below
    // settings must also be updated.
    //
    CpuTimer0Regs.TCR.all = 0x4001;

    //
    // Step 5. User specific code, enable __interrupts:
    // Configure GPIO34 as a GPIO output pin
    //
    //EALLOW;
    //GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;
    //GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;
    //EDIS;

    //
    // Enable CPU INT1 which is connected to CPU-Timer 0:
    //
    IER |= M_INT1;

    //
    // Enable TINT0 in the PIE: Group 1 __interrupt 7
    //
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    LCD_Show_Startup();

    GPS_UART_Init();
    //
    // Enable global Interrupts and higher priority real-time debug events:
    //
    EINT;   // Enable Global __interrupt INTM
    ERTM;   // Enable Global realtime __interrupt DBGM

    //
    //Configure the ADCs and power them up
    //
    ConfigureADC();

    //
    //Setup the ADCs for software conversions
    //
    SetupADCSoftware();

    Sensor_Gpio_Init();
    //Sensor_PWM_Init();
    DHT11_Gpio_Init();

    Key_GPIO_Init();

    EALLOW;
    GpioCtrlRegs.GPADIR.bit.GPIO13 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO12 = 1;
    EDIS;
    //GPIO_SetupPinOptions(17, GPIO_OUTPUT, GPIO_PUSHPULL);
    //GPIO_SetupPinMux(17, GPIO_MUX_CPU1, 0);
    DELAY_US(10);
    GpioDataRegs.GPADAT.bit.GPIO13 = 1;
    DELAY_US(10);
    GpioDataRegs.GPADAT.bit.GPIO12 = 0;

    PM25_UART_Init();

   /**char* msg;
    msg = "Hello World!\0";
    scib_msg(msg);*/
    //DELAY_US(2000000);
    DELAY_MS(3000);
    wifi_UART_Init();
    DELAY_MS(200);
    //wifi_ConnectServer();
    //LCD_Clear_Screen();
    LCD_Show_Main();
    system_ready_flag = 1;

    while (1) {
        /*dustADC_Value = ReadADC();
        if (dustADC_Value>36.455){
            ValueArray[arrayindex] = dustADC_Value;
            PM25_Value = (float) (((float)dustADC_Value / 4096 * 3.3 - 0.0356) * 120000 * 0.035);
            arrayindex ++;
            if (arrayindex>= 100) {
                arrayindex =0;
                asm ("  NOP");
            }
        }*/
        /*PM25_Array[1] = Sensor_Get_PM25();
        PM25_Value = low_filter(PM25_Array);
        PM25_Array[0] = PM25_Array[1];*/

        //GpioDataRegs.GPADAT.bit.GPIO13 = 1;
        //DELAY_US(10);
        GpioDataRegs.GPADAT.bit.GPIO12 = 1;

        //PM25_Value = mid_filter();
        //two_second_filter();

        //GpioDataRegs.GPADAT.bit.GPIO13 = 1;
        //DELAY_US(10);
        GpioDataRegs.GPADAT.bit.GPIO12 = 0;

        DHT11_receive();

        //DELAY_US(2000000);



        //PM25_Value = Sensor_Get_PM25();
        /*GpioDataRegs.GPADAT.bit.GPIO13 = 0;
        DELAY_US(10);
        GpioDataRegs.GPADAT.bit.GPIO12 = 1;
        DELAY_US(1000000);
        GpioDataRegs.GPADAT.bit.GPIO13 = 1;
        DELAY_US(10);
        GpioDataRegs.GPADAT.bit.GPIO12 = 0;
        DELAY_US(1000000);*/
    }
}
void two_second_filter() {
    if (PM25_time_flag == 0){
        PM25_Sum += mid_filter();
        PM25_index++;
    }else if  (PM25_time_flag == 1){
        PM25_Value = PM25_Sum / PM25_index;
        PM25_time_flag = 0;
        PM25_Sum = 0;
        PM25_index = 0;
        PM25_ready_flag = 1;
    }
}
float low_filter(float low_buf[]){
    float sample_value;
    float X=0.01;
    sample_value=(1-X)*low_buf[1]+X*low_buf[0];
    return sample_value;
}

float smooth_filter(){
    char count;
    float sum = 0;
    PM25_Array_buf[i] = Sensor_Get_PM25();
    i++;
    //当数据大于数组长度，替换数据组的一个数据  相当于环形队列更新，先进先出！
    if(i == N) {
        i = 0;
        PM25_filter_ready_flag = 1;
        for(count = 0; count < N; count++) sum+=PM25_Array_buf[count];
        return sum / N;
    }else{
        if (PM25_filter_ready_flag == 0) {
            return PM25_Array_buf[i-1];
        }else {
            for(count = 0;count < N;count++) sum+=PM25_Array_buf[count];
            return sum / N;
        }
    }
}
float mid_filter() {
    char count,i,j;
    float value_buf[N];
    float sum=0,temp=0;
    for  (count=0;count<N;count++) {
        value_buf[count] = Sensor_Get_PM25();
        //DELAY_US(100);
    }
    for (j=0;j<N;j++) {
        for (i=0;i<N-j;i++) {
            if (value_buf[i]>value_buf[i+1]){
                temp = value_buf[i];
                value_buf[i] = value_buf[i+1];
                value_buf[i+1] = temp;
            }
        }
    }
    for(count=1;count<N-1;count++) sum += value_buf[count];
    return sum/(N-2);
}
float filter() {
    float value_buf[N],temp;
    char count,i,j;
    for (count=0;count<N;count++) {
        value_buf[count] = Sensor_Get_PM25();
        DELAY_US(100);
    }
    for (j=0;j<N-1;j++) {
        for (i=0;i<N-j;i++) {
            if (value_buf[i]>value_buf[i+1]) {
                temp = value_buf[i];
                value_buf[i] = value_buf[i+1];
                value_buf[i+1] = temp;
            }
        }
    }
    return value_buf[(N-1)/2];
}

void Data_String_Update() {
    //float longtitude, latitude;

    PM25_str[0] = (unsigned char) ('0' + (unsigned char) PM25_Value / 100);
    PM25_str[1] = (unsigned char) ('0' + (unsigned char) PM25_Value % 100 / 10);
    PM25_str[2] = (unsigned char) ('0' + (unsigned char) PM25_Value % 100 % 10);
    PM25_str[3] = '\0';

    PM10_str[0] = (unsigned char) ('0' + (unsigned char) PM10_Value / 100);
    PM10_str[1] = (unsigned char) ('0' + (unsigned char) PM10_Value % 100 / 10);
    PM10_str[2] = (unsigned char) ('0' + (unsigned char) PM10_Value % 100 % 10);
    PM10_str[3] = '\0';

    temperature_str[0] = (unsigned char) ('0' + (unsigned char) temperature / 10);
    temperature_str[1] = (unsigned char) ('0' + (unsigned char) temperature % 10);
    temperature_str[2] = '.';
    temperature_str[3] = (unsigned char) ('0' + (unsigned char) (temperature * 10) % 10);
    temperature_str[4] = '\0';

    humidity_str[0] = (unsigned char) ('0' + (unsigned char) humidity / 10);
    humidity_str[1] = (unsigned char) ('0' + (unsigned char) humidity % 10);
    humidity_str[2] = '.';
    humidity_str[3] = (unsigned char) ('0' + (unsigned char) (humidity * 10) % 10);
    humidity_str[4] = '\0';
/*
    if(gpsData.navMode == 'A' || gpsData.navMode == 'E' || gpsData.navMode == 'D'){
        longtitude = (float) (1.0 * gpsData.longitude / 100000);
        longitude_str[0] = (unsigned char) ('0' + (unsigned char)longtitude / 100);
        longitude_str[1] = (unsigned char) ('0' + (unsigned char)longtitude % 100 / 10);
        longitude_str[2] = (unsigned char) ('0' + (unsigned char)longtitude % 100 % 10);
        longitude_str[3] = '.';
        longitude_str[4] = (unsigned char) ('0' + (unsigned char)(longtitude * 10) % 10);
        longitude_str[5] = '\0';

        latitude = (float) (1.0 * gpsData.latitude / 100000);
        latitude_str[0] = (unsigned char) ('0' + (unsigned char)latitude / 100);
        latitude_str[1] = (unsigned char) ('0' + (unsigned char)latitude % 100 / 10);
        latitude_str[2] = (unsigned char) ('0' + (unsigned char)latitude % 100 % 10);
        latitude_str[3] = '.';
        latitude_str[4] = (unsigned char) ('0' + (unsigned char)(latitude * 10) % 10);
        latitude_str[5] = '\0';
    }else{
        longitude_str[0] = '0';
        longitude_str[1] = '\0';
        latitude_str[0] = '0';
        latitude_str[1] = '\0';
    }
*/
     if(gpsData.navMode == 'A' || gpsData.navMode == 'E' || gpsData.navMode == 'D'){
         Float2Char((float) (1.0 * gpsData.longitude / 100000), longitude_str);
         Float2Char((float) (1.0 * gpsData.latitude / 100000), latitude_str);
     }else{
         longitude_str[0] = '0';
         longitude_str[1] = '\0';
         latitude_str[0] = '0';
         latitude_str[1] = '\0';
     }
}

void LCD_Show_Startup(){
    LCD_Show(1, 1, (unsigned char *)Screen_Startup[0]);
    LCD_Show(2, 1, (unsigned char *)Screen_Startup[1]);
    LCD_Show(3, 1, (unsigned char *)Screen_Startup[2]);
    LCD_Show(4, 1, (unsigned char *)Screen_Startup[3]);
}

void LCD_Show_Main() {
    LCD_Show(1, 1, (unsigned char *)Screen_Main[0]);
    LCD_Show(2, 1, (unsigned char *)Screen_Main[1]);
    LCD_Show(3, 1, (unsigned char *)Screen_Main[2]);
    LCD_Show(4, 1, (unsigned char *)Screen_Main[3]);
    LCD_Show_Update();
}

void LCD_Show_Update() {
    /*if (wifi_connect_flag == 1){
        LCD_Show(1, 3, (unsigned char *) Screen_Wifi_GPS_Status[1]);
    }else{
        LCD_Show(1, 3, (unsigned char *) Screen_Wifi_GPS_Status[0]);
    }

    if(gpsData.navMode == 'A' || gpsData.navMode == 'E' || gpsData.navMode == 'D'){
        LCD_Show(1, 7, (unsigned char *) Screen_Wifi_GPS_Status[1]);
    }else{
        LCD_Show(1, 7, (unsigned char *) Screen_Wifi_GPS_Status[0]);
    }
*/
    /*if (PM25_Value >= 0 && PM25_Value <= 35) {
        LCD_Show(2, 4, (unsigned char *) air_quality[0]);
    } else if (PM25_Value <= 75) {
        LCD_Show(2, 4, (unsigned char *)air_quality[1]);
    } else if (PM25_Value <= 115) {
        LCD_Show(2, 4, (unsigned char *)air_quality[2]);
    } else if (PM25_Value <= 150) {
        LCD_Show(2, 4, (unsigned char *)air_quality[3]);
    } else if (PM25_Value <= 250) {
        LCD_Show(2, 4, (unsigned char *)air_quality[4]);
    } else if (PM25_Value <= 500) {
        LCD_Show(2, 4, (unsigned char *)air_quality[5]);
    }*/

    if (PM10_Value >= 0){
        LCD_Show(3, 4, PM10_str);
    }

    if (PM25_Value >= 0){
        LCD_Show(4, 4, PM25_str);
    }

    if (temperature >= 0){
        LCD_Show(1, 3, temperature_str);
    }

    if (humidity >= 0){
        LCD_Show(2, 3, humidity_str);
    }


    /*if (PM25_Value >= 0){
        LCD_Position(3, 4);
        LCD_WriteData('0' + (unsigned char) PM25_Value / 100);
        LCD_WriteData('0' + (unsigned char) PM25_Value % 100 / 10);
        LCD_WriteData('0' + (unsigned char) PM25_Value % 100 % 10);
        LCD_WriteData('.');
        LCD_WriteData('0' + (unsigned char) (PM25_Value * 10) % 10);
    }

    if (temperature >= 0){
        LCD_Position(4, 3);
        LCD_WriteData('0' + (unsigned char) temperature / 10);
        LCD_WriteData('0' + (unsigned char) temperature % 10);
        LCD_WriteData('.');
        LCD_WriteData('0' + (unsigned char) (temperature * 10) % 10);
    }

    if (humidity >= 0){
        LCD_Position(4, 7);
        LCD_WriteData('0' + (unsigned char) humidity / 10);
        LCD_WriteData('0' + (unsigned char) humidity % 10);
        LCD_WriteData('.');
        LCD_WriteData('0' + (unsigned char) (humidity * 10) % 10);
    }*/
   
        //LCD_Show_Get_Data(waterPressure);
        /*displayCache[0] = ' ';
        displayCache[1] = ' ';
        displayCache[2] = '\0';
        LCD_Show(4, 3, displayCache);
        LCD_Show(4, 7, displayCache);*/


        //sprintf(displayCache,"%.1f", GetPressure(Capture_voltage));


        /*sprintf(displayCache,"%.1f",frequency / 7.5);
        //waterFlow = (unsigned int) (frequency / 7.5 * 10);
        //LCD_Show_Get_Data(waterFlow);
        LCD_Show(4, 6, displayCache);*/

   // sprintf(displayCache,"%6.5f",Capture_voltage);
    //waterPressure = (unsigned int) (Capture_voltage * 10);
    //LCD_Show_Get_Data(waterPressure);


    /*waterFlow = (unsigned int) (frequency / 7.5 * 10);
    LCD_Show_Get_Data(waterFlow);
    LCD_Show(4, 6, displayCache);*/
}

//
// cpu_timer0_isr - CPU Timer0 ISR that toggles GPIO32 once per 500ms
//
__interrupt void cpu_timer0_isr(void) {     //10ms 进中断
    CpuTimer0.InterruptCount++;
    //GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1;

    scan_key();

    if (CpuTimer0.InterruptCount % 200 == 0 && system_ready_flag == 1){
        PM25_time_flag = 1;
    }

    if (CpuTimer0.InterruptCount % 300 == 0 && system_ready_flag == 1){
        Data_String_Update();
        LCD_Show_Update();
    }

    if (CpuTimer0.InterruptCount % 1000 == 0){
        if (wifi_connect_flag == 1){
            wifi_sendData();
        }else if (wifi_connect_flag == 0){
            wifi_ConnectServer();
        }
    }

    /*lcd_twinkle_num++;

    if (lcd_twinkle_num >= LCD_REFRESH) {   //500MS
        lcd_twinkle_num = 0;
        LCD_Show_Update();
    }*/


    //
    // Acknowledge this __interrupt to receive more __interrupts from group 1
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

void InitTMS(void) {
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the DSP2803x_SysCtrl.c file.
    InitSysCtrl();
    InitSysPll(XTAL_OSC,IMULT_20,FMULT_0,PLLCLK_BY_2);
    //PLLSYSCLK = 10Mhz(OSCCLK) * 20 (IMULT) * 1 (FMULT) / 2 (PLLCLK_BY_2)
#ifdef _FLASH
    // Copy time critical code and Flash setup code to RAM
    // The  RamfuncsLoadStart, RamfuncsLoadEnd, and RamfuncsRunStart
    // symbols are created by the linker. Refer to the linker files.
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
    //MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
#endif

#ifdef _FLASH
    // Call Flash Initialization to setup flash waitstates
    // This function must reside in RAM
    InitFlash_Bank0();
    InitFlash_Bank1();// Call the flash wrapper init function
#endif //(FLASH)
}

void MemCopy(Uint16 *SourceAddr, Uint16 *SourceEndAddr, Uint16 *DestAddr) {
    while (SourceAddr < SourceEndAddr) {
        *DestAddr++ = *SourceAddr++;
    }
}

