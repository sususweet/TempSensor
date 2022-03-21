//
// Created by tangyq on 2017/7/18.
//

#include "F28x_Project.h"
#include "F28377_Keyboard.h"

#define KEY_WAIT 4    /*����ɨ���ӳ�����*/
#define NONE_KEY_NUM 0

/****************�˿ں궨��*****************/
#define R1 GpioDataRegs.GPBDAT.bit.GPIO60
#define R2 GpioDataRegs.GPBDAT.bit.GPIO61
#define R3 GpioDataRegs.GPBDAT.bit.GPIO43
#define R4 GpioDataRegs.GPBDAT.bit.GPIO42
#define ROW_OUT EALLOW;\
                GpioCtrlRegs.GPBDIR.bit.GPIO60 = 1; GpioCtrlRegs.GPBDIR.bit.GPIO61 = 1;\
                GpioCtrlRegs.GPBDIR.bit.GPIO43 = 1; GpioCtrlRegs.GPBDIR.bit.GPIO42 = 1;\
                EDIS;
#define ROW_IN  EALLOW;\
                GpioCtrlRegs.GPBDIR.bit.GPIO60 = 0; GpioCtrlRegs.GPBDIR.bit.GPIO61 = 0;\
                GpioCtrlRegs.GPBDIR.bit.GPIO43 = 0; GpioCtrlRegs.GPBDIR.bit.GPIO42 = 0;\
                GpioCtrlRegs.GPBPUD.bit.GPIO60 = 0; GpioCtrlRegs.GPBPUD.bit.GPIO61 = 0;\
                GpioCtrlRegs.GPBPUD.bit.GPIO43 = 0; GpioCtrlRegs.GPBPUD.bit.GPIO42 = 0;\
                EDIS;
#define ROW_OUT_HIGH  ROW_OUT; R1 = 1; R2 = 1; R3 = 1; R4 = 1;
#define ROW_OUT_LOW  ROW_OUT; R1 = 0; R2 = 0; R3 = 0; R4 = 0;

#define C4 GpioDataRegs.GPCDAT.bit.GPIO71
#define C3 GpioDataRegs.GPCDAT.bit.GPIO90
#define C2 GpioDataRegs.GPCDAT.bit.GPIO89
#define C1 GpioDataRegs.GPBDAT.bit.GPIO41
#define COL_OUT EALLOW;\
                GpioCtrlRegs.GPCDIR.bit.GPIO71 = 1; GpioCtrlRegs.GPCDIR.bit.GPIO90 = 1;\
                GpioCtrlRegs.GPCDIR.bit.GPIO89 = 1; GpioCtrlRegs.GPBDIR.bit.GPIO41 = 1;\
                EDIS;
#define COL_IN  EALLOW;\
                GpioCtrlRegs.GPCDIR.bit.GPIO71 = 0; GpioCtrlRegs.GPCDIR.bit.GPIO90 = 0;\
                GpioCtrlRegs.GPCDIR.bit.GPIO89 = 0; GpioCtrlRegs.GPBDIR.bit.GPIO41 = 0;\
                GpioCtrlRegs.GPCPUD.bit.GPIO71 = 0; GpioCtrlRegs.GPCPUD.bit.GPIO90 = 0;\
                GpioCtrlRegs.GPCPUD.bit.GPIO89 = 0; GpioCtrlRegs.GPBPUD.bit.GPIO41 = 0;\
                EDIS;
#define COL_OUT_HIGH COL_OUT; C1 = 1; C2 = 1; C3 = 1; C4 = 1;
#define COL_OUT_LOW COL_OUT; C1 = 0; C2 = 0; C3 = 0; C4 = 0;

enum key_states_e {
    KEY_STATE_RELEASE,
    KEY_STATE_WAITING,
    KEY_STATE_PRESSED
};

void Key_GPIO_Init(void) {
    ROW_OUT_LOW;
    COL_IN;
}

void scan_key() {
    static unsigned char key_state = KEY_STATE_RELEASE;   /*״̬��״̬��ʼ��������static����״̬*/
    static unsigned char key_code = NONE_KEY_NUM;
    unsigned char pressed = press_key(); /*press_keyΪ����Ƿ��а������µĺ���*/
    static unsigned char scan_time = 0;
    switch (key_state) {
        case KEY_STATE_RELEASE: {   /*��ԭʼ״̬Ϊ�ް�������RELEASE��ͬʱ�ּ�⵽�������£���״̬ת����WAITING*/
            if (pressed == 1) {
                key_state = KEY_STATE_WAITING;
            }
            break;
        }
        case KEY_STATE_WAITING: {   /*ԭʼ״̬ΪWAITING���԰������ж���ж�*/
            if (pressed) {
                scan_time++;
                if (scan_time >= KEY_WAIT) {   /*���������µ�ʱ�䳬��һ��ʱ�䣬����Ϊ�������£�������*/
                    key_state = KEY_STATE_PRESSED;
                    scan_time = 0;
                    key_code = read_key();  /*read_keyΪ�������ĺ���*/
                }
            } else {    /*�������ɿ�����ָ�����ʼ״̬*/
                scan_time = 0;
                key_state = KEY_STATE_RELEASE;
            }
            break;
        }
        case KEY_STATE_PRESSED: {   /*��������ȷ�ϰ��£���ȴ������ɿ��ٽ��в���*/
            if (pressed == 0) {
                //opr_key(key_code);  /*opr_keyΪ�����¼���Ӧ����*/
                key_state = KEY_STATE_RELEASE;
                key_code = NONE_KEY_NUM;
            }
            break;
        }
        default: {
            key_state = KEY_STATE_RELEASE;
            break;
        }

    }
}

/**
 * @desc: ɨ����̷��ؼ�ֵ
 * @return: unsigned char, ��������
 * */
unsigned char read_key() {
    Uint16 x = 0, y = 0;
    unsigned char key_code;                              // ɨ����

    if (!C1) x = 0x10;
    if (!C2) x = 0x20;
    if (!C3) x = 0x30;
    if (!C4) x = 0x40;

    COL_OUT_LOW;
    ROW_IN;
    DELAY_US(200); //���������������ת
    if (!R1) y = 0x01;
    if (!R2) y = 0x02;
    if (!R3) y = 0x03;
    if (!R4) y = 0x04;
    ROW_OUT_LOW;
    COL_IN;
    x |= y;
    switch (x) {
        case 0x11: key_code = 1;break;
        case 0x21: key_code = 2;break;
        case 0x31: key_code = 3;break;
        case 0x41: key_code = 4;break;
        case 0x12: key_code = 5;break;
        case 0x22: key_code = 6;break;
        case 0x32: key_code = 7;break;
        case 0x42: key_code = 8;break;
        case 0x13: key_code = 9;break;
        case 0x23: key_code = 10;break;
        case 0x33: key_code = 11;break;
        case 0x43: key_code = 12;break;
        case 0x14: key_code = 13;break;
        case 0x24: key_code = 14;break;
        case 0x34: key_code = 15;break;
        case 0x44: key_code = 16;break;
        default:
            key_code = NONE_KEY_NUM;
    }

    return key_code;
}

/**
 * @desc:�ж��Ƿ��м�����
 * @return: 1, �м�����; 0, �޼�����
 * */
unsigned char press_key() {
    Uint16 i = C1 & C2 & C3 & C4;  //����ж�ȡ���е͵�ƽ˵���а�������
    if (i == 0) return 1;
    return 0;
}
