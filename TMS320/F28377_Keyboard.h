//
// Created by tangyq on 2018/5/7.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H

void Key_GPIO_Init(void);

void scan_key();

/**
 * @desc: ɨ����̷��ؼ�ֵ
 * @return: unsigned char, ��������
 * */
unsigned char read_key();
/**
 * @desc:�ж��Ƿ��м�����
 * @return: 1, �м�����; 0, �޼�����
 * */
unsigned char press_key();

#endif //KEYBOARD_H
