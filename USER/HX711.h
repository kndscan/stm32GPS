#ifndef __HX711_H
#define __HX711_H

#include "sys.h"
#include "common.h"

#define HX711_SCK 		PAout(3)	// PA3
#define HX711_DOUT 		PAin(2)		// PA2

u32 HX711_ReadB(void);

extern void Init_HX711pin(void);
extern u32 HX711_Read(void);
extern void Get_Maopi(void);
extern void Get_Weight(void);

extern u32 HX711_Buffer;
extern u32 Weight_Maopi;
extern s32 Weight_Shiwu;
extern u8 Flag_Error;
extern char MaoPiBuf[INT_DIGIT_NUM];
extern char ShiWuBuf[INT_DIGIT_NUM];

#endif

