/************************************************************************************
	
*************************************************************************************/

#include "delay.h"
#include "usart.h"
#include "GPRS.h"
#include "TM1638.h"
#include "task.h"
#include "timer.h"
#include "HX711.h"

int main(void)
{
	delay_init();
	NVIC_Configuration(); 	 	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	UART_INIT(USART1, 115200);
	//ENABLE_UART(USART1);
	GPIO_ResetBits(GPIOA, A7PWR);

	//CLEAR_EVENT(TFT_EVENT);
	CLEAR_EVENT(LED_EVENT);
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);  
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	InitTask(tpHIGH);			//��ʼ�����������
	TM1638_Init();
	//Init_HX711pin();
	//InitTFT();
	PowerOnGPRS();
	//SetTask(tpHIGH, TASK_TIMER, ENABLE, TASK_1S, tsSTOP);
	while(1)
	{
		TaskCycle(tpHIGH);
	}
}

