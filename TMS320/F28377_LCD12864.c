/**
 * 文件名:   LCD12864.c
 * 功能:   I/O口实现12864液晶显示功能；
 * 作者: tangyq
 * GPIO 12--DB7
 * GPIO 13--DB6
 * GPIO 14--DB5
 * GPIO 15--DB4
 * GPIO 16--DB3
 * GPIO 17--DB2
 * GPIO 18--DB1
 * GPIO 19--DB0
 * GPIO 59--PSB  H-并行  L-串行
 * GPIO 72--E
 * GPIO 73--RW
 * GPIO 78--RS
 */

/**
 *    根据在RAM中调试的需要，这个项目配置成"boot to SARAM".280x引导模式
 *    表如下显示. 常用的还有"boot to Flash"模式，当程序在RAM调试完善后就
 *    可以将代码烧进Flash中并使用"boot to Flash"引导模式.
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
    // 用户根据需要可以使能内部上拉或禁止.
    // 其他不需要的行注释掉；
    /*GpioCtrlRegs.GPCPUD.bit.GPIO72 = 1;   // 禁止引脚内部上拉
    GpioCtrlRegs.GPCPUD.bit.GPIO78 = 0;   // 使能引脚内部上拉
    GpioCtrlRegs.GPCPUD.bit.GPIO73 = 0;   // 使能引脚内部上拉*/

    /* 通过I/O功能选择寄存器配置通用I/O功能*/
    //GpioCtrlRegs.GPAMUX1.all = 0x000000;   // 配置GPIO0-GPIO15为通用I/O口

    /* 设置I/O口为输出引脚*/
    GpioCtrlRegs.GPBDIR.bit.GPIO59 = 1;
    GpioCtrlRegs.GPCDIR.bit.GPIO72 = 1;
    GpioCtrlRegs.GPCDIR.bit.GPIO73 = 1;
    GpioCtrlRegs.GPCDIR.bit.GPIO78 = 1;
    GpioCtrlRegs.GPADIR.all |= 0x000FF000;// 配置GPIO12-GPIO19为输出引脚

    // 每个输入口可以有不同的输入限定
    // a) 输入与系统时钟 SYSCLKOUT同步
    // b) 输入被指定的采样窗口限定
    // c) 输入异步 (仅对外设输入有效)
    //GpioCtrlRegs.GPAQSEL1.all = 0x0000;    // GPIO0-GPIO15与系统时钟SYSCLKOUT 同步
    EDIS;
    //输出数据LCD_RS置1和LCD_EN清零
    LCD_PSB = 1;
    LCD_RS = 1;
    LCD_EN = 0;
    LCD_RW = 0;
}

/**
* 名称 : Convert(uchar In_Date)
* 功能 : 因为电路设计时，P0.0--P0.7接法刚好了资料中的相反，所以设计该函数。
* 输入 : 1602资料上的值
* 输出 : 送到1602的值
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
;模块名称:LCD_init();
;功    能:初始化LCD1602
;占用资源:--
;参数说明:--
;-------------------------------------*/
void LCD_init() {
    DELAY_MS(100);                                          //延迟100ms
    LCD_WriteCommand(0x34);                                 //扩充指令操作
    DELAY_MS(5);
    LCD_WriteCommand(0x30);                                 //基本指令操作
    DELAY_MS(5);
    LCD_WriteCommand(0x0C);                                 //显示开，关光标
    DELAY_MS(5);
    LCD_WriteCommand(0x01);                                 //清除LCD的显示内容
    DELAY_MS(5);

}

/**
 * 模块名称:LCD_WriteCommand();
 * 功    能:LCD1602写指令函数
 * 参数说明:command为写命令参数
 */
void LCD_WriteCommand(unsigned char command) {
    LCD_RS = 0;         //指令
    LCD_RW = 0;
    LCD_EN = 0;          //允许
    DELAY_US(40);
    LCD_DB = (((Uint32)Convert(command)) << 12) & 0x000ff000;  //赋值给D0-D7对应的GPIO12-GPIO19引脚
    DELAY_US(50);
    LCD_EN = 1;
    DELAY_US(50);
    LCD_EN = 0;
}

/**
 * 模块名称:LCD_WriteData();
 * 功    能:LCD1602写数据函数
 * 占用资源: P2.0--RS(LCD_RS),P2.1--RW(LCD_RW),P2.2--E(LCD_E).
 * 参数说明:dat为写数据参数
 */
void LCD_WriteData(unsigned char dat) {
    LCD_RS = 1;         //指令
    LCD_RW = 0;
    LCD_EN = 0;
    DELAY_US(20);
    LCD_DB = (((Uint32)Convert(dat)) << 12) & 0x000ff000;  //赋值给D0-D7对应的GPIO12-GPIO19引脚
    DELAY_US(50);
    LCD_EN = 1;
    DELAY_US(50);
    LCD_EN = 0;
}

/**
 * 清屏函数
 */
void LCD_Clear_Screen() {
    LCD_WriteCommand(0x34);                                  //扩充指令操作
    DELAY_MS(5);
    LCD_WriteCommand(0x30);                                  //基本指令操作
    DELAY_MS(5);
    LCD_WriteCommand(0x01);                                  //清屏
    DELAY_MS(5);
}

/**
 * 设定汉字显示位置
 * eg:LCD_Position(4,0); ----表示从第四行的第0个字符开始写
 * 即从第四行的第0个字开始写
 * eg:LCD_Position(4,1); ----表示从第四行的第2个字符开始写
 * --------------------------即从第四行的第1个字开始写
 **/
void LCD_Position(unsigned char X, unsigned char Y) {
    unsigned char Pos;
    if (X == 1) X = 0x80;
    else if (X == 2) X = 0x90;
    else if (X == 3) X = 0x88;
    else if (X == 4) X = 0x98;
    Pos = (unsigned char) (X + Y - 1);
    LCD_WriteCommand(Pos);                                   //显示地址
}

void LCD_Show(unsigned char x, unsigned char y, unsigned char *str) {
    LCD_Position(x,y);
    while (*str != '\0')  //连续写入字符串数据，直到检测到结束符
    {
        LCD_WriteData(*str++);
    }
}
/**********************************************************
; 显示字符表代码
**********************************************************/
void LCD_Char_Display() {
    unsigned char s;
    LCD_Clear_Screen();                                      //清屏
    LCD_WriteCommand(0x80);                                  //设置显示位置为第一行
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x30 + s);
    }
    LCD_WriteCommand(0x90);                                  //设置显示位置为第二行
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x40 + s);
    }
    LCD_WriteCommand(0x88);                                  //设置显示位置为第三行
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x50 + s);
    }
    LCD_WriteCommand(0x98);                                  //设置显示位置为第四行
    for (s = 0; s < 16; s++) {
        LCD_WriteData(0x60 + s);
    }
}
/*********************************************************
*                                                        *
* 图形显示                                               *
*                                                        *
*********************************************************/
void LCD_Photo_Display(const unsigned char *Bmp) {
    unsigned char i, j;

    LCD_WriteCommand(0x34);                                   //写数据时,关闭图形显示

    for (i = 0; i < 32; i++) {
        LCD_WriteCommand(0x80 + i);                               //先写入水平坐标值
        LCD_WriteCommand(0x80);                                 //写入垂直坐标值
        for (j = 0; j < 16; j++)                                       //再写入两个8位元的数据
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
    LCD_WriteCommand(0x36);                                   //写完数据,开图形显示
}
