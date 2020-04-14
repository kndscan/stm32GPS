
#include "GPRS.h"
#include "GPS.h"
#include "usart.h"
#include "task.h"
#include "common.h"
#include "hx711.h"
#include <string.h>

/********************************************************************
目前仅支持如下指令集，阻塞模式处理。
发出一条AT指令后，等待返回帧，若返回帧在指定时间内被接收到，正常结束指令；
若超时，异常结束指令并报错。

GPRS上电初始化流程：
上电—>循环发送AT指令->AT返回OK->置位isATOK标志
*********************************************************************/
static CMD_INFO ATCmds[] = 
{
	//type      overtime   repeat	cmdBuf	            name
	{ctNULL,		   0,		0,	"",					"NULL"},
	{ctAT,			1000,	10000,	"AT\r\n",			"AT"},
	{ctATE0,		1000,		3,	"ATE0\r\n",			"ATE0"},
	{ctATW,			1000,		3,	"AT&W\r\n",			"ATW"},
	{ctAskCPIN,		1000,		3,	"AT+CPIN?\r\n",		"CPIN?"},
	{ctAskCOPS,		1000,		3,	"AT+COPS?\r\n",		"COPS?"},
	{ctAskCSQ,		1000,		3,	"AT+CSQ\r\n",		"CSQ?"},
	{ctAskCSMINS,	1000,		3,	"AT+CSMINS?\r\n",	"CSMINS?"},
	{ctAskCCID,		1000,		3,	"AT+CCID\r\n",		"CCID?"},
	{ctAskCREG,		1000,		3,	"AT+CREG?\r\n",		"CREG?"},
	{ctCREG2,		1000,		3,	"AT+CREG=2\r\n",	"CREG2"},

	{ctOpenGPS,		1000,		3,	"AT+GPS=1\r\n",		"GPS1"},
	{ctCloseGPS,	1000,		3,	"AT+GPS=0\r\n",		"GPS0"},
	{ctAskGPS,		1000,		3,	"AT+GPS?\r\n",		"GPS?"},
	{ctOpenAGPS,	2000,		3,	"AT+AGPS=1\r\n",	"AGPS1"},
	{ctCloseAGPS,	1000,		3,	"AT+AGPS=0\r\n",	"AGPS0"},
	{ctAskAGPS,		1000,		3,	"AT+AGPS?\r\n",		"AGPS?"},

//	{ctOpenGPSRD1,	1000,		3,	"AT+GPSRD=1\r\n",	"GPSRD1"},
//	{ctOpenGPSRD2,	1000,		3,	"AT+GPSRD=2\r\n",	"GPSRD2"},
	{ctOpenGPSRD5,	1000,		3,	"AT+GPSRD=5\r\n",	"GPSRD5"},
	{ctCloseGPSRD,	1000,		3,	"AT+GPSRD=0\r\n",	"GPSRD0"},
	{ctAskGPSRD,	1000,		3,	"AT+GPSRD?\r\n",	"GPSRD?"},
	
	{ctCGATT,		1000,		3,	"AT+CGATT=1\r\n",	"CGATT"},
	{ctCGDCONT,		1000,		3,	"AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n",	"CGDCONT"},
	{ctCGACT,		1000,		3,	"AT+CGACT=1,1\r\n",						"CGACT"},
	{ctCIPSTART,	2000,		3,	"AT+CIPSTART=\"TCP\",\"webtcp.tongxinmao.com\",10002\r\n",	"CIPST"},
	//{ctCIPSTART,	2000,		3,	"AT+CIPSTART=\"TCP\",\"118.190.208.143\",13333\r\n",	"CIPST"},
	{ctCIPSTATUS,	1000,		3,	"AT+CIPSTATUS?\r\n","CIPSTA?"},
	{ctCIPSendWt,	2000,		3,	"AT+CIPSEND\r\n",	"CIPSd>"},
	{ctCIPSendDt,	2000,		3,	"AT+CIPSEND=3,123\r\n",	"CIPSd="}
};

