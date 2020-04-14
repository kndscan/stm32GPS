
#ifndef __GPRS_H__
#define __GPRS_H__

//#define _DEBUG_

/***************************************************************************************************************
本系统用到的AT指令：
指令					成功							说明
"AT"					OK								检查模块通讯是否正常
"AT+CPIN?"				+CPIN:READY						已正确输入PIN码，SIM卡可正常工作
						+CPIN:SIM PIN					等待输入PIN码
						+CPIN:SIM PUK					等待输入PUK码
						+CPIN:SIM PIN2					等待输入PIN2码
						+CPIN:SIM PUK2					等待输入PUK2码
						OK
						ERROR
						+CME ERROR: <err>
						
"AT+CSQ"				+CSQ: 14,99"					检测手机信号强度
"AT+CCID"				+CCID:89860042178449850053"		测试SIM卡ID
"AT+CREG?"				+CREG: 1,1"						网络注册及状态查询
"AT+CREG=2;CREG?"		+CREG: 2,1,"1877","0002""		GPRS定位（定位基站）
"AT+COPS?"				+COPS: 0,2,"46000""				以数字的形式返回运营商标识46000表示运营商标识，460表示中国，00表示中国移动，01表示中国联通

"AT+GPS=1"				AT+GPS=1	OK" "+CME ERROR:58" 
"AT+GPS=0"				
"AT+GPS?"				+GPS: 0	OK"
"AT+AGPS=1"
"AT+AGPS=0"
"AT+AGPS?"				+AGPS:0	OK"

AT+IPR?					+IPR: 9600		OK
AT+IPR=9600				OK

"AT+GPSRD=0"			关闭 NEMA 从 AT 口输出  
"AT+GPSRD=N"			NEMA 信息 N 秒从 AT 口输出一次,实际使用将 N 换成数字； 

AT+CGATT=1					OK		附着网络,时间约5秒
AT+CGDCONT=1,"IP","CMNET"	OK							指定PDP上下文  
AT+CGACT=1,1				OK							激活指定的PDP上下文  
AT+CIPSTART="TCP","webtcp.tongxinmao.com",10002			CONNECT OK				连接服务器域名或ip、端口号

AT+CIPSEND				发送数据到服务器,以CTRL+Z(hex:1A)结束符发送  
> 12345	OK
+CIPRCV:9,AiThinker										接收服务器发来的数据 

*/


//调试GPRS模块发送数据可以使用下面网址
//blog: http://blog.csdn.net/Leytton/article/details/72724081

//关于AT指令相关的学习可以参考下面blog
//http://blog.csdn.net/laozhuxinlu/article/details/52085150

#include "stm32f10x.h"
#include "USART.h"
//#define ServerName	"webtcp.tongxinmao.com"
//#define Protocal		"TCP"
//#define Port			10002

#define ARRAY_CNT(x)			(sizeof(x)/sizeof(x[0]))

#define A7PWR					GPIO_Pin_8

#define AT_USART				USART1
#define CLOUD_RX_BUF_LEN_LEN	3
#define CLOUD_RX_BUF_LEN		100
#define CLOUD_TX_BUF_LEN		100
#define INIT_CMD_CNT			20
#define RESPONSE_HEAD_LEN		20

#define IS_SIMPLE_AT(x)			( (x)==ctAT				\
							    ||(x)==ctATE0			\
							    ||(x)==ctATW			\
							    ||(x)==ctCREG2			\
							    ||(x)==ctOpenGPS		\
							    ||(x)==ctCloseGPS		\
							    ||(x)==ctOpenAGPS		\
							    ||(x)==ctOpenAGPS		\
							    ||(x)==ctOpenGPSRD5		\
							    ||(x)==ctCloseAGPS		\
							    ||(x)==ctCloseAGPS		\
							    ||(x)==ctCloseGPSRD		\
							    ||(x)==ctCGATT			\
							    ||(x)==ctCGDCONT		\
							    ||(x)==ctCGACT			\
								||(x)==ctCIPSendDt		)

#define CAN_SEND_AT(x)			((x)==stNULL||(x)==stOvertime||(x)==stError)

#define CHECK_HEAD1(i,a)		(ParseInfo.head[(i)]==(a))
#define CHECK_HEAD2(i,a,b)		(ParseInfo.head[(i)]==(a)&&ParseInfo.head[(i)+1]==(b))
#define CHECK_HEAD3(i,a,b,c)	(ParseInfo.head[(i)]==(a)&&ParseInfo.head[(i)+1]==(b)&&ParseInfo.head[(i)+2]==(c))
#define CHECK_HEAD4(i,a,b,c,d)	(ParseInfo.head[(i)]==(a)&&ParseInfo.head[(i)+1]==(b)&&ParseInfo.head[(i)+2]==(c)&&ParseInfo.head[(i)+3]==(d))
#define IS_ERR_HEAD				(CHECK_HEAD4(0,'+','C','M','E')||(CHECK_HEAD4(0,'+','C','M','S'))||(CHECK_HEAD4(0,'E','R','R','O')))

