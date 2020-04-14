/************************************************************************************
							本例程提供自以下店铺：
								Ilovemcu.taobao.com
								epic-mcu.taobao.com
							实验相关外围扩展模块均来自以上店铺
							作者：神秘藏宝室							
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

//计算实物的实际重量
//因为不同的传感器特性曲线不一样，因此，每一个传感器需要矫正这里的716这个除数。
//当发现测试出来的重量偏大时，增加该数值。
//如果测试出来的重量偏小时，减小改数值。

//#define WEIGHT_RATIO 	447			//128增益
#define WEIGHT_RATIO 	113			//32增益


void Init_HX711pin(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PF端口时钟

	//HX711_SCK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 // 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB
	
	//HX711_DOUT
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			//输入上拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);  
	
	GPIO_SetBits(GPIOA,GPIO_Pin_3);							//初始化设置为0
	
	delay_ms(10);
	HX711_ReadB();
	delay_ms(10);
	Get_Maopi();
	SetTask(tpHIGH, TASK_GET_WEIGHT, ENABLE, TASK_2S, tsSTOP);
}

//****************************************************
//读取HX711
//****************************************************
u32 HX711_Read(void)	//增益128
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
    count=count^0x800000;//第25个脉冲下降沿来时，转换数据
	delay_us(1);
	HX711_SCK=0;  
	return(count);
}
u32 HX711_ReadB(void)	//增益128
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
    count=count^0x800000;//第25个脉冲下降沿来时，转换数据
	delay_us(1);
	HX711_SCK=0;

 	HX711_SCK=1; 
	delay_us(1);
	HX711_SCK=0;
	
	return(count);
}
//****************************************************
//获取毛皮重量
//****************************************************
void Get_Maopi(void)
{
	Weight_Maopi = HX711_ReadB();
	toString(Weight_Maopi, MaoPiBuf);
	SET_EVENT(TFT_EVENT, EVENT_WEIGHT);
} 

//****************************************************
//称重
//****************************************************
void Get_Weight(void)
{
	HX711_Buffer = HX711_ReadB();
	if(HX711_Buffer > Weight_Maopi)			
	{
		Weight_Shiwu = HX711_Buffer;
		Weight_Shiwu = Weight_Shiwu - Weight_Maopi;				//获取实物的AD采样数值。
	
		//Weight_Shiwu = (s32)((float)Weight_Shiwu/GapValue+0.05); 	//计算实物的实际重量
																		//因为不同的传感器特性曲线不一样，因此，每一个传感器需要矫正这里的GapValue这个除数。
																		//当发现测试出来的重量偏大时，增加该数值。
																		//如果测试出来的重量偏小时，减小改数值。
																		//该数值一般在4.0-5.0之间。因传感器不同而定。
																		//+0.05是为了四舍五入百分位
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
