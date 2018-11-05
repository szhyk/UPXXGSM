/********************************************************
File name: Configuration.c
Function: communication
Editor: Martin
Date: 20/06/2008
Version: V101
Description:
*********************************************************/
#include "define.h"
#include "Configuration.h"
#include "MODBUSM.h"
#include "wxcw.h"
#include <string.h>
#include "uart.h"

#ifdef CONFIGURTION_XX


//����һЩ����
#define WH_YXNUM		//�����ȡ��ң������
#define WH_YCNUM		//�����ȡ��ң������
#define WH_YKNUM		//����ң������
#define WH_CINUM		//�����ȡ�ĵ������


//ά����־ͷ��β
#define WHCMD_HEAD1		0xFF
#define WHCMD_HEAD2		0x12
#define WHCMD_END		0xFF

//����
#define WHCMD_BEGIN		0x34
#define WHCMD_PAUSE		0x35
#define WHCMD_QUIT		0x36
#define WHCMD_CRC_BEG	(WHCMD_HEAD2+WHCMD_BEGIN)
#define WHCMD_CRC_PAUSE	(WHCMD_HEAD2+WHCMD_PAUSE)
#define WHCMD_CRC_QUIT	(WHCMD_HEAD2+WHCMD_QUIT)

//Ӧ��
#define WHCMD_ACQ_BEG	0x66
#define WHCMD_ACQ_PAUSE	0x77
#define WHCMD_ACQ_QUIT	0x88
#define WHCMD_CRC_AB	(WHCMD_HEAD2+WHCMD_ACQ_BEG)
#define WHCMD_CRC_AP	(WHCMD_HEAD2+WHCMD_ACQ_PAUSE)
#define WHCMD_CRC_AQ	(WHCMD_HEAD2+WHCMD_ACQ_QUIT)

extern uint32 dTCounter;
WH_STRUCT wh_com[2];

enum{
	FUN_EX_DI = 0x41,	//DI״̬
	FUN_EX_AI,			//AI
	FUN_EX_DO,			//DO
	FUN_EX_CI,			//CI
	FUN_EX_RPN = 0x45,	//����������
	FUN_EX_RP,			//����������
	FUN_EX_RX,			//����ַ
	FUN_EX_RPH,			//���绰����
	FUN_EX_RT,			//��ʱ��
	FUN_EX_RUD,			//������DI
	FUN_EX_RUA,			//������AI
	FUN_EX_RUO,			//������DO
	FUN_EX_WUD,			//д����DI
	FUN_EX_WUA,			//д����AI
	FUN_EX_WUO,			//д����DO

	FUN_EX_WPN,			//д��������
	FUN_EX_WP,			//д��������
	FUN_EX_WX,			//д��ַ
	FUN_EX_WPH,			//д�绰����
	FUN_EX_WT,			//дʱ��
	FUN_EX_NUM
};

enum
{
	ILLEGAL_FUNCTION = 01,
	ILLEGAL_DATA_ADDRESS = 02,
	ILLEGAL_DATA_VALUE = 03,
	SLAVE_DEVICE_FAILURE = 04
};


//////////////////////////////////////////////////////////////////////////////////////////////
//	Function: communication interface function 
//	Date:
//	Description:
//////////////////////////////////////////////////////////////////////////////////////////////

uint16  Crc16CodeC(uint8 *pInFrame, uint16 dwInLen)
{
	uint16  crccode = 0xFFFF;
	uint16	i;
	uint16	j;
	for( j=0; j < dwInLen; j++)
	{
		crccode ^= (uint16 ) (*pInFrame);
		for(i=0; i<8; i++)
		{
			if ( crccode & 0x0001 )
			{
				crccode = crccode>>1;
				crccode ^= 0xA001;
			}
			else
			{
				crccode = crccode>>1;
			}
		}
		pInFrame ++;
	}
	return crccode;
}


/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

//��ȡ�˿�����
uint16 DealXAIFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16	addr,j;
	
	addr = buffer[2];
	addr <<= 8;
	addr += buffer[3];
	j = 3;
	j += ReadPortData(addr,sendBuf+3);
	
	if(j==3)
	{
		sendBuf[2] = ILLEGAL_DATA_ADDRESS;
	}
	else
		sendBuf[2] = j-3;
	
	return j;
}

//��ȡ�˿ڲ���
uint16 DealXRPNFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16	j;
	
	j = 3;
	j += ReadPortParam(sendBuf+3);

	sendBuf[2] = j-3;

	return j;
}

