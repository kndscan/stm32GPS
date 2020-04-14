/********************************************************************************************** 
**Program Assignment: Driver for TM1638 digital tube 
**Author        : Wuwang 
**Date              : 2014.8.26 9:00 
**Description       : This is a driver for the board which is controled by thechip of tm1638.  
              The board has eight digital tubes which have eight segments and eight keys. 
***********************************************************************************************/  
#include "TM1638.h"
#include "GPRS.h"
#include "task.h"
#include "common.h"

//the char and its segment code
static u8 const tm_dat[2][14]=
{
	{'0' ,'1' ,'2' ,'3' ,'4' ,'5' ,'6' ,'7' ,'8' ,'9' ,'.' ,'-' ,'_' ,' ' },  
	{0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x80,0x40,0x08,0x00}
};
static u8 gCurLampState = 1;
static u8 PressedKeyCode = 0;

/*********************************************************************************************** 
*Function Name: RCC_Config       
*Purpose      : Configration Clock 
***********************************************************************************************/  
void RCC_Config()
{  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  
}  
  
/*********************************************************************************************** 
*Function Name: GPIO_Config      
*Purpose      : Configration GPIO 
***********************************************************************************************/  
void GPIO_Config()
{  
    GPIO_InitTypeDef GPIO_InitStructure;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	//开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = STB | CLK | DIO;  
    GPIO_Init(GPIOB, &GPIO_InitStructure);  
}  
/*********************************************************************************************** 
*Function Name: Write_Byte       
*Purpose      : Write one byte to the data port 
*params       : byte  -------8-bits byte   
*return       : none 
***********************************************************************************************/  
void Write_Byte(uint8_t byte)
{ 	
	u8 i = 0;
	for(i=0;i<8;i++)
	{
		Reset(CLK);
		if(byte & 0x01)
			Set(DIO);
		else
			Reset(DIO);
		Set(CLK);
		
		byte >>= 1;
	}
}
/*********************************************************************************************** 
*Function Name: Read_Byte        
*Purpose      : Read one byte from data port 
*params       : none 
*return       : the 8-bits byte which is read from data port 
***********************************************************************************************/  
uint8_t Read_Byte()
{	
	u8 i, temp = 0;
	Set(DIO);				//这行代码不能删除
	Reset(CLK);
	for(i=0; i<8; i++)
	{
		temp >>= 1;
		Set(CLK);
		if(Get(DIO))
			temp |= 0x80;
		Reset(CLK);
	}
	return temp;
}  
  
/*********************************************************************************************** 
*Function Name: Write_Cmd        
*Purpose      : Write a conmand to the data port 
*params       : cmd  -------8-bits byte,the conmand,check the data sheet to find the conmand  
*return       : none 
***********************************************************************************************/ 
void Write_Cmd(u8 cmd)
{   
    Reset(STB);
    Write_Byte(cmd);
	Set(STB);
}
/*********************************************************************************************** 
*Function Name: Read_Key         
*Purpose      : Read the key number which has been pressed 
*params       : none 
*return       : the number of the key. 0-8.  "return 0" represents no key has been pressed. 
***********************************************************************************************/  
u8 Read_Key(void)
{  
    u8  i, c[4] = {0,0,0,0};  
    u16 key = 0x00;
	
	Reset(STB);
	Write_Byte(CMD_RD_KEY_REG);
    for(i=0; i<4; i++) 
        c[i] = Read_Byte();
	Set(STB);

    for(i=0; i<4; i++)
        key |= (c[i] << i); 
	
    for(i=0; i<8; i++)
	{
        if((0x01 << i) == key)
			return i + 1;
    }  
    return 0;  
}  
  
/*********************************************************************************************** 
*Function Name: Write_Data
*Purpose      : Write data to the specified location  
*params       : index ------the address,0x00 to 0x0f 
                data  ------the data,segment code 
*return       : none 
***********************************************************************************************/  
void Write_Single_Data(u8 index, u8 data)
{
	Write_Cmd(CMD_ADDR_MODE_STEP);
	Reset(STB);
    Write_Byte(CMD_ADDR_START + index);
    Write_Byte(data);
	Set(STB);
}  
  
/*********************************************************************************************** 
*Function Name: TM1638_SendData      
*Purpose      : Write data to the location specified 
*params       : i     ------the bit code of digtal tube,0 to 7 
                str   ------the string,the char which was not in tm_data will be replace with "''". 
*return       : none 
***********************************************************************************************/  
void WriteLEDs(u8 i, char *str)
{
	bool bValid;
    u8 j=0, chr;
    for(;i<8;i++)
	{
		bValid = false;
        for(j=0; j<14; j++)
		{  
            if(*str == tm_dat[0][j])
			{  
                chr = tm_dat[1][j];  
				bValid = true;
                break;  
            }
        }
          
        if(bValid == false)
            chr = 0x00;
          
        if(*(str+1) == '.')
		{  
            chr |= 0x80;  
            Write_Single_Data(i * 2, chr);  
            str++;  
        }
		else
		{  
            Write_Single_Data(i * 2, chr);  
        }
        str++;
		
        if(*str == '\0')
			break;
    }
}  