static AT_RUN_INFO ATCmdStatus = {	ctNULL,	stNULL,	0,	};
static AT_CMD_STATUS_INFO ATCmdStatusInfo[] = 
{//	status			name
	{stNULL, 		"Nu"},
	{stWaiting,		"Wa"},
	{stParsing,		"Pa"},
	{stOK,			"Ok"},	
	{stError,		"Er"},	
	{stReady,		"Rd"},	
	{stSend,		"Sd"},
	{stOvertime,	"Ov"},
	{stCnt, 		"Mx"},
};
CMD_GROUP SIMCmds = 
{
	5,	0,{ctATE0,		ctAskCCID,	ctAskCREG,		ctAskCOPS,		ctAskCPIN,}
};
CMD_GROUP GPSCmds = 
{
	4,	0,{ctOpenGPS,	ctAskGPS,	ctOpenGPSRD5, 	ctAskGPSRD,}
};
CMD_GROUP GPRSCmds = 
{
	6,	0,{ctCGATT,		ctCGDCONT,	ctCGACT,		ctCIPSTATUS,	ctCIPSTART,		ctCIPSTATUS,}
};

static GPRS_INFO GPRSInfo;
GPRS_INFO *GetGPRSInfo(void)
{
	return &GPRSInfo;
}
void InitGPRSInfo(void)
{
	GPRSInfo.bFirstOK = true;
	GPRSInfo.bA7Ready = false;
	GPRSInfo.bSimReady = false;
	GPRSInfo.bGPSReady = false;
	GPRSInfo.bAGPSReady = false;
	GPRSInfo.bGPRSReady = false;
	
	memset(GPRSInfo.aSimID, 0, 20);
	memset(GPRSInfo.aCPIN, 0, 8);
	
	memset(GPRSInfo.aSignal, 0, 2);
	memset(GPRSInfo.aErRate, 0, 2);
	
	GPRSInfo.cGPS = 0;
	GPRSInfo.cAGPS = 0;
	GPRSInfo.cGPSRD = 0;
	
	GPRSInfo.netRegMode = 0;
	GPRSInfo.netRegState = 0;
	memset(GPRSInfo.locationID, 0, 10);
	memset(GPRSInfo.blockID, 0, 10);
	
	GPRSInfo.copsMode = 0;
	GPRSInfo.copsStyle = 0;
	memset(GPRSInfo.copsOpe, 0, 7);
	
	GPRSInfo.cIPChannel = 0;
	memset(GPRSInfo.aIPStatus, 0, 20);
	
	GPRSInfo.pAT = &ATCmdStatus;
	GPRSInfo.pData = &CloudRxBuf;
}
bool GetFirstOK(void)			{	return GPRSInfo.bFirstOK;	}
void SetFirstOK(bool first)		{	GPRSInfo.bFirstOK = first;	}

static CLOUD_RX_BUF CloudRxBuf;
CLOUD_RX_BUF *GetCloudRxBuf()	{return &CloudRxBuf;}
static char CloudTxBuf[60];

static RESPONSE_PARSE_INFO ParseInfo;
void InitResponseInfo(void)
{
	ParseInfo.type = gtNULL;
	ParseInfo.headIndex = 0;
	memset(ParseInfo.head, 0, RESPONSE_HEAD_LEN);
	
	ParseInfo.start = false;
	ParseInfo.headParsed = false;
	
	ParseInfo.block = 0;
	ParseInfo.blockIndex = 0;
}

