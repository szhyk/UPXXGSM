#include "lpc17xx.h"                              /* LPC17xx definitions    */
#include "GPRS.h"
#include "Gsm.h"
#include "comm.h"
#include "uart.h"
#include "iic.h"
#include "ModbusM.h"
#include "configuration.h"
#include "wxcw.h"
#include "work.h"


#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

COM_STRUCT g_Comm1;
COM_STRUCT g_Comm2;
COM_STRUCT g_Comm3;


void UART0_Init (void)
{
	uint16_t usFdiv;

 	LPC_SC->PCONP |= (1<<3);               //��UART0���ʡ�ʱ�ӿ���λ  
	
	LPC_PINCON->PINSEL0 |= (1 << 4);             // Pin P0.2 used as TXD0 (Com0)
  LPC_PINCON->PINSEL0 |= (1 << 6);             // Pin P0.3 used as RXD0 (Com0)
	//ʹ�ܷ��ʳ��������� ���趨������  
	LPC_UART0->LCR  = 0x83;                      // ���ô������ݸ�ʽ��8λ�ַ����ȣ�1��ֹͣλ����У�飬
    usFdiv = (FPCLK/16)/ UART0_BPS;              // ���ò�����
    LPC_UART0->DLM  = usFdiv / 256;
    LPC_UART0->DLL  = usFdiv % 256; 
    LPC_UART0->LCR  = 0x03;                      // ��ֹ���ʳ���������������������

//	LPC_UART0->FCR  = 0x87;						 // ����UARTn RX��TX FIFO�Ĳ�������������8���ֽڴ����ж� 
	LPC_UART0->FCR  = 0x07;						 // ����UARTn RX��TX FIFO�Ĳ�������������һ���ֽڴ����ж� 
	
	NVIC_EnableIRQ(UART0_IRQn);					// enable irq in nvic 
    LPC_UART0->IER = 0x07;						// ʹ�ܽ����жϣ������жϣ�RX���ж�
}

//LPC1768��UART1����MC52i��Uart0,�����ʱ����Ӧ����.
void UART1_Init (void)
{
	uint16 usFdiv;
	LPC_SC->PCONP |= (1<<4);               //��UART1���ʡ�ʱ�ӿ���λ  
	LPC_PINCON->PINSEL0 |= (1 << 30);    //ѡ��P0.15ΪTXD1
	LPC_PINCON->PINSEL1 |= 0x1;          //ѡ��P0.16ΪRXD1
	//ʹ�ܷ��ʳ��������� ���趨������  
	LPC_UART1->LCR  = 0x83;                      // ���ô������ݸ�ʽ��8λ�ַ����ȣ�1��ֹͣλ����У��
	usFdiv = (FPCLK/16)/ UART0_BPS;              // ���ò�����,MC52i��Uart0Ĭ�ϲ�����Ϊ9600
	LPC_UART1->DLM  = usFdiv / 256; 
	LPC_UART1->DLL  = usFdiv % 256; 
	LPC_UART1->LCR  = 0x03;					// ��ֹ���ʳ��������������������� 
//	LPC_UART1->FCR = 0x87;					// ����UARTn RX��TX FIFO�Ĳ�������������8���ֽڴ����ж� 
	LPC_UART1->FCR  = 0x07;				// ����UARTn RX��TX FIFO�Ĳ�������������һ���ֽڴ����ж� 
	NVIC_EnableIRQ(UART1_IRQn);				// enable irq in nvic
	LPC_UART1->IER = 0x07;                    //ʹ�ܽ����жϣ������жϣ�RX���ж�
}

