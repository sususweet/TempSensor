/**
 * �ļ���:  F28377_ESP8266_UART.c
 * ����:  WIFI ģ����������
 * ����:  tangyq
 * GPIO 87--RXD
 * GPIO 86--TXD
 *
 */
#include "F28x_Project.h"
#include "F28377_Common.h"
#include "F28377_ESP8266_UART.h"
#include "string.h"

/**
 * The SCI, SPI, and McBSP modules can communicate at bit rates that are much slower than the CPU
 * frequency. These modules are connected to a shared clock divider, which generates a low-speed
 * peripheral clock (LSPCLK) derived from SYSCLK. LSPCLK uses a /4 divider by default, but the ratio can
 * be changed via the LOSPCP register. Each SCI, SPI, and McBSP module's clock (PERx.LSPCLK) can be
 * gated independently via the PCLKCRx registers.
 */
#define DELAY_MS(x) DELAY_US(x*1000)
#define SCI_BAUD        9600
#define SCI_PRD         (LSPCLK_FREQ/SCI_BAUD/8)-1
#define DEV_ID "\"device_f4557af1e63c355\""
#define DEV_SECRET "\"7f5d6ff0fdf2d40e952a\""

const unsigned char wifi_Reset[]="AT+RST\r\n\r\n";
const unsigned char wifi_Connect[]="AT+CWJAP=\"Xiaomi_778B\",\"NongfuSpringYeLao\"\r\n\r\n";
//AT+CIPSTART="TCP","192.168.199.134",8080  ����TCP������
const unsigned char wifi_TCPConnect[]="AT+CIPSTART=\"TCP\",\"123.206.112.29\",80\r\n\r\n";
const unsigned char wifi_CIPMODE[]="AT+CIPMODE=1\r\n\r\n";
//AT+CIPSEND=  ��������
const unsigned char wifi_CIPSEND[]="AT+CIPSEND\r\n\r\n";
const unsigned char wifi_CIPSENDOUT[]="+++";

extern float PM25_Value;
extern float humidity;
extern float temperature;

extern unsigned char PM25_str[6];
extern unsigned char PM10_str[6];
extern unsigned char humidity_str[6];
extern unsigned char temperature_str[6];
extern unsigned char longitude_str[6];
extern unsigned char latitude_str[6];

extern unsigned char wifi_connect_flag;
unsigned char wifi_receive_flag = 0;

unsigned char RxCharBuf_wifi[16];
unsigned int RxCharBuf_wifi_index = 0;
unsigned char Wifi_flag[16];
unsigned int Wifi_flag_index = 0;
//
// wifi_scib_init - Initialize the SCI FIFO
//
void wifi_scib_init() {
    // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    ScibRegs.SCICCR.all = 0x0007;

    ScibRegs.SCICTL1.all = 0x0023;  // Relinquish SCI from Reset
    // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    //ScibRegs.SCICTL2.all = 0x0023;

    ScibRegs.SCICTL2.bit.TXINTENA = 1;          //�����ж�ʹ��
    ScibRegs.SCICTL2.bit.RXBKINTENA = 1;        //�����ж�ʹ��

    //
    // SCIA at 9600 baud
    // @LSPCLK = 25 MHz (100 MHz SYSCLK) HBAUD = 0x01 and LBAUD = 0x44.
    //
    ScibRegs.SCIHBAUD.all = 0x0001;
    ScibRegs.SCILBAUD.all = 0x0044;

    // Enable loop back
    ScibRegs.SCICCR.bit.LOOPBKENA = 0;

    //Enable FIFO
    /*ScibRegs.SCIFFTX.all = 0xE040;
    ScibRegs.SCIFFRX.all = 0x2044;
    ScibRegs.SCIFFCT.all = 0x0;*/

    //ScibRegs.SCIFFTX.all = 0xC022;
    //ScibRegs.SCIFFRX.all = 0x0022;
    //ScibRegs.SCIFFCT.all = 0x00;

    ScibRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScibRegs.SCIFFRX.bit.RXFIFORESET = 1;
}

