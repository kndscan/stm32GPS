#ifndef __GPS_H__
#define __GPS_H__

#include "stm32f10x.h"
#include "USART.h"

//���䶨λ����
typedef struct tagGPRMC
{
    u8 UTCTime[10]; 	//<1>  hhmmss.sss
    u8 Status; 			//<2>  A-valid; V-invalid
    char Latitude[9];     //<3>  ddmm.mmmm
    u8 NS;              //<4>  N/S
    char Longitude[10];   //<5>  dddmm.mmmm
    u8 EW;              //<6>  E/W
    u8 Speed[6];        //<7>  ����000.0~1851.8��
    u8 Course[5];       //<8>  ����000.0~359.9��
	u8 UTCDate[6];		//<9>  DDMMYY
	u8 MagVar[5];		//<10> 000.0~180.0
	u8 Declination;		//<11> E/W
	u8 ModeIndicator;	//<12> ģʽָʾA-������λ��D-��֣�E-���㣬N-������Ч(��NMEA0183 3.00�汾���)
	u8 Seprator;		//<13> '*'
	u8 Checksum;		//<14> У���
}GPRMC;

//ȫ��λ����
typedef struct tagGPGGA
{
    u8 UTCTime[10]; 	//<1>  hhmmss.sss
    char Latitude[10];	//={"1234.56789"};    //<2>  ddmm.mmmmm
    char NS;            //<3>  N/S
    char Longitude[11];	//={"98765.43210"};   //<4>  dddmm.mmmmm
    char EW;            //<5>  E/W
    u8 Status; 			//<6>  1-valid; 0-invalid
	u8 SatelliteNum[2];	//<7>  ʹ��������������00��12��ǰ��λ�����㲹0��
	u8 HoriPrecison[4];	//<8>  ˮƽ��ȷ�ȣ�0.5��99.9
	u8 AntennaHeight[7];//<9>  �����뺣ƽ��ĸ߶ȣ�-9999.9��9999.9��
	u8 AHUnit;			//<10> �߶ȵ�λ��M��ʾ��λ��
	u8 EarthHeight[6];	//<11> �����������Ժ�ƽ��ĸ߶ȣ�-999.9��9999.9��
	u8 EHUnit;			//<12> �߶ȵ�λ��M��ʾ��λ��
	u8 DataTerm;		//<13> ���GPS�������ޣ�RTCM SC-104�����������RTCM���͵�������
	u8 BaseStation[4];	//<14> ��ֲο���վ��ţ���0000��1023��ǰ��λ��������0��
	u8 Seprator;		//<15> '*'
	u8 Checksum;		//<16> У���
}GPGGA;

//�����ٶ���Ϣ(VTG)
typedef struct tagGPVTG
{
    u8 GeoNorthDir[3];	//<1>  ���汱Ϊ�ο���׼�ĵ��溽��(000~359�ȣ�ǰ���0Ҳ��������)
	u8 T;				//<2>  
    u8 MagNorthDir[3];	//<3>  �Դű�Ϊ�ο���׼�ĵ��溽��(000~359�ȣ�ǰ���0Ҳ��������)
	u8 M;               //<4>  
    u8 SpeedKnot[5];	//<5>  ��������(000.0~999.9�ڣ�ǰ���0Ҳ��������)
	u8 N;               //<6>  
    u8 SpeedKilo[10];   //<7>  ��������(0000.0~1851.8����/Сʱ��ǰ���0Ҳ��������)
	u8 K;               //<8>  
    u8 Status;			//<9>  ģʽָʾA-������λ��D-��֣�E-���㣬N-������Ч(��NMEA0183 3.00�汾���)
	u8 Seprator;		//<10> '*'
	u8 Checksum;		//<11> У���
}GPVTG;

//����PRN����
typedef struct tagGPGSA
{
    u8 Mode;			//<1>  ģʽ: M=�ֶ���A = �Զ�
    u8 Mode2;			//<2>  ��λ��ʽ: 1 = δ��λ��2 = ��ά��λ��3 = ��ά��λ
    u8 SatList[12];		//<3>  01~32 ��ʹ���е����Ǳ�ţ����ɽ���12��������Ϣ
    u8 PDOP[4];			//<4>  PDOPλ�þ������ӣ�0.5~99.9��
    u8 HDOP[4];			//<5>  HDOPˮƽ�������ӣ�0.5~99.9��
    u8 VDOP[4];			//<6>  VDOP��ֱ�������ӣ�0.5~99.9��
	u8 Seprator;		//<7>  '*'
	u8 Checksum;		//<8>  У���
}GPGSA;

//����״̬����
typedef struct tagSatInfo
{
	u8 SatIndex[2];			//<4> ���Ǳ�ţ�01��32�� 
	u8 ElevationAngle[2];	//<5> �������ǣ�00��90�ȡ� 
	u8 AzimuthAngle[3];		//<6> ���Ƿ�λ�ǣ�000��359�ȡ�ʵ��ֵ�� 
	u8 SNR;					//<7> �����(signal-to-noise ratio)��00��99dB���ޱ�δ���յ�Ѷ�š� 
}SatInfo;
//��������״̬������
typedef struct tagGPGSV
{
	u8 FrameCnt;		//<1>  �ܵ�GSV���������� 
	u8 FrameIndex;		//<2>  ��ǰGSV���š� 
	u8 SatCnt[2];		//<3>  ��������������00��12�� 
	SatInfo Info[4];	//<4>  ����״̬���ݣ�ÿ������������4����������
	u8 Seprator;		//<5>  '*'
	u8 Checksum;		//<6>  У���
}GPGSV;

#define NMEA_NULL    	0x00            //GPS�������
#define NMEA_GPGGA    	0x01
#define NMEA_GPGSA    	0x02
#define NMEA_GPGSV    	0x04
#define NMEA_GPRMC    	0x08

#define IS_NSEW(x)		((x)=='N'||(x)=='S'||(x)=='E'||(x)=='W')

void GPS(u8 SBUF);

extern GPGGA GPS_GGA_Data;

extern void ParseGPS(RECV_BUF *pBuf);
extern GPGGA* GetGPSBuf(void);
extern bool IsGPSOK(void);
extern bool GetLocationInfo(char *buf);

#endif

