
#define _MAIN_INIT_

#define _DEBUG_
#define LWFSXX	//�ö���Ϊ����֮���Զ����һ������ĺ��뷢�Ϳ�����Ϣ

#define USE_UART3

#include <stdio.h>
#include <string.h>
#include "lpc17xx.h"
#include "timer.h"
#include "mwork.h"
#include "comm.h"
#include "uart.h"
#include "Work.h"			
#include "Gsm.h"
#include "GPRS.h"
#include "iic.h"
#include "MODBUSM.h"
#include "wxcw.h"

RTCtime timertc;
uint16 IPR_time=0;
uint8 resetMark;
uint8 Mc52i_rst_count;	// ��λ����
uint16 Mc52i_rst_time;	// ��λʱ��
uint8 g_bCandeleteMsg;
uint16 g_deleteMsg_Rst_Count;
uint16 g_deleteCountTick = 0;
uint8 first_connect_count;
extern uint8 g_error_count;

void init_app(void)
{
	funcflag3 = 0;
	funcflag5 = 0;
	mc52igprsflag = 0;
	funcflag7 = 0;
	Mc52igprssetflag = 0;
	Mc52i2gprssetflag = 0;
	mc52iworkflag = 0;
	MC52istaflag = 0;
	Mc52igprsipsetflag = 0;
	Mc52masterNetf = 0;
	Mc52prepareNetf = 0;
	Gsmstatureflag = 0;
	Gsmstature4flag.Word = 0;
	first_connect_count = 0;
	g_error_count = 0;
}

__asm void SystemReset(void)
{
	MOV R0, #1			;
	MSR FAULTMASK, R0	;	// ���FAULTMASK ��ֹһ���жϲ���  
	LDR R0, =0xE000ED0C	;	//  
	LDR R1, =0x05FA0004	;	//    
	STR R1, [R0]        ;	// ϵͳ�����λ      
deadloop      
	B deadloop			;		// ��ѭ��ʹ�������в�������Ĵ���
} 

void restartinit(void )
{
	LPC_SC->PCONP |= (1 << 19);
	//	LPC_WDT->WDTC = 0x200;			// ��ʱʱ�� Twdclk x 0x200 x 4= 1/4000000 x 512 x4 = 
	LPC_WDT->WDTC = 1500000ul;		// ��ʱʱ�� Twdclk x 1500000 x 4= 1.5s 
	LPC_WDT->WDMOD = 2;				// ��λʹ��
	LPC_WDT->WDCLKSEL=0x80000000;	// ѡ���ڲ�RC����
	LPC_WDT->WDMOD |=1;				// ʹ�ܿ��Ź�
	//ι��
	LPC_WDT->WDFEED=0xaa;
	LPC_WDT->WDFEED=0x55;
}

void reset(void)
{
	resetMark = 6;
}

