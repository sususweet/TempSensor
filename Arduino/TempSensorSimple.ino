#define _USE_OLED
#define _USE_WIFI
// #define _USE_PMS5003

#include <avr/sleep.h>
#include <avr/wdt.h>

#include <SoftwareSerial.h>
//#include <MsTimer2.h>

#ifdef _USE_OLED
#include <U8glib.h>
#else
#include <LiquidCrystal_I2C.h>
#endif

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define u8 unsigned short
#define u16 unsigned int
#define u32 unsigned long

#define DEV_ID "\"device_f4557af1e63c355\""
#define DEV_SECRET "\"7f5d6ff0fdf2d40e952a\""

#define DHT12Pin 2

SoftwareSerial pm25Serial(11, 12); // RX, TX

#ifdef _USE_WIFI
SoftwareSerial wifiSerial(4, 5); // RX, TX
#endif

#ifdef _USE_OLED
/* D0(OLED_CLK)     6
   D1(OLED_MOSI)    7
   RES(OLED_RESET)  8
   D/C(OLED_DC)     9
   CS（OLED_CS）    10
*/
U8GLIB_SSD1306_128X64 u8g(6, 7, 10, 9, 8);	// SW SPI Com: SCK = 6, MOSI = 7, CS = 10, A0 = 9, RST = 8
#else
LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif

volatile byte data = 0;

uint32_t delayMS;

enum lcd_show_states {
  TEMP_AND_HUMIDITY,
  PM2_5_AND_PM10
};

const int MAX_FRAME_LEN = 64;
const bool DEBUG = false;
bool firstSetup = true;

double temperature = 0.0;
double humidity = 0.0;
char humidity_str[6];
char temperature_str[6];
char PM25_str[6];
char PM10_str[6];
char hcho_str[6];

unsigned char lcd_show_stage = TEMP_AND_HUMIDITY;
unsigned char lcd_show_count = 0;
unsigned char lcd_change_count = 0;
unsigned long previousMillis = 0;
// 定义常量来表示固定的时间间隔，
// 此处为1000*10毫秒，也即10秒钟，
// 变量类型为长整型。
const long interval = 20000;

uint8_t frameBuf[MAX_FRAME_LEN];
int detectOff = 0;
int frameLen = MAX_FRAME_LEN;
bool inFrame = false;
char printbuf[256];
int incomingByte = 0;
uint16_t calcChecksum = 0;
struct PMS5003T_framestruct {
  uint8_t frameHeader[2];
  uint16_t frameLen = MAX_FRAME_LEN;
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
  uint16_t hcho;
  uint8_t version;
  uint8_t errorCode;
  uint16_t checksum;
} pms5003Frame;


int DHT12_flag = 0;
int DHT12_error = 0;
//#ifndef _USE_OLED
void Data_String_Update() {
  temperature_str[0] = (char) ('0' + (unsigned char) temperature / 10);
  temperature_str[1] = (char) ('0' + (unsigned char) temperature % 10);
  temperature_str[2] = '.';
  temperature_str[3] = (char) ('0' + (unsigned char) (temperature * 10) % 10);
  temperature_str[4] = '\0';

  humidity_str[0] = (char) ('0' + (unsigned char) humidity / 10);
  humidity_str[1] = (char) ('0' + (unsigned char) humidity % 10);
  humidity_str[2] = '.';
  humidity_str[3] = (char) ('0' + (unsigned char) (humidity * 10) % 10);
  humidity_str[4] = '\0';

  uint16_t PM25_Value = pms5003Frame.concPM2_5_amb;
  PM25_str[0] = (char) ('0' + (unsigned char) PM25_Value / 100);
  PM25_str[1] = (char) ('0' + (unsigned char) PM25_Value % 100 / 10);
  PM25_str[2] = (char) ('0' + (unsigned char) PM25_Value % 100 % 10);
  PM25_str[3] = '\0';
  /*PM25_str[3] = '.';
    PM25_str[4] = (unsigned char) ('0' + (unsigned char) (PM25_Value * 10) % 10);
    PM25_str[5] = '\0';*/

  uint16_t PM10_Value = pms5003Frame.concPM10_0_amb;
  PM10_str[0] = (char) ('0' + (unsigned char) PM10_Value / 100);
  PM10_str[1] = (char) ('0' + (unsigned char) PM10_Value % 100 / 10);
  PM10_str[2] = (char) ('0' + (unsigned char) PM10_Value % 100 % 10);
  PM10_str[3] = '\0';

  uint16_t hcho_Value = pms5003Frame.hcho;
  hcho_str[0] = (char) ('0' + (unsigned char) hcho_Value / 100);
  hcho_str[1] = (char) ('0' + (unsigned char) hcho_Value % 100 / 10);
  hcho_str[2] = (char) ('0' + (unsigned char) hcho_Value % 100 % 10);
  hcho_str[3] = '\0';
  
  /*PM10_str[3] = '.';
    PM10_str[4] = (unsigned char) ('0' + (unsigned char) (PM10_Value * 10) % 10);
    PM10_str[5] = '\0';*/
}
//#endif
void DHT12_Init() {
  pinMode(DHT12Pin, OUTPUT);
  digitalWrite(DHT12Pin, LOW);
  delay(50);
}

