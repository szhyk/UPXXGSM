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

 	LPC_SC->PCONP |= (1<<3);               //打开UART0功率、时钟控制位  
	
	LPC_PINCON->PINSEL0 |= (1 << 4);             // Pin P0.2 used as TXD0 (Com0)
  LPC_PINCON->PINSEL0 |= (1 << 6);             // Pin P0.3 used as RXD0 (Com0)
	//使能访问除数锁存器 ，设定波特率  
	LPC_UART0->LCR  = 0x83;                      // 设置串口数据格式，8位字符长度，1个停止位，无校验，
    usFdiv = (FPCLK/16)/ UART0_BPS;              // 设置波特率
    LPC_UART0->DLM  = usFdiv / 256;
    LPC_UART0->DLL  = usFdiv % 256; 
    LPC_UART0->LCR  = 0x03;                      // 禁止访问除数锁存器，锁定波特率

//	LPC_UART0->FCR  = 0x87;						 // 控制UARTn RX和TX FIFO的操作。接收数据8个字节触发中断 
	LPC_UART0->FCR  = 0x07;						 // 控制UARTn RX和TX FIFO的操作。接收数据一个字节触发中断 
	
	NVIC_EnableIRQ(UART0_IRQn);					// enable irq in nvic 
    LPC_UART0->IER = 0x07;						// 使能接收中断，发送中断，RX线中断
}

//LPC1768的UART1连接MC52i的Uart0,波特率必须对应才行.
void UART1_Init (void)
{
	uint16 usFdiv;
	LPC_SC->PCONP |= (1<<4);               //打开UART1功率、时钟控制位  
	LPC_PINCON->PINSEL0 |= (1 << 30);    //选择P0.15为TXD1
	LPC_PINCON->PINSEL1 |= 0x1;          //选择P0.16为RXD1
	//使能访问除数锁存器 ，设定波特率  
	LPC_UART1->LCR  = 0x83;                      // 设置串口数据格式，8位字符长度，1个停止位，无校验
	usFdiv = (FPCLK/16)/ UART0_BPS;              // 设置波特率,MC52i的Uart0默认波特率为9600
	LPC_UART1->DLM  = usFdiv / 256; 
	LPC_UART1->DLL  = usFdiv % 256; 
	LPC_UART1->LCR  = 0x03;					// 禁止访问除数锁存器，锁定波特率 
//	LPC_UART1->FCR = 0x87;					// 控制UARTn RX和TX FIFO的操作。接收数据8个字节触发中断 
	LPC_UART1->FCR  = 0x07;				// 控制UARTn RX和TX FIFO的操作。接收数据一个字节触发中断 
	NVIC_EnableIRQ(UART1_IRQn);				// enable irq in nvic
	LPC_UART1->IER = 0x07;                    //使能接收中断，发送中断，RX线中断
}

void UART2_Init (void)
{
	uint16_t usFdiv;
	LPC_SC->PCONP |=(1<<24); //uart2 power on
	
	LPC_PINCON->PINSEL0 |= (1 << 20);   // pin P0.10 used as TXD2
	LPC_PINCON->PINSEL0 |= (1 << 22);   // pin P0.11 used as RXD2
	//使能访问除数锁存器 ，设定波特率  
//	LPC_UART2->LCR  = 0x83;				// 设置串口数据格式，8位字符长度，1个停止位，无校验，
	LPC_UART2->LCR  = 0x9B;				// 设置串口数据格式，8位字符长度，1个停止位，偶校验，8E1
    usFdiv = (FPCLK/16)/ UART0_BPS;     // 设置波特率
    LPC_UART2->DLM  = usFdiv / 256;
    LPC_UART2->DLL  = usFdiv % 256; 
 //  LPC_UART2->LCR  = 0x03;             // 禁止访问除数锁存器，锁定波特率 
    LPC_UART2->LCR  &= 0x7F;             // 禁止访问除数锁存器，锁定波特率 
	//	LPC_UART2->FCR  = 0x87;				// 控制UARTn RX和TX FIFO的操作。接收数据8个字节触发中断
	//	LPC_UART2->FCR  = 0x47;				// 控制UARTn RX和TX FIFO的操作。接收数据4个字节触发中断 
	LPC_UART2->FCR  = 0x07;				// 控制UARTn RX和TX FIFO的操作。接收数据1个字节触发中断 ,因为keil新版本的lpc17xx.h中没有了FIFOLVL所以，只能用1个字节去触发中断了
	NVIC_EnableIRQ(UART2_IRQn);		   // enable irq in nvic 
	LPC_UART2->IER=0x07;			   // 使能接收中断，发送中断，RX线中断
}


