
#include "stm32f10x.h"
#include "stm32f10x_flash.h"                     //flash�����ӿ��ļ����ڿ��ļ��У�������Ҫ����

int ReadFlashNBtye(uint32_t ReadAddress, uint8_t *ReadBuf, int32_t ReadNum);
void WriteFlashOneWord(uint32_t WriteAddress,uint32_t WriteData);