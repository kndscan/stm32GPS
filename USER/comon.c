
#include "common.h"

EVENT TFT_EVENT;
EVENT LED_EVENT;
EVENT MONITOR_EVENT;

int number(char *buf, unsigned char len)
{
	unsigned char i, j, k;
	if(0 == buf || len == 0)		return -1;
	j = 0;
	for(i=0; i<len; i++)
	{
		if(buf[i] < 0x30 || buf[i] > 0x39)
			break;
		j++;
	}
	if(j==0)	return -1;

	k = 0;
	for(i=0; i<j; i++)
	{
		k += toUChar(buf[i])*pow(10, (j-i-1));
	}
	return k;
}

void toString(int n, char *str)
{
	int i,j,sign;
	char s[12];
	if((sign=n) <0 )
		n=-n;
	i=0;
	s[i++] = '\0';
	do
	{
		s[i++] = n%10+'0';
	}while ((n/=10)>0);
	
	if(sign<0)
		s[i++]='-';
	i--;
	for(j=i; j>=0; j--)
	{
		str[i-j] = s[j];
		//*str++ = s[j];
	}
	//str[i] = '\0';
}



