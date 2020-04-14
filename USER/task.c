
#include "task.h"
#include "TM1638.h"
#include "usart.h"
#include "GPRS.h"
#include "GPS.h"
#include "timer.h"
#include "stdlib.h"
#include "HX711.h"

static _TASK *TaskListHigh;
static _TASK *TaskList;
static _TASK *TaskListLow;
static TASK Tasks[] = 
{
	{DISABLE,	tsSTOP, 0,	TASK_STEP, 		TaskParseUsartData},		// 帧解析
	{DISABLE,   tsSTOP, 0,	TASK_100ms,   	TaskMonitorTxUSART1},		//
	{DISABLE,   tsSTOP, 0,	TASK_100ms,   	TaskProcessCloudData},		// 处理云端数据
	{DISABLE,	tsSTOP, 0,	TASK_5S, 		TaskSendCloudData},			// 若stReady，则把缓冲区数据发出
	{DISABLE,	tsSTOP, 0,	TASK_1S, 		TaskMonitorA7State},		// 周期性轮询GPRS状态
	{DISABLE,   tsSTOP, 0,	TASK_10S,   	TaskMonitorSignal},			// 监控GPRS信号强度
	{DISABLE,	tsSTOP, 0,	TASK_10S, 		TaskATCmdOvertimeCheck},	// AT指令超时判断，默认1秒
    {DISABLE,	tsSTOP, 0,	TASK_20ms, 		TaskKeyScan},				// 按键扫描
	{DISABLE,   tsSTOP, 0,	TASK_10S,   	TaskWaitA7Ready},			// 监控SIM卡是否READY
	{DISABLE,   tsSTOP, 0,	TASK_2S,   		TaskWaitSIMReady},			// 
	{DISABLE,   tsSTOP, 0,	TASK_2S,   		TaskWaitGPSReady},			// 
	{DISABLE,   tsSTOP, 0,	TASK_2S,   		TaskWaitGPRSReady},			// 
	{DISABLE,	tsSTOP, 0,	TASK_3S, 		TaskPowerOnGPRS},			// A7 PWR管脚置位
	{DISABLE,	tsSTOP, 0,	TASK_2S, 		TaskGetWeight},				// 读取压力传感器
	{DISABLE,	tsSTOP, 0,	TASK_1S, 		TaskTimer},
	{DISABLE,	tsSTOP, 0,	TASK_200ms, 	TaskTFT},
};
static TASK TasksLow[] = 
{
	{DISABLE,	tsSTOP, 0,	TASK_3S, 		TaskPowerOnGPRS},
};
/**************************************************************
任务调度
***************************************************************/
TaskID CreateTask(TaskPRI pri, int interval, TaskState state, HOOK hook)
{
	TaskID id = -1;
	_TASK *pTemp;
	_TASK *p =(_TASK*)malloc(sizeof(_TASK));
	if(p)
	{
		p->interval = interval;
		p->timer = 0;
		p->run = state;
		p->hook = hook;
		p->priority = pri;
		
		pTemp = TaskTail(pri);
		if(pTemp)
		{
			p->next = NULL;
			p->id = pTemp->id + 1;
			pTemp->next = p;
			id = p->id;
		}
		else
		{
			free(p);
			p = NULL;
		}
	}
	return id;
}
bool DeleteTask(TaskPRI pri, TaskID id)
{
	_TASK *pPrev;
	_TASK *p = TaskHead(pri);
	if(NULL == p)
		return false;
	while(p)
	{
		if(p->id == id)
		{
			pPrev->next = p->next;
			free(p);
			return true;
		}
		pPrev = p;
		p = p->next;
	}
	return false;
}
_TASK *TaskTail(TaskPRI pri)
{
	_TASK *p = TaskHead(pri);
	if(NULL == p)
		return NULL;
	while(p->next)
	{
		p = p->next;
	}
	return p;
}
_TASK *TaskHead(TaskPRI pri)
{
	if(tpHIGH == pri)
		return TaskListHigh;
	else if(tpLOW == pri)
		return TaskListLow;
	else
		return NULL;
}
_TASK *GetTask(TaskPRI pri, int id)
{
	return NULL;
}
int TaskCount(void)
{
	_TASK *p = TaskList;
	int count = 0;
	while(p)
	{
		count++;
		p = p->next;
	}
	return count;
}

void InitTask(TaskPRI pri)
{
	u8 i;
	if(pri == tpHIGH)
	{
		for (i=0; i<TASKS_CNT; i++)
		{
			Tasks[i].Enable = DISABLE;
			Tasks[i].Run = tsSTOP;
			Tasks[i].Timer = Tasks[i].ItvTime;
			Tasks[i].cnt = 0;
			Tasks[i].stampStart = 0;
			Tasks[i].stampEnd = 0;
		}
		TIMx_Init(TIM3, 99, 7199);	//10ms定时器初始化
	}
	else
	{
		for (i=0; i<TASKS_LOW_CNT; i++)
		{
			TasksLow[i].Enable = DISABLE;
			TasksLow[i].Run = tsSTOP;
			TasksLow[i].Timer = Tasks[i].ItvTime;
			TasksLow[i].cnt = 0;
			TasksLow[i].stampStart = 0;
			TasksLow[i].stampEnd = 0;
		}
		TIMx_Init(TIM4, 99, 7199);	//10ms定时器初始化
	}
}
void TaskManage(TaskPRI pri)
{
	u8 i;
	if(pri == tpHIGH)
	{
		for (i=0; i<TASKS_CNT; i++)
		{
			if (Tasks[i].Enable && --Tasks[i].Timer <= 0)
			{
				Tasks[i].Timer = Tasks[i].ItvTime;
				Tasks[i].Run = tsRUN;
			}
		}		
	}
	else
	{
		for (i=0; i<TASKS_LOW_CNT; i++)
		{
			if (TasksLow[i].Enable && --TasksLow[i].Timer <= 0)
			{
				TasksLow[i].Timer = TasksLow[i].ItvTime;
				TasksLow[i].Run = tsRUN;
			}
		}
	}
}

