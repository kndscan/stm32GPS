#ifndef __COMMON_H__
#define __COMMON_H__

#include <math.h>
#include <stdlib.h>

typedef enum {OFF = 0, ON = !OFF}LED_STATE;

typedef struct
{
	int id;
	int period;
    void (*TimeEvent)(void);	// 定时器到后要运行的任务函数
}TIMER;

#define INT_DIGIT_NUM	12		//

#define ARRAY_LEN(x)	(sizeof(x)/sizeof(x[0]))
#define toChar(x)		((char)((x)+0x30))
#define toUChar(x)		((unsigned char)((x)-0x30))
#define toShort(h, l)	((unsigned short)(((h)<<8)|(l)))
#define toUShort(h, l)	((short)(((h)<<8)|(l)))

#define isDigitChar(x)	((x)>='0' && (x)<='9')
#define isFloatChar(x)	(isDigitChar(x) || (x)=='.')

typedef unsigned int 			EVENT;
typedef unsigned int 			EVENT_FLAG;

#define NO_EVENT(evt)			(0==(evt))
#define HAS_EVENT(evt)			(0!=(evt))
#define GET_EVENT(evt,flag)		((evt)&(flag))
#define SET_EVENT(evt,flag)		((evt)|=(flag))
#define RESET_EVENT(evt,flag)	((evt)&=(~(flag)))
#define CLEAR_EVENT(evt)		((evt)=0)

#define EVENT_SIM_OFF			(0x00000001)
#define EVENT_SIM_ON			(0x00000002)
#define EVENT_GPS_ON			(0x00000004)
#define EVENT_GPS_OFF			(0x00000008)
#define EVENT_AGPS_ON			(0x00000010)
#define EVENT_AGPS_OFF			(0x00000020)
#define EVENT_GPRS_ON			(0x00000040)
#define EVENT_GPRS_OFF			(0x00000080)
#define EVENT_SIG				(0x00000100)
#define EVENT_GPSRD_ON			(0x00000200)
#define EVENT_GPSRD_OFF			(0x00000400)
#define EVENT_REG				(0x00000800)
#define EVENT_COPS				(0x00001000)
#define EVENT_AT_TYPE			(0x00002000)
#define EVENT_AT_STATUS			(0x00004000)
#define EVENT_DATA_UP			(0x00008000)
#define EVENT_DATA_DOWN			(0x00010000)
#define EVENT_CCID				(0x00020000)
#define EVENT_GPS_DATA			(0x00040000)
#define EVENT_USART_DATA		(0x00080000)

#define EVENT_OV_TIME_ON		(0x00100000)
#define EVENT_OV_TIME_OFF		(0x00200000)
#define EVENT_AT_ERR_ON			(0x00400000)
#define EVENT_AT_ERR_OFF		(0x00800000)

#define EVENT_A7_ON				(0x01000000)
#define EVENT_A7_OFF			(0x02000000)

#define EVENT_WEIGHT			(0x10000000)


extern EVENT TFT_EVENT;
extern EVENT LED_EVENT;
extern int number(char *buf, unsigned char len);
extern void toString(int n, char *str);

#endif