void WriteLamps(u8 state)
{
	u8 i;
	for(i=0; i<8; i++)
	{
		if(state & (1<<i))
			Write_Single_Data(2*i+1, 2);
		else
			Write_Single_Data(2*i+1, 0);
	}
}
//流水灯，主函数里周期调用即可
void LampsFlow(void)
{
	bool b = false;
	if(gCurLampState & (0x01))
		b = true;
	
	gCurLampState >>= 1;
	if(b)
		gCurLampState |= 0x80;
	
	WriteLamps(gCurLampState);
}
void SetLamp(u8 index)
{
	if(index < 8)
	{
		gCurLampState |= (1<<index);
		WriteLamps(gCurLampState);
	}
}
void ResetLamp(u8 index)
{
	if(index < 8)
	{
		gCurLampState &= ~(1<<index);
		WriteLamps(gCurLampState);
	}
}

/*********************************************************************************************** 
*Function Name: TM1638_Init      
*Purpose      : the initialization of tm1638 
*params       : none 
*return       : none 
***********************************************************************************************/  
void TM1638_Init(void)
{  
    int i=0;
    RCC_Config(); 
    GPIO_Config();
	
	//初始化显示寄存器
    Write_Cmd(CMD_DISP_ON);				//显示开+亮度调节
    Write_Cmd(CMD_ADDR_MODE_STEP);		//
	for(i=0; i<16; i++)
	{
		Reset(STB);
		Write_Byte(CMD_ADDR_START + i);
		Write_Byte(0x00);
		Set(STB);
	}
	SetTask(tpHIGH, TAST_KEY_SCAN, ENABLE, TASK_20ms, tsSTOP);
	//UpdateLED(true);
//	SetLamp(LED_GPS);
//	SetLamp(LED_GPRS);
}

/*
键盘扫描处理
key1: 开始初始化GPRS，并执行当前指令
key2: 打开GPS，3秒
key3: 关闭GPS
key4: 发送数据
*/
void KeyScan(void)
{
	u8 newKeyCode;
	if(PressedKeyCode > 0)
	{
		newKeyCode = Read_Key();
		if(newKeyCode == PressedKeyCode)
		{
			switch(newKeyCode)
			{
				case 1:{
					CLEAR_EVENT(TFT_EVENT);
					CLEAR_EVENT(LED_EVENT);
					
					InitTask(tpHIGH);			//初始化任务管理器
					PowerOnGPRS();
					
					TM1638_Init();
					Init_HX711pin();
					InitTFT();
				}break;
				case 2:{
				}break;
				case 3:{
				}break;
				default:
					break;
			}
			PressedKeyCode = 0;
		}
		else
		{
			PressedKeyCode = newKeyCode;
		}
	}
	else
	{
		PressedKeyCode = Read_Key();
	}
}

void UpdateLED(bool bForce)
{	
	if(NO_EVENT(LED_EVENT))
		return;
		
	if(GET_EVENT(LED_EVENT, EVENT_A7_ON) || bForce)
	{
		SetLamp(LED_A7);
	}
	if(GET_EVENT(LED_EVENT, EVENT_A7_OFF) || bForce)
	{
		ResetLamp(LED_A7);
	}

	if(GET_EVENT(LED_EVENT, EVENT_SIM_ON) || bForce)
	{
		SetLamp(LED_SIM);
	}
	if(GET_EVENT(LED_EVENT, EVENT_SIM_OFF) || bForce)
	{
		ResetLamp(LED_SIM);
	}
	
	if(GET_EVENT(LED_EVENT, EVENT_GPS_ON) || bForce)
	{
		SetLamp(LED_GPS);
	}
	if(GET_EVENT(LED_EVENT, EVENT_GPS_OFF) || bForce)
	{
		ResetLamp(LED_GPS);
	}

	if(GET_EVENT(LED_EVENT, EVENT_GPRS_ON) || bForce)
	{
		SetLamp(LED_GPRS);
	}
	if(GET_EVENT(LED_EVENT, EVENT_GPRS_OFF) || bForce)
	{
		ResetLamp(LED_GPRS);
	}
	
	if(GET_EVENT(LED_EVENT, EVENT_OV_TIME_ON) || bForce)
	{
		SetLamp(LED_OV_TIME);
	}
	if(GET_EVENT(LED_EVENT, EVENT_OV_TIME_OFF) || bForce)
	{
		ResetLamp(LED_OV_TIME);
	}
	
	if(GET_EVENT(LED_EVENT, EVENT_AT_ERR_ON) || bForce)
	{
		SetLamp(LED_AT_ERR);
	}
	if(GET_EVENT(LED_EVENT, EVENT_AT_ERR_OFF) || bForce)
	{
		ResetLamp(LED_AT_ERR);
	}

	CLEAR_EVENT(LED_EVENT);
}