u8 DHT12_Rdata(void) {
  u8 i;
  u16 j;
  u8 data = 0;
  u8 dbit = 0;
  pinMode(DHT12Pin, INPUT);
  for (i = 0; i < 8; i++) {
    while (digitalRead(DHT12Pin) == LOW) //check the last low state is over
    {
      if (++j >= 50000) {

      }
    }
    delayMicroseconds(50);//delay 26-70us,to overed data0 high level

    dbit = 0;
    if (digitalRead(DHT12Pin) == HIGH) {
      dbit = 1;
    }
    while (digitalRead(DHT12Pin) == HIGH) //等待高电平结束
    {
      if (++j >= 50000) //防止进入死循环
      {
        break;
      }
    }
    data <<= 1;
    data |= dbit;

  }
  return data;
}

u8 DHT12_ReadTemp() {
  u32 j;
  double Humi = 0, Temprature = 0;
  u8 Humi_H, Humi_L, Temp_H, Temp_L, Temp_CAL, temp;
  pinMode(DHT12Pin, OUTPUT);
  digitalWrite(DHT12Pin, LOW);
  delay(20);
  digitalWrite(DHT12Pin, HIGH);
  pinMode(DHT12Pin, INPUT);
  delayMicroseconds(30);
  DHT12_flag = 0;
  if (digitalRead(DHT12Pin) == LOW) {
    DHT12_flag = 1;

    j = 0;
    while (digitalRead(DHT12Pin) == LOW) //判断从机发出 80us 的低电平响应信号是否结束
    {
      if (++j >= 500) //防止进入死循环
      {
        DHT12_error = 1;
        break;
      }
    }
    j = 0;
    while (digitalRead(DHT12Pin) == HIGH) //判断从机是否发出 80us 的高电平，如发出则进入数据接收状态
    {
      if (++j >= 800) //防止进入死循环
      {
        DHT12_error = 1;
        break;
      }
    }
    Humi_H = DHT12_Rdata();
    Humi_L = DHT12_Rdata();
    Temp_H = DHT12_Rdata();
    Temp_L = DHT12_Rdata();
    Temp_CAL = DHT12_Rdata();
    temp = (u8) (Humi_H + Humi_L + Temp_H + Temp_L);

    //Verify
    //if success ,continue
    if (Temp_CAL == temp) //如果校验成功，往下运行
    {
      Humi = Humi_H * 10 + Humi_L; //湿度

      if (Temp_L & 0X80) //为负温度
      {
        Temprature = 0 - (Temp_H * 10 + ((Temp_L & 0x7F)));
      } else   //为正温度
      {
        Temprature = Temp_H * 10 + Temp_L; //为正温度
      }
      //判断数据是否超过量程（温度：-20℃~60℃，湿度20％RH~95％RH）
      if (Humi > 950) {
        Humi = 950;
      }
      if (Humi < 200) {
        Humi = 200;
      }
      if (Temprature > 600) {
        Temprature = 600;
      }
      if (Temprature < -200) {
        Temprature = -200;
      }
      Temprature = Temprature / 10; //计算为温度值
      Humi = Humi / 10;
      Serial.print("\r\n温度为: ");
      Serial.print(Temprature);
      Serial.print("  ℃\r\n");
      Serial.print("湿度为: ");
      Serial.print(Humi);
      Serial.print(" RH\r\n");
      temperature = Temprature;
      humidity = Humi;
    } else {
      Serial.print("CAL Error!!\r\n");
      // Serial.print("%d \r%d \r%d \r%d \r%d \r%d \r\n", Humi_H, Humi_L, Temp_H, Temp_L, Temp_CAL, temp);
    }
  } else {
    DHT12_error = 0;
    Serial.print("Sensor Error!!\r\n");
  }

  return 1;
}


