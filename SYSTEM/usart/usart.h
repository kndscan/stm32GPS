#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

//SendString����Ĭ��ʹ��\0��Ϊ�ַ������������˺궨��δָ��\0ʱ������ַ������ͳ���
#define USART_TRS_MAX_LEN		100
#define USART_REC_LEN  			200  	//�����������ֽ��� 200
#define REC_BUF_CNT				5
#define IS_GPRS_HEAD(x)			((x)=='+'||(x)=='O'||(x)=='C'||(x)=='E'||(x)=='>'||(x)=='^')
/**********************************************************
���ջ�����
**********************************************************/
typedef enum 
{
	rbtUnexpect = 0,
	rbtGPS = 1,
	rbtGPRS,
	rbtDATA,
	rbtSkip,			//����֡���ε�ָ��������OK�������OK
	rbtInit				//��ʾ�յ�A7ģ���ʼ����������'^'��ͷ���ַ���
}RecvBufType;
typedef enum 
{
	rbsReady = 0,		//�����������������������
	rbsReceiving = 1,	//���ڽ��գ�����δ�յ�������(0x0D/0x0A)
	rbsFinished			//���յ������� �� �����������ñ�־�����ж�����λ
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


