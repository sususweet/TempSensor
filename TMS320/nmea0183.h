//
// Created by tangyq on 2018/5/7.
//

#ifndef PM25_SCANNER_NMEA0183_H
#define PM25_SCANNER_NMEA0183_H
//GPS NMEA-0183协议重要参数结构体定义 
//卫星信息
typedef struct  
{
    unsigned char num;		//卫星编号
    unsigned char eledeg;	//卫星仰角
    unsigned int azideg;	//卫星方位角
    unsigned char sn;		//信噪比		   
}nmea_slmsg;  
//UTC时间信息
typedef struct  
{										    
 	unsigned int year;	//年份
	unsigned char month;	//月份
	unsigned char date;	//日期
	unsigned char hour; 	//小时
	unsigned char min; 	//分钟
	unsigned char sec; 	//秒钟
}nmea_utc_time;   	   
//NMEA 0183 协议解析后数据存放结构体
typedef struct  
{										    
 	unsigned char svnum;					//可见卫星数
	nmea_slmsg slmsg[12];		//最多12颗卫星
	nmea_utc_time utc;			//UTC时间
    unsigned long latitude;				//纬度 分扩大100000倍,实际要除以100000
	unsigned char nshemi;					//北纬/南纬,N:北纬;S:南纬				  
    unsigned long longitude;			    //经度 分扩大100000倍,实际要除以100000
	unsigned char ewhemi;					//东经/西经,E:东经;W:西经
	unsigned char gpssta;					//GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.				  
 	unsigned char posslnum;				//用于定位的卫星数,0~12.
 	unsigned char possl[12];				//用于定位的卫星编号
	unsigned char fixmode;					//定位类型:1,没有定位;2,2D定位;3,3D定位
	unsigned int pdop;					//位置精度因子 0~500,对应实际值0~50.0
	unsigned int hdop;					//水平精度因子 0~500,对应实际值0~50.0
	unsigned int vdop;					//垂直精度因子 0~500,对应实际值0~50.0
    unsigned char navMode;

	int altitude;			 	//海拔高度,放大了10倍,实际除以10.单位:0.1m	 
	unsigned int speed;					//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时	 
}Nmea_msg;
//////////////////////////////////////////////////////////////////////////////////////////////////// 	
//UBLOX NEO-6M 时钟脉冲配置结构体
typedef struct
{										    
 	unsigned int header;					//cfg header,固定为0X62B5(小端模式)
	unsigned int id;						//CFG TP ID:0X0706 (小端模式)
	unsigned int dlength;				//数据长度
    unsigned long interval;				//时钟脉冲间隔,单位为us
    unsigned long length;				 	//脉冲宽度,单位为us
	signed char status;			//时钟脉冲配置:1,高电平有效;0,关闭;-1,低电平有效.			  
	unsigned char timeref;			   		//参考时间:0,UTC时间;1,GPS时间;2,当地时间.
	unsigned char flags;					//时间脉冲设置标志
	unsigned char reserved;				//保留			  
 	signed short antdelay;	 	//天线延时
 	signed short rfdelay;		//RF延时
	signed int userdelay; 	 	//用户延时	
	unsigned char cka;						//校验CK_A 							 	 
	unsigned char ckb;						//校验CK_B							 	 
}_ublox_cfg_tp; 
//UBLOX NEO-6M 刷新速率配置结构体
typedef struct
{										    
 	unsigned int header;					//cfg header,固定为0X62B5(小端模式)
	unsigned int id;						//CFG RATE ID:0X0806 (小端模式)
	unsigned int dlength;				//数据长度
	unsigned int measrate;				//测量时间间隔，单位为ms，最少不能小于200ms（5Hz）
	unsigned int navrate;				//导航速率（周期），固定为1
	unsigned int timeref;				//参考时间：0=UTC Time；1=GPS Time；
 	unsigned char cka;						//校验CK_A 							 	 
	unsigned char ckb;						//校验CK_B							 	 
}_ublox_cfg_rate;

unsigned long NMEA_Str2num(unsigned char *buf,unsigned char*dx);
void GPS_Analysis(Nmea_msg *gpsx,unsigned char *buf);
void NMEA_GPGSV_Analysis(Nmea_msg *gpsx,unsigned char *buf);
void NMEA_GPGGA_Analysis(Nmea_msg *gpsx,unsigned char *buf);
void NMEA_GPGSA_Analysis(Nmea_msg *gpsx,unsigned char *buf);
void NMEA_GPGSA_Analysis(Nmea_msg *gpsx,unsigned char *buf);
void NMEA_GPRMC_Analysis(Nmea_msg *gpsx,unsigned char *buf);
void NMEA_GPVTG_Analysis(Nmea_msg *gpsx,unsigned char *buf);

#endif //PM25_SCANNER_NMEA0183_H
