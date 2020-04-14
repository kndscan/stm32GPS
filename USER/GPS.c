
//-------------------------------------------------------
//GPS 解析模块 By wowbanui
//--------------------------------------------------------- 

#include "GPS.H"
#include "stm32f10x.h"
#include "usart.h"
#include "common.h"
#include <stdlib.h>

/* static 全局变量仅能在本文件内调用 */
static u8 NMEA_CMD = NMEA_NULL;			//NMEA 语句类型
static u8 NMEA_CMD_Buff[] = "$GPxxx,";	//NMEA 语句类型缓存 
static u8 NMEA_CMD_Index = 0;			//读取 CMD字符的个数
static u8 NMEA_DAT_Block = 0;			//NMEA 数据字段号 从0开始
static u8 NMEA_DAT_BlockIndex = 0;		//NMEA 数据每个字段内字符索引 从0开始

static bool NMEA_CMD_Parsered = false;	//CMD类型解析完毕置位
static bool NMEA_CMD_Start = false;		//NMEA 语句开始. 检测到 $ 时置位

GPRMC GPS_RMC_Data;
GPGGA GPS_GGA_Data;
GPGSA GPS_GSA_Data;
GPGSV GPS_GSV_Data;

static bool isGPSOK = false;
bool IsGPSOK(void)
{
	return isGPSOK;
}

GPGGA* GetGPSBuf(void)
{
	return &GPS_GGA_Data;
}

bool GetLocationInfo(char *buf)
{
	u8 i, j;
	if(GPS_GGA_Data.NS != 'N'&& GPS_GGA_Data.NS != 'S')
		return false;
	if(GPS_GGA_Data.EW != 'E'&& GPS_GGA_Data.EW != 'W')
		return false;
	
	strcpy(buf, "AT+CIPSEND=24,");
	j = 14;
	*(buf+j++) = GPS_GGA_Data.NS;
	for(i=0; i<10; i++)
	{
		if(isFloatChar(GPS_GGA_Data.Latitude[i]))
			*(buf+j++) = GPS_GGA_Data.Latitude[i];
		else
			return false;
	}
	*(buf+j++) = ',';
	*(buf+j++) = GPS_GGA_Data.EW;
	for(i=0; i<11; i++)
	{
		if(isFloatChar(GPS_GGA_Data.Longitude[i]))
			*(buf+j++) = GPS_GGA_Data.Longitude[i];
		else
			return false;
	}
	*(buf+j++) = '\r';
	*(buf+j++) = '\n';
	*(buf+j++) = '\0';
	
	return true;
}

static void ParseGPGGA(unsigned char SBUF)
{
    switch(SBUF)
    {
        case '*': //语句结束
            NMEA_CMD_Start = false;
            break;
        case ',': //该字段结束
            NMEA_DAT_Block++;
            NMEA_DAT_BlockIndex=0;
            break;
        default:    //字段字符
            switch(NMEA_DAT_Block) //判断当前处于哪个字段
            {
				case 0:
					GPS_GGA_Data.UTCTime[NMEA_DAT_BlockIndex] = SBUF;
					break;
				case 1:{
					GPS_GGA_Data.Latitude[NMEA_DAT_BlockIndex] = SBUF;
					if(NMEA_DAT_BlockIndex == 10)
						isGPSOK = true;
				}break;
				case 2:
					GPS_GGA_Data.NS = SBUF;
					break;
				case 3:
					GPS_GGA_Data.Longitude[NMEA_DAT_BlockIndex] = SBUF;
					break;
				case 4:
					GPS_GGA_Data.EW = SBUF;
					break;
                case 5:
                    GPS_GGA_Data.Status = SBUF;
                    break;
                case 6:
                    GPS_GGA_Data.SatelliteNum[NMEA_DAT_BlockIndex] = SBUF;
                    break;
                case 7:
                    GPS_GGA_Data.HoriPrecison[NMEA_DAT_BlockIndex] = SBUF;
                    break;
				case 8:
                    GPS_GGA_Data.AntennaHeight[NMEA_DAT_BlockIndex] = SBUF;
                    break;
                case 9:
                    GPS_GGA_Data.AHUnit = SBUF;
                    break;
				case 10:
                    GPS_GGA_Data.EarthHeight[NMEA_DAT_BlockIndex] = SBUF;
                    break;
                case 11:
                    GPS_GGA_Data.EHUnit = SBUF;
                    break;
                case 12:
                    GPS_GGA_Data.DataTerm = SBUF;
                    break;
				case 13:
                    GPS_GGA_Data.BaseStation[NMEA_DAT_BlockIndex] = SBUF;
                    break;
            }
            NMEA_DAT_BlockIndex++;     //字段字符索引++, 指向下一个字符
    }
}

