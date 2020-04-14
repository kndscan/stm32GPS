#include "sys.h"
#include "usart.h"
#include "delay.h"
#include "GPS.h"
#include "GPRS.h"
#include "task.h"
#include "common.h"
#include <string.h>

/*--------------------------------------------------------
USART1使用中断发送字符串，pSendData是指向被发送字符串的指针
--------------------------------------------------------*/
static const char *pSendData = NULL;
void ResetSendPointer(USART_TypeDef* USARTx)
{
	pSendData = NULL;
	USART_ClearFlag(USARTx, USART_FLAG_TC);
}

/*--------------------------------------------------------
USART1接收GPS帧和AT指令回复帧，下述结构管理接收缓冲，每个缓冲
以0x0A|0x0D|0x1A作为结束符。
--------------------------------------------------------*/
RECV_BUF *pRecvBuf = NULL;
RECV_BUF RecvBuf[REC_BUF_CNT];
void InitRecvBuf(u8 index)
{
	if(index < REC_BUF_CNT)
	{
		//RecvBuf[index].buf[RecvBuf[index].len] = 0;
		RecvBuf[index].len = 0;
		RecvBuf[index].state = rbsReady;	//最后设置Ready
		
		SET_EVENT(TFT_EVENT, EVENT_USART_DATA);
	}
}
void InitRecvBufEx(RECV_BUF *pBuf)
{
	if(pBuf != NULL)
	{
		//pBuf->buf[pBuf->len] = 0;
		pBuf->len = 0;
		pBuf->state = rbsReady;		//最后设置Ready
		
		SET_EVENT(TFT_EVENT, EVENT_USART_DATA);
	}
}
void InitRecvBufs(void)
{
	u8 i;
	for(i=0; i<REC_BUF_CNT; i++)
	{
		InitRecvBuf(i);
	}
}
bool RecvBufFilter(RECV_BUF *pBuf)
{
	if(	pBuf->buf[0]=='$' || 	//$GPRMC,$GPVTG返回值
		pBuf->buf[1]==',' )		//AT+CIPSTATUS?返回值为若干"x,IP INITIAL"字符串
	{
		InitRecvBufEx(pBuf);
		return true;
	}
	else
		return false;
}	

/**********************************************************
获得接收缓冲区指针
***********************************************************/
RECV_BUF *GetRecvBufByIndex(unsigned char i)
{
	if(i<REC_BUF_CNT)
		return &RecvBuf[i];
	else
		return NULL;
}
RECV_BUF *GetRecvBuf(void)
{
	u8 i;
	for(i=0; i<REC_BUF_CNT; i++)
	{
		if(RecvBuf[i].state == rbsReady)
			return &RecvBuf[i];
	}
	return NULL;
}

RecvBufType GetRecvBufType(RECV_BUF *pBuf)
{
	RecvBufType type = rbtUnexpect;
	if(NULL == pBuf || pBuf->len <= 0)
		return type;	
	
	switch(pBuf->buf[0])
	{
		case 'O':
		{	
			if(IS_SIMPLE_AT(GetATCmdType()))
				type = rbtGPRS;
			else
				type = rbtSkip;
		}break;
		case '+':
		{
			if(pBuf->buf[7] == '$')
				type = rbtGPS;
			else if(pBuf->buf[2]=='I' && pBuf->buf[3]=='P' && pBuf->buf[4]=='R')
				type = rbtDATA;
			else
				type = rbtGPRS;
		}break;
		
		case '^':	type = rbtInit;		break;
		case '>':
		case 'E':
		case 'C':
		default:	type = rbtGPRS;		break;
	}
	return type;
}

