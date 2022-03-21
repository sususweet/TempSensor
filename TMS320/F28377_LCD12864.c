/**
 * �ļ���:   LCD12864.c
 * ����:   I/O��ʵ��12864Һ����ʾ���ܣ�
 * ����: tangyq
 * GPIO 12--DB7
 * GPIO 13--DB6
 * GPIO 14--DB5
 * GPIO 15--DB4
 * GPIO 16--DB3
 * GPIO 17--DB2
 * GPIO 18--DB1
 * GPIO 19--DB0
 * GPIO 59--PSB  H-����  L-����
 * GPIO 72--E
 * GPIO 73--RW
 * GPIO 78--RS
 */

/**
 *    ������RAM�е��Ե���Ҫ�������Ŀ���ó�"boot to SARAM".280x����ģʽ
 *    ��������ʾ. ���õĻ���"boot to Flash"ģʽ����������RAM�������ƺ��
 *    ���Խ������ս�Flash�в�ʹ��"boot to Flash"����ģʽ.
 *
 *        Boot      GPIO18     GPIO29    GPIO34
 *        Mode      SPICLKA    SCITXDA
 *                  SCITXB
 *        -------------------------------------
 *        Flash       1          1        1
 *        SCI-A       1          1        0
 *        SPI-A       1          0        1
 *        I2C-A       1          0        0
 *        ECAN-A      0          1        1
 *        SARAM       0          1        0  <- "boot to SARAM"
 *        OTP         0          0        1
 *        I/0         0          0        0
 *
 */

#include "F28x_Project.h"
#include "F28377_LCD12864.h"

#define DELAY_MS(x) DELAY_US(x*1000)

#define LCD_PSB GpioDataRegs.GPBDAT.bit.GPIO59
#define LCD_EN GpioDataRegs.GPCDAT.bit.GPIO72
#define LCD_RW GpioDataRegs.GPCDAT.bit.GPIO73
#define LCD_RS GpioDataRegs.GPCDAT.bit.GPIO78
#define LCD_DB GpioDataRegs.GPADAT.all

void LCD_Gpio_Init() {
    EALLOW;
    // �û�������Ҫ����ʹ���ڲ��������ֹ.
    // ��������Ҫ����ע�͵���
    /*GpioCtrlRegs.GPCPUD.bit.GPIO72 = 1;   // ��ֹ�����ڲ�����
    GpioCtrlRegs.GPCPUD.bit.GPIO78 = 0;   // ʹ�������ڲ�����
    GpioCtrlRegs.GPCPUD.bit.GPIO73 = 0;   // ʹ�������ڲ�����*/

    /* ͨ��I/O����ѡ��Ĵ�������ͨ��I/O����*/
    //GpioCtrlRegs.GPAMUX1.all = 0x000000;   // ����GPIO0-GPIO15Ϊͨ��I/O��

    /* ����I/O��Ϊ�������*/
    GpioCtrlRegs.GPBDIR.bit.GPIO59 = 1;
    GpioCtrlRegs.GPCDIR.bit.GPIO72 = 1;
    GpioCtrlRegs.GPCDIR.bit.GPIO73 = 1;
    GpioCtrlRegs.GPCDIR.bit.GPIO78 = 1;
    GpioCtrlRegs.GPADIR.all |= 0x000FF000;// ����GPIO12-GPIO19Ϊ�������

    // ÿ������ڿ����в�ͬ�������޶�
    // a) ������ϵͳʱ�� SYSCLKOUTͬ��
    // b) ���뱻ָ���Ĳ��������޶�
    // c) �����첽 (��������������Ч)
    //GpioCtrlRegs.GPAQSEL1.all = 0x0000;    // GPIO0-GPIO15��ϵͳʱ��SYSCLKOUT ͬ��
    EDIS;
    //�������LCD_RS��1��LCD_EN����
    LCD_PSB = 1;
    LCD_RS = 1;
    LCD_EN = 0;
    LCD_RW = 0;
}

/**
* ���� : Convert(uchar In_Date)
* ���� : ��Ϊ��·���ʱ��P0.0--P0.7�ӷ��պ��������е��෴��������Ƹú�����
* ���� : 1602�����ϵ�ֵ
* ��� : �͵�1602��ֵ
*/
unsigned char Convert(unsigned char In_Date) {
    unsigned char i, Out_Date = 0, temp = 0;
    for (i = 0; i < 8; i++) {
        temp = ((In_Date >> i) & 0x01);
        Out_Date |= (temp << (7 - i));
    }
    return Out_Date;
}
/**
;ģ������:LCD_init();
;��    ��:��ʼ��LCD1602
;ռ����Դ:--
;����˵��:--
;-------------------------------------*/
void LCD_init() {
    DELAY_MS(100);                                          //�ӳ�100ms
    LCD_WriteCommand(0x34);                                 //����ָ�����
    DELAY_MS(5);
    LCD_WriteCommand(0x30);                                 //����ָ�����
    DELAY_MS(5);
    LCD_WriteCommand(0x0C);                                 //��ʾ�����ع��
    DELAY_MS(5);
    LCD_WriteCommand(0x01);                                 //���LCD����ʾ����
    DELAY_MS(5);

}