void UART3_Init (void)
{
	uint16_t usFdiv;
	
	LPC_SC->PCONP |=(1<<25); //uart3 power on
 	LPC_PINCON->PINSEL9 |= (3 << 24);   // pin P4.28 used as TXD3
 	LPC_PINCON->PINSEL9 |= (3 << 26);   // pin P4.29 used as RXD3
//    LPC_PINCON->PINSEL0 |= (2 << 0);             // Pin P0.2 used as TXD3
//    LPC_PINCON->PINSEL0 |= (2 << 2);             // Pin P0.3 used as RXD3
	//使能访问除数锁存器 ，设定波特率  
// 	LPC_UART3->LCR  = 0x83;				// 设置串口数据格式，8位字符长度，1个停止位，无校验，8N1
// 	LPC_UART3->LCR  = 0x87;				// 设置串口数据格式，8位字符长度，2个停止位，无校验，8N2
// 	LPC_UART3->LCR  = 0x8B;				// 设置串口数据格式，8位字符长度，1个停止位，奇校验，8O1
	LPC_UART3->LCR  = 0x9B;				// 设置串口数据格式，8位字符长度，1个停止位，偶校验，8E1
    usFdiv = (FPCLK/16)/ UART0_BPS;     // 设置波特率
    LPC_UART3->DLM  = usFdiv / 256;
    LPC_UART3->DLL  = usFdiv % 256; 
//    LPC_UART3->LCR  = 0x03;             // 禁止访问除数锁存器，锁定波特率 
    LPC_UART3->LCR  &= 0x7F;             // 禁止访问除数锁存器，锁定波特率 
	//	LPC_UART2->FCR  = 0x87;				// 控制UARTn RX和TX FIFO的操作。接收数据8个字节触发中断
	//	LPC_UART2->FCR  = 0x47;				// 控制UARTn RX和TX FIFO的操作。接收数据4个字节触发中断 
	LPC_UART3->FCR  = 0x07;				// 控制UARTn RX和TX FIFO的操作。接收数据1个字节触发中断 ,因为keil新版本的lpc17xx.h中没有了FIFOLVL所以，只能用1个字节去触发中断了
	NVIC_EnableIRQ(UART3_IRQn);		   // enable irq in nvic 
	LPC_UART3->IER = 0x07;			   // 使能接收中断，发送中断，RX线中断

}

//Uart0未使用
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
				CLRBIT(pComm->funcflag,uartsenddata);//结束发送
			}
		}
		break;	
	}	
}

//短信模块mc52i与MCU通信
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
		//FIFO触发中断
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
				{//一次最多发送8个字节
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
				CLRBIT(pComm->funcflag,uartsenddata);	//	结束发送
			}
		}
		break;	
	}
}


//MCU跟控制器通信
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
		//接收线中断 RLS
		iir = LPC_UART2->LSR; 
		break;
	case IIR_RDA:
		//接收数据可用 RDA。RDA中断触发为8个字节，读取7个字节，留一个字节以便触发CTI中断
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
		//字符超时指示 CTI
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
		//THRE 中断
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
				CLRBIT(pComm->funcflag,uartsenddata);	//	结束发送
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
		//接收线中断 RLS
		iir = LPC_UART3->LSR; 
		break;
	case IIR_RDA:
		//接收数据可用 RDA。RDA中断触发为8个字节，读取7个字节，留一个字节以便触发CTI中断
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
		//字符超时指示 CTI
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
		//THRE 中断
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
				CLRBIT(pComm->funcflag,uartsenddata);	//	结束发送
			}
		}
		break;	
	}
}

//串口数据向外发送
void Uart_Send_start(COM_STRUCT *pComm)
{
	SETBIT(pComm->funcflag,uartsenddata);	//设定发送数据标志
	if(pComm->COMM_ID==1)
		LPC_UART1->THR = pComm->TXdata[0];
	else if(pComm->COMM_ID==2)
		LPC_UART2->THR = pComm->TXdata[0];
	else if(pComm->COMM_ID==3)
		LPC_UART3->THR = pComm->TXdata[0];
	
	pComm->SendIndex = 1;	// 已发送一个数据
	Nodatatratbuf = 0;
}