/**********************************************************************
ATCmdStatus变量存储正在执行的AT指令状态信息，为异步执行AT指令和任务间信息
交互提供支持。
***********************************************************************/
void InitATCmdStatus(void)
{
	ATCmdStatus.type = ctNULL;
	ATCmdStatus.status = stNULL;
	ATCmdStatus.repeatTimes = 0;
	
	SET_EVENT(LED_EVENT, EVENT_OV_TIME_OFF);
	SET_EVENT(LED_EVENT, EVENT_AT_ERR_OFF);
	
	SET_EVENT(TFT_EVENT, EVENT_AT_TYPE);
}
void SetATCmdStatus(STATUS_TYPE status)
{
	ATCmdStatus.status = status;

	if(status == stOvertime)
		SET_EVENT(LED_EVENT, EVENT_OV_TIME_ON);
	else
		SET_EVENT(LED_EVENT, EVENT_OV_TIME_OFF);
	
	if(status == stError)
		SET_EVENT(LED_EVENT, EVENT_AT_ERR_ON);
	else
		SET_EVENT(LED_EVENT, EVENT_AT_ERR_OFF);
	
	SET_EVENT(TFT_EVENT, EVENT_AT_STATUS);
}
AT_CMD_STATUS GetATCmdStatus(void)
{
	return ATCmdStatus.status;
}
AT_CMD_TYPE GetATCmdType(void)
{
	return ATCmdStatus.type;
}
void SetATCmdType(AT_CMD_TYPE type)
{
	ATCmdStatus.type = type;
	SET_EVENT(TFT_EVENT, EVENT_AT_TYPE);
}
void SetATCmdRepeatTimes(u16 times)
{
	ATCmdStatus.repeatTimes = times;
}
char* GetATCmdTypeName(AT_CMD_TYPE type)
{
	return ATCmds[type].pName;
}
char* GetATCmdStatusName(AT_CMD_STATUS status)
{
	return ATCmdStatusInfo[status].pName;
}
/***********************************************************************/

void ATCmdRepeat(void)
{
	if((ATCmdStatus.status == stOvertime || ATCmdStatus.status == stError) && 
	   (ATCmdStatus.repeatTimes < ATCmds[ATCmdStatus.type].repeatTimes) && 
	   (ATCmdStatus.type > ctNULL && ATCmdStatus.type < ctCnt))
	{
		SendATCmd(ATCmdStatus.type);
		ATCmdStatus.repeatTimes++;
	}
}
void SendATCmd(AT_CMD_TYPE type)
{
	if((type > ctNULL) && (type < ctCnt))
	{
		SendString(AT_USART, ATCmds[type].pCmd);
		SetATCmdType(type);
		SetATCmdStatus(stWaiting);
		SetTask(tpHIGH, TASK_AT_OVERTIME_CHECK, ENABLE, ATCmds[type].overtime, tsSTOP);
	}
}
char SendATCmds(CMD_GROUP *group)
{
	if(group->index < group->count)
	{	
		if(ATCmdStatus.status == stError || ATCmdStatus.status == stOvertime)
			SendATCmd(group->cmds[group->index]);
		else if(ATCmdStatus.status == stNULL)
			SendATCmd(group->cmds[group->index++]);
		
		return 0;
	}
	else
	{
		return 1;
	}
}

void SendCloudData()
{
	unsigned char i, j, k;
	strcpy(CloudTxBuf, "AT+CIPSEND=38,");
	if(IS_NSEW(GPS_GGA_Data.NS))
		CloudTxBuf[14] = GPS_GGA_Data.NS;
	else
		CloudTxBuf[14] = '?';
	for(i=0; i<10; i++)
	{
		if(isFloatChar(GPS_GGA_Data.Latitude[i]))
			CloudTxBuf[15+i] = GPS_GGA_Data.Latitude[i];
		else
			CloudTxBuf[15+i] = '0';
	}
	CloudTxBuf[25] = ';';
	
	if(IS_NSEW(GPS_GGA_Data.EW))
		CloudTxBuf[26] = GPS_GGA_Data.EW;
	else
		CloudTxBuf[26] = '?';
	for(i=0; i<11; i++)
	{
		if(isFloatChar(GPS_GGA_Data.Longitude[i]))
			CloudTxBuf[27+i] = GPS_GGA_Data.Longitude[i];
		else
			CloudTxBuf[27+i] = '0';
	}
	CloudTxBuf[38] = ';';
	
	CloudTxBuf[39] = 'w';
	for(i=0; i<INT_DIGIT_NUM; i++)
	{
		if(!isDigitChar(ShiWuBuf[i]))
			break;
	}
	j = i;
	k = 0;
	for(i=INT_DIGIT_NUM; i>j; i--)
	{
		CloudTxBuf[40+k++] = '0';
	}
	for(i=0; i<=j; i++)
	{
		CloudTxBuf[40+k+i] = ShiWuBuf[i];
	}
	
	CloudTxBuf[40+INT_DIGIT_NUM]='\r';
	CloudTxBuf[41+INT_DIGIT_NUM]='\n';
	CloudTxBuf[42+INT_DIGIT_NUM]='\0';
	
	SendString(AT_USART, CloudTxBuf);
	SetATCmdType(ctCIPSendDt);
	SetATCmdStatus(stWaiting);
	SetTask(tpHIGH, TASK_AT_OVERTIME_CHECK, ENABLE, ATCmds[ctCIPSendDt].overtime, tsSTOP);
}

