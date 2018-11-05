/********************************************************
File name: MODBUSM.c
Function: communication
Editor: HYK
Date: 20/09/2018
Version: V10
Description:
*********************************************************/
#include "define.h"
#include "modbusm.h"
#include "wxcw.h"
#include "uart.h"
#include "configuration.h"


extern uint32 dTCounter;

#define DEV_BLOCK_SIZE	6	//6个字=12个字节
#define LINE_DEV_NUMS	12	//每根天线接12个
#define PORT_NUM		6	//端口数

uint16 status[PORT_NUM][LINE_DEV_NUMS];	//状态
uint16 temp[PORT_NUM][LINE_DEV_NUMS];	//温度
uint16 valid[PORT_NUM];		//有效性

//通信配置数据类型
enum
{
	COMM_AI_TYPE = 0,
	COMM_DI_TYPE,
	COMM_DO_TYPE,
	COMM_CI_TYPE,
	COMM_PTTYPE_NUM
};

#define MODBUS_MAX_ERRORS  3
#define ERRORINTERVALTIME  (1000L * 300)

MODBUS_STRUCT MODBS[2];

uint16 IsTimeOut(uint32 preTickCount, uint32 OutMisSec)
{
	if(dTCounter >= preTickCount)
	{
		if((dTCounter - preTickCount) >= OutMisSec)//
		{
			return 1;
		}
	}
	else
	{
		if(((0xFFFFFFFF - preTickCount) + dTCounter) > OutMisSec)//2秒
		{
			return 1;			
		}
	}
	
	return 0;
}

