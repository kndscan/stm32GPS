
#ifndef __GPRS_H__
#define __GPRS_H__

//#define _DEBUG_

/***************************************************************************************************************
��ϵͳ�õ���ATָ�
ָ��					�ɹ�							˵��
"AT"					OK								���ģ��ͨѶ�Ƿ�����
"AT+CPIN?"				+CPIN:READY						����ȷ����PIN�룬SIM������������
						+CPIN:SIM PIN					�ȴ�����PIN��
						+CPIN:SIM PUK					�ȴ�����PUK��
						+CPIN:SIM PIN2					�ȴ�����PIN2��
						+CPIN:SIM PUK2					�ȴ�����PUK2��
						OK
						ERROR
						+CME ERROR: <err>
						
"AT+CSQ"				+CSQ: 14,99"					����ֻ��ź�ǿ��
"AT+CCID"				+CCID:89860042178449850053"		����SIM��ID
"AT+CREG?"				+CREG: 1,1"						����ע�ἰ״̬��ѯ
"AT+CREG=2;CREG?"		+CREG: 2,1,"1877","0002""		GPRS��λ����λ��վ��
"AT+COPS?"				+COPS: 0,2,"46000""				�����ֵ���ʽ������Ӫ�̱�ʶ46000��ʾ��Ӫ�̱�ʶ��460��ʾ�й���00��ʾ�й��ƶ���01��ʾ�й���ͨ

"AT+GPS=1"				AT+GPS=1	OK" "+CME ERROR:58" 
"AT+GPS=0"				
"AT+GPS?"				+GPS: 0	OK"
"AT+AGPS=1"
"AT+AGPS=0"
"AT+AGPS?"				+AGPS:0	OK"

AT+IPR?					+IPR: 9600		OK
AT+IPR=9600				OK

"AT+GPSRD=0"			�ر� NEMA �� AT �����  
"AT+GPSRD=N"			NEMA ��Ϣ N ��� AT �����һ��,ʵ��ʹ�ý� N �������֣� 

AT+CGATT=1					OK		��������,ʱ��Լ5��
AT+CGDCONT=1,"IP","CMNET"	OK							ָ��PDP������  
AT+CGACT=1,1				OK							����ָ����PDP������  
AT+CIPSTART="TCP","webtcp.tongxinmao.com",10002			CONNECT OK				���ӷ�����������ip���˿ں�

AT+CIPSEND				�������ݵ�������,��CTRL+Z(hex:1A)����������  
> 12345	OK
+CIPRCV:9,AiThinker										���շ��������������� 

*/


//����GPRSģ�鷢�����ݿ���ʹ��������ַ
//blog: http://blog.csdn.net/Leytton/article/details/72724081

//����ATָ����ص�ѧϰ���Բο�����blog
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
	ctAT=1,				//ģ���Ƿ���������
	ctATE0,				//�ر��������
	ctATW,				//�洢��ǰ�Ĳ���
	ctAskCPIN,			//����ֻ�SIM���Ƿ�װ��
	ctAskCOPS,			//�����ֵ���ʽ������Ӫ�̱�ʶ
	ctAskCSQ,			//����ֻ��ź�ǿ��
	ctAskCSMINS,		//���SIM���Ƿ����
	ctAskCCID,			//����SIM��ID
	ctAskCREG,			//����ע�ἰ״̬��ѯ
	ctCREG2,			//��������ע���λ����Ϣ
	
	ctOpenGPS,			//
	ctCloseGPS,			//
	ctAskGPS,
	ctOpenAGPS,			//
	ctCloseAGPS,		//
	ctAskAGPS,
	
//	ctOpenGPSRD1,		//�� GPS���ݴ�AT���������1��һ֡
//	ctOpenGPSRD2,		//�� GPS���ݴ�AT���������2��һ֡
	ctOpenGPSRD5,		//�� GPS���ݴ�AT���������5��һ֡
	ctCloseGPSRD,		//�ر� GPS���ݴ�AT�������
	ctAskGPSRD,
	
	ctCGATT,			//��������
	ctCGDCONT,			//ָ��PDP������
	ctCGACT,			//����ָ����PDP������
	ctCIPSTART,			//���ӷ�����
	ctCIPSTATUS,
	ctCIPSendWt,		//AT+CIPSEND, wait for >
	ctCIPSendDt,		//AT+CIPSEND=x,xxxx
	ctCnt				//�ܵ�ָ������
}AT_CMD_TYPE;
typedef enum
{
	stNULL = 0,
	stWaiting = 1,		//ָ���ѷ���
	stParsing,			//���ڽ�������
	stOK,				//��Ҫ���ز�����ָ������������ݣ��޲�ָ��ɹ�
	stError,			//ָ��ز���ʧ�ܻ����
	stReady,			//����>������������������
	stSend,				//����>�����벢����������
	stOvertime,			//����֡��ʱ
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
	tcdtLocation = 1,	//�ڵ�����
	tcdtWeight,			//����
	tcdtCnt
}TX_CLOUD_DATA_TYPE;

typedef enum
{
	msSignal = 0,
	msCnt
}MONITOR_STAGE;

typedef struct
{
	AT_CMD_STATUS status;	//ָ��ִ��״̬
	char *pName;			//ָ������
}AT_CMD_STATUS_INFO;
typedef struct
{
	AT_CMD_TYPE type;	//ָ�����ͣ�CMD_TYPE֮һ
	u16 overtime;		//ָ���ֵ��ʱʱ�䣬��λ����
	u16 repeatTimes;	//��ʱ�������Ѿ��ظ����ʹ�����Ĭ��Ϊ0
	char *pCmd;			//ָ��ָ���ַ�������
	char *pName;			//ָ������
}CMD_INFO, AT_CMD_INFO;
typedef struct
{
	AT_CMD_TYPE type;		//��ǰָ�����ͣ�CMD_TYPE֮һ
	AT_CMD_STATUS status;	//��ǰָ��ִ��״̬
	u8 repeatTimes;		//�Ѿ��ظ�����
}CMD_STATUS, AT_RUN_INFO;

typedef struct
{
	RESPONSE_TYPE type;	//�ظ�֡����
	
	char head[RESPONSE_HEAD_LEN];	//�ظ�֡ͷ����
	u8 headIndex; 	//�ظ�֡ͷ��������
	
	bool start;			//��ʼ����ظ�֡ͷ����
	bool headParsed;	//�ظ�֡�������жϳ�����ʼ�жϲ���
	
	u8 block;			//����֡������
	u8 blockIndex;		//����֡�����Ż�������
}RESPONSE_PARSE_INFO;

typedef struct
{
	char len[CLOUD_RX_BUF_LEN_LEN];
	char buf[CLOUD_RX_BUF_LEN];
	RecvBufState state;
}CLOUD_RX_BUF;
/*********************************************
GPRS���ݽ���������ݽṹ
**********************************************/
typedef struct
{
	bool bFirstOK;			//�ϵ�Ϊtrue,�յ�OK���Ϊfalse
	bool bA7Ready;			//CPIN=READY
	bool bSimReady;			//SIMCmdsָ����ִ����ȷ
	bool bGPSReady;			//GPSCmdsָ����ִ����ȷ
	bool bAGPSReady;		//AGPS
	bool bGPRSReady;		//GPRSCmdsָ����ִ����ȷ true-���ӵ�������
	
	char cGPS;				//AT+GPS=?����ֵ
	char cAGPS;				//AT+AGPS=?����ֵ
	char cGPSRD;			//AT+GPSRD=?����ֵ
	
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

