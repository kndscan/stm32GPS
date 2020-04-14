#ifndef __TM1638_H__
#define __TM1638_H__

/********************************************************************************************** 
**Program Assignment: Driver for TM1638 digital tube 
**Author        : Wuwang 
**Date              : 2014.8.26 9:00 
**Description       : This is a driver for the board which is controled by thechip of tm1638.  
              The board has eight digital tubes which have eight segments and eight keys. 
***********************************************************************************************/  
#include "stm32f10x.h"

/*********************define and global variables*********************************************/  
#define STB 		GPIO_Pin_14                        		//chip-select line  
#define CLK 		GPIO_Pin_13                              //clock line  
#define DIO 		GPIO_Pin_12                              //data line

#define Set(x)		(GPIO_SetBits(GPIOB,(x)))					//Sets the selected data port bits  
#define Reset(x) 	(GPIO_ResetBits(GPIOB,(x)))					//Resets the selected data port bits  
#define Get(x) 		(GPIO_ReadInputDataBit(GPIOB,(x)) != RESET)	//Read the specified input port pin  
 
#define CMD_WT_LED_REG		0x40
#define CMD_RD_KEY_REG		0x42

#define CMD_ADDR_MODE_AUTO	0x40
#define CMD_ADDR_MODE_STEP	0x44

#define CMD_TEST_NORMAL		0x40
#define CMD_TEST_TEST		0x48

#define CMD_ADDR_START		0xC0

#define CMD_DISP_ON			0x88
#define CMD_DISP_OFF		0x80

#define LED_GPS			7
#define LED_GPRS		6
#define LED_A7			5
#define LED_SIM			4
#define LED_OV_TIME		3
#define LED_AT_ERR		2

u8 Read_Key(void);
void WriteLEDs(u8 i, char *str);
void WriteLamps(u8 state);
void LampsFlow(void);

extern void TM1638_Init(void);
extern void SetLamp(u8 index);
extern void ResetLamp(u8 index);
extern void KeyScan(void);
extern void UpdateLED(bool bForce);

#endif