//��ȡ��������
uint16 DealXRPFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16	j;
	
	j = 3;
	j += ReadBaseParam(sendBuf+3);

	sendBuf[2] = j-3;

	return j;
}

//��ȡ�绰����
uint16 DealXRPHFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16	j;
	
	j = 3;
	j += ReadPhoneParam(sendBuf+3);

	sendBuf[2] = j-3;
	
	return j;
}

//��ȡ�豸��ַ
uint16 DealXRXFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16	j;
	
	j = 3;

	sendBuf[j++] = 0;
	sendBuf[j++] = devPS.m_Addr;

	sendBuf[2] = j-3;
	
	return j;
}

//��ȡʱ��
uint16 DealXRTFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16	i,j,k;
	
	j = 3;
	//��Ч��
	for(i=0; i<PORT_NUM; i++)
	{
		sendBuf[j++] = (devPS.Valid[i]>>8)&0xFF;
		sendBuf[j++] = devPS.Valid[i]&0xFF;
	}
	//����
	for(i=0; i<PORT_NUM; i++)
	{
		sendBuf[j++] = (devPS.BjEnalbe[i]>>8)&0xFF;
		sendBuf[j++] = devPS.BjEnalbe[i]&0xFF;
	}
	
	//�˿�����
	for(i=0; i<PORT_NUM; i++)
	{
		for(k=0; k<5; k++)
		{
			sendBuf[j++] = (devPS.strPortName[i][k]>>8)&0xFF;
			sendBuf[j++] = devPS.strPortName[i][k]&0xFF;
		}
	}

	return j;
}

//д��������
uint16 DealXWPFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16 iStartAddr;
	uint16 iWriteNum,iByteCount;
	
	iStartAddr = ((uint16)buffer[2]<<8) + buffer[3];
	iWriteNum   = ((uint16)buffer[4]<<8) + buffer[5];
	
	iByteCount =  buffer[6];
	
	if( iByteCount != 2*iWriteNum)
	{// ��������
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}
	if(iStartAddr != 0)
	{//��ַ����
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_ADDRESS;
		return 5;
	}
	if(iByteCount != ((10+9)*2))
	{
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}

	WriteBaseParam(buffer+7);

	sendBuf[2] = buffer[2];
	sendBuf[3] = buffer[3];
	sendBuf[4] = buffer[4];
	sendBuf[5] = buffer[5];
	
	return 6;
}

//д�˿ڲ���
uint16 DealXWPNFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16 iStartAddr;
	uint16 iWriteNum,iByteCount;
	
	iStartAddr = ((uint16)buffer[2]<<8) + buffer[3];
	iWriteNum   = ((uint16)buffer[4]<<8) + buffer[5];
	
	iByteCount =  buffer[6];
	
	if(iByteCount != 2*iWriteNum)
	{// ��������
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}
	if(iStartAddr != 0)
	{//��ַ����
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_ADDRESS;
		return 5;
	}
	if(iByteCount != ((PORT_NUM*7)*2) )
	{
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}

	WritePortParam(buffer+7);

	sendBuf[2] = buffer[2];
	sendBuf[3] = buffer[3];
	sendBuf[4] = buffer[4];
	sendBuf[5] = buffer[5];

	return 6;
}


//д�绰����
uint16 DealXWPHFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16 iStartAddr;
	uint16 iWriteNum,iByteCount;
	
	iStartAddr = ((uint16)buffer[2]<<8) + buffer[3];
	iWriteNum   = ((uint16)buffer[4]<<8) + buffer[5];
	
	iByteCount =  buffer[6];
	
	if(iByteCount != 2*iWriteNum)
	{// ��������
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}
	if(iStartAddr != 0)
	{//��ַ����
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_ADDRESS;
		return 5;
	}

	if(iByteCount != (2+10*11+2))
	{
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}
	
	WritePhoneParam(buffer+7);

	sendBuf[2] = buffer[2];
	sendBuf[3] = buffer[3];
	sendBuf[4] = buffer[4];
	sendBuf[5] = buffer[5];
	
	return 6;
}

//д��ַ
uint16 DealXWXFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16 iStartAddr;
	uint16 iWriteNum,iByteCount;
	
	iStartAddr = ((uint16)buffer[2]<<8) + buffer[3];
	iWriteNum   = ((uint16)buffer[4]<<8) + buffer[5];
	
	iByteCount =  buffer[6];
	
	if(iByteCount != 2*iWriteNum)
	{// ��������
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}
	if(iStartAddr != 0)
	{//��ַ����
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_ADDRESS;
		return 5;
	}
	
	devPS.m_Addr = buffer[8];
	
	sendBuf[2] = buffer[2];
	sendBuf[3] = buffer[3];
	sendBuf[4] = buffer[4];
	sendBuf[5] = buffer[5];
	
	return 6;
}

