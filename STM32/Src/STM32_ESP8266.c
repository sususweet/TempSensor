//
// Created by tangyq on 2017/9/29.
//

#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"

#include "string.h"
#include "common.h"
#include "STM32_ESP8266.h"
#include "STM32_PMS7003.h"

#define DEV_ID "\"device_11c342dbb5e5465\""
#define DEV_SECRET "\"0cfb1533b5c59f1fefc4\""

extern uint8_t usart2_RxBuffer[2];
extern uint8_t usart2_aRxData[USART1_RXDATASIZE];
extern unsigned char usart2_aRxData_Index;
extern __IO ITStatus UartReady;

extern PM_Sensor_DataStruct PM_Sensor_Data;
extern float humidity;
extern float temperature;
extern unsigned char PM25_str[6];
extern unsigned char PM10_str[6];
extern unsigned char humidity_str[6];
extern unsigned char temperature_str[6];
unsigned char wifi_receive_flag = 0;
unsigned char wifi_init_stage = WIFI_TO_CONNECTED;
unsigned char wifi_stage_change = 0;

/* Buffer used for transmission */
UART_HandleTypeDef huart3;

uint8_t usart2_aTxBuffer[] = " ****UART_TwoBoards_ComIT****";

const unsigned char wifi_Reset[]="AT+RST\r\n\r\n";
const unsigned char wifi_Connect[]="AT+CWJAP=\"TempSensor\",\"ESP8266TEST\"\r\n\r\n";
//AT+CIPSTART="TCP","192.168.199.134",8080  连接TCP服务器
const unsigned char wifi_ZJUWLAN_TCPConnect[]="AT+CIPSTART=\"TCP\",\"net.zju.edu.cn\",80\r\n\r\n";
const unsigned char wifi_ZJUWLAN_TCPClose[]="AT+CIPCLOSE\r\n\r\n";
const unsigned char wifi_TCPConnect[]="AT+CIPSTART=\"TCP\",\"123.206.112.29\",80\r\n\r\n";
const unsigned char wifi_CIPMODE[]="AT+CIPMODE=1\r\n\r\n";
//AT+CIPSEND=  发送数据
const unsigned char wifi_CIPSEND[]="AT+CIPSEND\r\n\r\n";
const unsigned char wifi_CIPSENDOUT[]="+++";

unsigned char RxCharBuf_wifi[16];
unsigned int RxCharBuf_wifi_index = 0;
unsigned char Wifi_flag[16];
unsigned int Wifi_flag_index = 0;

void _Error_Handler(char * file, int line);
/* USART3 init function */
void MX_USART3_UART_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 9600;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    //huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    //huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    //huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
    __HAL_UART_ENABLE(&huart3);
    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    if (HAL_UART_Receive_IT(&huart3, (uint8_t *) usart2_RxBuffer, 1) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
   // ESP8266_UART_Send((char *) wifi_Reset);
}
/* USART2 init function *//*
void MX_USART2_UART_Init(void) {

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 9600;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    //huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    //huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    //huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
    __HAL_UART_ENABLE(&huart2);
    NVIC_SetPriority(USART2_IRQn, 1);
    NVIC_EnableIRQ(USART2_IRQn);
    if (HAL_UART_Receive_IT(&huart2, (uint8_t *) usart2_RxBuffer, 1) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

}*/

