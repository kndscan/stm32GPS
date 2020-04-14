
//-------------------------------------------------------
//GPS ����ģ�� By wowbanui
//--------------------------------------------------------- 

#include "GPS.H"
#include "stm32f10x.h"
#include "usart.h"
#include "common.h"
#include <stdlib.h>

/* static ȫ�ֱ��������ڱ��ļ��ڵ��� */
static u8 NMEA_CMD = NMEA_NULL;			//NMEA �������
static u8 NMEA_CMD_Buff[] = "$GPxxx,";	//NMEA ������ͻ��� 
static u8 NMEA_CMD_Index = 0;			//��ȡ CMD�ַ��ĸ���
static u8 NMEA_DAT_Block = 0;			//NMEA �����ֶκ� ��0��ʼ
static u8 NMEA_DAT_BlockIndex = 0;		//NMEA ����ÿ���ֶ����ַ����� ��0��ʼ

static bool NMEA_CMD_Parsered = false;	//CMD���ͽ��������λ
static bool NMEA_CMD_Start = false;		//NMEA ��俪ʼ. ��⵽ $ ʱ��λ

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
        case '*': //������
            NMEA_CMD_Start = false;
            break;
        case ',': //���ֶν���
            NMEA_DAT_Block++;
            NMEA_DAT_BlockIndex=0;
            break;
        default:    //�ֶ��ַ�
            switch(NMEA_DAT_Block) //�жϵ�ǰ�����ĸ��ֶ�
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
            NMEA_DAT_BlockIndex++;     //�ֶ��ַ�����++, ָ����һ���ַ�
    }
}

/*
static void ParseGPRMC(unsigned char SBUF)
{
    switch(SBUF)
    {
        case '*':
            NMEA_CMD_Start = false;
            ReciveFlag = false;     //�������, ���Դ���
            break;
        case ',':
            NMEA_DAT_Block++;
            NMEA_DAT_BlockIndex=0;
            break;
        default:
            switch(NMEA_DAT_Block)
            {
                case 0:         //<1> UTCʱ�� hhmmss.mmm
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
                case 1:         //<2> ��λ״̬ A=��Ч��λ, V=��Ч��λ
                    GPS_RMC_Data.Status=SBUF;
                    break;
                case 2:         //<3> γ�� ddmm.mmmm
                    GPS_RMC_Data.Latitude[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 3:         //<4> γ�Ȱ��� N/S
                    GPS_RMC_Data.NS=SBUF;
                    break;
                case 4:         //<5> ���� dddmm.mmmm
                    GPS_RMC_Data.Longitude[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 5:         //<6> ���Ȱ��� E/W
                    GPS_RMC_Data.EW=SBUF;
                    break;
                case 6:         //<7> �������� 000.0~999.9 ��
                    GPS_RMC_Data.Speed[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 7:         //<8> ���溽�� 000.0~359.9 ��, ���汱Ϊ�ο���׼
                    GPS_RMC_Data.Course[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 8:         //<9> UTC���� ddmmyy
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
            //�����ʹ�����Ǻ�, �����ϴ����ݸ���
            if((NMEA_DAT_Block>=2)||(NMEA_DAT_Block<=13))
                GPS_GSA_Data.SatUsedList[NMEA_DAT_Block-2]=0x00;
            break;
        default:
            switch(NMEA_DAT_Block)
            {
                case 0:         //<1>ģʽ M=�ֶ�, A=�Զ�
                    GPS_GSA_Data.Mode=SBUF;
                    break;
                case 1:         //<2>��λ��ʽ 1=δ��λ, 2=��ά��λ, 3=��ά��λ
                    GPS_GSA_Data.Mode2=SBUF;
                    break;
                case 2:         //<3> PRN 01~32 ʹ���е����Ǳ��
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
                case 14:        //<4> PDOP λ�þ������� 0.5~99.9
                    GPS_GSA_Data.PDOP[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 15:        //<5> HDOP ˮƽ�������� 0.5~99.9
                    GPS_GSA_Data.HDOP[NMEA_DAT_BlockIndex]=SBUF;
                    break;
                case 16:        //<6> VDOP ��ֱ�������� 0.5~99.9
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
                
                case 1:         //<2> ����GSV�ı��
                    if (SBUF=='1') SateInfoIndex=0;
                    //��������һ�� GSV ��� ���ж�������Ϣ���Ŀ�ʼ
                    break;
                case 2:         //<3> �ɼ����ǵ����� 00~12
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
                case 3:         //<4> ���Ǳ�� 01~32
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
                            //������������, �����ϴ����ݸ���
                            //��Ϊ�������������ʱGPS�����NMEA������������ֶ�û��,
                            //������ֱ��������һ�ֶ�, ��ʱ�ϴ�����������ֱ�ӻ��������

                            SateInfoIndex++;
                            //������Ϣ����+1, ���¶�-1����
                            //ͬ��, �������"�����"�ֶ�Ϊ��ʱ������

                            break;
                    }
                    break;
                
                case 6:         //<7>Ѷ�������� C/No 00~99
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
    {             //��������$��ʼ�� NMEA ���, ����NMEA ��������:
        if(NMEA_CMD_Parsered)
        {         //CMD������ͽ������, �����������ý�������
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
                default:    //�޷�ʶ��ĸ�ʽ, ��λ
                    NMEA_CMD=NMEA_NULL;
                    NMEA_CMD_Parsered = false;
                    NMEA_CMD_Index = 1;
                    NMEA_CMD_Start = false;
            }
        }
        else
        {         //��Ҫ����CMD�������
            switch(SBUF)
            {
                case ',':     //��һ���ֶν���
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
						//�˴������������, ����䲻��ʶ��, ��NMEA_CMDΪNULL������,
						//��תΪ�������ͽ���ʱ����ת���޷�ʶ��ĸ�ʽ, ����λ
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
                default:        //���ڵ�һ���ֶ���, ��������
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
    {             //δ������$, ѭ�����ղ��ж� ֱ�� $
        if (SBUF == '$')
        {         //���յ�$, ��һ���ַ���Ϊ�����ж��ַ�, �Ƚ�����ر�����ʼ��
            NMEA_CMD_Buff[0] = SBUF;
            NMEA_CMD_Start = true;  //�´ε��������NMEA ��������:
            NMEA_CMD_Index = 1;     //��ͷ���GPS�����ַ�������
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