//дʱ��
uint16 DealXWTFrame(uint8 *buffer, uint8 *sendBuf)
{
/*	//	unsigned char i;
	uint16 iStartAddr;
	uint16 iWriteNum,iByteCount;
	//	uint16 tim[7];
	
	iStartAddr = ((uint16)buffer[2]<<8) + buffer[3];
	iWriteNum   = ((uint16)buffer[4]<<8) + buffer[5];
	
	iByteCount =  buffer[6];
	
	//	if( (iByteCount != 2*iWriteNum) || iWriteNum==0 || iWriteNum>0x07B)
	if( (iByteCount != 2*iWriteNum) || (iWriteNum != 6) )
	{// ��������
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}
	if(iStartAddr != 0)
	{//��ַ����
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_ADDRESS;
		return 5;
	}
	
	// ����Ϊ����ʱ��У׼
// 	Old_Clock[0] = buffer[7];
// 	Old_Clock[0] = (Old_Clock[0]<<8);
// 	Old_Clock[0] += buffer[8];
// 	if(Old_Clock[0] < 2000)
// 		Old_Clock[0] += 2000;
// 	Old_Clock[1] = buffer[10];
// 	Old_Clock[2] = buffer[12];
// 	Old_Clock[3] = buffer[14];
// 	Old_Clock[4] = buffer[16];
// 	Old_Clock[5] = buffer[18];
// 	
// 	// �ж�����ʱ��
// 	if(CheckTime(Old_Clock) == 1)	// ʱ����ȷ,�ı�ϵͳʱ��
// 	{
// 		Register[REG_CLOCK_YEAR] = Old_Clock[0];
// 		Register[REG_CLOCK_MONTH] = Old_Clock[1];
// 		Register[REG_CLOCK_DAY] = Old_Clock[2];
// 		Register[REG_CLOCK_HOUR] = Old_Clock[3];
// 		Register[REG_CLOCK_MINUTE] = Old_Clock[4];
// 		Register[REG_CLOCK_SECOND] = Old_Clock[5];
// 		
// 		rtc_write((uint16 *)&Register[REG_CLOCK_BEG]);
// 	}
// 	else
// 	{
// 		sendBuf[1] += 0x80;
// 		sendBuf[2] = ILLEGAL_DATA_VALUE;
// 		return 5;
// 		
// 	}
	
	sendBuf[3] = buffer[3];
	sendBuf[4] = buffer[4];
	sendBuf[5] = buffer[5];
	sendBuf[2] = 3;
	*/
	return 6;
}

//����֡���ܽӿ�
uint16 DealWhFrame(COM_STRUCT *pComms, uint8 *buffer, uint8 *sendBuf)        //uint8 *pInFrame previous
{
	uint8 Command;
	uint16 crcCode;
	uint16 framelen;

	framelen = 0;
	sendBuf[0] = buffer[0];
	sendBuf[1] = buffer[1];

	Command = buffer[1];

	{
		switch(Command)
		{
		case FUN_EX_AI:
			framelen = DealXAIFrame(buffer,sendBuf);
			break;
		case FUN_EX_RPN:
			framelen = DealXRPNFrame(buffer,sendBuf);
			break;
		case FUN_EX_RP:
			framelen = DealXRPFrame(buffer,sendBuf);
			break;
		case FUN_EX_RPH:
			framelen = DealXRPHFrame(buffer,sendBuf);
			break;
		case FUN_EX_RX:
			framelen = DealXRXFrame(buffer,sendBuf);
			break;
		case FUN_EX_RT:
			framelen = DealXRTFrame(buffer,sendBuf);
			break;
		case FUN_EX_WP:
			framelen = DealXWPFrame(buffer,sendBuf);
			break;
		case FUN_EX_WPN:
			framelen = DealXWPNFrame(buffer,sendBuf);
			break;
		case FUN_EX_WPH:
			framelen = DealXWPHFrame(buffer,sendBuf);
			break;
		case FUN_EX_WX:
			framelen = DealXWXFrame(buffer,sendBuf);
			break;
		case FUN_EX_WT:
			framelen = DealXWTFrame(buffer,sendBuf);
			break;
		default:	
			sendBuf[2] = ILLEGAL_FUNCTION;	//	command  error
			framelen = 3;
			break;
		}
	}
	
//EEND:
	if(framelen > 0)
	{
		if(framelen==3)
		{
			sendBuf[1] += 0x80;
		}
		// ����У���롣֡�����У�CRC �����ǵ�λ��ǰ����λ�ں������ݲ��������෴
		crcCode = Crc16CodeC( sendBuf, framelen);
		sendBuf[framelen++] = (crcCode & 0x00FF);
		sendBuf[framelen++] = (crcCode & 0xFF00) >> 8;
	}

	return framelen;
}

