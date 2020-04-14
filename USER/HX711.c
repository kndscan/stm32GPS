/************************************************************************************
							�������ṩ�����µ��̣�
								Ilovemcu.taobao.com
								epic-mcu.taobao.com
							ʵ�������Χ��չģ����������ϵ���
							���ߣ����زر���							
*************************************************************************************/
#include "HX711.h"
#include "delay.h"
#include "task.h"

u32 HX711_Buffer;
u32 Weight_Maopi;
s32 Weight_Shiwu;
u8 Flag_Error = 0;
char MaoPiBuf[INT_DIGIT_NUM];
char ShiWuBuf[INT_DIGIT_NUM];

//����ʵ���ʵ������
//��Ϊ��ͬ�Ĵ������������߲�һ������ˣ�ÿһ����������Ҫ���������716���������
//�����ֲ��Գ���������ƫ��ʱ�����Ӹ���ֵ��
//������Գ���������ƫСʱ����С����ֵ��

//#define WEIGHT_RATIO 	447			//128����
#define WEIGHT_RATIO 	113			//32����


void Init_HX711pin(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PF�˿�ʱ��

	//HX711_SCK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 // �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB
	
	//HX711_DOUT
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  
	
	GPIO_SetBits(GPIOA,GPIO_Pin_3);							//��ʼ������Ϊ0
	
	delay_ms(10);
	HX711_ReadB();
	delay_ms(10);
	Get_Maopi();
	SetTask(tpHIGH, TASK_GET_WEIGHT, ENABLE, TASK_2S, tsSTOP);
}

//****************************************************
//��ȡHX711
//****************************************************
u32 HX711_Read(void)	//����128
{
	unsigned long count; 
	unsigned char i; 
  	HX711_DOUT=1; 
	delay_us(1);
  	HX711_SCK=0;
  	count=0; 
  	while(HX711_DOUT); 
  	for(i=0;i<24;i++)
	{ 
	  	HX711_SCK=1; 
	  	count=count<<1; 
		delay_us(1);
		HX711_SCK=0; 
	  	if(HX711_DOUT)
			count++; 
		delay_us(1);
	} 
 	HX711_SCK=1; 
    count=count^0x800000;//��25�������½�����ʱ��ת������
	delay_us(1);
	HX711_SCK=0;  
	return(count);
}
u32 HX711_ReadB(void)	//����128
{
	unsigned long count; 
	unsigned char i; 
  	HX711_DOUT=1; 
	delay_us(1);
  	HX711_SCK=0;
  	count=0; 
  	while(HX711_DOUT); 
  	for(i=0;i<24;i++)
	{ 
	  	HX711_SCK=1; 
	  	count=count<<1; 
		delay_us(1);
		HX711_SCK=0; 
	  	if(HX711_DOUT)
			count++; 
		delay_us(1);
	} 
 	HX711_SCK=1; 
    count=count^0x800000;//��25�������½�����ʱ��ת������
	delay_us(1);
	HX711_SCK=0;

 	HX711_SCK=1; 
	delay_us(1);
	HX711_SCK=0;
	
	return(count);
}
//****************************************************
//��ȡëƤ����
//****************************************************
void Get_Maopi(void)
{
	Weight_Maopi = HX711_ReadB();
	toString(Weight_Maopi, MaoPiBuf);
	SET_EVENT(TFT_EVENT, EVENT_WEIGHT);
} 

//****************************************************
//����
//****************************************************
void Get_Weight(void)
{
	HX711_Buffer = HX711_ReadB();
	if(HX711_Buffer > Weight_Maopi)			
	{
		Weight_Shiwu = HX711_Buffer;
		Weight_Shiwu = Weight_Shiwu - Weight_Maopi;				//��ȡʵ���AD������ֵ��
	
		//Weight_Shiwu = (s32)((float)Weight_Shiwu/GapValue+0.05); 	//����ʵ���ʵ������
																		//��Ϊ��ͬ�Ĵ������������߲�һ������ˣ�ÿһ����������Ҫ���������GapValue���������
																		//�����ֲ��Գ���������ƫ��ʱ�����Ӹ���ֵ��
																		//������Գ���������ƫСʱ����С����ֵ��
																		//����ֵһ����4.0-5.0֮�䡣�򴫸�����ͬ������
																		//+0.05��Ϊ����������ٷ�λ
		Weight_Shiwu = Weight_Shiwu/WEIGHT_RATIO;
		toString(Weight_Shiwu, ShiWuBuf);
		SET_EVENT(TFT_EVENT, EVENT_WEIGHT);
	}
}

bool GetWeightInfo(char *buf)
{
	u8 i;
	for(i=0; i<INT_DIGIT_NUM; i++)
	{
		if(isFloatChar(ShiWuBuf[i]))
			*(buf+i) = ShiWuBuf[i];
		else
			break;
	}
	return true;
}
