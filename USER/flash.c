
/****************************************************************
*Function:	STM32F103ϵ���ڲ�Flash��д����
*Author:    ValerianFan
*Date:		2014/04/09
*E-Mail:	fanwenjingnihao@163.com
*Other:		�ó�����ֱ�ӱ������У�ֻ������Flash��д����
****************************************************************/


#include "stm32f10x.h"
#include "stm32f10x_flash.h"                     //flash�����ӿ��ļ����ڿ��ļ��У�������Ҫ����


#define  STARTADDR  0x08010000                   	 //STM32F103RB �����ͺŻ������ã�δ���� 
volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;      //Flash����״̬����


/****************************************************************
*Name:		ReadFlashNBtye
*Function:	���ڲ�Flash��ȡN�ֽ�����
*Input:		ReadAddress�����ݵ�ַ��ƫ�Ƶ�ַ��ReadBuf������ָ��	ReadNum����ȡ�ֽ���
*Output:	��ȡ���ֽ���  
*Author:    ValerianFan
*Date:		2014/04/09
*E-Mail:	fanwenjingnihao@163.com
*Other:		
****************************************************************/
int ReadFlashNBtye(uint32_t ReadAddress, uint8_t *ReadBuf, int32_t ReadNum) 
{
        int DataNum = 0;
		ReadAddress = (uint32_t)STARTADDR + ReadAddress; 
        while(DataNum < ReadNum) 
		{
           *(ReadBuf + DataNum) = *(__IO uint8_t*) ReadAddress++;
           DataNum++;
        }
        return DataNum;
}

/****************************************************************
*Name:		WriteFlashOneWord
*Function:	���ڲ�Flashд��32λ����
*Input:		WriteAddress�����ݵ�ַ��ƫ�Ƶ�ַ��WriteData��д������
*Output:	NULL 
*Author:    ValerianFan
*Date:		2014/04/09
*E-Mail:	fanwenjingnihao@163.com
*Other:		
****************************************************************/

void WriteFlashOneWord(uint32_t WriteAddress,uint32_t WriteData)
{
	FLASH_UnlockBank1();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
    FLASHStatus = FLASH_ErasePage(STARTADDR);

	if(FLASHStatus == FLASH_COMPLETE)
	{
		FLASHStatus = FLASH_ProgramWord(STARTADDR + WriteAddress, WriteData);    //flash.c ��API����
		//FLASHStatus = FLASH_ProgramWord(StartAddress+4, 0x56780000);                      //��Ҫд���������ʱ����
		//FLASHStatus = FLASH_ProgramWord(StartAddress+8, 0x87650000);                      //��Ҫд���������ʱ����
	}
	FLASH_LockBank1();
} 

/****************************************************************
*Name:		main
*Function:	д���ȡ�ڲ�flash
*Input:		NULL
*Output:	NULL
*Author:        ValerianFan
*Date:		2014/04/09
*E-Mail:	fanwenjingnihao@163.com
*Other:		
****************************************************************/

/*
void main(void)
{  
	uint8_t Temp_Data[20] = {0};
	unsigned char Temp = 0;
	int ReadNum = 0;
	int i = 0;

	Dveice_Init(72);  //ϵͳ��ʼ��������ʱ��Ϊ72M
	MCU_USART_Init();

        WriteFlashOneWord(0,0x12345678);     //д������
	Delay_Ms(10);                         
	ReadNum = ReadFlashNBtye(0, Temp_Data,4); //��ȡ����
	Temp = (unsigned char)ReadNum; 
	USART1_Send_One(Temp);	              //���Ͷ�ȡ���ֽ���
	Delay_Ms(10);

	while(1)
	{
	    for(i = 0;i < ReadNum;i++)			   
		{
		    USART1_Send_One(Temp_Data[i]);	 //���Ͷ�ȡ�������ݣ��ɹ۲����ݵĴ洢��ʽ
		    Delay_Ms(100); 
		}
	} 
	
}*/