bool IsSIMReady(void)
{
	
	GPRSInfo.bSimReady = (bool)(!(	GPRSInfo.aSimID[0] == '\0' 	|| 	//未检测到SIM卡
									GPRSInfo.netRegMode == '0' 	|| 	//SIM卡未注册
									GPRSInfo.aCPIN[0] != 'R'));		//CPIN指令返回非"READY",可能需要输入PIN码
	
	if(GPRSInfo.bSimReady)
	{
		SET_EVENT(TFT_EVENT, EVENT_SIM_ON);
		SET_EVENT(LED_EVENT, EVENT_SIM_ON);
	}
	else
	{
		SET_EVENT(TFT_EVENT, EVENT_SIM_OFF);
		SET_EVENT(LED_EVENT, EVENT_SIM_OFF);
	}
	
	return GPRSInfo.bSimReady;
}
bool IsGPSReady(void)
{
	GPRSInfo.bGPSReady = (bool)(GPRSInfo.cGPS == '1' && GPRSInfo.cGPSRD == '5');
	
	if(GPRSInfo.bGPSReady)
		SET_EVENT(LED_EVENT, EVENT_GPS_ON);
	else
		SET_EVENT(LED_EVENT, EVENT_GPS_OFF);

	return GPRSInfo.bGPSReady;
}
bool IsGPRSReady(void)
{
	GPRSInfo.bGPRSReady = (bool)(GPRSInfo.cIPChannel == '0' && GPRSInfo.aIPStatus[0] == 'C');
	
	if(GPRSInfo.bGPRSReady)
		SET_EVENT(LED_EVENT, EVENT_GPRS_ON);
	else
		SET_EVENT(LED_EVENT, EVENT_GPRS_OFF);

	return GPRSInfo.bGPRSReady;
}

void InitSIM(void)
{
	SetTask(tpHIGH, TASK_WAIT_GPS_READY, DISABLE, TASK_2S, tsSTOP);
	SetTask(tpHIGH, TASK_WAIT_GPRS_READY, DISABLE, TASK_2S, tsSTOP);
	SetTask(tpHIGH, TASK_WAIT_SIM_READY, ENABLE, TASK_3S, tsSTOP);
	SIMCmds.index = 0;
}
void InitGPS(void)
{
	SetTask(tpHIGH, TASK_WAIT_GPRS_READY, DISABLE, TASK_2S, tsSTOP);
	SetTask(tpHIGH, TASK_WAIT_SIM_READY, DISABLE, TASK_2S, tsSTOP);
	SetTask(tpHIGH, TASK_WAIT_GPS_READY, ENABLE, TASK_2S, tsSTOP);
	GPSCmds.index = 0;
}
void InitGPRS(void)
{
	SetTask(tpHIGH, TASK_WAIT_GPS_READY, DISABLE, TASK_2S, tsSTOP);
	SetTask(tpHIGH, TASK_WAIT_SIM_READY, DISABLE, TASK_2S, tsSTOP);
	SetTask(tpHIGH, TASK_WAIT_GPRS_READY, ENABLE, TASK_5S, tsSTOP);
	GPRSCmds.index = 0;
}

void WaitA7Ready(void)
{
	SET_EVENT(LED_EVENT, EVENT_A7_ON);
	SetTask(tpHIGH, TASK_WAIT_A7_READY, DISABLE, TASK_3S, tsSTOP);
	InitSIM();
}
void WaitSIMReady(void)
{
	if(IsSIMReady())
	{
		InitGPRS();
	}
	else
	{
		if(1 == SendATCmds(&SIMCmds))
			InitSIM();
	}
}

