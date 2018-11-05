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


//定义一些常量
#define WH_YXNUM		//定义读取的遥信数量
#define WH_YCNUM		//定义读取的遥测数量
#define WH_YKNUM		//定义遥控数量
#define WH_CINUM		//定义读取的电度数量


//维护标志头和尾
#define WHCMD_HEAD1		0xFF
#define WHCMD_HEAD2		0x12
#define WHCMD_END		0xFF

//发送
#define WHCMD_BEGIN		0x34
#define WHCMD_PAUSE		0x35
#define WHCMD_QUIT		0x36
#define WHCMD_CRC_BEG	(WHCMD_HEAD2+WHCMD_BEGIN)
#define WHCMD_CRC_PAUSE	(WHCMD_HEAD2+WHCMD_PAUSE)
#define WHCMD_CRC_QUIT	(WHCMD_HEAD2+WHCMD_QUIT)

//应答
#define WHCMD_ACQ_BEG	0x66
#define WHCMD_ACQ_PAUSE	0x77
#define WHCMD_ACQ_QUIT	0x88
#define WHCMD_CRC_AB	(WHCMD_HEAD2+WHCMD_ACQ_BEG)
#define WHCMD_CRC_AP	(WHCMD_HEAD2+WHCMD_ACQ_PAUSE)
#define WHCMD_CRC_AQ	(WHCMD_HEAD2+WHCMD_ACQ_QUIT)

extern uint32 dTCounter;
WH_STRUCT wh_com[2];

enum{
	FUN_EX_DI = 0x41,	//DI状态
	FUN_EX_AI,			//AI
	FUN_EX_DO,			//DO
	FUN_EX_CI,			//CI
	FUN_EX_RPN = 0x45,	//读参数名称
	FUN_EX_RP,			//读参数设置
	FUN_EX_RX,			//读地址
	FUN_EX_RPH,			//读电话号码
	FUN_EX_RT,			//读时间
	FUN_EX_RUD,			//读上送DI
	FUN_EX_RUA,			//读上送AI
	FUN_EX_RUO,			//读上送DO
	FUN_EX_WUD,			//写上送DI
	FUN_EX_WUA,			//写上送AI
	FUN_EX_WUO,			//写上送DO

	FUN_EX_WPN,			//写参数名称
	FUN_EX_WP,			//写参数设置
	FUN_EX_WX,			//写地址
	FUN_EX_WPH,			//写电话号码
	FUN_EX_WT,			//写时间
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

//读取端口数据
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

//读取端口参数
uint16 DealXRPNFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16	j;
	
	j = 3;
	j += ReadPortParam(sendBuf+3);

	sendBuf[2] = j-3;

	return j;
}

//读取公共参数
uint16 DealXRPFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16	j;
	
	j = 3;
	j += ReadBaseParam(sendBuf+3);

	sendBuf[2] = j-3;

	return j;
}

//读取电话号码
uint16 DealXRPHFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16	j;
	
	j = 3;
	j += ReadPhoneParam(sendBuf+3);

	sendBuf[2] = j-3;
	
	return j;
}

//读取设备地址
uint16 DealXRXFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16	j;
	
	j = 3;

	sendBuf[j++] = 0;
	sendBuf[j++] = devPS.m_Addr;

	sendBuf[2] = j-3;
	
	return j;
}

//读取时间
uint16 DealXRTFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16	i,j,k;
	
	j = 3;
	//有效性
	for(i=0; i<PORT_NUM; i++)
	{
		sendBuf[j++] = (devPS.Valid[i]>>8)&0xFF;
		sendBuf[j++] = devPS.Valid[i]&0xFF;
	}
	//报警
	for(i=0; i<PORT_NUM; i++)
	{
		sendBuf[j++] = (devPS.BjEnalbe[i]>>8)&0xFF;
		sendBuf[j++] = devPS.BjEnalbe[i]&0xFF;
	}
	
	//端口名称
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

