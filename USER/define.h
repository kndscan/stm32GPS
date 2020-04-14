#ifndef __DEFINE_H__
#define __DEFINE_H__

#define ARRAY_LEN(x)	(sizeof(x)/sizeof(x[0]))
#define toChar(x)		((char)(x+0x30))
#define toUChar(x)		((unsigned char)(x-0x30))
#define toShort(h, l)	((unsigned short)((h<<8)|(l)))
#define toUShort(h, l)	((short)((h<<8)|(l)))

#endif