void WaitGPRSReady(void)
{
	if(IsGPRSReady())
	{
		InitGPS();
	}
	else
	{
		if(1 == SendATCmds(&GPRSCmds))
			InitGPRS();
	}
}

void WaitGPSReady(void)
{
	if(IsGPSReady())
	{
		SetTask(tpHIGH, TASK_WAIT_GPS_READY, DISABLE, TASK_2S, tsSTOP);
		SetTask(tpHIGH, TASK_WAIT_SIM_READY, DISABLE, TASK_2S, tsSTOP);
		SetTask(tpHIGH, TASK_WAIT_GPRS_READY, DISABLE, TASK_2S, tsSTOP);
		
		SetTask(tpHIGH, TASK_MONITOR_SIGNAL, ENABLE, TASK_5S, tsSTOP);
		SetTask(tpHIGH, TASK_SEND_CLOUD_DATA, ENABLE, TASK_10S, tsSTOP);
	}
	else
	{
		if(1 == SendATCmds(&GPSCmds))
			InitGPS();
	}
}

void PowerOnGPRS(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Pin = A7PWR;				//PWR
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA, A7PWR);
	SetTask(tpHIGH, TASK_GPRS_PWR_ON, ENABLE, TASK_5S, tsSTOP);
	
//	InitGPRSInfo();
//	SetTask(tpHIGH, TASK_PARSE_USART, ENABLE, TASK_STEP, tsSTOP);
//	ENABLE_UART(AT_USART);
}

void ProcessCloudData()
{
	if(CloudRxBuf.state == rbsFinished)
	{
		
	}
}