void UART2_Init (void)
{
	uint16_t usFdiv;
	LPC_SC->PCONP |=(1<<24); //uart2 power on
	
	LPC_PINCON->PINSEL0 |= (1 << 20);   // pin P0.10 used as TXD2
	LPC_PINCON->PINSEL0 |= (1 << 22);   // pin P0.11 used as RXD2
	//ʹ�ܷ��ʳ��������� ���趨������  
//	LPC_UART2->LCR  = 0x83;				// ���ô������ݸ�ʽ��8λ�ַ����ȣ�1��ֹͣλ����У�飬
	LPC_UART2->LCR  = 0x9B;				// ���ô������ݸ�ʽ��8λ�ַ����ȣ�1��ֹͣλ��żУ�飬8E1
    usFdiv = (FPCLK/16)/ UART0_BPS;     // ���ò�����
    LPC_UART2->DLM  = usFdiv / 256;
    LPC_UART2->DLL  = usFdiv % 256; 
 //  LPC_UART2->LCR  = 0x03;             // ��ֹ���ʳ��������������������� 
    LPC_UART2->LCR  &= 0x7F;             // ��ֹ���ʳ��������������������� 
	//	LPC_UART2->FCR  = 0x87;				// ����UARTn RX��TX FIFO�Ĳ�������������8���ֽڴ����ж�
	//	LPC_UART2->FCR  = 0x47;				// ����UARTn RX��TX FIFO�Ĳ�������������4���ֽڴ����ж� 
	LPC_UART2->FCR  = 0x07;				// ����UARTn RX��TX FIFO�Ĳ�������������1���ֽڴ����ж� ,��Ϊkeil�°汾��lpc17xx.h��û����FIFOLVL���ԣ�ֻ����1���ֽ�ȥ�����ж���
	NVIC_EnableIRQ(UART2_IRQn);		   // enable irq in nvic 
	LPC_UART2->IER=0x07;			   // ʹ�ܽ����жϣ������жϣ�RX���ж�
}


void UART3_Init (void)
{
	uint16_t usFdiv;
	
	LPC_SC->PCONP |=(1<<25); //uart3 power on
 	LPC_PINCON->PINSEL9 |= (3 << 24);   // pin P4.28 used as TXD3
 	LPC_PINCON->PINSEL9 |= (3 << 26);   // pin P4.29 used as RXD3
//    LPC_PINCON->PINSEL0 |= (2 << 0);             // Pin P0.2 used as TXD3
//    LPC_PINCON->PINSEL0 |= (2 << 2);             // Pin P0.3 used as RXD3
	//ʹ�ܷ��ʳ��������� ���趨������  
// 	LPC_UART3->LCR  = 0x83;				// ���ô������ݸ�ʽ��8λ�ַ����ȣ�1��ֹͣλ����У�飬8N1
// 	LPC_UART3->LCR  = 0x87;				// ���ô������ݸ�ʽ��8λ�ַ����ȣ�2��ֹͣλ����У�飬8N2
// 	LPC_UART3->LCR  = 0x8B;				// ���ô������ݸ�ʽ��8λ�ַ����ȣ�1��ֹͣλ����У�飬8O1
	LPC_UART3->LCR  = 0x9B;				// ���ô������ݸ�ʽ��8λ�ַ����ȣ�1��ֹͣλ��żУ�飬8E1
    usFdiv = (FPCLK/16)/ UART0_BPS;     // ���ò�����
    LPC_UART3->DLM  = usFdiv / 256;
    LPC_UART3->DLL  = usFdiv % 256; 
//    LPC_UART3->LCR  = 0x03;             // ��ֹ���ʳ��������������������� 
    LPC_UART3->LCR  &= 0x7F;             // ��ֹ���ʳ��������������������� 
	//	LPC_UART2->FCR  = 0x87;				// ����UARTn RX��TX FIFO�Ĳ�������������8���ֽڴ����ж�
	//	LPC_UART2->FCR  = 0x47;				// ����UARTn RX��TX FIFO�Ĳ�������������4���ֽڴ����ж� 
	LPC_UART3->FCR  = 0x07;				// ����UARTn RX��TX FIFO�Ĳ�������������1���ֽڴ����ж� ,��Ϊkeil�°汾��lpc17xx.h��û����FIFOLVL���ԣ�ֻ����1���ֽ�ȥ�����ж���
	NVIC_EnableIRQ(UART3_IRQn);		   // enable irq in nvic 
	LPC_UART3->IER = 0x07;			   // ʹ�ܽ����жϣ������жϣ�RX���ж�

}

