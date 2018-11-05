#include "lpc17xx.h"                              /* LPC17xx definitions    */
#include "GPRS.h"
#include "Gsm.h"
#include "uart.h"
#include "iic.h"
#include "comm.h"
#include "gsmcomm.h"
#include "ucs2.h"
#include "work.h"

static uint8  DataBuf[UART0TXSETCounter][datamaxlen];

void Comm_Init(uint8 commId)
{
	COM_STRUCT *pComm;
	int i;

	if(commId==1)
	{
		pComm = &g_Comm1;
	}
	else if(commId==2)
	{
		pComm = &g_Comm2;
	}
	else if(commId==3)
	{
		pComm = &g_Comm3;
	}
	else
		return;

	pComm->COMM_ID = commId;
	pComm->funcflag = 0;
	pComm->RXIndex = 0;
	pComm->RxOut= 0;
	pComm->RXtime = 0;
	pComm->SendIndex = 0;
	pComm->Senddatanum = 0;
	pComm->TXBUFC_flag = 0;
	pComm->TXBUFNumber = 0;
	pComm->TXCounter = 0;
	pComm->TXIndex = 0;
	pComm->TxOut = 0;
	for(i=0; i<UART0TXSETCounter; i++)
		pComm->TxDataBuf[i] = &DataBuf[i][0];
}


uint8 Find_Str_Hz(uint8 *pStr)
{
	
	while(*pStr != '\0')
	{
        if (*pStr++ > 0x80)
		{
			return 1;
			
		}
	}
	return 0;
}

void AddUCS2Group(uint8 *src, uint16 len ,uint8 groupno,uint8 ucs)
{
	uint8 i;
	COM_STRUCT *pComm = &g_Comm1;

	pComm->TXTELBufNUM[pComm->TXBUFNumber] = groupno;//���ظ��绰�ĺ���
	pComm->TXBufUcs[pComm->TXBUFNumber] = ucs;

	for(i=0; i<len; i++)
	{
		pComm->TxDataBuf[pComm->TXBUFNumber][i] = src[i]; 
	}
	pComm->TXBufLen[pComm->TXBUFNumber] = len;
	UART_com_sub(pComm);
}

//������Ϣ����
void AddMsgGroup(const char *src ,uint8 groupno)
{
	uint16 j;
	COM_STRUCT *pComm = &g_Comm1;
	char *pPdubuf = (char *)&pComm->TxDataBuf[pComm->TXBUFNumber][0];
	
	//������Ϣ
	j = SendStrToBuf(pPdubuf, src);
	pPdubuf[j] = '\0';	//0
	if(Find_Str_Hz((uint8 *)pPdubuf))
	{//�к���,UCS2����
		j = Str2UCS2(pPdubuf,sCurSMS);
		//copy�����ͻ���
		AddUCS2Group((uint8 *)sCurSMS,j,groupno,1);
	}
	else
	{//ȫ��Ϊ�ַ�,7λ����
		j = gsmEncode7bit(pPdubuf,sCurSMS,j);
		//copy�����ͻ���
		AddUCS2Group((uint8 *)sCurSMS,j,groupno,0);
	}
	
}

void Comm_Module_Run()
{
	WDR();
	Uart_Rece_Time_sub(&g_Comm1);
	Uart1RXData();		//�����ģ��ͨ��
	UART1_txbuf_sub();	// �������ݵ�����ģ��
	WDR();
	Uart_Rece_Time_sub(&g_Comm2);
	Uart2RXData();		//��ɼ���ͨ��
	WDR();
	Uart_Rece_Time_sub(&g_Comm3);
	Uart3RXData();		//��ɼ���ͨ��
	WDR();
}

