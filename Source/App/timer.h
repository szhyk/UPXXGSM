/****************************************Copyright (c)****************************************************
**                                 http://www.PowerAVR.com
**								   http://www.PowerMCU.com
**--------------File Info---------------------------------------------------------------------------------
** File name:           timer.h
** Last modified Date:  2010-05-12
** Last Version:        V1.00
** Descriptions:        The main() function example template
**
**--------------------------------------------------------------------------------------------------------
** Created by:          PowerAVR
** Created date:        2010-05-10
** Version:             V1.00
** Descriptions:        ±àÐ´Ê¾Àý´úÂë
**
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/
#ifndef __TIMER_H 
#define __TIMER_H

	
#define TIME_INTERVAL	(9000000/100 - 1)
#include "Define.h"
extern void IOinit(void);
extern void init_timer0(  uint32 timerInterval );
extern void enable_timer0(void );
extern void disable_timer0( void);
extern void TIMER0_IRQHandler (void);

typedef struct
{		  
	uint16 MS;		//0-999
	uint8 SEC;		//0-59
	uint8 MIN;		//0-59
	uint8 HOUR;	//0-59
	uint8 DAY;		//1-31
	uint8 wDAY;	//1-7
	uint8 MON;		//1-12
	uint8 YEAR;	//0-99
}clk_structure;
extern clk_structure g_CLK;
int GetWeek(int y, int m, int d);

#endif /* end __TIMER_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
