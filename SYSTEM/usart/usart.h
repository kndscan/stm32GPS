#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

//SendString函数默认使用\0作为字符串结束符。此宏定义未指定\0时的最大字符串发送长度
#define USART_TRS_MAX_LEN		100
#define USART_REC_LEN  			200  	//定义最大接收字节数 200
#define REC_BUF_CNT				5
#define IS_GPRS_HEAD(x)			((x)=='+'||(x)=='O'||(x)=='C'||(x)=='E'||(x)=='>'||(x)=='^')
/**********************************************************
接收缓冲区
**********************************************************/
typedef enum 
{
	rbtUnexpect = 0,
	rbtGPS = 1,
	rbtGPRS,
	rbtDATA,
	rbtSkip,			//返回帧带参的指令先跳过OK，最后处理OK
	rbtInit				//表示收到A7模块初始化发出的以'^'开头的字符串
}RecvBufType;
typedef enum 
{
	rbsReady = 0,		//缓冲区就绪，可以填充数据
	rbsReceiving = 1,	//正在接收，但还未收到结束符(0x0D/0x0A)
	rbsFinished			//接收到结束符 或 缓冲区满，该标志仅在中断里置位
}RecvBufState;
typedef struct
{
	RecvBufState state;
	u8 len;
	char buf[USART_REC_LEN];
}RECV_BUF;

void InitRecvBufs(void);
extern RECV_BUF *GetRecvBufByIndex(unsigned char i);
RECV_BUF *GetRecvBuf(void);
RecvBufType GetRecvBufType(RECV_BUF *pBuf);
bool HeadCheck(const char *src, const char *sub);


extern void ResetSendPointer(USART_TypeDef* USARTx);
extern void InitRecvBuf(u8 index);
extern void InitRecvBufEx(RECV_BUF *pBuf);

extern void ParseUSARTData(void);

extern void SendString(USART_TypeDef* USARTx, const char *p);
extern void UART_INIT(USART_TypeDef* USARTx, u32 baudRate);

extern void ENABLE_UART(USART_TypeDef* USARTx);
extern void DISABLE_UART(USART_TypeDef* USARTx);


#endif