/*
static void ParseGPRMC(unsigned char SBUF)
{
    switch(SBUF)
    {
        case '*':
            NMEA_CMD_Start = false;
            ReciveFlag = false;     //接收完毕, 可以处理
            break;
        case ',':
            NMEA_DAT_Block++;
            NMEA_DAT_BlockIndex=0;
            break;
        default:
            switch(NMEA_DAT_Block)
            {
                case 0:         //<1> UTC时间 hhmmss.mmm
                    switch(NMEA_DAT_BlockIndex)
                    {
                        case 0:
                        case 2:
                        case 4:
                            ucTempA=SBUF-'0';
                            break;
                        case 1:
                            GPS_RMC_Data.UTCDateTime[3]=ucTempA*10+SBUF-'0';
                            break;
                        case 3:
                            GPS_RMC_Data.UTCDateTime[4]=ucTempA*10+SBUF-'0';
                            break;
                        case 5:
                            GPS_RMC_Data.UTCDateTime[5]=ucTempA*10+SBUF-'0';
                            break;
                    }
                    break;
                case 1:         //<2> 定位状态 A=有效定位, V=无效定位
                    GPS_RMC_Data.Status=SBUF;
                    break;
                case 2:         //<3> 纬度 ddmm.mmmm
                    GPS_RMC_Data.Latitude[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 3:         //<4> 纬度半球 N/S
                    GPS_RMC_Data.NS=SBUF;
                    break;
                case 4:         //<5> 经度 dddmm.mmmm
                    GPS_RMC_Data.Longitude[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 5:         //<6> 经度半球 E/W
                    GPS_RMC_Data.EW=SBUF;
                    break;
                case 6:         //<7> 地面速率 000.0~999.9 节
                    GPS_RMC_Data.Speed[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 7:         //<8> 地面航向 000.0~359.9 度, 以真北为参考基准
                    GPS_RMC_Data.Course[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 8:         //<9> UTC日期 ddmmyy
                    switch(NMEA_DAT_BlockIndex)
                    {
                        case 0:
                        case 2:
                        case 4:
                            ucTempA=SBUF-'0';
                            break;
                        case 1:
                            GPS_RMC_Data.UTCDateTime[2]=ucTempA*10+SBUF-'0';
                            break;
                        case 3:
                            GPS_RMC_Data.UTCDateTime[1]=ucTempA*10+SBUF-'0';
                            break;
                        case 5:
                            GPS_RMC_Data.UTCDateTime[0]=ucTempA*10+SBUF-'0';
                            break;
                    }
                    break;
            }
            NMEA_DAT_BlockIndex++;
    }
}

static void ParseGPGSA(unsigned char SBUF)
{
    switch(SBUF)
    {
        case '*':
            NMEA_CMD_Start=false;
            break;
        case ',':
            NMEA_DAT_Block++;
            NMEA_DAT_BlockIndex=0;
            //清空已使用卫星号, 避免上次数据干扰
            if((NMEA_DAT_Block>=2)||(NMEA_DAT_Block<=13))
                GPS_GSA_Data.SatUsedList[NMEA_DAT_Block-2]=0x00;
            break;
        default:
            switch(NMEA_DAT_Block)
            {
                case 0:         //<1>模式 M=手动, A=自动
                    GPS_GSA_Data.Mode=SBUF;
                    break;
                case 1:         //<2>定位型式 1=未定位, 2=二维定位, 3=三维定位
                    GPS_GSA_Data.Mode2=SBUF;
                    break;
                case 2:         //<3> PRN 01~32 使用中的卫星编号
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                case 13:
                    switch(NMEA_DAT_BlockIndex)
                    {
                        case 0:
                            ucTempA=SBUF-'0';
                            break;
                        case 1:
                            GPS_GSA_Data.SatUsedList[NMEA_DAT_Block-2]=ucTempA*10+SBUF-'0';
                            break;
                    }
                    break;
                case 14:        //<4> PDOP 位置精度因子 0.5~99.9
                    GPS_GSA_Data.PDOP[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 15:        //<5> HDOP 水平精度因子 0.5~99.9
                    GPS_GSA_Data.HDOP[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 16:        //<6> VDOP 垂直精度因子 0.5~99.9
                    GPS_GSA_Data.VDOP[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                }
            NMEA_DAT_BlockIndex++;
    }
}

static void ParseGPGSV(unsigned char SBUF)
{
    switch(SBUF)
    {
        case '*':
            NMEA_CMD_Start=false;
            break;
        case ',':
            NMEA_DAT_Block++;
            NMEA_DAT_BlockIndex=0;
            break;
        default:
            switch(NMEA_DAT_Block)
            {
                
                case 1:         //<2> 本句GSV的编号
                    if (SBUF=='1') SateInfoIndex=0;
                    //解析到第一句 GSV 语句 则判断卫星信息从心开始
                    break;
                case 2:         //<3> 可见卫星的总数 00~12
                    switch(NMEA_DAT_BlockIndex)
                    {
                        case 0:
                            ucTempA=SBUF-'0';
                            break;
                        case 1:
                            GPS_GSV_Data.SatInView=ucTempA*10+SBUF-'0';
                            break;
                    }
                    break;
                case 3:         //<4> 卫星编号 01~32
                case 7:
                case 11:
                case 15:
                    switch(NMEA_DAT_BlockIndex)
                    {
                        case 0:
                            ucTempA=SBUF-'0';
                            break;
                        case 1:
                            GPS_GSV_Data.SatInfo[SateInfoIndex].SatID=ucTempA*10+SBUF-'0';

                            GPS_GSV_Data.SatInfo[SateInfoIndex].SNR=0x00;
                            //清空信噪比数据, 避免上次数据干扰
                            //因为当卫星信噪比无时GPS输出的NMEA数据中信噪比字段没有,
                            //而导致直接跳到下一字段, 此时上次若有数据则直接会残留下来

                            SateInfoIndex++;
                            //卫星信息索引+1, 以下都-1处理
                            //同上, 避免放于"信噪比"字段为空时处理不到

                            break;
                    }
                    break;
                
                case 6:         //<7>讯号噪声比 C/No 00~99
                case 10:
                case 14:
                case 18:
                    switch(NMEA_DAT_BlockIndex)
                    {
                        case 0:
                            ucTempA=SBUF-'0';
                            break;
                        case 1:
                            GPS_GSV_Data.SatInfo[SateInfoIndex-1].SNR=ucTempA*10+SBUF-'0';
                            break;
                    }
                    break;
             }
        NMEA_DAT_BlockIndex++;
    }
}
*/
void GPS(unsigned char SBUF)
{
    if(NMEA_CMD_Start)
    {             //解析到以$开始的 NMEA 语句, 进入NMEA 解析流程:
        if(NMEA_CMD_Parsered)
        {         //CMD语句类型解析完毕, 根据类型条用解析函数
            switch(NMEA_CMD)
            {
                case NMEA_GPGGA:
                    ParseGPGGA(SBUF);
                    break;
                case NMEA_GPGSA:
                    //ParseGPGSA(SBUF);
                    break;
                case NMEA_GPGSV:
                    //ParseGPGSV(SBUF);
                    break;
                case NMEA_GPRMC:
                    //ParseGPRMC(SBUF);
                    break;
                default:    //无法识别的格式, 复位
                    NMEA_CMD=NMEA_NULL;
                    NMEA_CMD_Parsered = false;
                    NMEA_CMD_Index = 1;
                    NMEA_CMD_Start = false;
            }
        }
        else
        {         //需要解析CMD语句类型
            switch(SBUF)
            {
                case ',':     //第一个字段结束
                    if(NMEA_CMD_Buff[4]=='G'&&NMEA_CMD_Buff[5]=='A')
					{
						NMEA_CMD = NMEA_GPGGA;
						NMEA_CMD_Parsered = true;
					}
                    else if(NMEA_CMD_Buff[4]=='S'&&NMEA_CMD_Buff[5]=='A')
					{
						NMEA_CMD = NMEA_GPGSA;
						NMEA_CMD_Parsered = true;
					}
                    else if(NMEA_CMD_Buff[5]=='V') 
					{
						NMEA_CMD = NMEA_GPGSV;
						NMEA_CMD_Parsered = true;
					}
                    else if(NMEA_CMD_Buff[5]=='C')
					{
						NMEA_CMD = NMEA_GPRMC;
						NMEA_CMD_Parsered = true;
					}
					else
					{
						//此处如果都不成立, 即语句不被识别, 则NMEA_CMD为NULL或其他,
						//则转为根据类型解析时会跳转到无法识别的格式, 而后复位
						NMEA_CMD = NMEA_NULL;
						NMEA_CMD_Parsered = false;
					}
					NMEA_CMD_Index = 0;
					NMEA_DAT_Block = 0;
					NMEA_DAT_BlockIndex = 0;
                    break;
                case '*':
                    NMEA_CMD_Start = false; 
                    break;
                default:        //处于第一个字段中, 继续接收
				{
                    NMEA_CMD_Buff[NMEA_CMD_Index] = SBUF;
                    NMEA_CMD_Index++;
					
                    if (NMEA_CMD_Index > 6)
					{
						NMEA_CMD_Start = false;
					}
				}break;
            }
        }
    }
    else
    {             //未解析到$, 循环接收并判断 直到 $
        if (SBUF == '$')
        {         //接收到$, 下一个字符即为类型判断字符, 先进行相关变量初始化
            NMEA_CMD_Buff[0] = SBUF;
            NMEA_CMD_Start = true;  //下次调用则进入NMEA 解析流程:
            NMEA_CMD_Index = 1;     //从头存放GPS类型字符到变量
            NMEA_CMD_Parsered = false;
            NMEA_CMD = NMEA_NULL;
            NMEA_DAT_Block = 0;
            NMEA_DAT_BlockIndex = 0;
        }
    }
}

void ParseGPS(RECV_BUF *pBuf)
{
	u8 i;
	if(pBuf == NULL)
		return;
	
	for(i=0; i<pBuf->len; i++)
	{
		GPS(pBuf->buf[i]);
	}
	SET_EVENT(TFT_EVENT, EVENT_GPS_DATA);
	//InitRecvBufEx(pBuf);
}