uint16  Crc16Code(uint8  *pInFrame, uint16 dwInLen)
{
	uint16  crccode = 0xFFFF;
	uint16	i;
	uint16	j;
	for( j=0; j < dwInLen; j++)
	{
		crccode ^= (*pInFrame);
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

void InitModbus_Master_Com(MODBUS_STRUCT *pModbus)
{
	pModbus->Index = -1;
	pModbus->bInit = 0x5A;
	pModbus->MbStruct.PreTick = dTCounter;
}


int  GetNextSendIndex(uint8 nComNo)//得到下一个需要发送的结构, 如果返回-1表示没有需要发送的结构
{
#if 0
	static   int16  l_nCurLoopIndex = -1;
	uint32 l_OutMis;
	int16 l_Tmpint, i;

	if(nComNo >= MAX_COMNUM)
	{
		return -1;
	}

	if(l_nCurLoopIndex == -1)
	{
		l_nCurLoopIndex += g_ModbusStructCount;
	}
	
	//刷新的时候可能出现问题
	if(l_nCurLoopIndex >= g_ModbusStructCount)
	{
		l_nCurLoopIndex = 0;
	}

	l_Tmpint = l_nCurLoopIndex + 1;
	
	while(1)
	{

		switch(g_ModbusStruct.nType)
		{
		case COMM_AI_TYPE:
			{
				if(g_ModbusStruct.ErrorNumbers < MODBUS_MAX_ERRORS)
				{
			  		l_OutMis = 3000;//3秒
					if(IsTimeOut(g_ModbusStruct.PreTick, l_OutMis))
					{
						l_nCurLoopIndex = l_Tmpint;
						return l_Tmpint;
					}
				}
				else
				{
					if(IsTimeOut(g_ModbusStruct.PreTick, ERRORINTERVALTIME))
					{
						g_ModbusStruct[l_Tmpint].ErrorNumbers = 0;
						l_nCurLoopIndex = l_Tmpint;
						return l_Tmpint;						
					}
				}
			}
		break;
		case COMM_DI_TYPE:
			{
				if(g_ModbusStruct.ErrorNumbers < MODBUS_MAX_ERRORS)
				{
					l_OutMis = 400;//200毫秒
					if(IsTimeOut(g_ModbusStruct.PreTick, l_OutMis))
					{
						l_nCurLoopIndex[ = l_Tmpint;
						return l_Tmpint;
					}
				}
				else
				{
					if(IsTimeOut(g_ModbusStruct.PreTick, ERRORINTERVALTIME))
					{
						g_ModbusStruct.ErrorNumbers = 0;
						l_nCurLoopIndex = l_Tmpint;
						return l_Tmpint;						
					}
				}
			}
		break;
		default:
			if(l_Tmpint == l_nCurLoopIndex)
			{
				break;
			}
			l_Tmpint++;
			continue;//如果是DO,这个是不发送的
		}
		
		if(l_Tmpint == l_nCurLoopIndex)
		{
			break;
		}
		l_Tmpint++;
	}
#endif
	return -1;
}

//1ms执行一次
void ProModbusFrame_Master(COM_STRUCT *l_pCom,MODBUS_STRUCT *pModbus,uint8 recflag)
{
	uint16  l_RetLen;
	uint16  l_crc, i;
	uint8* l_pBuf;
//	uint8 l_Len;
	static uint8 t12=0;
	int16 l_nTmpIndex;


	l_pBuf = l_pCom->RXdata;

	if(CHKBITSET(l_pCom->funcflag,uartsenddata))
	{//上次数据还没发完,返回
		return;
	}

	if(pModbus->bInit != 0x5A)
	{
		InitModbus_Master_Com(pModbus);
	}

	if(pModbus->Index == -1)
	{//发新的命令
		if(IsTimeOut(pModbus->MbStruct.PreTick, 100)==0) 
		{//命令间隔100毫秒
			return;
		}

		l_nTmpIndex = GetModbusStruct(&(pModbus->MbStruct));
		if(l_nTmpIndex == -1)
		{
			return;
		}
		FormatSendFrame(l_pCom->TXdata, &l_RetLen, pModbus->MbStruct);
		if(l_RetLen == 0)
		{
			return;
		}
		pModbus->Index = l_nTmpIndex;
		pModbus->MbStruct.PreTick = dTCounter;

		l_pCom->Senddatanum = l_RetLen;
		l_pCom->Senddatanumold = l_RetLen;
		t12 = 0;
		Uart_Send_start(l_pCom);
	}
	else
	{//等待上次发的命令结果

		if(IsTimeOut(pModbus->MbStruct.PreTick, 1000))//如果没有超时,需要继续等待数据 
		{//命令超时1000毫秒
			SetCommStatusErr(1);
			t12++;
			if(t12 >= 2)
			{
				t12 = 0;
				pModbus->Index = -1;//可以继续发送
				pModbus->MbStruct.PreTick = dTCounter;
			}
			else
			{//重发一次
				pModbus->MbStruct.PreTick = dTCounter;
				l_pCom->Senddatanum = l_pCom->Senddatanumold;
				Uart_Send_start(l_pCom);
			}
			return;
		}
		//接收没有完成
		if(recflag==0)
			return;

		i = 0;
		while(1)
		{
			if(l_pCom->RXIndex < 5)
			{
				break;
			}
			if(l_pBuf[0] != pModbus->MbStruct.DeviceAddr)//如果当前的数据同地址不对
			{
				pModbus->Index = -1;//可以继续发送
				break;
			}
			if(l_pBuf[1] & 0x80)//功能码错误
			{
				pModbus->Index = -1;//可以继续发送
				break;
			}

			if( (l_pBuf[1] & 0x7F) != pModbus->MbStruct.FunCode)
			{//功能码不匹配
				pModbus->Index = -1;//可以继续发送
				break;

			}
			if(l_pCom->RXIndex < l_pBuf[2] + 5)
			{
				break;//继续等待
			}

				//数据够,可以继续处理
			switch(pModbus->MbStruct.nType)
			{
			case COMM_AI_TYPE:
			case COMM_DI_TYPE:
				{
					l_crc = Crc16Code( l_pBuf, l_pBuf[2]+3);
					//组帧时，CRC的低8位在前，高8位在后
					if( (l_pBuf[3 + l_pBuf[2]] == ((l_crc) & 0xFF))
						&& (l_pBuf[3 + l_pBuf[2] + 1] == ((l_crc >> 8) & 0xFF)))//如果校验和相等
					{
						DealFrame(1, l_pBuf, l_pBuf[2]+5, pModbus->MbStruct);
					}
				}
				break;
			}

			pModbus->Index = -1;//可以继续发送
			break;
		}
		//清除接收数据
		for(i=0; i<l_pCom->RXIndex; i++)
		{
			l_pCom->RXdata[i] = 0;
		}
		l_pCom->RXIndex=0; 
	}
}

void FormatSendFrame(uint8* Data, uint16* DataLen,  MODBUSINFO ModbusInfo)//调用方需要保证参数的
{
	uint16 l_Crc;

	switch(ModbusInfo.nType)
	{
	case COMM_AI_TYPE://表示Ai
		FormatAiFrame(Data, DataLen, ModbusInfo);
		break;
	case COMM_DI_TYPE:
		FormatDiFrame(Data, DataLen, ModbusInfo);
		break;
	default:
		*DataLen = 0;
		break;
	}
	if(*DataLen > 0)
	{
		l_Crc	= Crc16Code(Data, 6);
		//组帧时，CRC的低8位在前，高8位在后
		Data[*DataLen-2] = (l_Crc ) & 0xFF;
		Data[*DataLen-1] = (l_Crc >> 8) & 0xFF;
	}
}

void FormatAiFrame(uint8* Data, uint16* DataLen, MODBUSINFO ModbusInfo)
{
	Data[0] = ModbusInfo.DeviceAddr;
	Data[1] = ModbusInfo.FunCode;
	Data[2] = (ModbusInfo.Regiseter >> 8) & 0xFF;
	Data[3] = (ModbusInfo.Regiseter) & 0xFF;
	Data[4] = (ModbusInfo.RegNumbers >> 8) & 0xFF;
	Data[5] = (ModbusInfo.RegNumbers) & 0xFF;
	*DataLen = 8;
}

void FormatDiFrame(uint8* Data, uint16* DataLen, MODBUSINFO ModbusInfo)
{
	Data[0] = ModbusInfo.DeviceAddr;
	Data[1] = ModbusInfo.FunCode;
	Data[2] = (ModbusInfo.Regiseter >> 8) & 0xFF;
	Data[3] = (ModbusInfo.Regiseter) & 0xFF;
	Data[4] = (ModbusInfo.RegNumbers >> 8) & 0xFF;
	Data[5] = (ModbusInfo.RegNumbers) & 0xFF;
	*DataLen = 8;
}

void FormatDoFrame(uint8* Data, uint16* DataLen, MODBUSINFO ModbusInfo, uint16 DOInfoNo, uint16 bClose)
{
	Data[0] = ModbusInfo.DeviceAddr;
	Data[1] = ModbusInfo.FunCode;
	Data[2] = (DOInfoNo >> 8) & 0xFF;
	Data[3] = DOInfoNo & 0xFF;
	if(bClose)
	{
		Data[4] = (ModbusInfo.CloseCode >> 8) & 0xFF;
		Data[5] = (ModbusInfo.CloseCode) & 0xFF;
	}
	else
	{
		Data[4] = (ModbusInfo.OpenCode >> 8) & 0xFF;
		Data[5] = (ModbusInfo.OpenCode) & 0xFF;
	}
	*DataLen = 8;
}

void DealFrame(uint8 nComNo, uint8* Data, uint16 DataLen, MODBUSINFO ModbusInfo)
{
	//把这个地址的全部清除
	switch(ModbusInfo.nType)
	{
	case COMM_AI_TYPE:
		DealAiFrame(nComNo, Data + 3, DataLen - 5, ModbusInfo);
		break;
	case COMM_DI_TYPE:
		DealDiFrame(nComNo, Data + 3, DataLen - 5, ModbusInfo);
		break;
	case COMM_DO_TYPE:
		DealDoFrame(nComNo, Data + 3, DataLen - 5, ModbusInfo);
		break;
	default:
		break;
	}
}

void DealAiFrame(uint8 nComNo, uint8* Data, uint16 DataLen, MODBUSINFO ModbusInfo)
{
	ProcessData(&Data[3], DataLen-3);
	SetCommStatusErr(0);
}

void DealDiFrame(uint8 nComNo, uint8* Data, uint16 DataLen, MODBUSINFO ModbusInfo)
{
#if 0
	uint16 l_nTmpIndex,l_nArrayIndex;
	uint16 i, l_nBitPos;
	if(DataLen * 8 < ModbusInfo.RegNumbers)//数据部分
	{
		return;
	}
	
	for(i = 0; i < ModbusInfo.RegNumbers; i++)
	{
		l_nTmpIndex = ModbusInfo.SysInfoNo + i;
		 if(l_nTmpIndex >= YX_NUM_MAX+ GetPerDevPointNum(COMM_DI_TYPE))
			 continue;
		 l_nArrayIndex = l_nTmpIndex  - YX_NUM_MAX;
//		if(l_nTmpIndex < YX_ALL_NUM)
		{
			l_nBitPos = l_nTmpIndex % ONE_WORD_BITS;
			if((GetPerDevPtInit(l_nArrayIndex) >> l_nBitPos) & 0x01)//如已经初始化了， 可以产生SOE
			{
				if(((Data[i / 8] >> (i % 8)) & 0x01) != ((g_yx_Word_value[l_nTmpIndex / ONE_WORD_BITS] >> l_nBitPos) & 0x01))
				{
					if((Data[i / 8] >> (i % 8)) & 0x01)
					{
						SET_YXVALUE_NO(l_nTmpIndex);
// 						g_yx_Word_value[l_nTmpIndex / ONE_WORD_BITS] |= (1 << l_nBitPos);
						Add_ChangeDi(l_nTmpIndex, 1);
						AddSoe(l_nTmpIndex, 1, &g_CLK);
					}
					else
					{
						CLEAR_YXVALUE_NO(l_nTmpIndex);
// 						g_yx_Word_value[l_nTmpIndex / ONE_WORD_BITS] &= ~(1 << l_nBitPos);
						Add_ChangeDi(l_nTmpIndex, 0);
						AddSoe(l_nTmpIndex, 0, &g_CLK);
					}
//					SaveDTUInfo(FLASH_SOE_TYPE, (Data[i / 8] >> (i % 8)) & 0x01, l_nTmpIndex, &CLK);
				}
			}
			else
			{
				SetPerDevPtInit(l_nArrayIndex, (0x01 << l_nBitPos));
				if((Data[i / 8] >> (i % 8)) & 0x01)
				{
					SET_YXVALUE_NO(l_nTmpIndex);
// 					g_yx_Word_value[l_nTmpIndex / ONE_WORD_BITS] |= (1 << l_nBitPos);
				}
				else
				{
					CLEAR_YXVALUE_NO(l_nTmpIndex);
// 					g_yx_Word_value[l_nTmpIndex / ONE_WORD_BITS] &= ~(1 << l_nBitPos);
				}
			}
		}
	}
#endif
}

void DealDoFrame(uint8 nComNo, uint8* Data, uint16 DataLen, MODBUSINFO ModbusInfo)
{
#if 0
	//把信息返回给后台,根据上送的串口的协议类型
	if(Data[2] & 0x80)//失败
	{
		if(g_Comm[g_Comm[nComNo].m_DOinfo.m_RetCom].m_DoUpInfo.m_nCurStep)
		{
			g_Comm[g_Comm[nComNo].m_DOinfo.m_RetCom].m_DoUpInfo.m_nSubReturn = YK_RETURN_EXEC_FAIL;
		}
	}
	else//成功
	{
		if(g_Comm[g_Comm[nComNo].m_DOinfo.m_RetCom].m_DoUpInfo.m_nCurStep)
		{
			g_Comm[g_Comm[nComNo].m_DOinfo.m_RetCom].m_DoUpInfo.m_nSubReturn = YK_RETURN_EXEC_SUCC;
		}
	}
	Master_YK_Init(nComNo);
#endif

}