void UpdateInfo()
{
	switch(GetATCmdStatus())
	{
		case stOK:
		{
			switch(ATCmdStatus.type)
			{
				case ctAskCSQ:
					SET_EVENT(TFT_EVENT, EVENT_SIG);
					break;
				case ctAskCOPS:
					SET_EVENT(TFT_EVENT, EVENT_COPS);
					break;
				case ctAskCPIN:
				{
					if(IsSIMReady())
						SET_EVENT(TFT_EVENT, EVENT_SIM_ON);
					else
						SET_EVENT(TFT_EVENT, EVENT_SIM_OFF);
				}break;
				case ctAskCCID:
				{
					SET_EVENT(TFT_EVENT, EVENT_CCID);
					if(IsSIMReady())
						SET_EVENT(TFT_EVENT, EVENT_SIM_ON);
					else
						SET_EVENT(TFT_EVENT, EVENT_SIM_OFF);
				}break;
				case ctAskCREG:
				{
					SET_EVENT(TFT_EVENT, EVENT_REG);
					if(IsSIMReady())
						SET_EVENT(TFT_EVENT, EVENT_SIM_ON);
					else
						SET_EVENT(TFT_EVENT, EVENT_SIM_OFF);
				}break;
				
				case ctAskGPS:
				case ctOpenGPS:
				case ctCloseGPS:
				{
					if(IsGPSReady())
						SET_EVENT(TFT_EVENT, EVENT_GPS_ON);
					else
						SET_EVENT(TFT_EVENT, EVENT_GPS_OFF);
				}break;
				
				case ctAskAGPS:
				case ctOpenAGPS:
				case ctCloseAGPS:
				{
					if(GPRSInfo.cAGPS)
						SET_EVENT(TFT_EVENT, EVENT_AGPS_ON);
					else
						SET_EVENT(TFT_EVENT, EVENT_AGPS_OFF);
				}break;
				
				case ctAskGPSRD:
				case ctOpenGPSRD5:
				{
					if(GPRSInfo.cGPSRD == '5')
						SET_EVENT(TFT_EVENT, EVENT_GPSRD_ON);
					else
						SET_EVENT(TFT_EVENT, EVENT_GPSRD_OFF);
				}break;
				
				case ctCIPSTATUS:
				case ctCIPSTART:
				{
					if(IsGPRSReady())
						SET_EVENT(TFT_EVENT, EVENT_GPRS_ON);
					else
						SET_EVENT(TFT_EVENT, EVENT_GPRS_OFF);
				}break;
			}
			InitATCmdStatus();
			SetTask(tpHIGH, TASK_AT_OVERTIME_CHECK, DISABLE, 0, tsSTOP);
		}break;
		case stError:
		{
			switch(ATCmdStatus.type)
			{
				case ctAskCSQ:
					memset(GPRSInfo.aSignal, 0, 2);
					memset(GPRSInfo.aErRate, 0, 2);
					SET_EVENT(TFT_EVENT, EVENT_SIG);
					break;
				case ctAskCOPS:
					GPRSInfo.copsMode = 0;
					GPRSInfo.copsStyle = 0;
					memset(GPRSInfo.copsOpe, 0, 7);
					SET_EVENT(TFT_EVENT, EVENT_COPS);
					break;
				case ctAskCPIN:
					memset(GPRSInfo.aCPIN, 0, 8);
					SET_EVENT(TFT_EVENT, EVENT_SIM_OFF);
					if(IsSIMReady())
						SET_EVENT(TFT_EVENT, EVENT_SIM_ON);
					else
						SET_EVENT(TFT_EVENT, EVENT_SIM_OFF);
					break;
				case ctAskCCID:
					memset(GPRSInfo.aSimID, 0, 20);
					SET_EVENT(TFT_EVENT, EVENT_CCID);
					if(IsSIMReady())
						SET_EVENT(TFT_EVENT, EVENT_SIM_ON);
					else
						SET_EVENT(TFT_EVENT, EVENT_SIM_OFF);
					break;
				case ctAskCREG:
					GPRSInfo.netRegMode = 0;
					GPRSInfo.netRegState = 0;
					memset(GPRSInfo.locationID, 0, 10);
					memset(GPRSInfo.blockID, 0, 10);
					SET_EVENT(TFT_EVENT, EVENT_REG);
					if(IsSIMReady())
						SET_EVENT(TFT_EVENT, EVENT_SIM_ON);
					else
						SET_EVENT(TFT_EVENT, EVENT_SIM_OFF);
					break;
				
				case ctAskGPS:
				case ctOpenGPS:
				case ctCloseGPS:
					break;
				
				case ctAskAGPS:
				case ctOpenAGPS:
				case ctCloseAGPS:
					break;
				
				case ctAskGPSRD:
				case ctOpenGPSRD5:
					break;
				
				case ctCIPSTART:
					break;
			}
			SetTask(tpHIGH, TASK_AT_OVERTIME_CHECK, DISABLE, 0, tsSTOP);
		}break;
		case stReady:
		default:break;
	}
}

void ParseCSQ(char c)
{
	switch(c)
	{
		case ',':
		{
			ParseInfo.block++;
			ParseInfo.blockIndex=0;
		}break;
		default:
		{
			switch(ParseInfo.block) //判断当前处于哪个字段
			{
				case 0:{
					if(ParseInfo.blockIndex < 2)
						GPRSInfo.aSignal[ParseInfo.blockIndex++] = c;
				}break;
				case 1:{
					if(ParseInfo.blockIndex < 2)
						GPRSInfo.aErRate[ParseInfo.blockIndex++] = c;
				}break;
			}
		}break;
	}
}

void ParseCIPStatus(char c)
{
	switch(c)
	{
		case ',':
		{
			ParseInfo.block++;
			ParseInfo.blockIndex=0;
		}break;
		default:
		{
			switch(ParseInfo.block) //判断当前处于哪个字段
			{
				case 0:{
					if(ParseInfo.blockIndex < 1)
						GPRSInfo.cIPChannel = c;
				}break;
				case 1:{
					if(ParseInfo.blockIndex < 10)
						GPRSInfo.aIPStatus[ParseInfo.blockIndex++] = c;
				}break;
			}
		}break;
	}
}