//�˺�����Ҫ��ֲ
#define MINFRAMELEN 8
uint8 Wh_Run(COM_STRUCT*  l_pCom)
{
	uint16 iFrameLen;
	uint16 crcCode;
	uint16 LruNo;
	uint16 Hi,Lo;
	uint16 Command;
	uint16 framelen;

	framelen = 0;

	//��С֡��
	if(l_pCom->RXIndex < MINFRAMELEN)
		return 0;

	LruNo = l_pCom->RXdata[0];		//ȡ�ñ��˿ڵ��豸��ַ
	
//	if(LruNo != devPS.m_Addr)
	if(LruNo != 0x0)
		return 0;

	//ȡ������
	Command = l_pCom->RXdata[1];

	{//��ַҪ��ͬ
		switch (Command)
		{
		case FUN_EX_AI:
		case FUN_EX_RPN:
		case FUN_EX_RP:
		case FUN_EX_RX:
		case FUN_EX_RPH:
		case FUN_EX_RT:
			iFrameLen = 8;
			break;
		case FUN_EX_WPN:
		case FUN_EX_WP:
		case FUN_EX_WX:
		case FUN_EX_WPH:
		case FUN_EX_WT:
			{
				iFrameLen = l_pCom->RXdata[6];
				iFrameLen += 9;
			}
			break;
		default:
			// �����Ĳ��ж�
			return 0;
		}
	}

	//���ݲ�����
	if(l_pCom->RXIndex < iFrameLen)
		return 0;

	// ����
	{
		crcCode = Crc16CodeC(&l_pCom->RXdata[0], iFrameLen-2);
		Hi = crcCode >> 8;
		Lo = crcCode & 0x00FF;
		if( Lo == l_pCom->RXdata[iFrameLen-2] && Hi == l_pCom->RXdata[iFrameLen-1] )
		{// У������ȷ,CRC �����ǵ�λ��ǰ����λ�ں������ݲ��������෴
			// �ı���ջ������Ĵ���λ�ã���Ϊ���δ�����֡��ΪiFrameLen
			framelen = DealWhFrame(l_pCom, &l_pCom->RXdata[0], l_pCom->TXdata);
		}
	}
	if(framelen)
	{
		l_pCom->Senddatanum = framelen;
		Uart_Send_start(l_pCom);
	}
	return framelen;
}

//����Ҫ�ڳ�ʼ��ʱִ��
void Init_Wh(WH_STRUCT *pWh)
{
	pWh->whflagold = 0;
	pWh->whPause = 0;
	pWh->l_lt1 = dTCounter;

 	pWh->whInitFlag = 0xA5;
 	pWh->whflag = 0;
}

void  CopyDataToBuffer(uint8* TmpBuffer,uint8* SourceBuffer, int outpos, int MaxSize, int  DatLen)
{
	int l_Pos = 0;
	int l_OuPos = outpos;
	while(DatLen--)
	{
		TmpBuffer[l_Pos] = SourceBuffer[l_OuPos];
		l_Pos++;
		l_OuPos++;
		if(l_OuPos >= MaxSize)
		{
			l_OuPos = 0;
		}
	}
}

uint8 CheckWHflag(COM_STRUCT*  l_pCom)
{
	uint8 len;
	uint8 *l_pBuf;
	
	if( l_pCom->RXIndex < 5 )
	{//���ݲ������򲻴���
		return 0;
	}

	l_pBuf = l_pCom->RXdata;
	len = l_pCom->RXIndex;
BEGIN:
	if((l_pBuf[0]==WHCMD_HEAD1) && (l_pBuf[4]==WHCMD_END))
	{
		if(l_pBuf[1] == WHCMD_HEAD2)
		{//����ά��
			switch(l_pBuf[2])
			{
			case WHCMD_BEGIN:
				{//����ά������û�����ݣ�5���Ӻ��Զ��˳�
					if(l_pBuf[3] != WHCMD_CRC_BEG)
						return 0;
					return 1;
				}
			case WHCMD_PAUSE:
				{//��ͣά��
					if(l_pBuf[3] != WHCMD_CRC_PAUSE)
						return 0;
					return 3;
				}
			case WHCMD_QUIT:
				{//�˳�ά��
					if(l_pBuf[3] != WHCMD_CRC_QUIT)
						return 0;
					return 2;
				}
			}
		}
	}
	l_pBuf++;
	len--;

	if( len >= 5 )
		goto BEGIN;
	
	return 0;
}


