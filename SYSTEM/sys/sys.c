#include "sys.h"

/*
USART1: 0, 0
USART2: 0, 1
USART3: 0, 2

TIM3:   1, 0
TIM4:   2, 0
*/
void NVIC_Configuration(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//����NVIC�жϷ���2: 2λ��ռ���ȼ���2λ��Ӧ���ȼ�
}