//写公共参数
uint16 DealXWPFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16 iStartAddr;
	uint16 iWriteNum,iByteCount;
	
	iStartAddr = ((uint16)buffer[2]<<8) + buffer[3];
	iWriteNum   = ((uint16)buffer[4]<<8) + buffer[5];
	
	iByteCount =  buffer[6];
	
	if( iByteCount != 2*iWriteNum)
	{// 个数错误
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}
	if(iStartAddr != 0)
	{//地址错误
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

//写端口参数
uint16 DealXWPNFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16 iStartAddr;
	uint16 iWriteNum,iByteCount;
	
	iStartAddr = ((uint16)buffer[2]<<8) + buffer[3];
	iWriteNum   = ((uint16)buffer[4]<<8) + buffer[5];
	
	iByteCount =  buffer[6];
	
	if(iByteCount != 2*iWriteNum)
	{// 个数错误
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}
	if(iStartAddr != 0)
	{//地址错误
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


//写电话号码
uint16 DealXWPHFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16 iStartAddr;
	uint16 iWriteNum,iByteCount;
	
	iStartAddr = ((uint16)buffer[2]<<8) + buffer[3];
	iWriteNum   = ((uint16)buffer[4]<<8) + buffer[5];
	
	iByteCount =  buffer[6];
	
	if(iByteCount != 2*iWriteNum)
	{// 个数错误
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}
	if(iStartAddr != 0)
	{//地址错误
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

//写地址
uint16 DealXWXFrame(uint8 *buffer, uint8 *sendBuf)
{
	uint16 iStartAddr;
	uint16 iWriteNum,iByteCount;
	
	iStartAddr = ((uint16)buffer[2]<<8) + buffer[3];
	iWriteNum   = ((uint16)buffer[4]<<8) + buffer[5];
	
	iByteCount =  buffer[6];
	
	if(iByteCount != 2*iWriteNum)
	{// 个数错误
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}
	if(iStartAddr != 0)
	{//地址错误
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

//写时间
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
	{// 个数错误
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_VALUE;
		return 5;
	}
	if(iStartAddr != 0)
	{//地址错误
		sendBuf[1] += 0x80;
		sendBuf[2] = ILLEGAL_DATA_ADDRESS;
		return 5;
	}
	
	// 以下为处理时间校准
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
// 	// 判断设置时间
// 	if(CheckTime(Old_Clock) == 1)	// 时间正确,改变系统时间
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

//处理帧的总接口
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
		// 计算校验码。帧数据中，CRC 部分是低位在前，高位在后，与数据部分正好相反
		crcCode = Crc16CodeC( sendBuf, framelen);
		sendBuf[framelen++] = (crcCode & 0x00FF);
		sendBuf[framelen++] = (crcCode & 0xFF00) >> 8;
	}

	return framelen;
}

//此函数需要移植
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

	//最小帧长
	if(l_pCom->RXIndex < MINFRAMELEN)
		return 0;

	LruNo = l_pCom->RXdata[0];		//取得本端口的设备地址
	
//	if(LruNo != devPS.m_Addr)
	if(LruNo != 0x0)
		return 0;

	//取得命令
	Command = l_pCom->RXdata[1];

	{//地址要相同
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
			// 其它的不判断
			return 0;
		}
	}

	//数据不够长
	if(l_pCom->RXIndex < iFrameLen)
		return 0;

	// 处理
	{
		crcCode = Crc16CodeC(&l_pCom->RXdata[0], iFrameLen-2);
		Hi = crcCode >> 8;
		Lo = crcCode & 0x00FF;
		if( Lo == l_pCom->RXdata[iFrameLen-2] && Hi == l_pCom->RXdata[iFrameLen-1] )
		{// 校验码正确,CRC 部分是低位在前，高位在后，与数据部分正好相反
			// 改变接收缓冲区的处理位置，因为本次处理的帧长为iFrameLen
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

//必须要在初始化时执行
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
	{//数据不够，则不处理
		return 0;
	}

	l_pBuf = l_pCom->RXdata;
	len = l_pCom->RXIndex;
BEGIN:
	if((l_pBuf[0]==WHCMD_HEAD1) && (l_pBuf[4]==WHCMD_END))
	{
		if(l_pBuf[1] == WHCMD_HEAD2)
		{//进入维护
			switch(l_pBuf[2])
			{
			case WHCMD_BEGIN:
				{//进入维护，如没有数据，5分钟后自动退出
					if(l_pBuf[3] != WHCMD_CRC_BEG)
						return 0;
					return 1;
				}
			case WHCMD_PAUSE:
				{//暂停维护
					if(l_pBuf[3] != WHCMD_CRC_PAUSE)
						return 0;
					return 3;
				}
			case WHCMD_QUIT:
				{//退出维护
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


#define TEN_MINUTES	(10L*60*1000)	//1ms执行一次
//维护程序判断，其实就是判断标志然后进入MODBUS协议
uint8 CheckWh(COM_STRUCT*  l_pCom,uint8 whno,uint8 recflag)
{
	uint8 kk=0;
	WH_STRUCT *pWh = &wh_com[whno];
	
	if(whno > 1)
		return 0;
		
	//没有初始化，则不执行
	if(pWh->whInitFlag != 0xA5)
	{//判断是否执行过初始化
		Init_Wh(pWh);
	}

	if(recflag)
	{
		kk = CheckWHflag(l_pCom);
	}
	//根据返回值进行应答
	switch(kk)
	{
	case 1:
		{//进入维护
			uint8*  l_Data = l_pCom->TXdata;
			
			l_Data[0] = WHCMD_HEAD1;
			l_Data[1] = WHCMD_HEAD2;
			l_Data[2] = WHCMD_ACQ_BEG;
			l_Data[3] = WHCMD_CRC_AB;
			l_Data[4] = WHCMD_END;
			
			l_Data[5] = devPS.m_Addr;
			
			l_pCom->Senddatanum = 6;
			Uart_Send_start(l_pCom);

			pWh->l_lt1 = 0;		// 初始化时间,可由后台机定时发心跳，以免退出维护

			pWh->whflag = 1;	// 设置维护标识
			pWh->whPause = 0;
		}
		break;
	case 3:
		{//暂停维护
			uint8*  l_Data = l_pCom->TXdata;
			
			l_Data[0] = WHCMD_HEAD1;
			l_Data[1] = WHCMD_HEAD2;
			l_Data[2] = WHCMD_ACQ_PAUSE;
			l_Data[3] = WHCMD_CRC_AP;
			l_Data[4] = WHCMD_END;
			
			l_Data[5] = devPS.m_Addr;
			
			l_pCom->Senddatanum = 6;
			Uart_Send_start(l_pCom);
			
			pWh->l_lt1 = 0;		// 初始化时间,可由后台机定时发心跳，以免退出维护
			
			pWh->whflag = 1;		// 设置维护标识
			pWh->whPause = 1;		// 设立暂停维护标志
		}
		break;
	case 2:
		{//退出维护
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
	{//进入维护标志

		pWh->l_lt1++;
		if(pWh->l_lt1 > TEN_MINUTES)
		{//10分钟自动退出维护程序
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
		//暂停时
		if(pWh->whPause)
			return 0;

		if(recflag)
		{//接收完成才处理
			if(Wh_Run(l_pCom))
				pWh->l_lt1 = 0;	//有正常数据，则重新计时
		}
		
		return 1;
	}
	return 0;	// 返回0则调用函数的代码会继续执行
}

#endif		//CONFIGURTION_XX