void ParseCloudData(char c)
{
	switch(c)
	{
		case ',':
		{
			ParseInfo.block++;
			ParseInfo.blockIndex=0;
		}break;
		default:
		{
			switch(ParseInfo.block) //判断当前处于哪个字段
			{
				case 0:
				{
					if(ParseInfo.blockIndex < CLOUD_RX_BUF_LEN_LEN)
						CloudRxBuf.len[ParseInfo.blockIndex++] = c;
				}break;
				case 1:
				{
					if(ParseInfo.blockIndex < CLOUD_RX_BUF_LEN)
						CloudRxBuf.buf[ParseInfo.blockIndex++] = c;
				}break;
			}
		}break;
	}
}

void ParseCPIN(char c)
{
	if(ParseInfo.blockIndex < 8)
		GPRSInfo.aCPIN[ParseInfo.blockIndex++] = c;
}

void ParseCCID(char c)
{
	if(ParseInfo.blockIndex < 20)
		GPRSInfo.aSimID[ParseInfo.blockIndex++] = c;
}

void ParseCREG(char c)
{
	switch(c)
	{
		case ',':
		{
			ParseInfo.block++;
			ParseInfo.blockIndex=0;
		}break;
		default:
		{
			switch(ParseInfo.block) //判断当前处于哪个字段
			{
				case 0:	GPRSInfo.netRegMode = c;		break;
				case 1:	GPRSInfo.netRegState = c;		break;
				case 2:	
				{
					if(ParseInfo.blockIndex < 6)
						GPRSInfo.locationID[ParseInfo.blockIndex++] = c;
				}break;
				case 3:
				{
					if(ParseInfo.blockIndex < 6)
						GPRSInfo.blockID[ParseInfo.blockIndex++] = c;
				}break;
			}
		}break;
	}
}

void ParseCOPS(char c)
{
	switch(c)
	{
		case ',':
		{
			ParseInfo.block++;
			ParseInfo.blockIndex=0;
		}break;
		default:
		{
			switch(ParseInfo.block) //判断当前处于哪个字段
			{
				case 0: GPRSInfo.copsMode = c;		break;
				case 1: GPRSInfo.copsStyle = c;		break;
				case 2: 
				{
					if(ParseInfo.blockIndex < 7)
						GPRSInfo.copsOpe[ParseInfo.blockIndex++] = c;
				}break;
			}
		}break;
	}
}

void ParseGPSState(char c)
{
	GPRSInfo.cGPS = c;
}

void ParseGPSRDState(char c)
{
	GPRSInfo.cGPSRD = c;
}

void ParseAGPSState(char c)
{
	GPRSInfo.cAGPS = c;
}

