#ifndef __TASK_H__
#define __TASK_H__

#include "stm32f10x.h"
#include "TFT.h"

#define		TASK_STEP	1				//10����
#define		TASK_20ms	2
#define		TASK_50ms	5
#define		TASK_100ms	10
#define		TASK_200ms	20
#define		TASK_1S		100
#define		TASK_2S		200
#define		TASK_3S		300
#define		TASK_5S		500
#define		TASK_10S	1000

//struct TASK;

typedef enum 
{
	tsSTOP = 0, 
	tsRUN = 1,
	tsSLEEP,
}TaskState;

typedef enum 
{
	tpLOW = 0,
	tpHIGH = 1,
}TaskPRI;

typedef enum
{
	TASK_PARSE_USART=0,			// ������������
	TASK_USART1_TX_MONITOR=1,	// USART1�����жϼ�أ�100ms����pSendData����Ϊ�գ�����ֹ�ϴη��ͣ�
	TASK_PROCESS_CLOUD,			// �����ƶ�����
	TASK_SEND_CLOUD_DATA,		// �����ƶ�����
	TASK_AT_CMD_CYCLE,			// ��ѯATCmdStatus״̬
	TASK_MONITOR_SIGNAL,		// ���GPRS�ź�ǿ��
	TASK_AT_OVERTIME_CHECK,		// ATָ�ʱ�ж�
	TAST_KEY_SCAN,				// ����ɨ��
	TASK_WAIT_A7_READY,			// �ȴ�A7����
	TASK_WAIT_SIM_READY,		// �ȴ�SIM������
	TASK_WAIT_GPS_READY,		// �ȴ�GPS������
	TASK_WAIT_GPRS_READY,		// �ȴ�GPRS������
	TASK_GPRS_PWR_ON,
	TASK_GET_WEIGHT,			// ������
	TASK_TIMER,					// ��ʱ��
	TASK_TFT,					// 
	TASKS_CNT,					// �ܵĿɹ�����Ķ�ʱ������Ŀ
	TASKS_LOW_CNT=TASKS_CNT
}TASK_TYPE;

typedef struct
{
	FunctionalState Enable;		// ʹ�ܱ�־�� false-���Σ�true-ʹ��
    TaskState Run; 				// �������б�ǣ�false-�����У�true-����
    u16 Timer;					// ��ʱ��
    u16 ItvTime;				// �������м��ʱ��
    void (*TaskHook)(void);		// Ҫ���е�������
	unsigned int cnt;			// ���д���
	unsigned int stampStart;	//
	unsigned int stampEnd;		//
}TASK;							// ������

typedef int TaskID;
typedef void (*HOOK)(void);
typedef struct node
{
	TaskPRI priority;
	TaskID id;
	char *name;
    int interval;				// �������м��ʱ��
    int timer;					// ��ʱ��
    TaskState run; 				// �������б�ǣ�false-�����У�true-����
    HOOK hook;					// Ҫ���е�������
	struct node *next;
}_TASK;

extern void InitTask(TaskPRI pri);
extern void TaskManage(TaskPRI pri);
extern void TaskCycle(TaskPRI pri);
extern void SetTask(TaskPRI pri, TASK_TYPE index, FunctionalState isEnabled, u16 itvTime, TaskState run);

extern TaskID CreateTask(TaskPRI pri, int interval, TaskState state, HOOK hook);
extern bool DeleteTask(TaskPRI pri, TaskID id);
_TASK *TaskTail(TaskPRI pri);
_TASK *TaskHead(TaskPRI pri);

void TaskNULL(void);
void TaskKeyScan(void);
void TaskTFT(void);
void TaskParseUsartData(void);
void TaskProcessCloudData(void);
void TaskATCmdOvertimeCheck(void);
void TaskMonitorA7State(void);
void TaskSendCloudData(void);
void TaskMonitorTxUSART1(void);
void TaskWaitSIMReady(void);
void TaskWaitGPSReady(void);
void TaskWaitGPRSReady(void);
void TaskMonitorSignal(void);
void TaskWaitA7Ready(void);
void TaskPowerOnGPRS(void);
void TaskGetWeight(void);
void TaskTimer(void);

#endif

