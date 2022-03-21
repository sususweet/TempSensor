/**
 * 文件名:  F28377_PM25_10_UART.c
 * 功能:  PM2.5传感器 模块驱动程序
 * 作者:  tangyq
 * GPIO 43--RXD
 * GPIO 42--TXD
 *
 */
#include "F28x_Project.h"
#include "F28377_Common.h"
#include "F28377_PM25_10_UART.h"
#include "string.h"

/**
 * The SCI, SPI, and McBSP modules can communicate at bit rates that are much slower than the CPU
 * frequency. These modules are connected to a shared clock divider, which generates a low-speed
 * peripheral clock (LSPCLK) derived from SYSCLK. LSPCLK uses a /4 divider by default, but the ratio can
 * be changed via the LOSPCP register. Each SCI, SPI, and McBSP module's clock (PERx.LSPCLK) can be
 * gated independently via the PCLKCRx registers.
 */
#define DELAY_MS(x) DELAY_US(x*1000)
#define SCIA_BAUD        9600
#define SCIA_PRD         (LSPCLK_FREQ/SCI_BAUD/8)-1

unsigned char scia_rxbuf[80];
unsigned int scia_rxcnt = 0;

int frameLen = MAX_FRAME_LEN;
uint16_t calcChecksum = 0;
bool inFrame = false;

uint16_t PM25_Value = 0;
uint16_t PM10_Value = 0;

struct PMS5003T_framestruct {
    uint8_t frameHeader[2];
    uint16_t frameLen;
    uint16_t concPM1_0_CF1;
    uint16_t concPM2_5_CF1;
    uint16_t concPM10_0_CF1;
    uint16_t concPM1_0_amb;
    uint16_t concPM2_5_amb;
    uint16_t concPM10_0_amb;
    uint16_t rawGt0_3um;
    uint16_t rawGt0_5um;
    uint16_t rawGt1_0um;
    uint16_t rawGt2_5um;
    uint16_t temperature;
    uint16_t humidity;
    uint8_t version;
    uint8_t errorCode;
    uint16_t checksum;
} pms5003Frame;

//
// wifi_scia_init - Initialize the SCI FIFO
//
void wifi_scia_init() {
    // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    SciaRegs.SCICCR.all = 0x0007;

    SciaRegs.SCICTL1.all = 0x0023;  // Relinquish SCI from Reset
    // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    //SciaRegs.SCICTL2.all = 0x0023;

    SciaRegs.SCICTL2.bit.TXINTENA = 1;          //发送中断使能
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;        //接收中断使能

    //
    // SCIA at 9600 baud
    // @LSPCLK = 25 MHz (100 MHz SYSCLK) HBAUD = 0x01 and LBAUD = 0x44.
    //
    SciaRegs.SCIHBAUD.all = 0x0001;
    SciaRegs.SCILBAUD.all = 0x0044;

    // Enable loop back
    SciaRegs.SCICCR.bit.LOOPBKENA = 0;

    //Enable FIFO
    /*SciaRegs.SCIFFTX.all = 0xE040;
    SciaRegs.SCIFFRX.all = 0x2044;
    SciaRegs.SCIFFCT.all = 0x0;*/

    //SciaRegs.SCIFFTX.all = 0xC022;
    //SciaRegs.SCIFFRX.all = 0x0022;
    //SciaRegs.SCIFFCT.all = 0x00;

    SciaRegs.SCIFFTX.bit.TXFIFORESET = 1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;
}