bool GPRS(char c)
{
	bool bEnd = false;
	if(c==' ')
		return bEnd;

	if(ParseInfo.start)
	{
		if(ParseInfo.headParsed)
		{
            switch(ParseInfo.type)
            {
				case gtCPIN:		ParseCPIN(c);			break;
                case gtCSQ:			ParseCSQ(c);			break;
				case gtCCID:		ParseCCID(c);			break;
				case gtCREG:		ParseCREG(c);			break;
				case gtCOPS:		ParseCOPS(c);			break;
                case gtAGPS:		ParseAGPSState(c);		break;
                case gtCloudData:	ParseCloudData(c);		break;
				case gtCIPStatus:	ParseCIPStatus(c);		break;
				case gtGPSRD:		ParseGPSRDState(c);		break;
                case gtGPS:			ParseGPSState(c);		break;
				
                case gtOK:
				case gtConnectOK:
				{
					SetATCmdStatus(stOK);
					bEnd = true;
				}break;
				
                case gtReady:
				{
					SetATCmdStatus(stReady);
					bEnd = true;
				}break;
				
				case gtCmdNoRes:
				case gtError:
				case gtCME:
                default:
				{
					SetATCmdStatus(stError);
					bEnd = true;
				}break;
            }
		}
		else
		{
			switch(c)
			{
				case ':':
				{
					if(CHECK_HEAD2(2, 'P', 'I'))			ParseInfo.type = gtCPIN;
					else if(CHECK_HEAD2(2, 'S', 'Q'))		ParseInfo.type = gtCSQ;
					else if(CHECK_HEAD2(2, 'C', 'I'))		ParseInfo.type = gtCCID;
					else if(CHECK_HEAD2(2, 'R', 'E'))		ParseInfo.type = gtCREG;
					else if(CHECK_HEAD2(2, 'O', 'P'))		ParseInfo.type = gtCOPS;
					else if(CHECK_HEAD2(2, 'G', 'P'))		ParseInfo.type = gtAGPS;
					else if(CHECK_HEAD3(2, 'I', 'P', 'R'))	ParseInfo.type = gtCloudData;
					else if(CHECK_HEAD3(2, 'I', 'P', 'S'))	ParseInfo.type = gtCIPStatus;
					else if(CHECK_HEAD3(2, 'P', 'S', 'R'))	ParseInfo.type = gtGPSRD;
					else if(CHECK_HEAD2(2, 'P', 'S'))		ParseInfo.type = gtGPS;
					else 									bEnd = true;
					
					ParseInfo.headParsed = true;
					SetATCmdStatus(stParsing);
				}break;
				default:
				{
					ParseInfo.head[ParseInfo.headIndex++] = c;
					if(ParseInfo.headIndex == 4)
					{
						if(CHECK_HEAD3(0, 'C', 'O', 'N'))		//CONNECT OK
						{
							ParseInfo.type = gtConnectOK;
							ParseInfo.headParsed = true;
						}
						else if(CHECK_HEAD3(0, 'C', 'O', 'M'))	//COMMAND NO RESPONSE!
						{
							ParseInfo.type = gtCmdNoRes;
							ParseInfo.headParsed = true;
						}
						else if(IS_ERR_HEAD)
						{
							ParseInfo.type = gtError;
							ParseInfo.headParsed = true;
						}
					}
					else if(ParseInfo.headIndex >= RESPONSE_HEAD_LEN)
					{
						bEnd = true;
					}
				}break;
			}
		}
	}
	else
	{
		switch(c)
		{
			case 'C':
			case 'E':
			case '+':
			{
				ParseInfo.head[0] = c;
				ParseInfo.headIndex = 1;
				ParseInfo.start = true;
			}break;
			case 'O':
			{
				ParseInfo.start = true;
				ParseInfo.headParsed = true;
				ParseInfo.type = gtOK;
			}break;
			case '>':
			{
				ParseInfo.start = true;
				ParseInfo.headParsed = true;
				ParseInfo.type = gtReady;
			}break;
			default:	bEnd = true;	break;
		}
	}
	return bEnd;
}

void ParseGPRS(RECV_BUF *pBuf)
{
	u8 i;
	if(pBuf == NULL)
		return;
	
	if(ATCmdStatus.type > stNULL)
	{
		InitResponseInfo();
		for(i=0; i<pBuf->len; i++)
		{
			if(GPRS(pBuf->buf[i]))
				break;
		}
		UpdateInfo();
	}
}

//+CIPRCV:5,12345
void ParseDATA(RECV_BUF *pBuf)
{	
	u8 i, block, blockIndex;
	bool start;
	if(pBuf == NULL)
		return;
	
	start = false;
	block = blockIndex = 0;
	for(i=0; i<pBuf->len; i++)
	{
		if(start)
		{
			switch(pBuf->buf[i])
			{
				case ',':
				{
					block++;
					blockIndex = 0;
				}break;
				default:
				{
					switch(block) //判断当前处于哪个字段
					{
						case 0:
						{
							if(blockIndex < CLOUD_RX_BUF_LEN_LEN)
								CloudRxBuf.len[blockIndex++] = pBuf->buf[i];
						}break;
						case 1:
						{
							if(blockIndex < CLOUD_RX_BUF_LEN)
								CloudRxBuf.buf[blockIndex++] = pBuf->buf[i];
						}break;
					}
				}break;	
			}				
		}
		else if(pBuf->buf[i] == ':')
		{
			start = true;
			block = 0;
			blockIndex = 0;
		}
	}
	SET_EVENT(TFT_EVENT, EVENT_DATA_DOWN);
}