bool PMS5003T_read() {
  //  Particle.publish("PMS5003T", printbuf, 60, PRIVATE);
  // send data only when you receive data:
  Serial.println("-- Reading PMS5003T");
  pm25Serial.begin(9600);
  bool packetReceived = false;
  while (!packetReceived) {
    if (pm25Serial.available() > 32) {
      int drain = pm25Serial.available();
      if (DEBUG) {
        Serial.print("-- Draining buffer: ");
        Serial.println(pm25Serial.available(), DEC);
      }
      for (int i = drain; i > 0; i--) {
        pm25Serial.read();
      }
    }
    if (pm25Serial.available() > 0) {
      if (DEBUG) {
        Serial.print("-- Available: ");
        Serial.println(pm25Serial.available(), DEC);
      }
      incomingByte = pm25Serial.read();
      if (DEBUG) {
        Serial.print("-- READ: ");
        Serial.println(incomingByte, HEX);
      }
      if (!inFrame) {
        if (incomingByte == 0x42 && detectOff == 0) {
          frameBuf[detectOff] = incomingByte;
          pms5003Frame.frameHeader[0] = incomingByte;
          calcChecksum = incomingByte; // Checksum init!
          detectOff++;
        } else if (incomingByte == 0x4D && detectOff == 1) {
          frameBuf[detectOff] = incomingByte;
          pms5003Frame.frameHeader[1] = incomingByte;
          calcChecksum += incomingByte;
          inFrame = true;
          detectOff++;
        } else {
          Serial.print("-- Frame syncing... ");
          Serial.print(incomingByte, HEX);
          if (DEBUG) {
          }
          Serial.println();
        }
      } else {
        frameBuf[detectOff] = incomingByte;
        calcChecksum += incomingByte;
        detectOff++;
        uint16_t val = frameBuf[detectOff - 1] + (frameBuf[detectOff - 2] << 8);
        switch (detectOff) {
          case 4:
            pms5003Frame.frameLen = val;
            frameLen = val + detectOff;
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
            Serial.println("BEGIN");
            Serial.print(frameBuf[detectOff - 2], HEX);
            Serial.println("");
            Serial.print(frameBuf[detectOff - 1], HEX);
            Serial.println("");
            Serial.print(val, DEC);
            Serial.println("");
            pms5003Frame.temperature = val;
            break;
          case 28:
            pms5003Frame.humidity = val;
            break;
          case 29:
            val = frameBuf[detectOff - 1];
            pms5003Frame.version = val;
            break;
          case 30:
            pms5003Frame.hcho = val;
            val = frameBuf[detectOff - 1];
            pms5003Frame.errorCode = val;
            break;
          case 32:
            pms5003Frame.checksum = val;
            calcChecksum -= ((val >> 8) + (val & 0xFF));
            break;
          default:
            break;
        }

        if (detectOff >= frameLen) {
          sprintf(printbuf, "PMS5003T ");
          sprintf(printbuf, "%s[%02x %02x] (%04x) ", printbuf,
                  pms5003Frame.frameHeader[0], pms5003Frame.frameHeader[1], pms5003Frame.frameLen);
          sprintf(printbuf, "%sCF1=[%04x %04x %04x] ", printbuf,
                  pms5003Frame.concPM1_0_CF1, pms5003Frame.concPM2_5_CF1, pms5003Frame.concPM10_0_CF1);
          sprintf(printbuf, "%samb=[%04x %04x %04x] ", printbuf,
                  pms5003Frame.concPM1_0_amb, pms5003Frame.concPM2_5_amb, pms5003Frame.concPM10_0_amb);
          sprintf(printbuf, "%sraw=[%04x %04x %04x %04x %04x %04x] ", printbuf,
                  pms5003Frame.rawGt0_3um, pms5003Frame.rawGt0_5um, pms5003Frame.rawGt1_0um,
                  pms5003Frame.rawGt2_5um, pms5003Frame.temperature, pms5003Frame.humidity);
          sprintf(printbuf, "%shcho=%02x", printbuf, pms5003Frame.hcho);
          sprintf(printbuf, "%sver=%02x err=%02x ", printbuf,
                  pms5003Frame.version, pms5003Frame.errorCode);
          sprintf(printbuf, "%scsum=%04x %s xsum=%04x", printbuf,
                  pms5003Frame.checksum, (calcChecksum == pms5003Frame.checksum ? "==" : "!="), calcChecksum);
          Serial.println(printbuf);
          packetReceived = true;
          detectOff = 0;
          inFrame = false;
        }
      }
    }
  }
  pm25Serial.end();
  return (calcChecksum == pms5003Frame.checksum);
}

