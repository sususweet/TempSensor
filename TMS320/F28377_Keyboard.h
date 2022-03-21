//
// Created by tangyq on 2018/5/7.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H

void Key_GPIO_Init(void);

void scan_key();

/**
 * @desc: 扫描键盘返回键值
 * @return: unsigned char, 完整键码
 * */
unsigned char read_key();
/**
 * @desc:判断是否有键按下
 * @return: 1, 有键按下; 0, 无键按下
 * */
unsigned char press_key();

#endif //KEYBOARD_H
