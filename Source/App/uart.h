/****************************************Copyright (c)****************************************************
**                                 http://www.PowerAVR.com
**								   http://www.PowerMCU.com
**--------------File Info---------------------------------------------------------------------------------
** File name:           uart.h
** Last modified Date:  2010-05-12
** Last Version:        V1.00
** Descriptions:        
**
**--------------------------------------------------------------------------------------------------------
** Created by:          PowerAVR
** Created date:        2010-05-10
** Version:             V1.00
** Descriptions:        ±àÐ´Ê¾Àý´úÂë
**
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/
#ifndef __UART_H
#define __UART_H
#include "Define.h"


void UART0_Init (void);
void UART1_Init (void);
void UART2_Init (void);
void UART3_Init (void);
//////////////////////////////////////////

void Uart_Rece_Time_sub(COM_STRUCT *pComm);
void Uart_Send_start(COM_STRUCT *pComm);

void Uart1RXData(void);
void Uart2RXData(void);
void Uart3RXData(void);

void UART1_txbuf_sub(void);
void UART2_txbuf_sub(void);
void UART3_txbuf_sub(void);

void UART2_232test_sub(COM_STRUCT *pComm);


void WriteUart1(const char *buf, uint16 len);
uint16 Uart1RxbufDataLen(void);
uint16 ReadUart1(char *buf, uint16 len);

#endif