#ifndef _USE_OLED
void LCD_Show_Stage_Change() {
  switch (lcd_show_stage) {
    case TEMP_AND_HUMIDITY: {
        lcd_show_stage = PM2_5_AND_PM10;
        lcd.setCursor(0, 0);
        lcd.print("PM2.5:     ug/m3");
        lcd.setCursor(0, 1);
        lcd.print("PM10:      ug/m3");
        break;
      }
    case PM2_5_AND_PM10: {
        lcd_show_stage = TEMP_AND_HUMIDITY;
        lcd.setCursor(0, 0);
        lcd.print("TEMP:          C");
        lcd.setCursor(0, 1);
        lcd.print("HUMIDITY:      %");
        break;
      }
    default:
      break;
  }
}
#endif
#ifndef _USE_OLED
void LCD_Show_Update() {
  switch (lcd_show_stage) {
    case TEMP_AND_HUMIDITY: {
        lcd.setCursor(10, 0);
        lcd.print((long)temperature); //显示温度整数位
        lcd.print("."); //显示小数点
        lcd.print((long)(temperature * 10) % 10); //显示温度小数点后一位

        lcd.setCursor(10, 1);
        lcd.print((long)humidity);
        lcd.print(".");
        lcd.print((long)(humidity * 10) % 10);
        break;
      }
    case PM2_5_AND_PM10: {
        //lcd1602_show_number(7, 0, PM_Sensor_Data.PM2_5);
        lcd.setCursor(8, 0);
        lcd.print(pms5003Frame.concPM2_5_amb);
        lcd.setCursor(8, 1);
        lcd.print(pms5003Frame.concPM10_0_amb);
        break;
      }
    default:
      break;
  }
}
#endif
#ifdef _USE_OLED
void draw(void) {
  // graphic commands to redraw the complete screen should be placed here
  /*u8g.setFont(u8g_font_unifont);
    //u8g.setFont(u8g_font_osb21);
    u8g.drawStr( 0, 22, "Hello World!");*/
  uint16_t PM25_Value = pms5003Frame.concPM2_5_amb;
  uint16_t PM10_Value = pms5003Frame.concPM10_0_amb;
  uint16_t hcho_Value = pms5003Frame.hcho;
  
  char sendBuff[20];

  u8g.setFont(u8g_font_courB24);
  u8g.setPrintPos(10, 30);
  u8g.print((float)temperature, 1);
  sprintf(sendBuff, "%cC", 0xB0);
  u8g.drawStr(90, 30, sendBuff);
    
  u8g.setPrintPos(10, 60);
  u8g.print((float)humidity, 1);
  u8g.setPrintPos(100, 60);
  u8g.print("%");
/*
  u8g.setFont(u8g_font_unifont_78_79);
  sprintf(sendBuff, "%c", 14);
  u8g.drawStr( 0, 18, sendBuff);
  u8g.setFont(u8g_font_unifont);
  // sprintf(sendBuff, "H:%2d%% T:%2d%cC", (int)humidity, (int)temperature, 0xB0);
  // u8g.drawStr( 20, 18, sendBuff);
  sprintf(sendBuff, "Temp: %.1f %cC", temperature, 0xB0);
  u8g.drawStr( 6, 39, sendBuff);
  sprintf(sendBuff, "Humi: %.1f %%", humidity);
  u8g.drawStr( 6, 60, sendBuff);*/
  //u8g.setFont(u8g_font_helvB08);
#ifdef _USE_PMS5003
  sprintf(sendBuff, "PM2.5:%3d ug/m3", PM25_Value);
  u8g.drawStr( 6, 39, sendBuff);
  sprintf(sendBuff, "PM 10:%3d ug/m3", PM10_Value);
  u8g.drawStr( 6, 60, sendBuff);
#endif
 /* sprintf(sendBuff, "PM2.5:%3d ug/m3", PM25_Value);
  u8g.drawStr( 6, 33, sendBuff);
  sprintf(sendBuff, "PM 10:%3d ug/m3", PM10_Value);
  u8g.drawStr( 6, 48, sendBuff);
  sprintf(sendBuff, "HCHO :%3d ug/m3", hcho_Value);
  u8g.drawStr( 6, 63, sendBuff);*/
}
#endif