typedef enum
{
	gtNULL=0,
	gtOK=1,			//OK
	gtError,		//ERROR
	gtReady,		//>
	gtCloudData,	//+CIPRCV:9,AiThinker
	gtCSQ,			//+CSQ: 14,99					OK
	gtCPIN,			//+CPIN:READY					OK
	gtCCID,			//+CCID:89860042178449850053	OK
	gtCREG,			//+CREG: 2,1,"1877","0002"		OK
	gtCOPS,			//+COPS: 0,2,"46000"			OK
	gtCME,			//+CME ERROR:58
	gtGPS,			//+GPS: 0						OK
	gtGPSRD,		//+GPSRD: 5						OK
	gtAGPS,			//+AGPS:0						OK
	gtCIPStatus,	//+CIPSTATUS:0,CONNECT OK		OK
					//+CIPSTATUS:0,IP CLOSE			OK
					//+CIPSTATUS:0,IP INITIAL		OK
	gtConnectOK,	//CONNECT OK					OK
	gtCmdNoRes,		//COMMAND ON RESPONSE!
	gtCnt,
}RESPONSE_TYPE;
typedef enum
{
	ctNULL=0,
	ctAT=1,				//模块是否正常工作
	ctATE0,				//关闭命令回显
	ctATW,				//存储当前的参数
	ctAskCPIN,			//检测手机SIM卡是否安装好
	ctAskCOPS,			//以数字的形式返回运营商标识
	ctAskCSQ,			//检测手机信号强度
	ctAskCSMINS,		//检测SIM卡是否插入
	ctAskCCID,			//测试SIM卡ID
	ctAskCREG,			//网络注册及状态查询
	ctCREG2,			//启用网络注册和位置信息
	
	ctOpenGPS,			//
	ctCloseGPS,			//
	ctAskGPS,
	ctOpenAGPS,			//
	ctCloseAGPS,		//
	ctAskAGPS,
	
//	ctOpenGPSRD1,		//打开 GPS数据从AT串口输出，1秒一帧
//	ctOpenGPSRD2,		//打开 GPS数据从AT串口输出，2秒一帧
	ctOpenGPSRD5,		//打开 GPS数据从AT串口输出，5秒一帧
	ctCloseGPSRD,		//关闭 GPS数据从AT串口输出
	ctAskGPSRD,
	
	ctCGATT,			//附着网络
	ctCGDCONT,			//指定PDP上下文
	ctCGACT,			//激活指定的PDP上下文
	ctCIPSTART,			//连接服务器
	ctCIPSTATUS,
	ctCIPSendWt,		//AT+CIPSEND, wait for >
	ctCIPSendDt,		//AT+CIPSEND=x,xxxx
	ctCnt				//总的指令数量
}AT_CMD_TYPE;
typedef enum
{
	stNULL = 0,
	stWaiting = 1,		//指令已发出
	stParsing,			//正在解析参数
	stOK,				//需要返回参数的指令继续处理数据，无参指令成功
	stError,			//指令返回操作失败或错误
	stReady,			//返回>，可以输入数据内容
	stSend,				//返回>后，输入并发送了数据
	stOvertime,			//返回帧超时
	stCnt
}STATUS_TYPE, AT_CMD_STATUS;
typedef enum
{
	gwsNULL = 0,
	gwsUp = 1,
	gwsDown = 2,
	gwsUpDown
}GPRS_WORK_STATE;

typedef enum
{
	tcdtNULL = 0,
	tcdtLocation = 1,	//节点坐标
	tcdtWeight,			//重量
	tcdtCnt
}TX_CLOUD_DATA_TYPE;

typedef enum
{
	msSignal = 0,
	msCnt
}MONITOR_STAGE;

typedef struct
{
	AT_CMD_STATUS status;	//指令执行状态
	char *pName;			//指令名称
}AT_CMD_STATUS_INFO;
typedef struct
{
	AT_CMD_TYPE type;	//指令类型，CMD_TYPE之一
	u16 overtime;		//指令返回值超时时间，单位：秒
	u16 repeatTimes;	//超时或错误后已经重复发送次数，默认为0
	char *pCmd;			//指向指令字符串内容
	char *pName;			//指令名称
}CMD_INFO, AT_CMD_INFO;
typedef struct
{
	AT_CMD_TYPE type;		//当前指令类型，CMD_TYPE之一
	AT_CMD_STATUS status;	//当前指令执行状态
	u8 repeatTimes;		//已经重复次数
}CMD_STATUS, AT_RUN_INFO;

