#ifndef __TFT_H__
#define __TFT_H__

#include "GPRS.h"

#define TFT_SIZE_W		128
#define TFT_SIZE_H		128

#define POS_SIG_W		20
#define POS_SIG_X		0
#define POS_SIG_Y		0

#define POS_COPS_W		30
#define POS_COPS_X		(POS_SIG_X+POS_SIG_W)
#define POS_COPS_Y		0

#define POS_CPIN_W		10
#define POS_CPIN_X		(POS_COPS_X+POS_COPS_W)
#define POS_CPIN_Y		0

#define POS_CCID_W		20
#define POS_CCID_X		(POS_CPIN_X+POS_CPIN_W)
#define POS_CCID_Y		0

#define POS_REG_W		30
#define POS_REG_X		(POS_CCID_X+POS_CCID_W)
#define POS_REG_Y		0

#define POS_NS_W		90		//γ����ʾ����
#define POS_NS_X		0		//γ����ʾ����
#define POS_NS_Y		16

#define POS_WE_W		100		//������ʾ����
#define POS_WE_X		0		//������ʾ����
#define POS_WE_Y		32

#define POS_CUR_AT_H	16

#define POS_CUR_AT_X	0		//��ǰATָ������
#define POS_CUR_AT_Y	48
#define POS_CUR_AT_ST_X	64		//��ǰATָ��״̬
#define POS_CUR_AT_ST_Y	48

#define POS_MAOPI_X		0		//ëƤ
#define POS_MAOPI_Y		64
#define POS_SHIWU_X		80		//ʵ��
#define POS_SHIWU_Y		64

#define POS_BUF_X		0
#define POS_BUF_Y		80

#define POS_DATA_X		0		//CloudData
#define POS_DATA_Y		96

extern void InitTFT(void);
extern void UpdateTFT(bool bForce);

void DrawSignalImage(const unsigned char *p);
void DrawGPRSImage(const unsigned char *p);
	
#endif