#ifdef _USE_WIFI
void SendWifiData() {
  wifiSerial.begin(115200);

  wifiSerial.write("AT+CIPSTART=\"TCP\",\"www.lewei50.com\",80\r\n\r\n");
  delay(100);
  wifiSerial.write("AT+CIPMODE=1\r\n\r\n");
  delay(100);
  wifiSerial.write("AT+CIPSEND\r\n\r\n");
  delay(100);
  
  char *dev_id, *dev_id_content, *dev_secret, *dev_secret_content, *data1, *data2, *data3, *data4, *data5, *data6, *data7, *data8;
  int length;
  //char *temperature_str, *PM25_str, *PM10_str, *humidity_str;
  Data_String_Update();
  //Float2Char(temperature, temperature_str);
  //Float2Char(humidity, humidity_str);
  //Float2Char(pms5003Frame.concPM10_0_amb, PM10_str);
  //	Float2Char(pms5003Frame.concPM2_5_amb, PM25_str);
  Serial.println("wifi");
  /*dtostrf(temperature, 2, 1, temperature_str);
    dtostrf(humidity, 2, 1, humidity_str);
    dtostrf(pms5003Frame.concPM10_0_amb, 3, 0, PM10_str);
    dtostrf(pms5003Frame.concPM2_5_amb, 3, 0, PM25_str);*/
  // if (humidity == 0 && temperature == 0) return;
  /*temperature_str = "0.0\0";
    humidity_str= "0.0\0";
    PM10_str= "0.0\0";
    PM25_str="0.0\0";*/
  char *longitude_str = "0.0\0";
  char *latitude_str = "0.0\0";

  char content_length_str[4];

  // wifiSerial.write("POST /Sensor/api/index.php/device/updateDeviceDetail HTTP/1.1\r\nHost:life.sususweet.ltd\r\n");
  // wifiSerial.write("Connection: close\r\n");

  // dev_id = "{\"dev_id\":";
  // dev_id_content = DEV_ID;
  // dev_secret = ",\"dev_secret\":";
  // dev_secret_content = DEV_SECRET;

  // data1 = ",\"data\":{\"temperature\":\"";

  // data2 = "\",\"humidity\":\"";

  // data3 = "\",\"PM25\":\"";

  // data4 = "\",\"PM10\":\"";

  // data5 = "\",\"hcho\":\"";

  // data6 = "\",\"longitude\":\"";

  // data7 = "\",\"latitude\":\"";

  // data8 = "\"}}\r\n\r\n";

  // length = (int) (strlen(dev_id) + strlen(dev_id_content) + strlen(dev_secret) + strlen(dev_secret_content) +
  //                 strlen(data1) + strlen(data2) + strlen(data3) + strlen(data4) +
  //                 strlen(data5) + strlen(data6) + strlen(data7) + strlen(data8) +strlen(PM25_str) +
  //                 strlen(PM10_str) + strlen(hcho_str) + strlen(humidity_str) +
  //                 strlen(temperature_str) + strlen(longitude_str) + strlen(latitude_str) - 4);

  // wifiSerial.write("Content-Length: ");
  // content_length_str[0] = (char) (length / 100 + '0');
  // content_length_str[1] = (char) (length % 100 / 10 + '0');
  // content_length_str[2] = (char) (length % 100 % 10 + '0');
  // content_length_str[3] = '\0';
  // wifiSerial.write(content_length_str);
  // wifiSerial.write("\r\n\r\n");
  // wifiSerial.write(dev_id);
  // wifiSerial.write(dev_id_content);
  // wifiSerial.write(dev_secret);
  // wifiSerial.write(dev_secret_content);
  // wifiSerial.write(data1);
  // wifiSerial.write(temperature_str);
  // wifiSerial.write(data2);
  // wifiSerial.write(humidity_str);
  // wifiSerial.write(data3);
  // wifiSerial.write(PM25_str);
  // wifiSerial.write(data4);
  // wifiSerial.write(PM10_str);
  // wifiSerial.write(data5);
  // wifiSerial.write(hcho_str);
  // wifiSerial.write(data6);
  // wifiSerial.write(longitude_str);
  // wifiSerial.write(data7);
  // wifiSerial.write(latitude_str);
  // wifiSerial.write(data8);
  
  // delay(100);
  // wifiSerial.write("+++");


  wifiSerial.write("POST /api/V1/gateway/UpdateSensors/01 HTTP/1.1\r\nHost:www.lewei50.com\r\nuserkey:2183e3c757ec4732900fb7d68315f5d2\r\n");
  wifiSerial.write("Connection: close\r\n");

  data1 = "[{\"Name\":\"01_temp\",\"Value\":\"";

  data2 = "\"},{\"Name\":\"01_humidity\",\"Value\":\"";

  data3 = "\"}]\r\n\r\n";

  length = (int) (strlen(data1) + strlen(data2) + strlen(data3) + strlen(humidity_str) +
                  strlen(temperature_str) - 4);

  wifiSerial.write("Content-Length: ");
  content_length_str[0] = (char) (length / 100 + '0');
  content_length_str[1] = (char) (length % 100 / 10 + '0');
  content_length_str[2] = (char) (length % 100 % 10 + '0');
  content_length_str[3] = '\0';
  wifiSerial.write(content_length_str);
  wifiSerial.write("\r\n\r\n");
  wifiSerial.write(data1);
  wifiSerial.write(temperature_str);
  wifiSerial.write(data2);
  wifiSerial.write(humidity_str);
  wifiSerial.write(data3);
  
  delay(100);
  wifiSerial.write("+++");
  wifiSerial.end();
}
#endif
/*void Timer2Interrupt() {
  lcd_show_count++;
  lcd_change_count++;
  Serial.println(lcd_change_count);
  if (lcd_show_count >= 9) {

    lcd_show_count = 0;
    LCD_Show_Update();
  }

  if (lcd_change_count >= 50) {
    lcd_change_count = 0;
    LCD_Show_Stage_Change();
  }

  }*/