void wifi_UART_Init(){
    //
    // For this example, only init the pins for the SCI-A port.
    //  GPIO_SetupPinMux() - Sets the GPxMUX1/2 and GPyMUX1/2 register bits
    //  GPIO_SetupPinOptions() - Sets the direction and configuration of the GPIOS
    // These functions are found in the F2837xS_Gpio.c file.
    //
    GPIO_SetupPinMux(87, GPIO_MUX_CPU2, 5);
    GPIO_SetupPinOptions(87, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(86, GPIO_MUX_CPU2, 5);
    GPIO_SetupPinOptions(86, GPIO_OUTPUT, GPIO_ASYNC);
    //
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    //
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.SCIB_RX_INT = &scibRxFifoIsr;
    PieVectTable.SCIB_TX_INT = &scibTxFifoIsr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

    //
    // Step 4. User specific code:
    //

    wifi_scib_init();       // Initialize the SCI FIFO and for echoback.

    IER |= M_INT9;                       // Enable CPU INT
    //
    // Enable interrupts required for this example
    //
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx3 = 1;   // PIE Group 9, INT3
    PieCtrlRegs.PIEIER9.bit.INTx4 = 1;   // PIE Group 9, INT4
}

//***********************************************************************
//	wifi ATָ��ؼ��
//***********************************************************************
int wifi_checkReturn(unsigned char flag){                   //���flagΪ��Ҫ�������B�ĵ����ڼ����ַ������Ϊ����������������ַ��Ļ�������ɹ� ����1�����򷵻�0
    unsigned int length;
    Uint16 j;
    //����ATָ��ǰҪ�ֶ���������дָ���0��flagWr=RxBufWr_wifi=RxBufRd_wifi=0��,�����дָ�벻�����˵�������ݻش�������������ʱ�ȴ�
    j = 0;
    while(Wifi_flag_index == RxCharBuf_wifi_index){
        if (++j >= 5000){                      //��ֹ������ѭ��
            asm ("  NOP");
            return 0;
        }
    }
    length = (unsigned int) strlen(Wifi_flag);
    if(length < flag){
        return 0;
    }
    if(Wifi_flag[length-flag] == 'K')
        return 1;
    if(Wifi_flag[length-flag] == 0x3E)     //�ж��Ƿ�Ϊ >
        return 1;
    if(Wifi_flag[length-flag] == 'D')
        return 1;
    return 0;
}

void wifi_sendData(void){
    char *dev_id,*dev_id_content,*dev_secret,*dev_secret_content,*data1,*data2,*data3,*data4,*data5,*data6,*data7;
    int length;
    if (PM25_Value == 0 && humidity == 0 && temperature == 0) return;
    scib_msg("POST /Sensor/api/device/updateDeviceDetail HTTP/1.1");
    scib_msg("\r\n");
    scib_msg("Host:123.206.112.29\r\n");
    scib_msg("Connection: close\r\n");

    dev_id = "{\"dev_id\":";
    dev_id_content = DEV_ID;
    dev_secret = ",\"dev_secret\":";
    dev_secret_content = DEV_SECRET;

    data1 = ",\"data\":{\"temperature\":\"";

    data2 = "\",\"humidity\":\"";

    data3 = "\",\"PM25\":\"";

    data4 = "\",\"PM10\":\"";

    data5 = "\",\"longitude\":\"";

    data6 = "\",\"latitude\":\"";

    data7 = "\"}}\r\n\r\n";

    length = (int) (strlen(dev_id) + strlen(dev_id_content) + strlen(dev_secret) + strlen(dev_secret_content) +
                    strlen(data1) + strlen(data2) + strlen(data3) + strlen(data4) +
                    strlen(data5) + strlen(data6) + strlen(data7) + strlen(PM25_str) +
                    strlen(PM10_str) + strlen(humidity_str) +
                    strlen(temperature_str) + strlen(longitude_str) + strlen(latitude_str) - 4);

    scib_msg("Content-Length: ");
    scib_xmit(length / 100 + '0');
    scib_xmit(length % 100 / 10 + '0');
    scib_xmit(length % 100 % 10 + '0');
    scib_msg("\r\n\r\n");
    scib_msg(dev_id);
    scib_msg(dev_id_content);
    scib_msg(dev_secret);
    scib_msg(dev_secret_content);
    scib_msg(data1);
    scib_msg(temperature_str);
    scib_msg(data2);
    scib_msg(humidity_str);
    scib_msg(data3);
    scib_msg(PM25_str);
    scib_msg(data4);
    scib_msg(PM10_str);
    scib_msg(data5);
    scib_msg(longitude_str);
    scib_msg(data6);
    scib_msg(latitude_str);
    scib_msg(data7);
}

//***********************************************************************
//	���ӷ�����
//***********************************************************************
void wifi_ConnectServer(void) {
    Uint16 j;
    /*scib_msg(wifi_CIPSENDOUT);
    DELAY_US(1000000);
    scib_msg(wifi_Reset);
    DELAY_US(5000000);
    scib_msg(wifi_TCPConnect);
    DELAY_US(1000000);
    scib_msg(wifi_CIPMODE);
    DELAY_US(1000000);
    scib_msg(wifi_CIPSEND);
    DELAY_US(1000000);*/

    //scib_msg(wifi_CIPSENDOUT);
    //DELAY_MS(100);
    //scib_msg(wifi_Reset);
    //DELAY_MS(5000);

    wifi_receive_flag = 1;

    //scib_msg(wifi_Connect);
    //DELAY_MS(500);

    scib_msg(wifi_TCPConnect);             //���������Ϊ����AT+CIPSTART="TCP","www.icharm.me",80 �ĺ���
    //if(wifi_checkReturn(1) != 1 && wifi_checkReturn(2) != 1) return;   //�������B�����һ���ַ����ͨ���Ļ� ���˳�ѭ��
    //if() return;   //�������B�ĵ����ڶ����ַ����ͨ���� ���˳�ѭ��
    DELAY_MS(500);
    scib_msg(wifi_CIPMODE);                     //�����������ָ��AT+CIPMODE=1
    //if(wifi_checkReturn(1) != 1) return;   //�������B�ĵ����ڶ����ַ����ͨ���� ���˳�ѭ��
    DELAY_MS(500);
    scib_msg(wifi_CIPSEND);                     //�����������ָ��AT+CIPSEND
    //if(wifi_checkReturn(1) != 1) return;   //�������B�ĵ����ڶ����ַ����ͨ���� ���˳�ѭ��
    wifi_connect_flag = 1;
    /*j = 0;
    do{
        scib_msg(wifi_TCPConnect);             //���������Ϊ����AT+CIPSTART="TCP","www.icharm.me",80 �ĺ���
        if(wifi_checkReturn(1) == 1)         //�������B�����һ���ַ����ͨ���Ļ� ���˳�ѭ��
            break;
        if(wifi_checkReturn(2) == 1)         //�������B�ĵ����ڶ����ַ����ͨ���� ���˳�ѭ��
            break;
        if (++j >= 500){                      //��ֹ������ѭ��
            asm ("  NOP");
            return;
        }
    }while(1);
    DELAY_MS(500);
    j=0;
    do{
        scib_msg(wifi_CIPMODE);                     //�����������ָ��AT+CIPMODE=1
        if (++j >= 500){
            asm ("  NOP");
            return;
        }
    }while(wifi_checkReturn(1) == 0);
    DELAY_MS(500);
    j=0;
    do{
        scib_msg(wifi_CIPSEND);                     //�����������ָ��AT+CIPSEND
        if (++j >= 500){
            asm ("  NOP");
            return;
        }
    }while(wifi_checkReturn(1) == 0);
    wifi_connect_flag = 1;*/
}


//
// scibTxFifoIsr - SCIB Transmit FIFO ISR
//
interrupt void scibTxFifoIsr(void) {
    //Uint16 i;
    //char *msg;
    //msg = "\r\n\n\nHello World!\0";
    //scib_msg(msg);

    ScibRegs.SCIFFTX.bit.TXFFINTCLR=1;   // Clear SCI Interrupt flag
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;       // Issue PIE ACK
}

//
// scibRxFifoIsr - SCIB Receive FIFO ISR
//
interrupt void scibRxFifoIsr(void) {
    unsigned char ReceivedChar;
    //char *msg;
    //
    // Get character
    //
    //while(ScibRegs.SCIFFRX.bit.RXFFST == 0);
    ReceivedChar = (unsigned char) ScibRegs.SCIRXBUF.all;

    if (wifi_receive_flag){
        if(ReceivedChar == 0x0d){                  //��ÿһ���س���ǰ���һ���ַ�����¼�����������жϵı�־
            if(RxCharBuf_wifi_index == 0){          //RxCharBuf_wifi��Ϊ����A, wifi_flag��Ϊ����B�� RxBufWr_wifiΪ����A��д��ָ��, flagWrΪ����B��д��ָ��
                Wifi_flag[Wifi_flag_index] = RxCharBuf_wifi[15];
            }else{
                Wifi_flag[Wifi_flag_index] = RxCharBuf_wifi[RxCharBuf_wifi_index-1];
            }
            Wifi_flag_index++;
            Wifi_flag_index &= 0x0f;                //���鳤��Ϊ16,��д��ָ��Ϊ�ﵽ16ʱ �Զ�����
            if(Wifi_flag_index != 0)
                Wifi_flag[Wifi_flag_index] = 0x00; //�ý��յ��ַ�������ĳ���Ϊ��ȷ��

        }else if(ReceivedChar != 0x0a) {
            RxCharBuf_wifi[RxCharBuf_wifi_index] = ReceivedChar;
            RxCharBuf_wifi_index++;
            RxCharBuf_wifi_index &= 0x0f;            //16->0
            if(RxCharBuf_wifi_index != 0)
                RxCharBuf_wifi[RxCharBuf_wifi_index] = 0x00; //�ý��յ��ַ�������ĳ���Ϊ��ȷ�ģ��Ӹ�0x00��β��
        }
    }


    //
    // Echo character back
    //
    //scib_xmit(ReceivedChar);

    /*rxbuf[rxcnt++]=ReceivedChar;
    if(rxcnt>20) rxcnt=0;*/

    ScibRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
    ScibRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;       // Issue PIE ack
}

//
// scib_xmit - Transmit a character from the SCI
//
void scib_xmit(int a) {
    //while (ScibRegs.SCIFFTX.bit.TXFFST != 0) {}
    while (ScibRegs.SCICTL2.bit.TXRDY != 1);
    ScibRegs.SCITXBUF.all = a;
}

//
// scib_msg - Transmit message via SCIB
//
void scib_msg(char *msg) {
    int i;
    i = 0;
    RxCharBuf_wifi_index = Wifi_flag_index = 0;
    while(msg[i] != '\0') {
        scib_xmit(msg[i]);
        i++;
        //DELAY_US(1000);
    }
}
