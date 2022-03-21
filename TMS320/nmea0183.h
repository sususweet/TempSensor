//
// Created by tangyq on 2018/5/7.
//

#ifndef PM25_SCANNER_NMEA0183_H
#define PM25_SCANNER_NMEA0183_H
//GPS NMEA-0183Э����Ҫ�����ṹ�嶨�� 
//������Ϣ
typedef struct  
{
    unsigned char num;		//���Ǳ��
    unsigned char eledeg;	//��������
    unsigned int azideg;	//���Ƿ�λ��
    unsigned char sn;		//�����		   
}nmea_slmsg;  
//UTCʱ����Ϣ
typedef struct  
{										    
 	unsigned int year;	//���
	unsigned char month;	//�·�
	unsigned char date;	//����
	unsigned char hour; 	//Сʱ
	unsigned char min; 	//����
	unsigned char sec; 	//����
}nmea_utc_time;   	   
//NMEA 0183 Э����������ݴ�Žṹ��
typedef struct  
{										    
 	unsigned char svnum;					//�ɼ�������
	nmea_slmsg slmsg[12];		//���12������
	nmea_utc_time utc;			//UTCʱ��
    unsigned long latitude;				//γ�� ������100000��,ʵ��Ҫ����100000
	unsigned char nshemi;					//��γ/��γ,N:��γ;S:��γ				  
    unsigned long longitude;			    //���� ������100000��,ʵ��Ҫ����100000
	unsigned char ewhemi;					//����/����,E:����;W:����
	unsigned char gpssta;					//GPS״̬:0,δ��λ;1,�ǲ�ֶ�λ;2,��ֶ�λ;6,���ڹ���.				  
 	unsigned char posslnum;				//���ڶ�λ��������,0~12.
 	unsigned char possl[12];				//���ڶ�λ�����Ǳ��
	unsigned char fixmode;					//��λ����:1,û�ж�λ;2,2D��λ;3,3D��λ
	unsigned int pdop;					//λ�þ������� 0~500,��Ӧʵ��ֵ0~50.0
	unsigned int hdop;					//ˮƽ�������� 0~500,��Ӧʵ��ֵ0~50.0
	unsigned int vdop;					//��ֱ�������� 0~500,��Ӧʵ��ֵ0~50.0
    unsigned char navMode;

	int altitude;			 	//���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m	 
	unsigned int speed;					//��������,�Ŵ���1000��,ʵ�ʳ���10.��λ:0.001����/Сʱ	 
}Nmea_msg;
//////////////////////////////////////////////////////////////////////////////////////////////////// 	
//UBLOX NEO-6M ʱ���������ýṹ��
typedef struct
{										    
 	unsigned int header;					//cfg header,�̶�Ϊ0X62B5(С��ģʽ)
	unsigned int id;						//CFG TP ID:0X0706 (С��ģʽ)
	unsigned int dlength;				//���ݳ���
    unsigned long interval;				//ʱ��������,��λΪus
    unsigned long length;				 	//������,��λΪus
	signed char status;			//ʱ����������:1,�ߵ�ƽ��Ч;0,�ر�;-1,�͵�ƽ��Ч.			  
	unsigned char timeref;			   		//�ο�ʱ��:0,UTCʱ��;1,GPSʱ��;2,����ʱ��.
	unsigned char flags;					//ʱ���������ñ�־
	unsigned char reserved;				//����			  
 	signed short antdelay;	 	//������ʱ
 	signed short rfdelay;		//RF��ʱ
	signed int userdelay; 	 	//�û���ʱ	
	unsigned char cka;						//У��CK_A 							 	 
	unsigned char ckb;						//У��CK_B							 	 
}_ublox_cfg_tp; 
//UBLOX NEO-6M ˢ���������ýṹ��
typedef struct
{										    
 	unsigned int header;					//cfg header,�̶�Ϊ0X62B5(С��ģʽ)
	unsigned int id;						//CFG RATE ID:0X0806 (С��ģʽ)
	unsigned int dlength;				//���ݳ���
	unsigned int measrate;				//����ʱ��������λΪms�����ٲ���С��200ms��5Hz��
	unsigned int navrate;				//�������ʣ����ڣ����̶�Ϊ1
	unsigned int timeref;				//�ο�ʱ�䣺0=UTC Time��1=GPS Time��
 	unsigned char cka;						//У��CK_A 							 	 
	unsigned char ckb;						//У��CK_B							 	 
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