/**
 * ģ������:LCD_WriteCommand();
 * ��    ��:LCD1602дָ���
 * ����˵��:commandΪд�������
 */
void LCD_WriteCommand(unsigned char command) {
    LCD_RS = 0;         //ָ��
    LCD_RW = 0;
    LCD_EN = 0;          //����
    DELAY_US(40);
    LCD_DB = (((Uint32)Convert(command)) << 12) & 0x000ff000;  //��ֵ��D0-D7��Ӧ��GPIO12-GPIO19����
    DELAY_US(50);
    LCD_EN = 1;
    DELAY_US(50);
    LCD_EN = 0;
}

/**
 * ģ������:LCD_WriteData();
 * ��    ��:LCD1602д���ݺ���
 * ռ����Դ: P2.0--RS(LCD_RS),P2.1--RW(LCD_RW),P2.2--E(LCD_E).
 * ����˵��:datΪд���ݲ���
 */
void LCD_WriteData(unsigned char dat) {
    LCD_RS = 1;         //ָ��
    LCD_RW = 0;
    LCD_EN = 0;
    DELAY_US(20);
    LCD_DB = (((Uint32)Convert(dat)) << 12) & 0x000ff000;  //��ֵ��D0-D7��Ӧ��GPIO12-GPIO19����
    DELAY_US(50);
    LCD_EN = 1;
    DELAY_US(50);
    LCD_EN = 0;
}

/**
 * ��������
 */
void LCD_Clear_Screen() {
    LCD_WriteCommand(0x34);                                  //����ָ�����
    DELAY_MS(5);
    LCD_WriteCommand(0x30);                                  //����ָ�����
    DELAY_MS(5);
    LCD_WriteCommand(0x01);                                  //����
    DELAY_MS(5);
}

/**
 * �趨������ʾλ��
 * eg:LCD_Position(4,0); ----��ʾ�ӵ����еĵ�0���ַ���ʼд
 * ���ӵ����еĵ�0���ֿ�ʼд
 * eg:LCD_Position(4,1); ----��ʾ�ӵ����еĵ�2���ַ���ʼд
 * --------------------------���ӵ����еĵ�1���ֿ�ʼд
 **/
void LCD_Position(unsigned char X, unsigned char Y) {
    unsigned char Pos;
    if (X == 1) X = 0x80;
    else if (X == 2) X = 0x90;
    else if (X == 3) X = 0x88;
    else if (X == 4) X = 0x98;
    Pos = (unsigned char) (X + Y - 1);
    LCD_WriteCommand(Pos);                                   //��ʾ��ַ
}

void LCD_Show(unsigned char x, unsigned char y, unsigned char *str) {
    LCD_Position(x,y);
    while (*str != '\0')  //����д���ַ������ݣ�ֱ����⵽������
    {
        LCD_WriteData(*str++);
    }
}
/**********************************************************
; ��ʾ�ַ������
**********************************************************/
void LCD_Char_Display() {
    unsigned char s;
    LCD_Clear_Screen();                                      //����
    LCD_WriteCommand(0x80);                                  //������ʾλ��Ϊ��һ��
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x30 + s);
    }
    LCD_WriteCommand(0x90);                                  //������ʾλ��Ϊ�ڶ���
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x40 + s);
    }
    LCD_WriteCommand(0x88);                                  //������ʾλ��Ϊ������
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x50 + s);
    }
    LCD_WriteCommand(0x98);                                  //������ʾλ��Ϊ������
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x60 + s);
    }
}
/*********************************************************
*                                                        *
* ͼ����ʾ                                               *
*                                                        *
*********************************************************/
void LCD_Photo_Display(const unsigned char *Bmp) {
    unsigned char i, j;

    LCD_WriteCommand(0x34);                                   //д����ʱ,�ر�ͼ����ʾ

    for (i = 0; i < 32; i++) {
        LCD_WriteCommand(0x80 + i);                               //��д��ˮƽ����ֵ
        LCD_WriteCommand(0x80);                                 //д�봹ֱ����ֵ
        for (j = 0; j < 16; j++)                                       //��д������8λԪ������
            LCD_WriteData(*Bmp++);
        DELAY_MS(1);
    }

    for (i = 0; i < 32; i++) {
        LCD_WriteCommand(0x80 + i);
        LCD_WriteCommand(0x88);
        for (j = 0; j < 16; j++)
            LCD_WriteData(*Bmp++);
        DELAY_MS(1);
    }
    LCD_WriteCommand(0x36);                                   //д������,��ͼ����ʾ
}
