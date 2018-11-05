/*****************************************
*****************************************/
#include "define.h"

#ifndef _gprs_H_
#define _gprs_H_

extern char sSCA[];		//�������ĺ��룺��8613800755500
extern uint16 MC52reSETTIMEBUF;



extern uint8  GsmOKfunbuf;
extern uint16 GsmEorrefunbuf;
extern uint8  Gsmmodeselect;   //GSM����ģʽ��ѡ��0ΪTEXTģʽ��1Ϊ����ģʽ
extern uint8  MC52iworkstatusbuf; //����ģʽ

extern uint8  Gprsststuretypebuf;
extern uint8  Gprserrnumbuf;
extern uint16 Gprssendatdelaytime;
extern uint16 Gprstesttimedelaybuf;
extern uint16 Gprstesttime2delaybuf;
extern uint16 Gprsbaohutimebuf;

extern uint16 mc52itimebuf ; //�������ͻ������ݸ�����Ķ�ʱ���á�
extern uint16 mc52gprseerobuf ;
/*********************************************/
//����Ϊ���ֱ�־λ

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

