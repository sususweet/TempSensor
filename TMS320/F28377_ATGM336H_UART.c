/**
 * 文件名:  F28377_ATGM336H_UART.c
 * 功能:  ATGM336H GPS 模块驱动程序
 * 作者:  tangyq
 * GPIO 62--RXD
 * GPIO 63--TXD
 *
 */
#include "F28x_Project.h"
#include "F28377_ATGM336H_UART.h"
#include "nmea0183.h"

/**
 * The SCI, SPI, and McBSP modules can communicate at bit rates that are much slower than the CPU
 * frequency. These modules are connected to a shared clock divider, which generates a low-speed
 * peripheral clock (LSPCLK) derived from SYSCLK. LSPCLK uses a /4 divider by default, but the ratio can
 * be changed via the LOSPCP register. Each SCI, SPI, and McBSP module's clock (PERx.LSPCLK) can be
 * gated independently via the PCLKCRx registers.
 */

#define SCIC_BAUD        9600
#define SCIC_PRD         (LSPCLK_FREQ/SCIC_BAUD/8)-1

unsigned char scic_rxbuf[80];
unsigned int scic_rxcnt = 0;
extern Nmea_msg gpsData;

//
// GPS_scic_init - Initialize the SCI FIFO
//
void GPS_scic_init() {
    // 1 stop bit,  No loopback
    // No parity,8 char bits,
    // async mode, idle-line protocol
    ScicRegs.SCICCR.all = 0x0007;

    ScicRegs.SCICTL1.all = 0x0023;  // Relinquish SCI from Reset
    // enable TX, RX, internal SCICLK,
    // Disable RX ERR, SLEEP, TXWAKE
    //ScicRegs.SCICTL2.all = 0x0023;

    ScicRegs.SCICTL2.bit.TXINTENA = 1;          //发送中断使能
    ScicRegs.SCICTL2.bit.RXBKINTENA = 1;        //接收中断使能

    //
    // SCIA at 9600 baud
    // @LSPCLK = 25 MHz (100 MHz SYSCLK) HBAUD = 0x01 and LBAUD = 0x44.
    //
    ScicRegs.SCIHBAUD.all = 0x0001;
    ScicRegs.SCILBAUD.all = 0x0044;

    // Enable loop back
    ScicRegs.SCICCR.bit.LOOPBKENA = 0;

    //Enable FIFO
    /*ScicRegs.SCIFFTX.all = 0xE040;
    ScicRegs.SCIFFRX.all = 0x2044;
    ScicRegs.SCIFFCT.all = 0x0;*/

    //ScicRegs.SCIFFTX.all = 0xC022;
    //ScicRegs.SCIFFRX.all = 0x0022;
    //ScicRegs.SCIFFCT.all = 0x00;

    ScicRegs.SCIFFTX.bit.TXFIFORESET = 1;
    ScicRegs.SCIFFRX.bit.RXFIFORESET = 1;
}

void GPS_UART_Init(){
    //
    // For this example, only init the pins for the SCI-A port.
    //  GPIO_SetupPinMux() - Sets the GPxMUX1/2 and GPyMUX1/2 register bits
    //  GPIO_SetupPinOptions() - Sets the direction and configuration of the GPIOS
    // These functions are found in the F2837xS_Gpio.c file.
    //
    GPIO_SetupPinMux(62, GPIO_MUX_CPU2, 1);
    GPIO_SetupPinOptions(62, GPIO_INPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(63, GPIO_MUX_CPU2, 1);
    GPIO_SetupPinOptions(63, GPIO_OUTPUT, GPIO_ASYNC);
    //
    // Interrupts that are used in this example are re-mapped to
    // ISR functions found within this file.
    //
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.SCIC_RX_INT = &scicRxFifoIsr;
    PieVectTable.SCIC_TX_INT = &scicTxFifoIsr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

    //
    // Step 4. User specific code:
    //

    GPS_scic_init();       // Initialize the SCI FIFO and for echoback.

    IER |= M_INT8;                       // Enable CPU INT
    //
    // Enable interrupts required for this example
    //
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
    PieCtrlRegs.PIEIER8.bit.INTx5 = 1;   // PIE Group 8, INT5
    PieCtrlRegs.PIEIER8.bit.INTx6 = 1;   // PIE Group 8, INT6
}

//
// scicTxFifoIsr - SCIC Transmit FIFO ISR
//
interrupt void scicTxFifoIsr(void) {
    //Uint16 i;
    //char *msg;
    //msg = "\r\n\n\nHello World!\0";
    //scic_msg(msg);



    ScicRegs.SCIFFTX.bit.TXFFINTCLR = 1;   // Clear SCI Interrupt flag
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP8;       // Issue PIE ACK
}

//
// scicRxFifoIsr - SCIC Receive FIFO ISR
//
interrupt void scicRxFifoIsr(void) {
    unsigned char ReceivedChar;
    //char *msg;
    //
    // Get character
    //
    //while(ScicRegs.SCIFFRX.bit.RXFFST == 0);
    ReceivedChar = (unsigned char) ScicRegs.SCIRXBUF.all;

    //
    // Echo character back
    //
    //scic_xmit(ReceivedChar);

    if(scic_rxcnt >= 79 || ReceivedChar == '\n') {
        scic_rxbuf[scic_rxcnt]='\0';
        scic_rxcnt = 0;
        NMEA_GPRMC_Analysis(&gpsData,scic_rxbuf);	//GPRMC解析
    }else if(ReceivedChar == '$'){
        scic_rxcnt = 0;
        scic_rxbuf[scic_rxcnt++]= ReceivedChar;
    }else{
        scic_rxbuf[scic_rxcnt++]=ReceivedChar;
    }

    ScicRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
    ScicRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag

    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP8;       // Issue PIE ack
}

//
// scic_xmit - Transmit a character from the SCI
//
void scic_xmit(int a) {
    //while (ScicRegs.SCIFFTX.bit.TXFFST != 0) {}
    while (ScicRegs.SCICTL2.bit.TXRDY != 1);
    ScicRegs.SCITXBUF.all = a;
}

//
// scic_msg - Transmit message via SCIC
//
void scic_msg(char *msg) {
    int i;
    i = 0;
    while(msg[i] != '\0') {
        scic_xmit(msg[i]);
        i++;
        //DELAY_US(1000);
    }
}
