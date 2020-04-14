#ifndef __GPS_H__
#define __GPS_H__

#include "stm32f10x.h"
#include "USART.h"

//运输定位数据
typedef struct tagGPRMC
{
    u8 UTCTime[10]; 	//<1>  hhmmss.sss
    u8 Status; 			//<2>  A-valid; V-invalid
    char Latitude[9];     //<3>  ddmm.mmmm
    u8 NS;              //<4>  N/S
    char Longitude[10];   //<5>  dddmm.mmmm
    u8 EW;              //<6>  E/W
    u8 Speed[6];        //<7>  速率000.0~1851.8节
    u8 Course[5];       //<8>  航向000.0~359.9度
	u8 UTCDate[6];		//<9>  DDMMYY
	u8 MagVar[5];		//<10> 000.0~180.0
	u8 Declination;		//<11> E/W
	u8 ModeIndicator;	//<12> 模式指示A-自主定位，D-差分，E-估算，N-数据无效(仅NMEA0183 3.00版本输出)
	u8 Seprator;		//<13> '*'
	u8 Checksum;		//<14> 校验和
}GPRMC;

//全球定位数据
typedef struct tagGPGGA
{
    u8 UTCTime[10]; 	//<1>  hhmmss.sss
    char Latitude[10];	//={"1234.56789"};    //<2>  ddmm.mmmmm
    char NS;            //<3>  N/S
    char Longitude[11];	//={"98765.43210"};   //<4>  dddmm.mmmmm
    char EW;            //<5>  E/W
    u8 Status; 			//<6>  1-valid; 0-invalid
	u8 SatelliteNum[2];	//<7>  使用卫星数量，从00到12（前导位数不足补0）
	u8 HoriPrecison[4];	//<8>  水平精确度，0.5到99.9
	u8 AntennaHeight[7];//<9>  天线离海平面的高度，-9999.9到9999.9米
	u8 AHUnit;			//<10> 高度单位，M表示单位米
	u8 EarthHeight[6];	//<11> 大地椭球面相对海平面的高度（-999.9到9999.9）
	u8 EHUnit;			//<12> 高度单位，M表示单位米
	u8 DataTerm;		//<13> 差分GPS数据期限（RTCM SC-104），最后设立RTCM传送的秒数量
	u8 BaseStation[4];	//<14> 差分参考基站标号，从0000到1023（前导位数不足则补0）
	u8 Seprator;		//<15> '*'
	u8 Checksum;		//<16> 校验和
}GPGGA;

//地面速度信息(VTG)
typedef struct tagGPVTG
{
    u8 GeoNorthDir[3];	//<1>  以真北为参考基准的地面航向(000~359度，前面的0也将被传输)
	u8 T;				//<2>  
    u8 MagNorthDir[3];	//<3>  以磁北为参考基准的地面航向(000~359度，前面的0也将被传输)
	u8 M;               //<4>  
    u8 SpeedKnot[5];	//<5>  地面速率(000.0~999.9节，前面的0也将被传输)
	u8 N;               //<6>  
    u8 SpeedKilo[10];   //<7>  地面速率(0000.0~1851.8公里/小时，前面的0也将被传输)
	u8 K;               //<8>  
    u8 Status;			//<9>  模式指示A-自主定位，D-差分，E-估算，N-数据无效(仅NMEA0183 3.00版本输出)
	u8 Seprator;		//<10> '*'
	u8 Checksum;		//<11> 校验和
}GPVTG;

//卫星PRN数据
typedef struct tagGPGSA
{
    u8 Mode;			//<1>  模式: M=手动，A = 自动
    u8 Mode2;			//<2>  定位型式: 1 = 未定位，2 = 二维定位，3 = 三维定位
    u8 SatList[12];		//<3>  01~32 表使用中的卫星编号，最多可接收12颗卫星信息
    u8 PDOP[4];			//<4>  PDOP位置精度因子（0.5~99.9）
    u8 HDOP[4];			//<5>  HDOP水平精度因子（0.5~99.9）
    u8 VDOP[4];			//<6>  VDOP垂直精度因子（0.5~99.9）
	u8 Seprator;		//<7>  '*'
	u8 Checksum;		//<8>  校验和
}GPGSA;

//卫星状态数据
typedef struct tagSatInfo
{
	u8 SatIndex[2];			//<4> 卫星编号，01至32。 
	u8 ElevationAngle[2];	//<5> 卫星仰角，00至90度。 
	u8 AzimuthAngle[3];		//<6> 卫星方位角，000至359度。实际值。 
	u8 SNR;					//<7> 信噪比(signal-to-noise ratio)，00至99dB；无表未接收到讯号。 
}SatInfo;
//可视卫星状态输出语句
typedef struct tagGPGSV
{
	u8 FrameCnt;		//<1>  总的GSV语句电文数。 
	u8 FrameIndex;		//<2>  当前GSV语句号。 
	u8 SatCnt[2];		//<3>  可视卫星总数，00至12。 
	SatInfo Info[4];	//<4>  卫星状态数据，每条语句最左包括4条卫星数据
	u8 Seprator;		//<5>  '*'
	u8 Checksum;		//<6>  校验和
}GPGSV;

#define NMEA_NULL    	0x00            //GPS语句类型
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

