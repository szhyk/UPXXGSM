/*****************************************
*****************************************/
#include "define.h"

#ifndef _gprs_H_
#define _gprs_H_

extern char sSCA[];		//短信中心号码：如8613800755500
extern uint16 MC52reSETTIMEBUF;



extern uint8  GsmOKfunbuf;
extern uint16 GsmEorrefunbuf;
extern uint8  Gsmmodeselect;   //GSM短信模式的选择，0为TEXT模式，1为中文模式
extern uint8  MC52iworkstatusbuf; //工作模式

extern uint8  Gprsststuretypebuf;
extern uint8  Gprserrnumbuf;
extern uint16 Gprssendatdelaytime;
extern uint16 Gprstesttimedelaybuf;
extern uint16 Gprstesttime2delaybuf;
extern uint16 Gprsbaohutimebuf;

extern uint16 mc52itimebuf ; //用来发送缓冲数据给网络的定时器用。
extern uint16 mc52gprseerobuf ;
/*********************************************/
//以下为各种标志位

extern uint8 funcflag3;
extern uint8 funcflag5;
extern uint8 mc52igprsflag;
extern uint8 funcflag7;
extern uint8 Mc52igprssetflag;
extern uint8 Mc52i2gprssetflag;

extern uint8 mc52iworkflag;
extern uint8 MC52istaflag;
extern uint8 Mc52igprsipsetflag;
extern uint8 Mc52masterNetf;
extern uint8 Mc52prepareNetf;
extern uint8 Gsmstatureflag;


void GPRS_AT_ATH_SUB(COM_STRUCT *pComm);
void GPRS_A_W_SUB(COM_STRUCT *pComm);
void AT_Send_SSYNC_SUB(COM_STRUCT *pComm);

void GSM_RX_Commun_SUB(COM_STRUCT *pCommKZQ,COM_STRUCT *pCommGSM);
void mc52_work_sub(void);

#endif