//Uart0δʹ��
void UART0_IRQHandler (void)
{
    uint8 temp,i,max;	
	COM_STRUCT *pComm= &g_Comm2;
   	temp=  LPC_UART0->IIR;

	switch(temp&0xf)
	{
	case 6:
		temp=  LPC_UART0->LSR; 
		break;
	case 0x04:	
		pComm->RXtime=100;
		SETBIT(pComm->funcflag,uartdelayf);
		pComm->RXdata[pComm->RXIndex++] = LPC_UART0->RBR;;
		
		break; 
	case 0x0c:
	       SETBIT(pComm->funcflag,uartdelayf);
// 		   max = LPC_UART0->FIFOLVL&0xf;
           pComm->RXtime = 100;//20ms
//           for(i=0; i<max; i++)
			   pComm->RXdata[pComm->RXIndex++] = LPC_UART0->RBR;;
		   break;
	case 0x2: 
		
		if(CHKBITSET(pComm->funcflag,uartsenddata))
		{
			max=0;
			if(pComm->SendIndex < pComm->Senddatanum)
			{   
				if(pComm->Senddatanum-pComm->SendIndex > 8)
				{
					max = 8;
				}
				else
				{
					max = pComm->Senddatanum-pComm->SendIndex;
				}
				for(i=0; i<max; i++)
				{  
					LPC_UART0->THR = pComm->TXdata[pComm->SendIndex+i];
				}
				pComm->SendIndex += max;
			}
			else 
			{
				for(i=0; i<=pComm->Senddatanum; i++)
				{
					pComm->TXdata[i] = 0;
				}
				pComm->SendIndex = 0;
				pComm->Senddatanum = 0;
				CLRBIT(pComm->funcflag,uartsenddata);//��������
			}
		}
		break;	
	}	
}