void PM25_UART_Init(){
    //
    // For this example, only init the pins for the SCI-A port.
    //  GPIO_SetupPinMux() - Sets the GPxMUX1/2 and GPyMUX1/2 register bits
    //  GPIO_SetupPinOptions() - Sets the direction and configuration of the GPIOS
    // These functions are found in the F2837xS_Gpio.c file.
    //
    GPIO_SetupPinMux(43, GPIO_MUX_CPU2, 15);
    GPIO_SetupPinOptions(43, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(42, GPIO_MUX_CPU2, 15);
    GPIO_SetupPinOptions(42, GPIO_OUTPUT, GPIO_ASYNC);
    //
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    //
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.SCIA_RX_INT = &sciaRxFifoIsr;
    PieVectTable.SCIA_TX_INT = &sciaTxFifoIsr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

    //
    // Step 4. User specific code:
    //

    wifi_scia_init();       // Initialize the SCI FIFO and for echoback.

    IER |= M_INT9;                       // Enable CPU INT
    //
    // Enable interrupts required for this example
    //
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER9.bit.INTx1 = 1;   // PIE Group 9, INT1
    PieCtrlRegs.PIEIER9.bit.INTx2 = 1;   // PIE Group 9, INT2
}

//
// sciaTxFifoIsr - scia Transmit FIFO ISR
//
interrupt void sciaTxFifoIsr(void) {
    //Uint16 i;
    //char *msg;
    //msg = "\r\n\n\nHello World!\0";
    //scia_msg(msg);

    SciaRegs.SCIFFTX.bit.TXFFINTCLR=1;   // Clear SCI Interrupt flag
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;       // Issue PIE ACK
}


void PMS5003_Analysis(){

}
//
// sciaRxFifoIsr - scia Receive FIFO ISR
//
interrupt void sciaRxFifoIsr(void) {
    unsigned char ReceivedChar;
    //char *msg;
    //
    // Get character
    //
    //while(SciaRegs.SCIFFRX.bit.RXFFST == 0);
    ReceivedChar = (unsigned char) SciaRegs.SCIRXBUF.all;

    //
    // Echo character back
    //
    //scic_xmit(ReceivedChar);

    if (!inFrame) {
        if (ReceivedChar == 0x42 && scia_rxcnt == 0) {
            scia_rxbuf[scia_rxcnt] = ReceivedChar;
            pms5003Frame.frameHeader[0] = ReceivedChar;
            calcChecksum = ReceivedChar; // Checksum init!
            scia_rxcnt++;
        } else if (ReceivedChar == 0x4D && scia_rxcnt == 1) {
            scia_rxbuf[scia_rxcnt] = ReceivedChar;
            pms5003Frame.frameHeader[1] = ReceivedChar;
            calcChecksum += ReceivedChar;
            inFrame = true;
            scia_rxcnt++;
        }
    }else{
        scia_rxbuf[scia_rxcnt] = ReceivedChar;
        calcChecksum += ReceivedChar;
        scia_rxcnt++;
        uint16_t val = scia_rxbuf[scia_rxcnt - 1] + (scia_rxbuf[scia_rxcnt - 2] << 8);
        switch (scia_rxcnt) {
            case 4:
                pms5003Frame.frameLen = val;
                frameLen = val + scia_rxcnt;
                break;
            case 6:
                pms5003Frame.concPM1_0_CF1 = val;
                break;
            case 8:
                pms5003Frame.concPM2_5_CF1 = val;
                break;
            case 10:
                pms5003Frame.concPM10_0_CF1 = val;
                break;
            case 12:
                pms5003Frame.concPM1_0_amb = val;
                break;
            case 14:
                pms5003Frame.concPM2_5_amb = val;
                break;
            case 16:
                pms5003Frame.concPM10_0_amb = val;
                break;
            case 18:
                pms5003Frame.rawGt0_3um = val;
                break;
            case 20:
                pms5003Frame.rawGt0_5um = val;
                break;
            case 22:
                pms5003Frame.rawGt1_0um = val;
                break;
            case 24:
                pms5003Frame.rawGt2_5um = val;
                break;
            case 26:
                pms5003Frame.temperature = val;
                break;
            case 28:
                pms5003Frame.humidity = val;
                break;
            case 29:
                val = scia_rxbuf[scia_rxcnt - 1];
                pms5003Frame.version = val;
                break;
            case 30:
                val = scia_rxbuf[scia_rxcnt - 1];
                pms5003Frame.errorCode = val;
                break;
            case 32:
                pms5003Frame.checksum = val;
                calcChecksum -= ((val >> 8) + (val & 0xFF));
                break;
            default:
                break;
        }

        if (scia_rxcnt >= frameLen) {
            scia_rxcnt = 0;
            inFrame = false;
            PM25_Value = pms5003Frame.concPM2_5_amb;
            PM10_Value = pms5003Frame.concPM10_0_amb;
        }
    }

    /*if(scia_rxcnt >= 100 || ReceivedChar == '\n') {
        scia_rxbuf[scia_rxcnt]='\0';
        scia_rxcnt = 0;
     //   NMEA_GPRMC_Analysis(scia_rxbuf);	//GPRMC解析
    }else if(ReceivedChar == '$'){
        scia_rxcnt = 0;
        scia_rxbuf[scia_rxcnt++]= ReceivedChar;
    }else{
        scia_rxbuf[scia_rxcnt++]=ReceivedChar;
    }*/


    //
    // Echo character back
    //
    //scia_xmit(ReceivedChar);

    /*rxbuf[rxcnt++]=ReceivedChar;
    if(rxcnt>20) rxcnt=0;*/

    SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
    SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP9;       // Issue PIE ack
}

//
// scia_xmit - Transmit a character from the SCI
//
void scia_xmit(int a) {
    //while (sciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    while (SciaRegs.SCICTL2.bit.TXRDY != 1);
    SciaRegs.SCITXBUF.all = a;
}

//
// scia_msg - Transmit message via scia
//
void scia_msg(char *msg) {
    int i;
    i = 0;
    while(msg[i] != '\0') {
        scia_xmit(msg[i]);
        i++;
        //DELAY_US(1000);
    }
}
