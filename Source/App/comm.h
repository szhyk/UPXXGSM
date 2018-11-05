
#ifndef __COMM_H
#define __COMM_H
#include "Define.h"


extern COM_STRUCT g_Comm1,g_Comm2,g_Comm3;


/*»Ø»·ºê*/
#define CIRCLE_INDEX(index, fifolen) {if (index >= fifolen){index = index%fifolen;}}


void Comm_Init(uint8 commId);
uint32 CalcTickWidth(uint32 tickcur, uint32 tickold);

uint16 SumCode(const char *pSrc, uint16 srclen);
void AddMsgGroup(const char *src ,uint8 groupno);
void AddUCS2Group(uint8 *src, uint16 len ,uint8 groupno,uint8 ucs);
void Comm_Module_Run(void);

#endif		//#ifndef __COMM_H