typedef struct
{
	RESPONSE_TYPE type;	//回复帧类型
	
	char head[RESPONSE_HEAD_LEN];	//回复帧头缓冲
	u8 headIndex; 	//回复帧头缓冲索引
	
	bool start;			//开始缓冲回复帧头内容
	bool headParsed;	//回复帧类型已判断出，开始判断参数
	
	u8 block;			//带参帧参数号
	u8 blockIndex;		//带参帧参数号缓冲索引
}RESPONSE_PARSE_INFO;

typedef struct
{
	char len[CLOUD_RX_BUF_LEN_LEN];
	char buf[CLOUD_RX_BUF_LEN];
	RecvBufState state;
}CLOUD_RX_BUF;
/*********************************************
GPRS数据接收相关数据结构
**********************************************/
typedef struct
{
	bool bFirstOK;			//上电为true,收到OK后变为false
	bool bA7Ready;			//CPIN=READY
	bool bSimReady;			//SIMCmds指令组执行正确
	bool bGPSReady;			//GPSCmds指令组执行正确
	bool bAGPSReady;		//AGPS
	bool bGPRSReady;		//GPRSCmds指令组执行正确 true-连接到服务器
	
	char cGPS;				//AT+GPS=?返回值
	char cAGPS;				//AT+AGPS=?返回值
	char cGPSRD;			//AT+GPSRD=?返回值
	
	char aSimID[20];
	char aCPIN[8];
	
	char aSignal[2];
	char aErRate[2];
	
	char netRegMode;		//CREG,block1
	char netRegState;		//CREG,block2
	char locationID[10];
	char blockID[10];
	
	char copsMode;
	char copsStyle;
	char copsOpe[7];
	
	char cIPChannel;
	char aIPStatus[20];
	
	AT_RUN_INFO* pAT;
	CLOUD_RX_BUF* pData;
}GPRS_INFO;

typedef struct
{
	u8 count;
	u8 index;
	const AT_CMD_TYPE cmds[INIT_CMD_CNT];
}CMD_GROUP;

//typedef struct
//{
//	bool bSignalChanged;
//	bool bRegChanged;
//	bool bCopsChanged;
//	bool bSimIDChanged;
//	bool bSimReadyChanged;
//	bool bCloudData;
//	
//	bool bGPRSStateChanged;
//	GPRS_WORK_STATE gprsState;
//	
//	bool bATCmdStatusChanged;
//	char* pCurATCmdName;
//	AT_CMD_STATUS curATCmdStatus;
//	
//	GPRS_INFO *pInfo;
//	CLOUD_RX_BUF *pData;
//}GPRS_STATE;



extern CLOUD_RX_BUF CloudRxBuf;
extern TX_CLOUD_DATA_TYPE SendCloudDataType;

extern void PowerOnGPRS(void);
void InitGPRSState(void);

extern CMD_GROUP SIMCmds;
extern CMD_GROUP GPSCmds;
extern CMD_GROUP GPRSCmds;

extern void SetFirstOK(bool first);
extern bool GetFirstOK(void);

bool IsSIMReady(void);

extern GPRS_INFO *GetGPRSInfo(void);
extern void InitGPRSInfo(void);

extern void InitATCmdStatus(void);
extern void SetATCmdStatus(STATUS_TYPE status);
extern AT_CMD_STATUS GetATCmdStatus(void);
extern AT_CMD_TYPE GetATCmdType(void);
extern char* GetATCmdStatusName(AT_CMD_STATUS status);
extern char* GetATCmdTypeName(AT_CMD_TYPE type);
extern void SetATCmdType(AT_CMD_TYPE type);
extern void SendATCmd(AT_CMD_TYPE type);
char SendATCmds(CMD_GROUP *group);

void InitSIM(void);
void InitGPS(void);
void InitGPRS(void);
extern void WaitA7Ready(void);
extern void WaitSIMReady(void);
extern void WaitGPSReady(void);
extern void WaitGPRSReady(void);

extern void ATCmdRepeat(void);
extern void SendCloudData(void);
extern void ProcessCloudData(void);

extern void ParseGPRS(RECV_BUF *pBuf);
extern void ParseDATA(RECV_BUF *pBuf);

#endif