/*****************************************************************************
**   Main Function  main()
*****************************************************************************/
uint8 curindex = 0;
uint32 g_nDealy;
uint32 dTCounter = 0u;
int main (void)
{	
	//mcu init
	dTCounter = 0u;

	SystemInit();
	IOinit();
	init_timer0(TIME_INTERVAL); // 1	
	enable_timer0();
	WDR();
	//
	UART1_Init();
	Comm_Init(1);
	UART2_Init();
	Comm_Init(2);
#ifdef USE_UART3
	UART3_Init();
	Comm_Init(3);
#endif
	Mc52i_init();

	WDR();

	Get_int_data_sub();


//	SmsInit();		//���Žṹ��ʼ��

	teleToalnumbuf = YongfuNumStart;	//��ʼ����ǰҪ���͵ĺ���Ϊ��һ���û�����
	eepbufflag.bit.Diwunworkingf1 = 1;
	MC52iworkstatusbuf = mc52workgsmf;
	resetMark = 0;
	Mc52i_rst_count = 0;
 #ifndef _DEBUG_
	restartinit();	//�������Ź�
 #endif

	g_bCandeleteMsg = 1;
	g_deleteMsg_Rst_Count = 0;

	Get_Init_temp_data();	//��ȡ���µĲ���

	while (1) 
	{
		if(flag_sys_1ms)
		{
			flag_sys_1ms = 0;
			
			WDR();
			sys_timeonems_sub();	//���ݱ�־λȥ������Ӧ�Ĵ���
			GSM_Send_onems_sub(&g_Comm1);
			mc52_work_sub();	// ģ��������������������ʼ����������
			WDR();
			Comm_Module_Run();
		}

		if(flag_sys_100ms)
		{
			WXCW_Run();
#ifdef LWFSXX
			if(Mc52i_CMD_flag1.bit.first_connect)
			{
				first_connect_count++;
				if(first_connect_count > 50)
				{
					first_connect_count = 0;
					Mc52i_CMD_flag1.bit.first_connect = 0;
					gsmsendteststr();
				}
			}
			else
				first_connect_count = 0;

#endif
			if(g_bCandeleteMsg == 0)//��λ��ɾ�����ŵı�־
			{
				g_deleteMsg_Rst_Count++;
				if(g_deleteMsg_Rst_Count > 600)
				{//60�뿪ʼɾ������
					g_bCandeleteMsg = 1;
					g_deleteMsg_Rst_Count = 0;
				}
			}
			g_deleteCountTick++;
			if(g_deleteCountTick >= 3000)//5����
			{
				g_deleteCountTick = 0;
				if(CHKBITSET(mc52iworkflag,mc52gsmworkingf)) //GSMģʽ
				{
					if(g_bCandeleteMsg)//���ܵĻ�,�Ѷ���ȫ��ɾ��,�����ܷ�ʹ��ɾ��ȫ�����ŵ�ָ��
					{
						DeleteOldMsg(&g_Comm1, curindex);
						curindex++;
						if(curindex >= 150)
						{
							curindex = 0;
						}
					}
				}
				else
					Mc52i_init();
			}
	
			flag_sys_100ms = 0;
			WDR();
			eeprom_write_sub();
			WDR();
			tele_write_sub();	
			WDR();
			timertc.rtctime = LPC_RTC->CTIME0;

			//��δ��뿴������������
			if( IPR_time < 60)
				IPR_time++;
			if( IPR_time==69)
				AT_Send_SSYNC_SUB(&g_Comm1);
			
			if(MC52IPOWERstat)	//��Դ�Źر�/�򿪣�Ϩ��/�������
				POWERLEDOFF ;
			else
				POWERLEDON;
			if(CHKBITCLR(mc52iworkflag,mc52gsmworkingf))
			{//���һֱû����
				Mc52i_rst_time++;
				if(Mc52i_rst_time > 6000)	//600��=10����
				{
					Mc52i_rst_time = 0 ;
					Mc52i_rst_count++;
					Mc52i_init();
					if(Mc52i_rst_count >= 3)
					{
						Mc52i_rst_count = 0;
						reset();
					}
				}
			}
			else
			{
				Mc52i_rst_time = 0;
				Mc52i_rst_count = 0;
			}
		}
		/*��λ ����ʱ ��λ 0.1S ��� 25 S  **/
		if(resetMark)
		{//ϵͳ��λ ����ʱ
			resetMark--;
			if(resetMark == 0)
			{
				//����ι���Ա�������λ
				LPC_WDT->WDFEED=0x55;
				LPC_WDT->WDFEED=0xaa;
				SystemReset(); 
			}
		}
	}// while (1) 
}

//���ط��͵��ֽڸ���
uint8 SendStrToBuf( char *dest, const char *src)
{
	char* l_pchar;
	l_pchar = dest;
	while(*src)
	{
		*dest = *src;
		src++;
		dest++;
	}
	return dest - l_pchar;
}

/**************************************************************************
�������ƣ�SumCode
�����汾��1.00 
���ߣ�   hjy
�������ڣ�2014.05.10 
��������˵�������
���������pSrc:Դ����, srclen:Դ���ݳ���
��������:
���������
����ֵ��
***************************************************************************/
uint16 SumCode(const char *pSrc, uint16 srclen)
{
    uint16 sum = 0;
    for (;srclen > 0; srclen--){
        sum += *pSrc;
        pSrc++;
    }
    
    return sum;
}

uint32 CalcTickWidth(uint32 tickcur, uint32 tickold)
{
    if (tickcur >= tickold)
	{
        return (tickcur - tickold);
    }
	else
	{
        return (0xFFFFFFFF - tickold + tickcur);
    }
}

void SleepSec(uint8 s)
{
    uint16 i;
	uint32 llt;
	i = s;
	i *= 1000;
	llt = dTCounter;
	WDR();
	while(1)
	{
        WDR();
		if(CalcTickWidth(dTCounter, llt) >= i)
			break;
	}
}

void SleepMSec(uint8 ms)
{
    uint16 i;
	uint32 llt;
	i = ms;
	llt = dTCounter;
	WDR();
	while(1)
	{
        WDR();
		if(CalcTickWidth(dTCounter, llt) >= i)
			break;
	}
}

/*****************************************************************************
**                            End Of File
******************************************************************************/