void ParseUSARTData(void)
{
	u8 i, indexOK = REC_BUF_CNT;
	for(i=0; i<REC_BUF_CNT; i++)
	{
		if(RecvBuf[i].state != rbsFinished)
			continue;
		
		switch(GetRecvBufType(&RecvBuf[i]))
		{
			case rbtGPS:	ParseGPS(&RecvBuf[i]);		break;
			case rbtGPRS:	ParseGPRS(&RecvBuf[i]);		break;
			case rbtDATA:	ParseDATA(&RecvBuf[i]);		break;
			case rbtInit:	SetTask(tpHIGH, TASK_TIMER, ENABLE, TASK_10S, tsSTOP);	break;
			case rbtSkip:{
				indexOK = i;
				continue;
			}break;
		}
		
		if(GetFirstOK())
		{
			SetTask(tpHIGH, TASK_TIMER, ENABLE, TASK_10S, tsSTOP);
			SetFirstOK(false);
		}	
		InitRecvBuf(i);
	}
	
	if(indexOK < REC_BUF_CNT)
	{
		ParseGPRS(&RecvBuf[indexOK]);
		InitRecvBuf(indexOK);
	}
}

bool HeadCheck(const char *src, const char *sub)
{
	u8 srcLen, subLen, i;
	
	srcLen = strlen(src);
	subLen = strlen(sub);
	if(srcLen > 0 && srcLen >= subLen)
	{
		for(i=0; i<subLen; i++)
		{
			if(src[i] != sub[i])
				return false;
		}
		return true;
	}
	else
		return false;
}

/*************************************************************
由指定串口发送字符串
USARTx: 指定串口，可使用USART1,USART2,USART3
*p: 	字符串指针
**************************************************************/
void SendString(USART_TypeDef* USARTx, const char *p)
{
	if(NULL != pSendData)
	{
		SetTask(tpHIGH, TASK_USART1_TX_MONITOR, ENABLE, TASK_100ms, tsSTOP);
		while(NULL != pSendData);
	}
	
	pSendData = p;
	USART_ClearFlag(USARTx, USART_FLAG_TC);
	//USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	USART_SendData(USARTx, *pSendData++);
}