//����ģ��mc52i��MCUͨ��
void UART1_IRQHandler(void)
{
	uint8 i,max;	
    uint16 iir;
	COM_STRUCT *pComm = &g_Comm1;
	
    /* read IIR and clear it */
    iir = LPC_UART1->IIR;
    iir >>= 1;			    /* skip pending bit in IIR */
    iir &= 0x07;			/* check bit 1~3, interrupt identification */

	switch(iir)
	{
	case IIR_RLS:	//Recieve Line Status
		iir =  LPC_UART1->LSR;  /* Receive Line Status */ 
		if ( iir & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
		{     /* There are errors or break interrupt */
			/* Read LSR will clear the interrupt */
			iir = LPC_UART1->RBR;  /* Dummy read on RX to clear interrupt, then bail out */    
			return;
		}
		if ( iir & LSR_RDR ) /* Receive Data Ready */
    {     /* If no error on RLS, normal ready, save into the data buffer. */
    /* Note: read RBR will clear the interrupt */
		{
			if(pComm->RXIndex < datamaxlen-1)
				pComm->RXdata[pComm->RXIndex++] = LPC_UART1->RBR;
			else
				iir = LPC_UART1->RBR;
		}
		SETBIT(pComm->funcflag,uartdelayf);
		pComm->RXtime = 200;
		} 
		break;
	case IIR_RDA:
		//FIFO�����ж�
//		for(i=0; i<7; i++)
		{
			if(pComm->RXIndex < datamaxlen-1)
				pComm->RXdata[pComm->RXIndex++] = LPC_UART1->RBR;
			else
				iir = LPC_UART1->RBR;
		}
		SETBIT(pComm->funcflag,uartdelayf);
		pComm->RXtime = 200;
		break; 
	case IIR_CTI:
		SETBIT(pComm->funcflag,uartdelayf);
// 		max = LPC_UART1->FIFOLVL&0xf;
		pComm->RXtime = 200;
//		for(i=0; i<max; i++)
		{
			if(pComm->RXIndex < datamaxlen-1)
				pComm->RXdata[pComm->RXIndex++] = LPC_UART1->RBR;
			else
				iir = LPC_UART1->RBR;
		}
		break;
	case IIR_THRE: 
		if(CHKBITSET(pComm->funcflag,uartsenddata))
		{	
			max = 0;
			if(pComm->SendIndex < pComm->Senddatanum)
			{   
				if(pComm->Senddatanum-pComm->SendIndex > 8)
				{//һ����෢��8���ֽ�
					max = 8;
				}
				else
				{
					max = pComm->Senddatanum-pComm->SendIndex;
				}
				for(i=0; i<max; i++)
				{  
					LPC_UART1->THR = pComm->TXdata[pComm->SendIndex++];
				}
			}
			else 
			{
				for(i=0; i<=pComm->Senddatanum; i++)
				{
					pComm->TXdata[i] = 0;
				}
				pComm->SendIndex = 0; 
				pComm->Senddatanum = 0;
				CLRBIT(pComm->funcflag,uartsenddata);	//	��������
			}
		}
		break;	
	}
}


//MCU��������ͨ��
void UART2_IRQHandler (void)
{
    uint8 i,max;	
    uint8 iir;
	COM_STRUCT *pComm= &g_Comm2;

    /* read IIR and clear it */
    iir = LPC_UART2->IIR;
    iir >>= 1;			    /* skip pending bit in IIR */
    iir &= 0x07;			/* check bit 1~3, interrupt identification */

	switch(iir)
	{
	case IIR_RLS:
		//�������ж� RLS
		iir = LPC_UART2->LSR; 
		break;
	case IIR_RDA:
		//�������ݿ��� RDA��RDA�жϴ���Ϊ8���ֽڣ���ȡ7���ֽڣ���һ���ֽ��Ա㴥��CTI�ж�
//		for(i=0; i<7; i++)
		{
			if(pComm->RXIndex < datamaxlen-1)
				pComm->RXdata[pComm->RXIndex++] = LPC_UART2->RBR;
			else
				iir = LPC_UART2->RBR;
		}
       SETBIT(pComm->funcflag,uartdelayf);
		pComm->RXtime = 100;//20ms
		break; 
	case IIR_CTI:
		//�ַ���ʱָʾ CTI
	       SETBIT(pComm->funcflag,uartdelayf);
           pComm->RXtime = 100;//20ms
// 		   max = LPC_UART2->FIFOLVL&0xf;	//rx
//            for(i=0; i<max; i++)
		   {
			   if(pComm->RXIndex < datamaxlen-1)
				   pComm->RXdata[pComm->RXIndex++] = LPC_UART2->RBR;
			   else
				   iir = LPC_UART2->RBR;
		   }
		   break;
	case IIR_THRE:
		//THRE �ж�
		if(CHKBITSET(pComm->funcflag,uartsenddata))
		{
			max = 0;
			if(pComm->SendIndex < pComm->Senddatanum)
			{   
				if(pComm->Senddatanum-pComm->SendIndex > 8)
				{
					max = 8;
				}
				else
				{
					max = pComm->Senddatanum-pComm->SendIndex;
				}
				for(i=0; i<max; i++)
				{  
					LPC_UART2->THR = pComm->TXdata[pComm->SendIndex++];
				}
			}
			else 
			{
// 				for(i=0; i<=pComm->Senddatanum; i++)
// 				{
// 					pComm->TXdata[i] = 0;
// 				}
				pComm->SendIndex = 0;
				pComm->Senddatanum = 0;
				CLRBIT(pComm->funcflag,uartsenddata);	//	��������
			}
		}
		break;	
	}
}

void UART3_IRQHandler (void)
{
    uint8 i,max;	
    uint8 iir;
	COM_STRUCT *pComm= &g_Comm3;
	
    /* read IIR and clear it */
    iir = LPC_UART3->IIR;
    iir >>= 1;			    /* skip pending bit in IIR */
    iir &= 0x07;			/* check bit 1~3, interrupt identification */
	
	switch(iir)
	{
	case IIR_RLS:
		//�������ж� RLS
		iir = LPC_UART3->LSR; 
		break;
	case IIR_RDA:
		//�������ݿ��� RDA��RDA�жϴ���Ϊ8���ֽڣ���ȡ7���ֽڣ���һ���ֽ��Ա㴥��CTI�ж�
		//		for(i=0; i<7; i++)
		{
			if(pComm->RXIndex < datamaxlen-1)
				pComm->RXdata[pComm->RXIndex++] = LPC_UART3->RBR;
			else
				iir = LPC_UART3->RBR;
		}
		SETBIT(pComm->funcflag,uartdelayf);
		pComm->RXtime = 100;//20ms
		break; 
	case IIR_CTI:
		//�ַ���ʱָʾ CTI
	       SETBIT(pComm->funcflag,uartdelayf);
           pComm->RXtime = 100;//20ms
		   // 		   max = LPC_UART2->FIFOLVL&0xf;	//rx
		   //            for(i=0; i<max; i++)
		   {
			   if(pComm->RXIndex < datamaxlen-1)
				   pComm->RXdata[pComm->RXIndex++] = LPC_UART3->RBR;
			   else
				   iir = LPC_UART3->RBR;
		   }
		   break;
	case IIR_THRE:
		//THRE �ж�
		if(CHKBITSET(pComm->funcflag,uartsenddata))
		{
			max = 0;
			if(pComm->SendIndex < pComm->Senddatanum)
			{   
				if(pComm->Senddatanum-pComm->SendIndex > 8)
				{
					max = 8;
				}
				else
				{
					max = pComm->Senddatanum-pComm->SendIndex;
				}
				for(i=0; i<max; i++)
				{  
					LPC_UART3->THR = pComm->TXdata[pComm->SendIndex++];
				}
			}
			else 
			{
// 				for(i=0; i<=pComm->Senddatanum; i++)
// 				{
// 					pComm->TXdata[i] = 0;
// 				}
				pComm->SendIndex = 0;
				pComm->Senddatanum = 0;
				CLRBIT(pComm->funcflag,uartsenddata);	//	��������
			}
		}
		break;	
	}
}

//�����������ⷢ��
void Uart_Send_start(COM_STRUCT *pComm)
{
	SETBIT(pComm->funcflag,uartsenddata);	//�趨�������ݱ�־
	if(pComm->COMM_ID==1)
		LPC_UART1->THR = pComm->TXdata[0];
	else if(pComm->COMM_ID==2)
		LPC_UART2->THR = pComm->TXdata[0];
	else if(pComm->COMM_ID==3)
		LPC_UART3->THR = pComm->TXdata[0];
	
	pComm->SendIndex = 1;	// �ѷ���һ������
	Nodatatratbuf = 0;
}

void UART1_txbuf_sub(void)
{
	uint8 i=0;
	COM_STRUCT *pComm = &g_Comm1;

	if(CHKBITSET(pComm->funcflag,uartsenddata))
	{
		mc52itimebuf=0;
		return;//�ڷ����ݵ�ʱ���ܽ���
	}
// 	if(Gprsworkingflag>0)
// 	{//
// 		mc52itimebuf=0;
// 		return;//�ڷ����ݵ�ʱ���ܽ���
// 	}
	if(CHKBITSET(funcflag5,mc52igprsdatasendf))
	{
		mc52itimebuf=0;
		return;//���ݻ�û���ǽ�������
	}
	if(CHKBITSET(mc52iworkflag,mc52sendbaohuf))
	{
		return;//���ݻ�û���ǽ�������
	}
	if(CHKBITSET(mc52iworkflag,mc52gsmsendbaohuf))
	{
		return;//GSM���ݻ�û���ǽ�������
	}
	/******************************/
	mc52itimebuf++;
	if(mc52itimebuf > Uart0sendjiangetime)
	{//ÿ��200ms����һ��
		mc52itimebuf = 0;
		if ((pComm->TXBUFNumber>0)||(pComm->TXBUFC_flag==1))
		{//���ͻ����������ݣ�����������Ҫ����
			if (pComm->TXCounter >= pComm->TXBUFNumber)
			{//�ѷ������ݻ��߻���ʱ
				if(pComm->TXBUFC_flag==1)
				{//������Ҫ��
					if (pComm->TXCounter>=UART0TXSETCounter)
					{
						pComm->TXCounter=0;
						pComm->TXBUFC_flag=0;
						if(pComm->TXBUFNumber==0) //�պ�������0����û��д�����
						{
							return;
						}
					}
				}
				else
				{
					pComm->TXCounter = 0; 
					pComm->TXBUFNumber=0;
					teleToalnumbuf = YongfuNumStart;	//�´�Ҫ���ĺ���Ϊ��һ���û�����
					return;
				}
			}
			/****************************************/
			if(CHKBITSET(mc52iworkflag,mc52gsmworkingf))
			{//GSMģʽ
// 				mc52isenddatanumbuf = pComm->TXBufLen[pComm->TXCounter];
				gsmSendData.len = pComm->TXBufLen[pComm->TXCounter];
// 				for (i=0;i<=mc52isenddatanumbuf;i++)
				for (i=0; i<=gsmSendData.len; i++)
				{//Ҫ���͵����ݴ���
//					Mc52senddatatonetbuf[i] = pComm->TxDataBuf[pComm->TXCounter][i];//Ҫ�з��ر�־����ܷ�����
					gsmSendData.buffer[i] = pComm->TxDataBuf[pComm->TXCounter][i];//Ҫ�з��ر�־����ܷ�����
				}

				if( (telenumoldbuf>0) && (pComm->TXTELBufNUM[pComm->TXCounter]==0) )
				{//�������û����룬�����������͵ģ�������������Ϣ����������
					GSM_Send_num_sub(pComm,teleToalnumbuf);
					SETBIT(mc52iworkflag,mc52gsmsendbaohuf);//����ָ�������
					telebaohunumbuf=teleToalnumbuf;//�ѱ����ĺ��뱣������
					teleToalnumbuf++;
					if(teleToalnumbuf >= teletolnumbuf) //teletolnumbufΪ��ֵ6+X
					{//���뷢��ͻص���һ���û�����
						teleToalnumbuf = YongfuNumStart;
						pComm->TXCounter++;	//������ѷ���������������
					}
				}
				else
				{//telenumoldbuf==0||pComm->TXTELBufNUM[pComm->TXCounter]>0
					GSM_Send_num_sub(pComm,pComm->TXTELBufNUM[pComm->TXCounter]);
					SETBIT(mc52iworkflag,mc52gsmsendbaohuf);//����ָ�������
					telebaohunumbuf = pComm->TXTELBufNUM[pComm->TXCounter];//�ѱ����ĺ��뱣������
					teleToalnumbuf = YongfuNumStart;
					pComm->TXCounter++;	 
				}
			}
		}
	}
}

void Uart1RXData(void)
{
	uint8 z=0;
	COM_STRUCT *pCommGSM = &g_Comm1;
	COM_STRUCT *pCommKZQ = &g_Comm2;

	if(CHKBITSET(pCommGSM->funcflag,uartrecflag))
	{
		CLRBIT(pCommGSM->funcflag,uartrecflag);
		GSM_RX_Commun_SUB(pCommKZQ,pCommGSM);
		
#if joyceGPRStest
		test_Send_data_to_uart(pCommKZQ, pCommGSM);	// ���԰�COM1�յ�������ֱ�ӷ���COM2
#endif
		for(z=0; z<pCommGSM->RXIndex; z++)
		{
			pCommGSM->RXdata[z] = 0;
		}
		pCommGSM->RXIndex=0; 
		pCommGSM->RxOut=0; 
	}
}

void Uart2RXData(void)
{
//	uint8 i;
	COM_STRUCT *pComm = &g_Comm2;

// 	ProModbusFrame_Master(pCommKZQ,&MODBS[0],0);
	if(CheckWh(pComm,0,0)==0)
	{
		if(getCommNO()==0)
			ProModbusFrame_Master(pComm,&MODBS[0],0);
	}

	if(CHKBITSET(pComm->funcflag,uartrecflag))
	{
// 		ProModbusFrame_Master(pCommKZQ,&MODBS[0],1);
		if(CheckWh(pComm,0,1)==0)
		{
			if(getCommNO()==0)
				ProModbusFrame_Master(pComm,&MODBS[0],1);
		}
		CLRBIT(pComm->funcflag,uartrecflag);

// 		for(i=0; i<pComm->RXIndex; i++)
// 		{
// 			pComm->RXdata[i] = 0;
// 		}
		pComm->RXIndex=0; 
	}
}

void Uart3RXData(void)
{
//	uint8 i;
	COM_STRUCT *pComm = &g_Comm3;
	
	if(CheckWh(pComm,1,0)==0)
	{
		if(getCommNO()==1)
			ProModbusFrame_Master(pComm,&MODBS[1],0);
	}
	
	if(CHKBITSET(pComm->funcflag,uartrecflag))
	{
		if(CheckWh(pComm,1,1)==0)
		{
			if(getCommNO()==1)
				ProModbusFrame_Master(pComm,&MODBS[1],1);
		}

		CLRBIT(pComm->funcflag,uartrecflag);
	
// 		for(i=0; i<pComm->RXIndex; i++)
// 		{
// 			pComm->RXdata[i] = 0;
// 		}
		pComm->RXIndex=0; 
	}
}

void Uart_Rece_Time_sub(COM_STRUCT *pComm)
{
	if(CHKBITSET(pComm->funcflag,uartdelayf))
	{
		if(pComm->RXtime>0)
		{
			pComm->RXtime--;
			if(pComm->RXtime==0)
			{
				if((pComm->RXIndex>UartRXminnum)&&(pComm->RXIndex<UartRXmaxnum))
				{
					SETBIT(pComm->funcflag,uartrecflag);//���ԶԽ��յ������ݽ����жϴ�����
				}
				else
				{
					pComm->RXIndex = 0;
					pComm->RXtime = 0;
				} 
				CLRBIT(pComm->funcflag,uartdelayf);
			}
		}
	}
}


//���԰�һ�����յ�������ֱ�ӷ�����һ����pCommS->RXdata����pCommT->TXdata
void test_Send_data_to_uart(COM_STRUCT *pCommT, COM_STRUCT *pCommS )
{
	uint8 i;

	for(i=0; i<pCommS->RXIndex; i++)
	{
		pCommT->TXdata[i] = pCommS->RXdata[i];
	}
	pCommT->Senddatanum = pCommS->RXIndex;
	Uart_Send_start(pCommT);
}


void UART2_txbuf_sub(void)
{
	COM_STRUCT *pComm = &g_Comm2;
	if(CHKBITSET(pComm->funcflag,uartsenddata))
	{//�ϴ����ݻ�û����,����
		return;
	}

	if(statusflag0.bit.telconconnect)
	{//��������,���߿����������ˡ�
		statusflag0.bit.telconconnect = 0;
		return;
	}
}

void UART3_txbuf_sub(void)
{
	COM_STRUCT *pComm = &g_Comm3;
	if(CHKBITSET(pComm->funcflag,uartsenddata))
	{//�ϴ����ݻ�û����,����
		return;
	}
	
	if(statusflag0.bit.telconconnect)
	{//��������,���߿����������ˡ�
		statusflag0.bit.telconconnect = 0;
		return;
	}
}

/**************************************************************************
�������ƣ�WriteUart1
�����汾��1.00 
���ߣ�   hjy
�������ڣ�2013.11.05   
��������˵����gx comд����
���������buf:���������ݻ���; len:���������ݳ���
��������:
���������
����ֵ��
***************************************************************************/
void WriteUart1(const char *buf, uint16 len)
{
	COM_STRUCT *pComm=&g_Comm1;
	uint8 i;

	for(i=0; i<len; i++)
	{
		pComm->TXdata[i] = buf[i];
	}
	pComm->Senddatanum = len;
	SETBIT(pComm->funcflag,uartsenddata);	//�趨�������ݱ�־
	LPC_UART1->THR = pComm->TXdata[0];

	pComm->SendIndex = 1;	// �ѷ���һ������
	Nodatatratbuf = 0;
}

/**************************************************************************
�������ƣ�Uart0RxbufDataLen
�����汾��1.00 
���ߣ�   hjy
�������ڣ�2013.11.06   
��������˵��������gx com ��������Ч���ݳ���
���������
��������:
���������
����ֵ��
***************************************************************************/
uint16 Uart1RxbufDataLen(void)
{
	COM_STRUCT *pComm=&g_Comm1;

   if (pComm->RXIndex >= pComm->RxOut)
   {
        return (pComm->RXIndex - pComm->RxOut);
    }
   else
   {
        return (datamaxlen - pComm->RxOut + pComm->RXIndex);
    }
}

/**************************************************************************
�������ƣ�ReadUart1
�����汾��1.00 
���ߣ�   hjy
�������ڣ�2013.11.06   
��������˵����gx com������
���������buf:�������ݻ���; len:Ԥ�ڶ������ݳ���;
��������:
���������
����ֵ����������ʵ�ʳ���
***************************************************************************/
uint16 ReadUart1(char *buf, uint16 len)
{
	COM_STRUCT *pComm=&g_Comm1;
   uint16 canreadlen = Uart1RxbufDataLen();
    canreadlen = (canreadlen>=len)? len : canreadlen;
    
    if (canreadlen > 0)
	{
        uint16 i, out;
        out = pComm->RxOut;
        for (i=0; i<canreadlen; i++)
		{
            buf[i] = pComm->RXdata[out];
            out++;
            CIRCLE_INDEX(out, datamaxlen);
        }
        pComm->RxOut = out;
    }
    
    return canreadlen;
}