#define TEN_MINUTES	(10L*60*1000)	//1msִ��һ��
//ά�������жϣ���ʵ�����жϱ�־Ȼ�����MODBUSЭ��
uint8 CheckWh(COM_STRUCT*  l_pCom,uint8 whno,uint8 recflag)
{
	uint8 kk=0;
	WH_STRUCT *pWh = &wh_com[whno];
	
	if(whno > 1)
		return 0;
		
	//û�г�ʼ������ִ��
	if(pWh->whInitFlag != 0xA5)
	{//�ж��Ƿ�ִ�й���ʼ��
		Init_Wh(pWh);
	}

	if(recflag)
	{
		kk = CheckWHflag(l_pCom);
	}
	//���ݷ���ֵ����Ӧ��
	switch(kk)
	{
	case 1:
		{//����ά��
			uint8*  l_Data = l_pCom->TXdata;
			
			l_Data[0] = WHCMD_HEAD1;
			l_Data[1] = WHCMD_HEAD2;
			l_Data[2] = WHCMD_ACQ_BEG;
			l_Data[3] = WHCMD_CRC_AB;
			l_Data[4] = WHCMD_END;
			
			l_Data[5] = devPS.m_Addr;
			
			l_pCom->Senddatanum = 6;
			Uart_Send_start(l_pCom);

			pWh->l_lt1 = 0;		// ��ʼ��ʱ��,���ɺ�̨����ʱ�������������˳�ά��

			pWh->whflag = 1;	// ����ά����ʶ
			pWh->whPause = 0;
		}
		break;
	case 3:
		{//��ͣά��
			uint8*  l_Data = l_pCom->TXdata;
			
			l_Data[0] = WHCMD_HEAD1;
			l_Data[1] = WHCMD_HEAD2;
			l_Data[2] = WHCMD_ACQ_PAUSE;
			l_Data[3] = WHCMD_CRC_AP;
			l_Data[4] = WHCMD_END;
			
			l_Data[5] = devPS.m_Addr;
			
			l_pCom->Senddatanum = 6;
			Uart_Send_start(l_pCom);
			
			pWh->l_lt1 = 0;		// ��ʼ��ʱ��,���ɺ�̨����ʱ�������������˳�ά��
			
			pWh->whflag = 1;		// ����ά����ʶ
			pWh->whPause = 1;		// ������ͣά����־
		}
		break;
	case 2:
		{//�˳�ά��
			uint8*  l_Data = l_pCom->TXdata;
			l_Data[0] = WHCMD_HEAD1;
			l_Data[1] = WHCMD_HEAD2;
			l_Data[2] = WHCMD_ACQ_QUIT;
			l_Data[3] = WHCMD_CRC_AQ;
			l_Data[4] = WHCMD_END;
			
			l_pCom->Senddatanum = 5;
			Uart_Send_start(l_pCom);

			pWh->l_lt1=0;
			pWh->whflag = 0;
			pWh->whPause = 0;
		}
		break;
	}
	
	if(pWh->whflag)
	{//����ά����־

		pWh->l_lt1++;
		if(pWh->l_lt1 > TEN_MINUTES)
		{//10�����Զ��˳�ά������
			uint8*  l_Data = l_pCom->TXdata;
			
			pWh->l_lt1=0;
			pWh->whflag = 0;
			pWh->whPause = 0;
			
			l_Data[0] = WHCMD_HEAD1;
			l_Data[1] = WHCMD_HEAD2;
			l_Data[2] = WHCMD_ACQ_QUIT;
			l_Data[3] = WHCMD_CRC_AQ;
			l_Data[4] = WHCMD_END;
			
			l_pCom->Senddatanum = 5;
			Uart_Send_start(l_pCom);

			return 1;
		}
		//��ͣʱ
		if(pWh->whPause)
			return 0;

		if(recflag)
		{//������ɲŴ���
			if(Wh_Run(l_pCom))
				pWh->l_lt1 = 0;	//���������ݣ������¼�ʱ
		}
		
		return 1;
	}
	return 0;	// ����0����ú����Ĵ�������ִ��
}

#endif		//CONFIGURTION_XX