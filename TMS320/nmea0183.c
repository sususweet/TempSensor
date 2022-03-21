#include <F28x_Project.h>
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "math.h"
#include "nmea0183.h"

//从buf里面得到第cx个逗号所在的位置
//返回值:0~0XFE,代表逗号所在位置的偏移.
//0XFF,代表不存在第cx个逗号
unsigned char NMEA_Comma_Pos(unsigned char *buf,unsigned char cx) {
    unsigned char *p=buf;
    while(cx)
    {
        if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
        if(*buf==',')cx--;
        buf++;
    }
    return buf-p;
}
//m^n函数
//返回值:m^n次方.
unsigned long NMEA_Pow(unsigned char m,unsigned char n)
{
    unsigned long result=1;
    while(n--)result*=m;
    return result;
}
//str转换为数字,以','或者'*'结束
//buf:数字存储区
//dx:小数点位数,返回给调用函数
//返回值:转换后的数值
unsigned long NMEA_Str2num(unsigned char *buf,unsigned char*dx)
{
    unsigned char *p=buf;
    unsigned long ires=0,fres=0,res=0;
    unsigned char ilen=0,flen=0,i;
    unsigned char mask=0;

    while(1) //得到整数和小数的长度
    {
        if(*p=='-'){mask|=0X02;p++;}//是负数
        if(*p==','||(*p=='*'))break;//遇到结束了
        if(*p=='.'){mask|=0X01;p++;}//遇到小数点了
        else if(*p>'9'||(*p<'0'))	//有非法字符
        {
            ilen=0;
            flen=0;
            break;
        }
        if(mask&0X01)flen++;
        else ilen++;
        p++;
    }
    if(mask&0X02)buf++;	//去掉负号
    for(i=0;i<ilen;i++)	//得到整数部分数据
    {
        ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
    }
    if(flen>5)flen=5;	//最多取5位小数
    *dx=flen;	 		//小数点位数
    for(i=0;i<flen;i++)	//得到小数部分数据
    {
        fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
    }
    res=ires*NMEA_Pow(10,flen)+fres;
    if(mask&0X02)res=-res;
    return res;
}
//分析GPGSV信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGSV_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    unsigned char *p,*p1,dx;
    unsigned char len,i,j,slx=0;
    unsigned char posx;
    p=buf;
    p1=(unsigned char*)strstr((const char *)p,"$GPGSV");
    len=p1[7]-'0';								//得到GPGSV的条数
    posx=NMEA_Comma_Pos(p1,3); 					//得到可见卫星总数
    if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
    for(i=0;i<len;i++)
    {
        p1=(unsigned char*)strstr((const char *)p,"$GPGSV");
        for(j=0;j<4;j++)
        {
            posx=NMEA_Comma_Pos(p1,4+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//得到卫星编号
            else break;
            posx=NMEA_Comma_Pos(p1,5+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角 
            else break;
            posx=NMEA_Comma_Pos(p1,6+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
            else break;
            posx=NMEA_Comma_Pos(p1,7+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//得到卫星信噪比
            else break;
            slx++;
        }
        p=p1+1;//切换到下一个GPGSV信息
    }
}
//分析GPGGA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGGA_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    unsigned char *p1,dx;
    unsigned char posx;
    p1=(unsigned char*)strstr((const char *)buf,"$GPGGA");
    posx=NMEA_Comma_Pos(p1,6);								//得到GPS状态
    if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,7);								//得到用于定位的卫星数
    if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,9);								//得到海拔高度
    if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);
}
//分析GPGSA信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPGSA_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    unsigned char *p1,dx;
    unsigned char posx;
    unsigned char i;
    p1=(unsigned char*)strstr((const char *)buf,"$GPGSA");
    posx=NMEA_Comma_Pos(p1,2);								//得到定位类型
    if(posx!=0XFF)gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);
    for(i=0;i<12;i++)										//得到定位卫星编号
    {
        posx=NMEA_Comma_Pos(p1,3+i);
        if(posx!=0XFF)gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
        else break;
    }
    posx=NMEA_Comma_Pos(p1,15);								//得到PDOP位置精度因子
    if(posx!=0XFF)gpsx->pdop=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,16);								//得到HDOP位置精度因子
    if(posx!=0XFF)gpsx->hdop=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,17);								//得到VDOP位置精度因子
    if(posx!=0XFF)gpsx->vdop=NMEA_Str2num(p1+posx,&dx);
}
//分析GPRMC信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPRMC_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    unsigned char *p1,dx;
    unsigned char posx;
    unsigned long temp;
    float rs;
    //buf = "$GNRMC,140749.000,A,2517.3798,N,11018.8912,E,0.10,42.48,060917,,,A*42";
    p1=(unsigned char*)strstr((const char *)buf,"$GNRMC");
    if (!p1) return;
    posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
        gpsx->utc.hour=temp/10000;
        gpsx->utc.min=(temp/100)%100;
        gpsx->utc.sec=temp%100;
    }
    posx=NMEA_Comma_Pos(p1,3);								//得到纬度
    if(posx!=0XFF) {
        temp=NMEA_Str2num(p1+posx,&dx);
        gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
        rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
        gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
    }
    posx=NMEA_Comma_Pos(p1,4);								//南纬还是北纬 
    if(posx!=0XFF)gpsx->nshemi=*(p1+posx);
    posx=NMEA_Comma_Pos(p1,5);								//得到经度
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx);
        gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
        rs=temp%NMEA_Pow(10,dx+2);				//得到'		 
        gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为° 
    }
    posx=NMEA_Comma_Pos(p1,6);								//东经还是西经
    if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);
    posx=NMEA_Comma_Pos(p1,9);								//得到UTC日期
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx);		 				//得到UTC日期
        gpsx->utc.date=temp/10000;
        gpsx->utc.month=(temp/100)%100;
        gpsx->utc.year=2000+temp%100;
    }
    posx=NMEA_Comma_Pos(p1,12);								//得到UTC日期
    if(posx!=0XFF) {
        temp=*(p1+posx);		 				//得到UTC日期
        gpsx->navMode=temp;
    }
}
//分析GPVTG信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void NMEA_GPVTG_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    unsigned char *p1,dx;
    unsigned char posx;
    p1=(unsigned char*)strstr((const char *)buf,"$GPVTG");
    posx=NMEA_Comma_Pos(p1,7);								//得到地面速率
    if(posx!=0XFF)
    {
        gpsx->speed=NMEA_Str2num(p1+posx,&dx);
        if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);	 	 		//确保扩大1000倍
    }
}
//提取NMEA-0183信息
//gpsx:nmea信息结构体
//buf:接收到的GPS数据缓冲区首地址
void GPS_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    NMEA_GPGSV_Analysis(gpsx,buf);	//GPGSV解析
    NMEA_GPGGA_Analysis(gpsx,buf);	//GPGGA解析 	
    NMEA_GPGSA_Analysis(gpsx,buf);	//GPGSA解析
    NMEA_GPRMC_Analysis(gpsx,buf);	//GPRMC解析
    NMEA_GPVTG_Analysis(gpsx,buf);	//GPVTG解析
}

//GPS校验和计算
//buf:数据缓存区首地址
//len:数据长度
//cka,ckb:两个校验结果.
void Ublox_CheckSum(unsigned char *buf,unsigned int len,unsigned char* cka,unsigned char*ckb)
{
    unsigned int i;
    *cka=0;*ckb=0;
    for(i=0;i<len;i++)
    {
        *cka=*cka+buf[i];
        *ckb=*ckb+*cka;
    }
}