void ESP8266_UART_Send(char *msg){
    RxCharBuf_wifi_index = Wifi_flag_index = 0;
    if (HAL_UART_Transmit(&huart3, (uint8_t *) msg, (uint16_t) (strlen(msg)), 0xffff) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    /*##-3- Wait for the end of the transfer ###################################*/
    while (UartReady != SET);

    /* Reset transmission flag */
    UartReady = RESET;

}
//***********************************************************************
//	wifi AT指令返回检查
//***********************************************************************
int wifi_checkReturn(unsigned char flag){                   //这个flag为你要检查数组B的倒数第几个字符，如果为上面分析的那三个字符的话，则检查成功 返回1，否则返回0
    unsigned int length;
    uint16_t j;
    //发送AT指令前要手动将三个读写指针归0（flagWr=RxBufWr_wifi=RxBufRd_wifi=0）,如果读写指针不相等则说明有数据回传过来，反则延时等待
    j = 0;
    while(Wifi_flag_index == RxCharBuf_wifi_index){
        if (++j >= 5000){                      //防止进入死循环
            __NOP();
            return 0;
        }
    }
    length = (unsigned int) strlen(Wifi_flag);
    if(length < flag){
        return 0;
    }
    if(Wifi_flag[length-flag] == 'K')
        return 1;
    if(Wifi_flag[length-flag] == 0x3E)     //判断是否为 >
        return 1;
    if(Wifi_flag[length-flag] == 'D')
        return 1;
    if(Wifi_flag[length-flag] == 'P')
        return 1;
    return 0;
}

void wifi_sendData(void){
    char *dev_id,*dev_id_content,*dev_secret,*dev_secret_content,*data1,*data2,*data3,*data4,*data5,*data6,*data7;
    int length;
   // if (humidity == 0 && temperature == 0) return;

    char* longitude_str = "0.0\0";
    char* latitude_str = "0.0\0";

    char content_length_str[4];

    ESP8266_UART_Send("POST /Sensor/api/device/updateDeviceDetail HTTP/1.1\r\nHost:123.206.112.29\r\n");
    ESP8266_UART_Send("Connection: close\r\n");

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

    ESP8266_UART_Send("Content-Length: ");
    content_length_str[0] = (char) (length / 100 + '0');
    content_length_str[1] = (char) (length % 100 / 10 + '0');
    content_length_str[2] = (char) (length % 100 % 10 + '0');
    content_length_str[3] = '\0';
    ESP8266_UART_Send(content_length_str);
    ESP8266_UART_Send("\r\n\r\n");
    ESP8266_UART_Send(dev_id);
    ESP8266_UART_Send(dev_id_content);
    ESP8266_UART_Send(dev_secret);
    ESP8266_UART_Send(dev_secret_content);
    ESP8266_UART_Send(data1);
    ESP8266_UART_Send(temperature_str);
    ESP8266_UART_Send(data2);
    ESP8266_UART_Send(humidity_str);
    ESP8266_UART_Send(data3);
    ESP8266_UART_Send(PM25_str);
    ESP8266_UART_Send(data4);
    ESP8266_UART_Send(PM10_str);
    ESP8266_UART_Send(data5);
    ESP8266_UART_Send(longitude_str);
    ESP8266_UART_Send(data6);
    ESP8266_UART_Send(latitude_str);
    ESP8266_UART_Send(data7);
}

void wifi_Data_Deal(){
    if (wifi_stage_change){
        switch (wifi_init_stage){
            case WIFI_TO_ZJUWLAN_RELOGIN:{
                ESP8266_UART_Send((char *) wifi_CIPSENDOUT);
                delay_ms(50);
                ESP8266_UART_Send((char *) wifi_ZJUWLAN_TCPClose);
                wifi_stage_change = 0;
                break;
            }
            case WIFI_TO_ZJUWLAN_LOGIN:{
                ESP8266_UART_Send((char *) wifi_ZJUWLAN_TCPConnect);
                wifi_stage_change = 0;
                break;
            }
            case WIFI_TO_ZJUWLAN_CIPMODE:{
                ESP8266_UART_Send((char *) wifi_CIPMODE);
                wifi_stage_change = 0;
                break;
            }
            case WIFI_TO_ZJUWLAN_SEND:{
                ESP8266_UART_Send((char *) wifi_CIPSEND);
                wifi_stage_change = 0;
                break;
            }
            case WIFI_TO_ZJUWLAN_DATA:{
                ESP8266_UART_Send("POST /srun_portal_phone.php?url=http://www.zju.edu.cn/&ac_id=3 HTTP/1.1\r\n");
                ESP8266_UART_Send("Connection: close\r\n");
                ESP8266_UART_Send("Content-Length: 107\r\n");
                ESP8266_UART_Send("Host: net.zju.edu.cn\r\n");
                ESP8266_UART_Send("Content-Type: application/x-www-form-urlencoded\r\n");
                ESP8266_UART_Send("\r\n");
                ESP8266_UART_Send("action=login&ac_id=3&user_ip=&nas_ip=&user_mac=&username=3150103802&password=19970217tyq&save_me=1&x=11&y=8\r\n\r\n");
                wifi_stage_change = 0;
                break;
            }/*case WIFI_TO_END_ZJUWLAN_SEND:{
                ESP8266_UART_Send((char *) wifi_CIPSENDOUT);
                delay_us(500);
                ESP8266_UART_Send((char *) wifi_CIPSENDOUT);
                wifi_stage_change = 0;
                break;
            }*/
            case WIFI_TO_END_ZJUWLAN_TCP:{
                ESP8266_UART_Send((char *) wifi_CIPSENDOUT);
                delay_ms(50);
                ESP8266_UART_Send((char *) wifi_ZJUWLAN_TCPClose);
                wifi_stage_change = 0;
                break;
            }
            case WIFI_TO_START_SENSOR_TCP:{
                ESP8266_UART_Send((char *) wifi_TCPConnect);
                wifi_stage_change = 0;
                break;
            }
            case WIFI_TO_SENSOR_SEND:{
                ESP8266_UART_Send((char *) wifi_CIPSEND);
                wifi_stage_change = 0;
                break;
            }
            default:break;
        }

    }
}

//***********************************************************************
//	连接服务器
//***********************************************************************
void wifi_ConnectServer(void) {
    uint16_t j;
    /*ESP8266_UART_Send(wifi_CIPSENDOUT);
    DELAY_US(1000000);
    ESP8266_UART_Send(wifi_Reset);
    DELAY_US(5000000);
    ESP8266_UART_Send(wifi_TCPConnect);
    DELAY_US(1000000);
    ESP8266_UART_Send(wifi_CIPMODE);
    DELAY_US(1000000);
    ESP8266_UART_Send(wifi_CIPSEND);
    DELAY_US(1000000);*/

    //ESP8266_UART_Send(wifi_CIPSENDOUT);
    //DELAY_MS(100);
    delay_ms(2000);
    ESP8266_UART_Send((char *) wifi_CIPSENDOUT);
    delay_ms(10);
    //ESP8266_UART_Send((char *) wifi_Reset);
    //DELAY_MS(5000);
    //delay_ms(1000);
    /*j = 0;
    do{
        if(wifi_checkReturn(1) == 1)         //如果数组B的最后一个字符检查通过的话 则退出循环
            break;
        if (++j >= 1000){                      //防止进入死循环
            __NOP();
            return;
        }
    }while(1);
*/
    //wifi_receive_flag = 1;

    /*ESP8266_UART_Send((char *) wifi_Connect);
    delay_ms(3000);
    j = 0;
    do{
        if(wifi_checkReturn(1) == 1)         //如果数组B的最后一个字符检查通过的话 则退出循环
            break;
        if (++j >= 5000){                      //防止进入死循环
            __NOP();
            return;
        }
    }while(1);
    */
    wifi_receive_flag = 1;
    ESP8266_UART_Send((char *) wifi_TCPConnect);             //这个函数即为发送AT+CIPSTART="TCP","www.icharm.me",80 的函数
    delay_ms(2000);
    /*j = 0;
    do{
        if(wifi_checkReturn(1) == 1)         //如果数组B的最后一个字符检查通过的话 则退出循环
            break;
        if(wifi_checkReturn(2) == 1)         //如果数组B的倒数第二个字符检查通过的 则退出循环
            break;
        if (++j >= 45000){                      //防止进入死循环
            __NOP();
            return;
        }
    }while(1);*/

    //if(wifi_checkReturn(1) != 1 && wifi_checkReturn(2) != 1) return;   //如果数组B的最后一个字符检查通过的话 则退出循环
    //if() return;   //如果数组B的倒数第二个字符检查通过的 则退出循环

    ESP8266_UART_Send((char *) wifi_CIPMODE);                     //这个函数发送指令AT+CIPMODE=1
    delay_ms(1500);
    /*j = 0;
    do{
        if(wifi_checkReturn(1) == 1)         //如果数组B的最后一个字符检查通过的话 则退出循环
            break;
        if (++j >= 5000){                      //防止进入死循环
            __NOP();
            return;
        }
    }while(1);*/
    //if(wifi_checkReturn(1) != 1) return;   //如果数组B的倒数第二个字符检查通过的 则退出循环

    ESP8266_UART_Send((char *) wifi_CIPSEND);                     //这个函数发送指令AT+CIPSEND
    delay_ms(50);
    /*j = 0;
    do{
        if(wifi_checkReturn(1) == 1)         //如果数组B的最后一个字符检查通过的话 则退出循环
            break;
        if (++j >= 1000){                      //防止进入死循环
            __NOP();
            return;
        }
    }while(1);*/
    //if(wifi_checkReturn(1) != 1) return;   //如果数组B的倒数第二个字符检查通过的 则退出循环

    /*
    DELAY_MS(500);
    j=0;
    do{
        ESP8266_UART_Send(wifi_CIPMODE);                     //这个函数发送指令AT+CIPMODE=1
        if (++j >= 500){
            __NOP();
            return;
        }
    }while(wifi_checkReturn(1) == 0);
    DELAY_MS(500);
    j=0;
    do{
        ESP8266_UART_Send(wifi_CIPSEND);                     //这个函数发送指令AT+CIPSEND
        if (++j >= 500){
            __NOP();
            return;
        }
    }while(wifi_checkReturn(1) == 0);
    wifi_connect_flag = 1;*/
}



