// 函数名：unsigned charFloat2Char(float Value,char *array)
// 描 述：将浮点数的各个位的数值转换成字符串，通过串口发送至上位机显示
// 参 数：float Value为欲转换的正数浮点数值，转换结果保存在字符数组*array里

unsigned char Float2Char(float value, unsigned char *array) {
    unsigned char IntegerPart;
    float DecimalPart;
    unsigned char i = 0;
    unsigned char j = 0;
    char temp;

//分离整数和小数
    if (value >= 1) {
        IntegerPart = (unsigned char) value;
        DecimalPart = value - IntegerPart;
    } else {
        IntegerPart = 0;
        DecimalPart = value - IntegerPart;
    }

    //处理整数部分
    if (IntegerPart == 0) {
        array[0] = '0';
        array[1] = '.';
        i = 1;
    } else {
        while (IntegerPart > 0) {
            array[i] = IntegerPart % 10 + '0';
            IntegerPart /= 10;
            i++;
        }
        i--;

//fix the result
        for (j = 0; j < i; j++) {
            temp = array[j];
            array[j] = array[i - j];
            array[i - j] = temp;
        }
        i++;
        array[i] = '.';
    }

//convert the Decimalpart
    i++;
    array[i++] = (unsigned char) (DecimalPart * 10) % 10 + '0';
    array[i++] = (unsigned char) (DecimalPart * 100) % 10 + '0';
    array[i++] = (unsigned char) (DecimalPart * 1000) % 10 + '0';
    array[i++] = (unsigned char) (DecimalPart * 10000) % 10 + '0';
    array[i] = '\0';

    return i;
}
