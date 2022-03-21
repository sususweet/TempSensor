/**
 * �ļ���:  F28377_PM25Sensor.c
 * ����:  PM2.5��������������
 * ����:  tangyq
 * GPIO 11--SENSOR_LED_OUT
 *
 */
#include "F28x_Project.h"
#include "F28377_PM25Sensor.h"

#define DELAY_MS(x) DELAY_US(x*1000)
// SENSOR_LED_POWER is any digital pin on the arduino connected to Pin 3 on the sensor
#define SENSOR_LED_POWER GpioDataRegs.GPADAT.bit.GPIO11
#define delayTime 280
#define delayTime2 40
#define offTime 9680

void Sensor_Gpio_Init() {
    EALLOW;
    // �û�������Ҫ����ʹ���ڲ��������ֹ.
    // ��������Ҫ����ע�͵���
    /*GpioCtrlRegs.GPCPUD.bit.GPIO72 = 1;   // ��ֹ�����ڲ�����
    GpioCtrlRegs.GPCPUD.bit.GPIO78 = 0;   // ʹ�������ڲ�����
    GpioCtrlRegs.GPCPUD.bit.GPIO73 = 0;   // ʹ�������ڲ�����*/

    /* ͨ��I/O����ѡ��Ĵ�������ͨ��I/O����*/
    //GpioCtrlRegs.GPAMUX1.all = 0x000000;   // ����GPIO0-GPIO15Ϊͨ��I/O��

    /* ����I/O��Ϊ�������*/
    GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;

    // ÿ������ڿ����в�ͬ�������޶�
    // a) ������ϵͳʱ�� SYSCLKOUTͬ��
    // b) ���뱻ָ���Ĳ��������޶�
    // c) �����첽 (��������������Ч)
    //GpioCtrlRegs.GPAQSEL1.all = 0x0000;    // GPIO0-GPIO15��ϵͳʱ��SYSCLKOUT ͬ��
    EDIS;
    SENSOR_LED_POWER = 1;
    //�������LCD_RS��1��LCD_EN����
}

void Sensor_PWM_Init(){
    //
    // enable PWM1 and PWM2
    //
    //CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;

    //
    // InitEPwmGpio_TZ - Initialize EPWM1A and EPWM2A GPIOs
    //
    EALLOW;
    //GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;    // Disable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;    // Disable pull-up on GPIO2 (EPWM2A)

    //GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;   // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;   // Configure GPIO2 as EPWM2A

    //GpioCtrlRegs.GPADIR.bit.GPIO0=1;    //����GPIO0�������
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;    //����GPIO2�������
    EDIS;

    ///////////////////////////////////////////////////////////////////
    //ʱ���׼�Ĵ���  CPU 50MHz
    EPwm2Regs.TBPRD = 49999;             // Period = 49999+1 ��TBʱ������  1ms
    EPwm2Regs.TBPHS.all = 0;            // ��λΪ��
    EPwm2Regs.TBCTR = 0;                  //��������ʼֵΪ��

    EPwm2Regs.CMPA.bit.CMPA = 25000;
    //EPwm2Regs.CMPB.bit.CMPB = 4000;

    //����TBCLK=50MHz
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;  //����ʱ���׼ʱ��Ԥ��Ƶ
    EPwm2Regs.TBCTL.bit.CLKDIV = 0;     //ʱ���׼ʱ��Ԥ��Ƶ

    EPwm2Regs.TBCTL.bit.CTRMODE = 0;    //��ģʽ

    //�趨����
    EPwm2Regs.AQCTLA.bit.CAU =0x1; //��ģʽ������������CPMAʱePWMA������
    EPwm2Regs.AQCTLA.bit.PRD =0x2; //��ģʽ����������������ֵʱwPWMA������
    //EPwm2Regs.AQCTLB.bit.CBU =0x1; //��ģʽ������������CPMBʱePWMB������
    //EPwm2Regs.AQCTLB.bit.PRD =0x2; //��ģʽ����������������ֵʱePWMB������

}

