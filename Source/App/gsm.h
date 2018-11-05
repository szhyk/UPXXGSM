/************************************************

*************************************************/


#ifndef _gsm_H_
#define _gsm_H_

#include "define.h"

/**********************************/
enum{
	msgSETED = 0,	//参数已设置
	msgBHERR,	//事件编号错误
	msgYKERR,	//操作超时
	msgSETERR,	//参数错误
	msgNumERR,	//号码错误
	msgNRERR,	//权限不足错误
	msgXTFW,	//系统复位
	msgErrNum
};

typedef struct
{
	uint8 buffer[datamaxlen];
	uint8 len;
}GSM_SEND_DATA_STRUCT;
extern GSM_SEND_DATA_STRUCT gsmSendData;

void RX_gsmwork_SUB(COM_STRUCT *pCommKZQ,COM_STRUCT *pCommGSM);

void GSM_Send_num_sub(COM_STRUCT *pComm,uint8 num);
void GSM_Send_onems_sub(COM_STRUCT *pComm);
void DeleteOldMsg(COM_STRUCT *pComm,uint8 msgNo);

void UART_com_sub(COM_STRUCT *pComm);

void gsmsendteststr(void);

extern char sCurSMS[700];
extern uint8  GsmEORRcfnumbuf;
extern uint16 GsmEORRtimebuf;
extern uint8  Soesearchnumbuf;

#endif		//#ifndef _gsm_H_


