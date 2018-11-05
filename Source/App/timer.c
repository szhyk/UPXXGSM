#include "lpc17xx.h"
#include "timer.h"

extern uint32 dTCounter;

uint8 flag_sys_1ms,flag_sys_10ms,flag_sys_100ms;
static uint8  time1msbuf;
static uint8  time10msbuf;

clk_structure g_CLK;
const uint8 month12[]={31,31,28,31,30,31,30,31,31,30,31,30,31};
uint8 CheckTime(uint8 *uiTp)
{
	uint8 monthday;
	
	// 月
	if(uiTp[5]>12 || uiTp[5]==0)
		return 0;
	// 日
	if(uiTp[4]==0)
		return 0;
	// 时
	if(uiTp[2]>23)
		return 0;
	// 分
	if(uiTp[1]>59)
		return 0;
	// 秒
	if(uiTp[0]>59)
		return 0;
	
	// 年月日判断
	monthday = month12[uiTp[5]];
	if(uiTp[5]==2)
	{
		if( (uiTp[6]%4==0 && uiTp[6]%100!=0) || uiTp[6] %400==0)   //year%100!=0;
		{// 闰年
			monthday++;
		}
	}
	if( uiTp[5] > monthday)
	{
		return 0;
	}
	
	return 1;
}

//给定日期算出星期几
//y--year > 1900
//m--month 1-12
//d--day 1-31
int GetWeek(int y,   int m,   int d)
{
	int w;
	y += 2000;
	if(m<3)
	{   
		m += 12;
		--y;   
	}   
	w = (d+1+2*m+3*(m+1)/5+y+(y>>2)-y/100+y/400) % 7;
	return w;
}

uint8 i3;
void TIMER0_IRQHandler (void) 
{  
	LPC_TIM0->IR = 1;			/* clear interrupt flag */

	flag_sys_1ms = 1;
	time1msbuf++;
	dTCounter++;
	if(time1msbuf>=100)
	{
		time1msbuf=0;
		flag_sys_100ms = 1;
	}
	time10msbuf++;
	if(time10msbuf>=10)
	{
		time10msbuf=0;
	}
	g_CLK.MS++;
	if(g_CLK.MS > 999)
	{
		g_CLK.MS = 0;
	g_CLK.SEC++;
	if(g_CLK.SEC > 59)
	{//秒
		g_CLK.SEC = 0;
		g_CLK.MIN++;
		if(g_CLK.MIN > 59)
		{//分
			g_CLK.MIN=0;
			if(	g_CLK.HOUR < 23)
			{
				g_CLK.HOUR++;   //hour
			}
			else
			{
				g_CLK.HOUR = 0;   // hour
				//星期
				if(g_CLK.wDAY==7)
					g_CLK.wDAY = 1;
				else
					g_CLK.wDAY++;
				
				// 判断当月应该有的最大天数
				if( g_CLK.MON>12 || g_CLK.MON<1)
					g_CLK.MON = 1;
				
				i3 = month12[g_CLK.MON];	// 取得该月的天数
				if( g_CLK.MON==2 && g_CLK.DAY==28 )
				{// 闰年，二月要加一天
					if( g_CLK.YEAR%4==0 )
						i3++;
				}
				
				if( g_CLK.DAY < i3)
				{
					g_CLK.DAY++;    // day
				}
				else
				{
					g_CLK.DAY = 1;	// day
					
					if(g_CLK.MON < 12)
					{
						g_CLK.MON++;  // month
					}
					else
					{
						g_CLK.MON = 1;	// month
						
						g_CLK.YEAR++;  //year
					}
				}
			}
		}
		}
	}
}

void enable_timer0()
{
	LPC_TIM0->TCR = 1;
}

void disable_timer0()
{
	LPC_TIM0->TCR = 0;
}

void init_timer0 ( uint32 TimerInterval ) 
{
	LPC_TIM0->IR=1;
	LPC_TIM0->TCR=0;
	LPC_TIM0->TC=0;
	LPC_TIM0->PR=0;
	LPC_TIM0->MR0=FOSC/1000-1;
	LPC_TIM0->MCR=0x3;
	NVIC_EnableIRQ(TIMER0_IRQn);
}

void IOinit(void)
{
	LPC_GPIO0->FIODIR|= 0x10700000;
	
	LPC_GPIO1->FIODIR = 0xa0000000;
	LPC_GPIO2->FIODIR = 0x00001301;
}


/******************************************************************************
**                            End Of File
******************************************************************************/