/*************************************************************
初始化串口
USARTx: 指定串口，暂支持USART1和USART2
baudRate: 波特率设置
**************************************************************/
void ENABLE_UART(USART_TypeDef* USARTx)
{
	USART_Cmd(USARTx, ENABLE);
}
void DIABLE_UART(USART_TypeDef* USARTx)
{
	USART_Cmd(USARTx, DISABLE);
}
void UART_INIT(USART_TypeDef* USARTx, u32 baudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
   	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
		
	if(USARTx == USART1)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
		
		//USART1_TX
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; 				//PA.9
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		//USART1_RX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				//PA.10
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
		GPIO_Init(GPIOA, &GPIO_InitStructure);  
		
		//USART 初始化设置
		USART_InitStructure.USART_BaudRate = baudRate;			//一般设置为9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;	//一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;		//无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		//收发模式
		USART_Init(USART1, &USART_InitStructure); 				//初始化串口

		//Usart1 NVIC 配置
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器
		
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);			//开启中断
		USART_ITConfig(USART1, USART_IT_TC, ENABLE); 
		delay_ms(10);
		USART_ClearFlag(USART1, USART_FLAG_TC | USART_FLAG_TXE | USART_IT_RXNE);
		//USART_Cmd(USART1, ENABLE);                    			//使能串口 
	}
	else if(USARTx == USART2)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART2
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能USART2
		
		//USART2_TX
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				//PA.2
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		//USART2_RX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				//PA.3
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		//USART 初始化设置
		USART_InitStructure.USART_BaudRate = baudRate;			//一般设置为9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;	//一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;		//无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		//收发模式
		USART_Init(USART2, &USART_InitStructure); 				//初始化串口

		//Usart2 NVIC 配置
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器
		
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);			//开启中断
		USART_ITConfig(USART2, USART_IT_TC, ENABLE); 
		delay_ms(10);
		USART_ClearFlag(USART2, USART_FLAG_TC | USART_FLAG_TXE | USART_IT_RXNE);
		//USART_Cmd(USART2, ENABLE);                    			//使能串口 
	}
	else if(USARTx == USART3)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能UART3所在GPIOB的时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
		
		//USART3_TX
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;				//PB.10
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			//复用推挽输出
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		//USART3_RX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				//PB.11
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		//USART 初始化设置
		USART_InitStructure.USART_BaudRate = baudRate;			//一般设置为9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;	//一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;		//无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		//收发模式
		USART_Init(USART3, &USART_InitStructure); 				//初始化串口

		//Usart3 NVIC 配置
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器
		
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);			//开启中断
		USART_ITConfig(USART3, USART_IT_TC, ENABLE); 
		delay_ms(10);
		USART_ClearFlag(USART3, USART_FLAG_TC | USART_FLAG_TXE | USART_IT_RXNE);
		//USART_Cmd(USART3, ENABLE);                    			//使能串口 
	}
	else
	{
		return;
	}
}

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 res;
	if(USART_GetITStatus(USART1, USART_IT_TC) == SET)
	{
		if(NULL != pSendData)
		{
			if(*pSendData == '\0')
			{
				USART_ClearFlag(USART1, USART_FLAG_TC);
				pSendData = NULL;
			}
			else
				USART_SendData(USART1, *pSendData++ );
		}
		else
		{
			USART_ClearFlag(USART1, USART_FLAG_TC);
		}
    }
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		res = USART_ReceiveData(USART1);		//读取接收到的数据(USART1->DR)
		
		if(pRecvBuf == NULL)
			pRecvBuf = GetRecvBuf();
		
		if(pRecvBuf != NULL)
		{
			switch(pRecvBuf->state)
			{
				case rbsReady:
				{
					if(res!=0x0D && res!=0x0A /*&& res!=0x1A*/)
					{
						pRecvBuf->buf[pRecvBuf->len++] = res;
						pRecvBuf->state = rbsReceiving;
					}
				}break;
				case rbsReceiving:
				{
					if(res==0x0D || res==0x0A /*|| res==0x1A*/)
					{
						if(pRecvBuf->buf[0] == 'O')
							pRecvBuf->state = rbsReceiving;

						if(!RecvBufFilter(pRecvBuf))	//放弃不关心的返回帧
						{
							pRecvBuf->state = rbsFinished;
						}
						pRecvBuf = NULL;
					}
					else
						pRecvBuf->buf[pRecvBuf->len++] = res;
				}break;
				default: break;
			}
			
			if(pRecvBuf->len >= USART_REC_LEN)
			{
				if(!RecvBufFilter(pRecvBuf))	//放弃不关心的返回帧
				{
					pRecvBuf->state = rbsFinished;
				}
				pRecvBuf = NULL;
			}
		}
	}
}

void USART2_IRQHandler(void)                	//串口2中断服务程序
{
	u8 res;
	if(USART_GetITStatus(USART2, USART_IT_TC) == SET)
	{
		if(NULL != pSendData)
		{
			if(*pSendData == '\0')
			{
				USART_ClearFlag(USART2, USART_FLAG_TC);
				pSendData = NULL;
			}
			else
				USART_SendData(USART2, *pSendData++ );
		}
		else
		{
			USART_ClearFlag(USART2, USART_FLAG_TC);
		}
    }
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		res = USART_ReceiveData(USART2);		//读取接收到的数据(USART1->DR)
		if(pRecvBuf == NULL)
			pRecvBuf = GetRecvBuf();
		
		if(pRecvBuf != NULL)
		{
			switch(pRecvBuf->state)
			{
				case rbsReady:
				{
					if(res!=0x0D && res!=0x0A && res!=0x1A)
					{
						pRecvBuf->buf[pRecvBuf->len++] = res;
						pRecvBuf->state = rbsReceiving;
					}
				}break;
				case rbsReceiving:
				{
					if(res==0x0D || res==0x0A || res==0x1A)
					{
						pRecvBuf->state = rbsFinished;
						pRecvBuf = NULL;
					}
					else
						pRecvBuf->buf[pRecvBuf->len++] = res;
				}break;
				default: break;
			}
			
			if(pRecvBuf->len >= USART_REC_LEN)
			{
				pRecvBuf->state = rbsFinished;
				pRecvBuf = NULL;
			}
		}
	}
}