float Sensor_Get_PM25(){
    Uint16 dustDCVal = 0;
    float calcVoltage = 0;
    float dustDensity = 0;

    SENSOR_LED_POWER = 0;
    DELAY_US(delayTime);
    //dustVal = analogRead(dustPin);
    dustDCVal = ReadADC();
    DELAY_US(delayTime2);
    SENSOR_LED_POWER = 1;
    DELAY_US(offTime);

    //pm25_value = (float) (((float)dustDCVal / 4096 - 0.0356) * 120000 * 0.035);

    //asm("   ESTOP0");
    //if (dustDCVal>36.455){
    // 0 - 5V mapped to 0 - 1023 integer values
    // recover voltage
    calcVoltage = (float) (dustDCVal * (3.3 / 4096.0));     //��ģ��ֵת��Ϊ��ѹֵ

    dustDensity = (calcVoltage) * 1000 / 10;//*1000�����ǽ���λת��Ϊug/m3

    //dustDensity = (float) ((0.17 * calcVoltage / 3.3 * 5.0 - 0.1) * 1000); //����ѹֵת��Ϊ�۳��ܶ������λ ug/m3
    //dustDensity = (float) ((0.1667 * calcVoltage - 0.10002) * 1000);
    /*dustDensity = (float) ((calcVoltage - 0.0356) * 120000 * 0.035);
    dustDensity = (calcVoltage) * 1000 / 10;//*1000�����ǽ���λת��Ϊug/m3
    dustDensity = (calcVoltage * 5 / 3.3) * 1000 / 10;//*1000�����ǽ���λת��Ϊug/m3*/
    /*calcVoltage = (float) (dustDCVal * (3.3 / 4096.0));     //��ģ��ֵת��Ϊ��ѹֵ
    dustDensity = (calcVoltage) * 1000 / 10;//*1000�����ǽ���λת��Ϊug/m3
        //dustDensity = (float) (0.17 * calcVoltage - 0.1);       //����ѹֵת��Ϊ�۳��ܶ������λ
        //pm25_value = (float) (((float)dustDCVal / 4096.0 *3.3 - 0.0356) * 120000 * 0.035);
    //asm ("  NOP");
    //}*/
    return dustDensity;
    //pm25_value = (float(dustVal/1024)-0.0356)*120000*0.035;
}

Uint16 ReadADC(){
    Uint16 AdcaResult0;
    //
    //convert, wait for completion, and store results
    //start conversions immediately via software, ADCA
    //
    AdcaRegs.ADCSOCFRC1.all = 0x0003; //SOC0 and SOC1

    //
    //start conversions immediately via software, ADCB
    //
    //AdcbRegs.ADCSOCFRC1.all = 0x0003; //SOC0 and SOC1

    //
    //wait for ADCA to complete, then acknowledge flag
    //
    while(AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0);
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

    //
    //wait for ADCB to complete, then acknowledge flag
    //
    //while(AdcbRegs.ADCINTFLG.bit.ADCINT1 == 0);
    //AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;

    //
    //store results
    //
    AdcaResult0 = AdcaResultRegs.ADCRESULT0;
    //AdcaResult1 = AdcaResultRegs.ADCRESULT1;
    //AdcbResult0 = AdcbResultRegs.ADCRESULT0;
    //AdcbResult1 = AdcbResultRegs.ADCRESULT1;

    //
    //at this point, conversion results are stored in
    //AdcaResult0, AdcaResult1, AdcbResult0, and AdcbResult1
    //

    //
    //software breakpoint, hit run again to get updated conversions
    //
    //asm("   ESTOP0");
    return AdcaResult0;
}


//
// ConfigureADC - Write ADC configurations and power up the ADC for both
//                ADC A and ADC B
//
void ConfigureADC(void) {
    EALLOW;
    //
    //write configurations
    //
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    //AdcbRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    //AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

    //
    //Set pulse positions to late
    //
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    //AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    //
    //power up the ADCs
    //
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    //AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;

    //
    //delay for 1ms to allow ADC time to power up
    //
    DELAY_US(1000);
    EDIS;
}


//
// SetupADCSoftware - Setup ADC channels and acquisition window
//
void SetupADCSoftware(void) {
    Uint16 acqps;

    //
    //determine minimum acquisition window (in SYSCLKS) based on resolution
    //
    if(ADC_RESOLUTION_12BIT == AdcaRegs.ADCCTL2.bit.RESOLUTION) {
        acqps = 14; //75ns
    }
    else{           //resolution is 16-bit
        acqps = 63; //320ns
    }

    //
    //Select the channels to convert and end of conversion flag
    //ADCA
    //
    EALLOW;
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;  //SOC0 will convert pin A0
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = acqps; //sample window is acqps +
    //1 SYSCLK cycles

    //AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;  //SOC1 will convert pin A1
    //AdcaRegs.ADCSOC1CTL.bit.ACQPS = acqps; //sample window is acqps +

    //1 SYSCLK cycles
    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 1; //end of SOC1 will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared

    /*//ADCB
    AdcbRegs.ADCSOC0CTL.bit.CHSEL = 0;  //SOC0 will convert pin B0
    AdcbRegs.ADCSOC0CTL.bit.ACQPS = acqps; //sample window is acqps +
    //1 SYSCLK cycles
    AdcbRegs.ADCSOC1CTL.bit.CHSEL = 1;  //SOC1 will convert pin B1
    AdcbRegs.ADCSOC1CTL.bit.ACQPS = acqps; //sample window is acqps +
    //1 SYSCLK cycles
    AdcbRegs.ADCINTSEL1N2.bit.INT1SEL = 1; //end of SOC1 will set INT1 flag
    AdcbRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared*/

    EDIS;
}
