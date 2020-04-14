
#include "TFT.h"
#include "Image.h"
#include "GPS.h"
#include "GUI.h"
#include "Lcd_Driver.h"
#include "common.h"
#include "HX711.h"
#include "task.h"

void InitTFT(void)
{
	Lcd_Init();
	Lcd_Clear(WHITE);
	SetTask(tpHIGH, TASK_TFT, ENABLE, TASK_100ms, tsSTOP);
	UpdateTFT(true);
}

void UpdateTFT(bool bForce)
{	
	GPRS_INFO *pInfo = GetGPRSInfo();
	if(NO_EVENT(TFT_EVENT))
		return;
	
	if(GET_EVENT(TFT_EVENT, EVENT_WEIGHT) || bForce)
	{
		Lcd_Clear_Region(WHITE, POS_SHIWU_X, POS_SHIWU_Y, 48, POS_SHIWU_Y+16);
		Gui_DrawFont_GBK16(POS_MAOPI_X, POS_MAOPI_Y, BLACK, WHITE, MaoPiBuf);
		Gui_DrawFont_GBK16(POS_SHIWU_X, POS_SHIWU_Y, BLACK, WHITE, ShiWuBuf);
	}
	if(GET_EVENT(TFT_EVENT, EVENT_USART_DATA) || bForce)
	{
		Lcd_Clear_Region(WHITE, POS_BUF_X, POS_BUF_Y, TFT_SIZE_W, POS_BUF_Y+16);
		DrawChars(POS_BUF_X, POS_BUF_Y, BLACK, WHITE, GetRecvBufByIndex(0)->buf, 3);
		DrawChars(POS_BUF_X+25, POS_BUF_Y, BLACK, WHITE, GetRecvBufByIndex(1)->buf, 3);
		DrawChars(POS_BUF_X+50, POS_BUF_Y, BLACK, WHITE, GetRecvBufByIndex(2)->buf, 3);
		DrawChars(POS_BUF_X+75, POS_BUF_Y, BLACK, WHITE, GetRecvBufByIndex(3)->buf, 3);
		DrawChars(POS_BUF_X+100, POS_BUF_Y, BLACK, WHITE, GetRecvBufByIndex(4)->buf, 3);
	}
	if(GET_EVENT(TFT_EVENT, EVENT_SIG) || bForce)
	{
		DrawChars(POS_SIG_X, POS_SIG_Y, BLACK, WHITE, pInfo->aSignal, 2);
		//DrawChars(POS_SIG_X+24, POS_SIG_Y, BLACK, WHITE, pInfo->aErRate, 2);
	}
	if(GET_EVENT(TFT_EVENT, EVENT_REG) || bForce)
	{
		DrawChar(POS_REG_X, POS_REG_Y, BLACK, WHITE, pInfo->netRegMode);
		DrawChar(POS_REG_X+8, POS_REG_Y, BLACK, WHITE, ',');
		DrawChar(POS_REG_X+16, POS_REG_Y, BLACK, WHITE, pInfo->netRegState);
	}
	if(GET_EVENT(TFT_EVENT, EVENT_SIM_ON) || bForce)
	{
		DrawChars(POS_CPIN_X, POS_CPIN_Y, BLACK, WHITE, pInfo->aCPIN, 1);
	}
	if(GET_EVENT(TFT_EVENT, EVENT_SIM_OFF) || bForce)
	{
		Lcd_Clear_Region(WHITE, POS_CPIN_X, POS_CPIN_Y, POS_CPIN_W, POS_SIG_Y+16);
	}
	
	if(GET_EVENT(TFT_EVENT, EVENT_CCID) || bForce)
	{
		DrawChars(POS_CCID_X, POS_CCID_Y, BLACK, WHITE, pInfo->aSimID, 3);
	}
	if(GET_EVENT(TFT_EVENT, EVENT_COPS) || bForce)
	{
		DrawChar(POS_COPS_X, POS_COPS_Y, BLACK, WHITE, pInfo->copsMode);
		DrawChar(POS_COPS_X+8, POS_COPS_Y, BLACK, WHITE, pInfo->copsStyle);
		//DrawChars(POS_COPS_X+16, POS_COPS_Y, BLACK, WHITE, pInfo->copsOpe, 7);
	}
	if(GET_EVENT(TFT_EVENT, EVENT_DATA_UP) || bForce)
	{
		//pGPRS->bGPRSStateChanged = false;
		//DrawGPRSImage(ImageGPRS[pGPRS->gprsState]);
	}
	if(GET_EVENT(TFT_EVENT, EVENT_DATA_DOWN) || bForce)
	{
		//pGPRS->bGPRSStateChanged = false;
		//DrawGPRSImage(ImageGPRS[pGPRS->gprsState]);
		DrawChars(POS_DATA_X, POS_DATA_Y, BLACK, WHITE, pInfo->pData->buf, 16);
	}
	if(GET_EVENT(TFT_EVENT, EVENT_AT_TYPE) || bForce)
	{
		if(GetATCmdType() != ctNULL)
		{
			Lcd_Clear_Region(WHITE, POS_CUR_AT_X, POS_CUR_AT_Y,64, POS_CUR_AT_Y+POS_CUR_AT_H);
			Gui_DrawFont_GBK16(POS_CUR_AT_X, POS_CUR_AT_Y, BLACK, WHITE, GetATCmdTypeName(pInfo->pAT->type));
		}
	}
	if(GET_EVENT(TFT_EVENT, EVENT_AT_STATUS) || bForce)
	{
		//if(GetATCmdStatus() != stNULL)
		//{
			Lcd_Clear_Region(WHITE, POS_CUR_AT_X+64, POS_CUR_AT_Y, TFT_SIZE_W, POS_CUR_AT_Y+POS_CUR_AT_H);
			Gui_DrawFont_GBK16(POS_CUR_AT_ST_X, POS_CUR_AT_ST_Y, BLACK, WHITE, GetATCmdStatusName(pInfo->pAT->status));
		//}
	}
	//GPS数据是否更新
	if(GET_EVENT(TFT_EVENT, EVENT_GPS_DATA) || bForce)
	{
		GPGGA *pBuf = GetGPSBuf();
		if(NULL != pBuf)
		{
			DrawChars(POS_NS_X, POS_NS_Y, BLACK, WHITE, pBuf->Latitude, 10);
			DrawChars(POS_WE_X, POS_WE_Y, BLACK, WHITE, pBuf->Longitude, 11);
		}
	}
	CLEAR_EVENT(TFT_EVENT);
}

void DrawSignalImage(const unsigned char *p)
{
  	int i, pxCnt;
	unsigned char pxH,pxL;

	Lcd_SetRegion(IMAGE_SIGNAL_TL_X, IMAGE_SIGNAL_TL_Y, IMAGE_SIGNAL_BR_X,IMAGE_SIGNAL_BR_Y);
	pxCnt = IMAGE_SIGNAL_PX;
	for(i=0; i<pxCnt; i++)
	{	
		pxL = *(p+i*2);
		pxH = *(p+i*2+1);
		LCD_WriteData_16Bit(pxH<<8|pxL);
	}
}
void DrawGPRSImage(const unsigned char *p)
{
  	int i, pxCnt;
	unsigned char pxH,pxL;

	Lcd_SetRegion(IMAGE_GPRS_TL_X, IMAGE_GPRS_TL_Y, IMAGE_GPRS_BR_X,IMAGE_GPRS_BR_Y);
	pxCnt = IMAGE_GPRS_PX;
	for(i=0; i<pxCnt; i++)
	{	
		pxL = *(p+i*2);
		pxH = *(p+i*2+1);			
		LCD_WriteData_16Bit(pxL<<8|pxH);			
	}
}