void TaskCycle(TaskPRI pri)
{
	u8 i;
	if(pri == tpHIGH)
	{
		for (i=0; i<TASKS_CNT; i++)
		{			if (Tasks[i].Enable && Tasks[i].Run)
			{
				Tasks[i].TaskHook();
				Tasks[i].cnt++;
				Tasks[i].Run = tsSTOP;
			}
		}   
	}
	else
	{
		for (i=0; i<TASKS_LOW_CNT; i++)
		{
			if (TasksLow[i].Enable && TasksLow[i].Run)
			{
				TasksLow[i].TaskHook();
				TasksLow[i].cnt++;
				TasksLow[i].Run = tsSTOP;
			}
		}
	}		
}

/**************************************************************
任务回调函数
***************************************************************/
void SetTask(TaskPRI pri, TASK_TYPE index, FunctionalState isEnabled, u16 itvTime, TaskState run)
{
	if(pri == tpHIGH)
	{
		if(index < TASKS_CNT)
		{
			if(isEnabled)
			{
				Tasks[index].ItvTime = itvTime;
				Tasks[index].Timer = itvTime;
				Tasks[index].Run = run;
				Tasks[index].Enable = isEnabled;
			}
			else
			{
				Tasks[index].Enable = isEnabled;
				Tasks[index].ItvTime = itvTime;
				Tasks[index].Timer = itvTime;
				Tasks[index].Run = run;
			}
		}
	}
	else
	{
		if(index < TASKS_LOW_CNT)
		{
			if(isEnabled)
			{
				TasksLow[index].ItvTime = itvTime;
				TasksLow[index].Timer = itvTime;
				TasksLow[index].Run = run;
				TasksLow[index].Enable = isEnabled;
			}
			else
			{
				TasksLow[index].Enable = isEnabled;
				TasksLow[index].ItvTime = itvTime;
				TasksLow[index].Timer = itvTime;
				TasksLow[index].Run = run;
			}
		}
	}
}

void TaskNULL(void)
{
	
}
void TaskKeyScan(void)
{
	KeyScan();
}
void TaskTFT(void)
{
	UpdateTFT(false);
	UpdateLED(false);
}
void TaskParseUsartData(void)
{
	ParseUSARTData();
}
void TaskProcessCloudData(void)
{
	ProcessCloudData();
}
void TaskATCmdOvertimeCheck(void)
{
	SetTask(tpHIGH, TASK_AT_OVERTIME_CHECK, DISABLE, 0, tsSTOP);
	SetATCmdStatus(stOvertime);
}
void TaskMonitorA7State(void)
{
	ATCmdRepeat();
}
void TaskSendCloudData(void)
{
	if(CAN_SEND_AT(GetATCmdStatus()))
		SendCloudData();
}
void TaskMonitorTxUSART1(void)
{
	ResetSendPointer(USART1);
	SetTask(tpHIGH, TASK_USART1_TX_MONITOR, DISABLE, 0, tsSTOP);
}

void TaskWaitA7Ready(void)
{
	WaitA7Ready();
}

void TaskWaitSIMReady(void)
{
	WaitSIMReady();
}

void TaskWaitGPSReady(void)
{
	WaitGPSReady();
}

void TaskWaitGPRSReady(void)
{
	WaitGPRSReady();
}

void TaskMonitorSignal(void)
{
	if(CAN_SEND_AT(GetATCmdStatus()))
	{
		SendATCmd(ctAskCSQ);
	}
}

void TaskPowerOnGPRS(void)
{
	GPIO_ResetBits(GPIOB, A7PWR);
	SetTask(tpHIGH, TASK_GPRS_PWR_ON, DISABLE, TASK_5S, tsSTOP);
	
	return;
	
	ENABLE_UART(AT_USART);
	InitGPRSInfo();
	SetTask(tpHIGH, TASK_PARSE_USART, ENABLE, TASK_STEP, tsSTOP);
	SetTask(tpHIGH, TASK_AT_CMD_CYCLE, ENABLE, TASK_2S, tsSTOP);
	
	SendATCmd(ctAT);	//in case of A7 is power-on already
}

void TaskGetWeight(void)
{
	Get_Weight();
}

int gCounter = 0;
void TaskTimer(void)
{
	SetTask(tpHIGH, TASK_TIMER, DISABLE, TASK_10S, tsSTOP);
	SetTask(tpHIGH, TASK_WAIT_A7_READY, ENABLE, TASK_3S, tsSTOP);
//	LampsFlow();
//	char buf[10];
//	toString(gCounter++, buf);
//	WriteLEDs(0, buf);
//	SendString(USART2, "123456\r\n");
}