void setup() {
  // put your setup code here, to run once:
  // Serial.begin(9600);
#ifdef _USE_OLED
  // flip screen, if required
  // u8g.setRot180();

  // set SPI backup if required
  //u8g.setHardwareBackup(u8g_backup_avr_spi);
  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255, 255, 255);
  }
  u8g.firstPage();
  do {
    draw();
  } while ( u8g.nextPage() );
#else
  lcd.init();
  // set the LCD address to 0x27 for a 16 chars and 2 line display
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("TEMP:          C");
  lcd.setCursor(0, 1);
  lcd.print("HUMIDITY:      %");
  LCD_Show_Update();
#endif
#ifdef _USE_PMS5003
  PMS5003T_read();
#endif
  DHT12_Init();
#ifdef _USE_WIFI
  wifiSerial.begin(115200);
  wifiSerial.write("+++");
  delay(500);
  wifiSerial.write("AT+CWMODE=1\r\n\r\n");
  delay(500);
  wifiSerial.write("AT+CWSTARTSMART=3\r\n\r\n");
  wifiSerial.end();
#endif
  updateData();
  firstSetup = false;
  setup_watchdog(9);
  // 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
  // 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec

  //MsTimer2::set(100, Timer2Interrupt);        // 中断设置函数，每 500ms 进入一次中断
  //MsTimer2::start();                //开始计时
}

