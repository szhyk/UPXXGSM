
#ifndef _UCS2_H
#define _UCS2_H

#define MAX_SMS_SIZE					(140)

int Str2UCS2(const char *pStr, char *pUCS2);
int UCS2ToStr(const char *pUCS2, char *pStr, int iLen);


#endif

