#include <F28x_Project.h>
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "math.h"
#include "nmea0183.h"

//��buf����õ���cx���������ڵ�λ��
//����ֵ:0~0XFE,����������λ�õ�ƫ��.
//0XFF,�������ڵ�cx������
unsigned char NMEA_Comma_Pos(unsigned char *buf,unsigned char cx) {
    unsigned char *p=buf;
    while(cx)
    {
        if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//����'*'���߷Ƿ��ַ�,�򲻴��ڵ�cx������
        if(*buf==',')cx--;
        buf++;
    }
    return buf-p;
}
//m^n����
//����ֵ:m^n�η�.
unsigned long NMEA_Pow(unsigned char m,unsigned char n)
{
    unsigned long result=1;
    while(n--)result*=m;
    return result;
}
//strת��Ϊ����,��','����'*'����
//buf:���ִ洢��
//dx:С����λ��,���ظ����ú���
//����ֵ:ת�������ֵ
unsigned long NMEA_Str2num(unsigned char *buf,unsigned char*dx)
{
    unsigned char *p=buf;
    unsigned long ires=0,fres=0,res=0;
    unsigned char ilen=0,flen=0,i;
    unsigned char mask=0;

    while(1) //�õ�������С���ĳ���
    {
        if(*p=='-'){mask|=0X02;p++;}//�Ǹ���
        if(*p==','||(*p=='*'))break;//����������
        if(*p=='.'){mask|=0X01;p++;}//����С������
        else if(*p>'9'||(*p<'0'))	//�зǷ��ַ�
        {
            ilen=0;
            flen=0;
            break;
        }
        if(mask&0X01)flen++;
        else ilen++;
        p++;
    }
    if(mask&0X02)buf++;	//ȥ������
    for(i=0;i<ilen;i++)	//�õ�������������
    {
        ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
    }
    if(flen>5)flen=5;	//���ȡ5λС��
    *dx=flen;	 		//С����λ��
    for(i=0;i<flen;i++)	//�õ�С����������
    {
        fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
    }
    res=ires*NMEA_Pow(10,flen)+fres;
    if(mask&0X02)res=-res;
    return res;
}
//����GPGSV��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPGSV_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    unsigned char *p,*p1,dx;
    unsigned char len,i,j,slx=0;
    unsigned char posx;
    p=buf;
    p1=(unsigned char*)strstr((const char *)p,"$GPGSV");
    len=p1[7]-'0';								//�õ�GPGSV������
    posx=NMEA_Comma_Pos(p1,3); 					//�õ��ɼ���������
    if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
    for(i=0;i<len;i++)
    {
        p1=(unsigned char*)strstr((const char *)p,"$GPGSV");
        for(j=0;j<4;j++)
        {
            posx=NMEA_Comma_Pos(p1,4+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//�õ����Ǳ��
            else break;
            posx=NMEA_Comma_Pos(p1,5+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//�õ��������� 
            else break;
            posx=NMEA_Comma_Pos(p1,6+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//�õ����Ƿ�λ��
            else break;
            posx=NMEA_Comma_Pos(p1,7+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//�õ����������
            else break;
            slx++;
        }
        p=p1+1;//�л�����һ��GPGSV��Ϣ
    }
}
//����GPGGA��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPGGA_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    unsigned char *p1,dx;
    unsigned char posx;
    p1=(unsigned char*)strstr((const char *)buf,"$GPGGA");
    posx=NMEA_Comma_Pos(p1,6);								//�õ�GPS״̬
    if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,7);								//�õ����ڶ�λ��������
    if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,9);								//�õ����θ߶�
    if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);
}
//����GPGSA��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPGSA_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    unsigned char *p1,dx;
    unsigned char posx;
    unsigned char i;
    p1=(unsigned char*)strstr((const char *)buf,"$GPGSA");
    posx=NMEA_Comma_Pos(p1,2);								//�õ���λ����
    if(posx!=0XFF)gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);
    for(i=0;i<12;i++)										//�õ���λ���Ǳ��
    {
        posx=NMEA_Comma_Pos(p1,3+i);
        if(posx!=0XFF)gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
        else break;
    }
    posx=NMEA_Comma_Pos(p1,15);								//�õ�PDOPλ�þ�������
    if(posx!=0XFF)gpsx->pdop=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,16);								//�õ�HDOPλ�þ�������
    if(posx!=0XFF)gpsx->hdop=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,17);								//�õ�VDOPλ�þ�������
    if(posx!=0XFF)gpsx->vdop=NMEA_Str2num(p1+posx,&dx);
}
//����GPRMC��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPRMC_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    unsigned char *p1,dx;
    unsigned char posx;
    unsigned long temp;
    float rs;
    //buf = "$GNRMC,140749.000,A,2517.3798,N,11018.8912,E,0.10,42.48,060917,,,A*42";
    p1=(unsigned char*)strstr((const char *)buf,"$GNRMC");
    if (!p1) return;
    posx=NMEA_Comma_Pos(p1,1);								//�õ�UTCʱ��
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//�õ�UTCʱ��,ȥ��ms
        gpsx->utc.hour=temp/10000;
        gpsx->utc.min=(temp/100)%100;
        gpsx->utc.sec=temp%100;
    }
    posx=NMEA_Comma_Pos(p1,3);								//�õ�γ��
    if(posx!=0XFF) {
        temp=NMEA_Str2num(p1+posx,&dx);
        gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//�õ���
        rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
        gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ�� 
    }
    posx=NMEA_Comma_Pos(p1,4);								//��γ���Ǳ�γ 
    if(posx!=0XFF)gpsx->nshemi=*(p1+posx);
    posx=NMEA_Comma_Pos(p1,5);								//�õ�����
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx);
        gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//�õ���
        rs=temp%NMEA_Pow(10,dx+2);				//�õ�'		 
        gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ�� 
    }
    posx=NMEA_Comma_Pos(p1,6);								//������������
    if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);
    posx=NMEA_Comma_Pos(p1,9);								//�õ�UTC����
    if(posx!=0XFF)
    {
        temp=NMEA_Str2num(p1+posx,&dx);		 				//�õ�UTC����
        gpsx->utc.date=temp/10000;
        gpsx->utc.month=(temp/100)%100;
        gpsx->utc.year=2000+temp%100;
    }
    posx=NMEA_Comma_Pos(p1,12);								//�õ�UTC����
    if(posx!=0XFF) {
        temp=*(p1+posx);		 				//�õ�UTC����
        gpsx->navMode=temp;
    }
}
//����GPVTG��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void NMEA_GPVTG_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    unsigned char *p1,dx;
    unsigned char posx;
    p1=(unsigned char*)strstr((const char *)buf,"$GPVTG");
    posx=NMEA_Comma_Pos(p1,7);								//�õ���������
    if(posx!=0XFF)
    {
        gpsx->speed=NMEA_Str2num(p1+posx,&dx);
        if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);	 	 		//ȷ������1000��
    }
}
//��ȡNMEA-0183��Ϣ
//gpsx:nmea��Ϣ�ṹ��
//buf:���յ���GPS���ݻ������׵�ַ
void GPS_Analysis(Nmea_msg *gpsx,unsigned char *buf)
{
    NMEA_GPGSV_Analysis(gpsx,buf);	//GPGSV����
    NMEA_GPGGA_Analysis(gpsx,buf);	//GPGGA���� 	
    NMEA_GPGSA_Analysis(gpsx,buf);	//GPGSA����
    NMEA_GPRMC_Analysis(gpsx,buf);	//GPRMC����
    NMEA_GPVTG_Analysis(gpsx,buf);	//GPVTG����
}

//GPSУ��ͼ���
//buf:���ݻ������׵�ַ
//len:���ݳ���
//cka,ckb:����У����.
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