void loop() {
  // 4 * 8 = 32s
  if (data >= 4) {
    data = 0;
    updateData();
    Sleep_avr();
  }
  else {
    Sleep_avr();  //Continue Sleep
  }
}

void updateData() {
  //-------------------------------
  unsigned long currentMillis = millis(); //获取当前的系统运行时间长度
  // put your main code here, to run repeatedly:
  Serial.println("123");
  lcd_show_count++;
  lcd_change_count++;

  /* mic_volume = analogRead(MICPIN);
    mic_volume = (long)(100*mic_volume/1024);
    Serial.println("MIC_VAL:");
    Serial.println(mic_volume);*/

  //human_val = digitalRead(HUMANPIN);   // read the input pin
  //Serial.println("HUMAN:");
  //Serial.println(human_val);
  //Serial.println(lcd_change_count);
  //if (lcd_show_count >= 3) {
#ifdef _USE_PMS5003
  PMS5003T_read();
#endif
  DHT12_ReadTemp();
  lcd_show_count = 0;
#ifdef _USE_OLED
  // picture loop
  u8g.firstPage();
  do {
    draw();
  } while ( u8g.nextPage() );
#else
  LCD_Show_Update();
#endif
  //}

  /*if (lcd_change_count >= 10) {
    lcd_change_count = 0;
    #ifdef _USE_OLED
    u8g.firstPage();
    do {
      draw();
    } while ( u8g.nextPage() );
    #else
    LCD_Show_Stage_Change();
    LCD_Show_Update();
    #endif


    }*/
#ifdef _USE_WIFI
if (!firstSetup){
  SendWifiData();
}
#endif
  if (currentMillis - previousMillis >= interval || currentMillis - previousMillis < 0) {
    // 更新previousMillis变量数值，
    // 也即更新上一次时间记录为当前时间。
    previousMillis = currentMillis;
    // SendWifiData();
  }


  /*if (!PMS5003T_read()) {

    LCD_Show_Stage_Change();
    LCD_Show_Update();
    Serial.println("PM25:");
    Serial.println(pms5003Frame.concPM2_5_amb);
    Serial.println("temp:");
    Serial.println(pms5003Frame.temperature);
    delay(2000);
    }*/
  /*pm25Serial.begin(9600);
    if (pm25Serial.available()) {
     Serial.print(pm25Serial.read(), HEX);
     //Serial.write(pm25Serial.read());
    }*/
  //--------------------------------
}

//Sleep mode is activated
void setup_watchdog(int ii) {
  byte bb;

  if (ii > 9 ) ii = 9;
  bb = ii & 7;
  if (ii > 7) bb |= (1 << 5);
  bb |= (1 << WDCE);

  MCUSR &= ~(1 << WDRF);
  // start timed sequence
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

//WDT interrupt
ISR(WDT_vect) {
  ++data;
  // wdt_reset();
}

void Sleep_avr() {
  ACSR |= _BV(ACD); //OFF ACD
  ADCSRA = 0; //OFF ADC
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  sleep_mode();                        // System sleeps here
}