void UART1_txbuf_sub(void)
{
	uint8 i=0;
	COM_STRUCT *pComm = &g_Comm1;

	if(CHKBITSET(pComm->funcflag,uartsenddata))
	{
		mc52itimebuf=0;
		return;//在发数据的时候不能进入
	}
// 	if(Gprsworkingflag>0)
// 	{//
// 		mc52itimebuf=0;
// 		return;//在发数据的时候不能进入
// 	}
	if(CHKBITSET(funcflag5,mc52igprsdatasendf))
	{
		mc52itimebuf=0;
		return;//数据还没发是进不来的
	}
	if(CHKBITSET(mc52iworkflag,mc52sendbaohuf))
	{
		return;//数据还没发是进不来的
	}
	if(CHKBITSET(mc52iworkflag,mc52gsmsendbaohuf))
	{
		return;//GSM数据还没发是进不来的
	}
	/******************************/
	mc52itimebuf++;
	if(mc52itimebuf > Uart0sendjiangetime)
	{//每隔200ms处理一次
		mc52itimebuf = 0;
		if ((pComm->TXBUFNumber>0)||(pComm->TXBUFC_flag==1))
		{//发送缓冲区有数据，或者有数据要发送
			if (pComm->TXCounter >= pComm->TXBUFNumber)
			{//已发完数据或者环回时
				if(pComm->TXBUFC_flag==1)
				{//有数据要发
					if (pComm->TXCounter>=UART0TXSETCounter)
					{
						pComm->TXCounter=0;
						pComm->TXBUFC_flag=0;
						if(pComm->TXBUFNumber==0) //刚好满，第0个还没填写的情况
						{
							return;
						}
					}
				}
				else
				{
					pComm->TXCounter = 0; 
					pComm->TXBUFNumber=0;
					teleToalnumbuf = YongfuNumStart;	//下次要发的号码为第一个用户号码
					return;
				}
			}
			/****************************************/
			if(CHKBITSET(mc52iworkflag,mc52gsmworkingf))
			{//GSM模式
// 				mc52isenddatanumbuf = pComm->TXBufLen[pComm->TXCounter];
				gsmSendData.len = pComm->TXBufLen[pComm->TXCounter];
// 				for (i=0;i<=mc52isenddatanumbuf;i++)
				for (i=0; i<=gsmSendData.len; i++)
				{//要发送的内容存入
//					Mc52senddatatonetbuf[i] = pComm->TxDataBuf[pComm->TXCounter][i];//要有返回标志后才能发数据
					gsmSendData.buffer[i] = pComm->TxDataBuf[pComm->TXCounter][i];//要有返回标志后才能发数据
				}

				if( (telenumoldbuf>0) && (pComm->TXTELBufNUM[pComm->TXCounter]==0) )
				{//已设置用户号码，且是主动发送的，则轮流发送信息给各个号码
					GSM_Send_num_sub(pComm,teleToalnumbuf);
					SETBIT(mc52iworkflag,mc52gsmsendbaohuf);//保护指令或数据
					telebaohunumbuf=teleToalnumbuf;//把保护的号码保存起来
					teleToalnumbuf++;
					if(teleToalnumbuf >= teletolnumbuf) //teletolnumbuf为定值6+X
					{//号码发完就回到第一个用户号码
						teleToalnumbuf = YongfuNumStart;
						pComm->TXCounter++;	//发完后，已发缓冲区计数增加
					}
				}
				else
				{//telenumoldbuf==0||pComm->TXTELBufNUM[pComm->TXCounter]>0
					GSM_Send_num_sub(pComm,pComm->TXTELBufNUM[pComm->TXCounter]);
					SETBIT(mc52iworkflag,mc52gsmsendbaohuf);//保护指令或数据
					telebaohunumbuf = pComm->TXTELBufNUM[pComm->TXCounter];//把保护的号码保存起来
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
		test_Send_data_to_uart(pCommKZQ, pCommGSM);	// 测试把COM1收到的数据直接发给COM2
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
					SETBIT(pComm->funcflag,uartrecflag);//可以对接收到的数据进行判断处理了
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


//测试把一个口收到的数据直接发给另一个口pCommS->RXdata发给pCommT->TXdata
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
	{//上次数据还没发完,返回
		return;
	}

	if(statusflag0.bit.telconconnect)
	{//请求连接,告诉控制器连上了。
		statusflag0.bit.telconconnect = 0;
		return;
	}
}

void UART3_txbuf_sub(void)
{
	COM_STRUCT *pComm = &g_Comm3;
	if(CHKBITSET(pComm->funcflag,uartsenddata))
	{//上次数据还没发完,返回
		return;
	}
	
	if(statusflag0.bit.telconconnect)
	{//请求连接,告诉控制器连上了。
		statusflag0.bit.telconconnect = 0;
		return;
	}
}

/**************************************************************************
函数名称：WriteUart1
函数版本：1.00 
作者：   hjy
创建日期：2013.11.05   
函数功能说明：gx com写数据
输入参数：buf:待发送数据缓冲; len:待发送数据长度
其他输入:
输出参数：
返回值：
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
	SETBIT(pComm->funcflag,uartsenddata);	//设定发送数据标志
	LPC_UART1->THR = pComm->TXdata[0];

	pComm->SendIndex = 1;	// 已发送一个数据
	Nodatatratbuf = 0;
}

/**************************************************************************
函数名称：Uart0RxbufDataLen
函数版本：1.00 
作者：   hjy
创建日期：2013.11.06   
函数功能说明：计算gx com 读缓冲有效数据长度
输入参数：
其他输入:
输出参数：
返回值：
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
函数名称：ReadUart1
函数版本：1.00 
作者：   hjy
创建日期：2013.11.06   
函数功能说明：gx com读数据
输入参数：buf:读出数据缓存; len:预期读出数据长度;
其他输入:
输出参数：
返回值：读出数据实际长度
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
