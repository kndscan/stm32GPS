#ifndef __TASK_H__
#define __TASK_H__

#include "stm32f10x.h"
#include "TFT.h"

#define		TASK_STEP	1				//10毫秒
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
	TASK_PARSE_USART=0,			// 解析串口数据
	TASK_USART1_TX_MONITOR=1,	// USART1发送中断监控（100ms内若pSendData还不为空，则终止上次发送）
	TASK_PROCESS_CLOUD,			// 处理云端数据
	TASK_SEND_CLOUD_DATA,		// 发送云端数据
	TASK_AT_CMD_CYCLE,			// 轮询ATCmdStatus状态
	TASK_MONITOR_SIGNAL,		// 监控GPRS信号强度
	TASK_AT_OVERTIME_CHECK,		// AT指令超时判断
	TAST_KEY_SCAN,				// 按键扫描
	TASK_WAIT_A7_READY,			// 等待A7就绪
	TASK_WAIT_SIM_READY,		// 等待SIM卡就绪
	TASK_WAIT_GPS_READY,		// 等待GPS卡就绪
	TASK_WAIT_GPRS_READY,		// 等待GPRS卡就绪
	TASK_GPRS_PWR_ON,
	TASK_GET_WEIGHT,			// 读重量
	TASK_TIMER,					// 定时器
	TASK_TFT,					// 
	TASKS_CNT,					// 总的可供分配的定时任务数目
	TASKS_LOW_CNT=TASKS_CNT
}TASK_TYPE;

typedef struct
{
	FunctionalState Enable;		// 使能标志： false-屏蔽，true-使能
    TaskState Run; 				// 程序运行标记：false-不运行，true-运行
    u16 Timer;					// 计时器
    u16 ItvTime;				// 任务运行间隔时间
    void (*TaskHook)(void);		// 要运行的任务函数
	unsigned int cnt;			// 运行次数
	unsigned int stampStart;	//
	unsigned int stampEnd;		//
}TASK;							// 任务定义

typedef int TaskID;
typedef void (*HOOK)(void);
typedef struct node
{
	TaskPRI priority;
	TaskID id;
	char *name;
    int interval;				// 任务运行间隔时间
    int timer;					// 计时器
    TaskState run; 				// 程序运行标记：false-不运行，true-运行
    HOOK hook;					// 要运行的任务函数
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

