#include "general.h"



s16 g_Communicate_yc_value[MAX_COMNUM][YC_ALL_NUM];	
u8 gzsb_flag;

u16 PerDeviceData[MAX_PORT_DEVICE][MAX_DEVICEBYTES];
u16 PerDeviceData_Len[MAX_PORT_DEVICE];
u16 g_PerDevicePointNumber[MAX_PORT_DEVICE];//�ⲿ�豸���ɵ�Ai, Di, Do,CI������
u16  g_PerDeviceDiDataInit[MAX_DI_PER_NUM];//��ʾ�����Di�Ƿ��ʼ����

COMM_SOE_STRUCT g_CommSendSoe[MAX_COMNUM];

//������豸����Ϣ�� ���豸��ô���Ƿ�������һ�����ݣ� �������ݣ����ٻ������յ���Frame����������g_m101Struct�ṹ����Ϣ�ŵ���Ӧ�ĵ�ַ��
IEC101_DEVICE_INFO  g_101DeviceInfo[MAX_COMNUM][MAX_101_DEVICE_NUM];
IEC101_SET_STRUCT Iec101set_S;		//��վ101��������
IEC101_SET_STRUCT Iec101set_M;		//��վ101��������
u8 ipRequire1,ipRequire2;


s16 g_ci_type[CI_ALL_NUM];	
u16 g_ci_valid[(CI_ALL_NUM+15)/16];	
u16 g_ai_valid[(YC_ALL_NUM+15)/16];	
u16 g_di_valid[(YX_ALL_NUM+15)/16];	

clk_structure g_ci_clk[CI_ALL_NUM];


COM_STRUCT g_Comm[MAX_COMNUM];
YK_STRUCT g_yk;


union PROTOCOL_STRUCT  g_ProtoclInfo[MAX_COMNUM];


u16 IsLeap(u16 Year)
{
	if(Year % 100 == 0)
	{
		if(Year % 400 == 0)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if(Year % 4 == 0)
	{
		return 1;
	}
	return 0;
}


u16 GetPerDevPointNum(u8 pttype)
{
	if(pttype >= COMM_PTTYPE_NUM)
		return 0;

	return g_PerDevicePointNumber[pttype];
}

u16 GetPerDevPtInit(u16 ptIdx)
{
	return g_PerDeviceDiDataInit[ptIdx	/ ONE_WORD_BITS];
}

void SetPerDevPtInit(u16 ptIdx, u16 val)
{
	g_PerDeviceDiDataInit[ptIdx / ONE_WORD_BITS] |= val;
}

u16 Is_DoInfoAddr_Valid(u32 DoInfoAddr)
{
	return (DoInfoAddr != INVALID_DOINFOADDR);
}

u32 GetTimeInterval(u32 preTickCount)
{
	u32 tt;
	if(g_TickCount >= preTickCount)
	{
		tt = g_TickCount - preTickCount;
	}
	else
	{
		tt = TICKMS_MAX - preTickCount + g_TickCount;
	}

	return tt;
}

u16 IsTimeOut(u32 preTickCount, u32 OutMisSec)
{
	if(g_TickCount >= preTickCount)
	{
		if((g_TickCount - preTickCount) >= OutMisSec)//
		{
			return 1;
		}
	}
	else
	{
		if(((TICKMS_MAX - preTickCount) + g_TickCount) > OutMisSec)//2��
		{
			return 1;			
		}
	}
	
	return 0;
}

u8 ByteAdd(u8 buff[],u16 nbytes)
{ 
	u16 i;                                                   
	u8 dmhv;

	dmhv=0;
	for(i=0; i<nbytes; i++)
    {
		dmhv += buff[i];
    }
    
	return (dmhv);
}

//�������ǰѶ˿ڼ��ϣ����߻��е�������ʲô����
void sendip(u8 commNo)
{
	static u8 ipbuf[40];
	u16 addr;

	if(commNo > 1)
		return;

	if(commNo==0)
	{
		if( ipRequire1 )
			ipRequire1 = 0;
	}
	else
	{
		if( ipRequire2 )
			ipRequire2 = 0;
	}

	//head1
	ipbuf[0] = 0xfe;
	ipbuf[1] = 0xfe;

	//head2
	ipbuf[2] = 0x68;
	ipbuf[3] = 30;
	ipbuf[4] = 30;
	ipbuf[5] = 0x68;

	ipbuf[6] = 0x08;
	ipbuf[7] = g_Comm[commNo].m_Addr;
//asdu
	//key value
	ipbuf[8] = 0x0a;
	ipbuf[9] = 0x81;
	ipbuf[10] = 0x07;
	ipbuf[11] = addr;
	ipbuf[12] = 254;//0xfe
	ipbuf[13] = 241;//0xf1
	ipbuf[14] = 0x00;
	ipbuf[15] = 0x02;

	ipbuf[16] = 0x00;
	ipbuf[17] = 0x01;
	ipbuf[18] = 0x01;
	ipbuf[19] = 0x01;
	ipbuf[20] = 0x01;
	ipbuf[21] = 0x04;
	//ip
// 	if(commNo==0)
	{
		ipbuf[22] = g_Parameter.ipSet.ipaddr[0];
		ipbuf[23] = g_Parameter.ipSet.ipaddr[1];
		ipbuf[24] = g_Parameter.ipSet.ipaddr[2];
		ipbuf[25] = g_Parameter.ipSet.ipaddr[3];
	}

	ipbuf[26] = 0x00;
	//gate
// 	if(commNo==0)
	{
		ipbuf[27] = g_Parameter.ipSet.ipgate[0];
		ipbuf[28] = g_Parameter.ipSet.ipgate[1];
		ipbuf[29] = g_Parameter.ipSet.ipgate[2];
		ipbuf[30] = g_Parameter.ipSet.ipgate[3];
	}


	ipbuf[31] = 0x04;
	//mask
//	if(commNo==0)
	{
		ipbuf[32] = g_Parameter.ipSet.ipmask[0];
		ipbuf[33] = g_Parameter.ipSet.ipmask[1];
		ipbuf[34] = g_Parameter.ipSet.ipmask[2];
		ipbuf[35] = g_Parameter.ipSet.ipmask[3];
	}

//cs
	ipbuf[36] = ByteAdd(&ipbuf[6], 30);
	ipbuf[37] = 0x16;

	ipbuf[38] = 0xfe;
	ipbuf[39] = 0xfe;

	g_Comm[commNo].busy = 1;

	usart_SendDataToBuffer(ipbuf, 40, commNo);
	g_Comm[commNo].busy = 0;

}

void SendCloseSocket(u8 commNo)
{
	u8 txBuf_close_socket[] = {0x70,0x04,0xE0,0x20,0xF0,0x60};

	g_Comm[commNo].busy = 1;

	usart_SendDataToBuffer(txBuf_close_socket, 6, commNo);
	g_Comm[commNo].busy = 0;
}

void  CopyDataToBuffer(u8* TmpBuffer,u8* SourceBuffer, int outpos, int MaxSize, int  DatLen)
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

//�ж��Ƿ�ΪIP֡
//u16 txBuf1[15] = {0x68,0x04,0x02,0x00,0x00,0x00,0xD0,0x01,0x03,0x00,0x00,0x00,0x00,0x00,0x00};
void CheckIPFrame(u8 CommNo)
{
	u16 l_OutPos,l_InPos;
	
	u8 pData[6];
	
	if(CommNo > 1)
		return;

	if(g_Comm[CommNo].m_RXBuffer->l_InPos == g_Comm[CommNo].m_RXBuffer->l_OutPos)
		return;	// δ�յ����ݣ��򲻴���
	
	l_OutPos = g_Comm[CommNo].m_RXBuffer->l_OutPos;
	l_InPos = g_Comm[CommNo].m_RXBuffer->l_InPos;
	if(l_InPos >= l_OutPos)
	{//���ݲ������򲻴���
		if( l_InPos < (l_OutPos+6) )
			return;
	}
	else
	{
		if( (l_InPos+g_Comm[CommNo].m_RXBuffer->l_Buffer_size) < (l_OutPos+6) )
			return;
	}
	
	CopyDataToBuffer(pData,g_Comm[CommNo].m_RXBuffer->l_Buffer,l_OutPos,g_Comm[CommNo].m_RXBuffer->l_Buffer_size,6);

	if( (pData[0] == 0x70) && (pData[1] == 0x04) )
	{
		if( (pData[2] == 0xC0) && (pData[3] == 0x20) && (pData[4] == 0xD0) && (pData[5] == 0x60) )
		{//�յ���������IP,����ip������
			sendip(CommNo);
			if(g_Comm[CommNo].ProtocolType==PROTOCOL_SLAVER_104)
				SLAVER_104_Init_Channel(CommNo);
		}
		else if( (pData[2] == 0xE0) && (pData[3] == 0x20) && (pData[4] == 0xF0) && (pData[5] == 0x60) )
		{//���������ж���������
			if(g_Comm[CommNo].ProtocolType==PROTOCOL_SLAVER_104)
				SLAVER_104_Init_Channel(CommNo);
		}
		else if( (pData[2] == 0xE0) && (pData[3] == 0x20) && (pData[4] == 0xE0) && (pData[5] == 0x60) )
		{//���������ͻ���������֪ͨ
			if(g_Comm[CommNo].ProtocolType==PROTOCOL_SLAVER_104)
			{
				SLAVER_104_Init_Channel(CommNo);
				SLAVER_104_Set_TCP(CommNo);	//�趨TCP����
			}
		}
		else
			return;
	}
	else
		return;

	//������ȷ����ȥ����Щ���ݣ������ض�Ӧֵ��
	l_OutPos += 6;
	if(l_OutPos >= g_Comm[CommNo].m_RXBuffer->l_Buffer_size)
	{
		l_OutPos = l_OutPos - g_Comm[CommNo].m_RXBuffer->l_Buffer_size;
	}
	g_Comm[CommNo].m_RXBuffer->l_OutPos = l_OutPos;

}


//ϵͳ��Ȱ���˳�����У����˳��Ҳ�Ǹ�����Ӵ��ڲɼ��ĵ��˳����һ�µġ�
//��������˳��ı��ˣ������˳��Ҳ����ı�!
void inital_Ci_type()
{
	u16 j,k;
	k = 0;
	for(j=0; j<8; j++)
		g_ci_type[k++] = CITYPE_CUR;
	for(j=0; j<8; j++)
		g_ci_type[k++] = CITYPE_FRZ15;
	for(j=0; j<8; j++)
		g_ci_type[k++] = CITYPE_FRZD;
	for(j=0; j<8; j++)
		g_ci_type[k++] = CITYPE_FLOW;
}

void Init_DevSysBasePoint()
{
	DevSysPt[COMM_AI_TYPE] = YC_NUM_MAX;
	DevSysPt[COMM_DI_TYPE] = YX_NUM_MAX;
	DevSysPt[COMM_DO_TYPE] = YK_NUM_MAX;
	DevSysPt[COMM_CI_TYPE] = CI_NUM_MAX;
}

//�ϵ�ʱ���Դ��ڽ��г�ʼ��
void  InitCommunicate()//��ʼ������ͨ����Ϣ
{
	u16 i;

	//��ʼ��, soe���м��仯�Ķ���
	for(i = 0; i < MAX_COMNUM; i++)
	{
		initComm(i);	
	}

	Iec101set_M.m_CauseLen = 2;
	Iec101set_M.m_InfoLen = 2;
	Iec101set_M.m_LinkAddrLen = 2;
	Iec101set_M.m_PubAddrLen = 2;
	Iec101set_M.m_nAiSendCycleS = 10;	//��λ��S

	Iec101set_S.m_CauseLen = 2;
	Iec101set_S.m_InfoLen = 2;
	Iec101set_S.m_LinkAddrLen = 2;
	Iec101set_S.m_PubAddrLen = 2;
	Iec101set_S.m_nAiSendCycleS = 10;	//��λ��S

	ipRequire1 = 0;
	ipRequire2 = 0;

	gzsb_flag = 0;
	for(i = YC_NUM_MAX; i < YC_ALL_NUM; i++)
	{
		g_yc_value[i] = 0;
	}
	for(i = YX_NUM_MAX/16; i < ((YX_ALL_NUM+15)/16); i++)
	{
		g_yx_Word_value[i] = 0;
	}
	for(i=0; i<CI_ALL_NUM; i++)
	{
		g_dd_value[i] = 0.0;
	}

	//ң����Ч��
	for(i=0; i<(YC_ALL_NUM+15)/16; i++)
	{
		g_ai_valid[i] = 0;
	}
	for(i = 0; i < ((YX_ALL_NUM+15)/16); i++)
	{
		g_di_valid[i] = 0;
	}
	//�����Ч��
	for(i=0; i<(CI_ALL_NUM+15)/16; i++)
	{
		g_ci_valid[i] = 0;
	}
	
	inital_Ci_type();

	for(i = 0; i< MAX_DI_PER_NUM; i++)
	{
		g_PerDeviceDiDataInit[i] = 0;
	}

	g_yk.yk_flag.on = 0;
	g_yk.yk_flag.off = 0;
	g_yk.yk_on_time = 0;
	g_yk.yk_off_time = 0;

	Init_DevSysBasePoint();

}

void  initComm(u8 CommNo)
{
	u16 i;
	if(CommNo >= MAX_COMNUM)
	{
		return;
	}
	g_Comm[CommNo].m_Addr = 1;
	g_Comm[CommNo].m_nSendLen = 0;
	switch(CommNo)
	{
	case 0:		//COM1��ΪMASTER�������裬������ģ���
		g_Comm[CommNo].ProtocolType = PROTOCOL_MASTER_101;
		break;
	case 1:		//COM2��ΪSLAVER������վ��101��modbus��
		g_Comm[CommNo].ProtocolType = PROTOCOL_SLAVER_101_POISED;
		break;
	default:	//����Ϊ��̫������ΪSLAVER������վ��104Э��
		g_Comm[CommNo].ProtocolType = PROTOCOL_SLAVER_104;
		break;
	}

	g_Comm[CommNo].m_pAIUpInfo_Num = &g_Parameter.upinfo.UpInfoNum[COMM_AI_TYPE];
	g_Comm[CommNo].m_pDIUpInfo_Num = &g_Parameter.upinfo.UpInfoNum[COMM_DI_TYPE];
	g_Comm[CommNo].m_pDOUpInfo_Num = &g_Parameter.upinfo.UpInfoNum[COMM_DO_TYPE];
	g_Comm[CommNo].m_pCIUpInfo_Num = &g_Parameter.upinfo.UpInfoNum[COMM_CI_TYPE];

	g_Comm[CommNo].m_pAIUpInfo = g_Parameter.upinfo.AiUpInfo;
	g_Comm[CommNo].m_pDIUpInfo = g_Parameter.upinfo.DiUpInfo;
	g_Comm[CommNo].m_pDOUpInfo = g_Parameter.upinfo.DoUpInfo;
	g_Comm[CommNo].m_pCIUpInfo = g_Parameter.upinfo.CiUpInfo;

	g_Comm[CommNo].ComNo = CommNo;//�������Ӧ�ÿ��Բ�Ҫ, ������±�ʹ����ں�
	
	g_Comm[CommNo].m_CheckTickCount = g_TickCount;
	g_Comm[CommNo].m_bCheckTime = 0;//����ʱ
	g_Comm[CommNo].m_TickCount = g_TickCount;
	g_Comm[CommNo].m_nLastSendTickCount = g_TickCount;
	g_Comm[CommNo].m_bCanAddChangeDi = 0;

	if(CommNo < 2)
	{
		g_Comm[CommNo].m_bRecvOverFlag = &g_usart[CommNo].m_bRecvOverFlag;
		g_Comm[CommNo].m_RXBuffer = &g_usart[CommNo].m_rxbuf;
		g_Comm[CommNo].m_TXBuffer = &g_usart[CommNo].m_txbuf;
	}
	else
	{
		
	}

	g_Comm[CommNo].m_pRecvProBuffer = g_ComProcesBuffer[CommNo];
	g_Comm[CommNo].m_pSendProBuffer = g_ComSendProBuffer[CommNo];
	g_Comm[CommNo].m_nSendTimes = 0;
	g_Comm[CommNo].busy = 0;

	g_Comm[CommNo].m_bxxaq = (g_Parameter.Encrpted&(1<<CommNo))?1:0;
	g_Comm[CommNo].m_bxxaq = 0;

	//Masterң�س�ʼ��
	Master_YK_Init(CommNo);

	//Slaverң�س�ʼ��
	Slaver_YK_Init(CommNo);

	for(i = MAX_AI_PER_NUM; i< YC_ALL_NUM; i++)
	{
		g_Communicate_yc_value[CommNo][i] = 0;
	}

	g_ChangeDI_List.m_nOutPos[CommNo] = g_ChangeDI_List.m_nInPos;//û����Ҫ���͵�

}


void Protocol_Init(u8 CommNo)
{
//������
	if(CommNo==0)
		g_Comm[CommNo].ProtocolType = PROTOCOL_SLAVER_101_POISED;

	switch(g_Comm[CommNo].ProtocolType)
	{
	case PROTOCOL_SLAVER_101:
		SLAVER101_InitProtocol_Info(CommNo);
		break;
//	case PROTOCOL_SLAVER_MODBUS:
//		InitModbus_Slave_Com(CommNo);
//		break;
// 	case PROTOCOL_MASTER_MODBUS:
// 		InitModbus_Master_Com(CommNo);
// 		break;
	case PROTOCOL_SLAVER_104:
		SLAVER_104_Init_Channel(CommNo);
		break;
	case PROTOCOL_SLAVER_101_POISED:
		POISED_SLAVER101_InitProtocol_Info(CommNo);
		break;
	case PROTOCOL_MASTER_101:
		IntiIEC101_Master(CommNo);
		break;
	default:
		break;
	}
}


u16  MoveRxBufferOutPoint(u8 nComNo, u16 nLen)
{
	u16 l_nTmpPos;
	l_nTmpPos = g_Comm[nComNo].m_RXBuffer->l_OutPos + nLen;
	if(l_nTmpPos >= g_Comm[nComNo].m_RXBuffer->l_Buffer_size)
	{
		l_nTmpPos -= g_Comm[nComNo].m_RXBuffer->l_Buffer_size;
	}
	g_Comm[nComNo].m_RXBuffer->l_OutPos = l_nTmpPos;

	return nLen;
}


//���ڴ��ڷ��͵ĺ���,����ֱ��д�봮�ڵĻ�����,��д��Э��Ļ�����
u16  ComSendData(BYTE* Data, u16 SendLen, u8 CommNo)//��������,���ط��ͳɹ�������
{
	if(g_Comm[CommNo].m_bxxaq)
	{//����
		SendIecMsg(CommNo, Data, SendLen);
		return 0;
	}
	return usart_SendDataToBuffer(Data, SendLen, CommNo);
}

//����1��nCommNo:�˿ں�
//����2��SysNo:ϵͳң�ŵ��
//����ֵ��ң�ŵ�ӳ����
u16 GetYxMapPoint(u8 nCommNo, u16 SysNo)
{
	u16 l_DiUpInfo;
	if(SysNo >= *(g_Comm[nCommNo].m_pDIUpInfo_Num))
		return 0xFFFF;

	l_DiUpInfo = g_Comm[nCommNo].m_pDIUpInfo[SysNo];	//��ȡӳ�����,��ϵͳ�еĵ��
	
	if(l_DiUpInfo >= (YX_NUM_MAX + MAX_DI_PER_BITS) )
		return 0xFFFF;
	else
		return l_DiUpInfo;
}

//����1��nCommNo:�˿ں�
//����2��SysNo:ϵͳң����
//����ֵ��ң���ӳ����
u16 GetYcMapPoint(u8 nCommNo, u16 SysNo)
{
	u16 l_AiUpInfo;
	if(SysNo >= *(g_Comm[nCommNo].m_pAIUpInfo_Num))
		return 0xFFFF;
	
	l_AiUpInfo = g_Comm[nCommNo].m_pAIUpInfo[SysNo];	//��ȡӳ�����,��ϵͳ�еĵ��
	
	if(l_AiUpInfo >= (YC_NUM_MAX + MAX_AI_PER_NUM) )
		return 0xFFFF;
	else
		return l_AiUpInfo;
}


//����1��nCommNo:�˿ں�
//����2��YkCtrlNo:�����е�ң��˳����,Ҫ��ȥ����ַ��0x6001
//����3��bClose:�Ƿ��բ
//����4��bDouble:�Ƿ�˫��ң��
//����ֵ��ң�ص�ӳ����
u16 GetYkMapPoint(u8 nCommNo, u16 YkCtrlNo, u16 bClose, u16 bDouble)
{
	u16 l_DoUpInfo;
	if(YkCtrlNo >= *(g_Comm[nCommNo].m_pDOUpInfo_Num))
		return 0xFFFF;
	if(bDouble)
	{//˫��ң��ʱ
		YkCtrlNo *= 2;	// ��բ���
		if(bClose==0)		// ��բ��Ž����ź�բ���ӳ��
		{
			if(YkCtrlNo < (*(g_Comm[nCommNo].m_pDOUpInfo_Num)-1))
				YkCtrlNo++;
		}
	}
	l_DoUpInfo = g_Comm[nCommNo].m_pDOUpInfo[YkCtrlNo];	//��ȡӳ�����,��ϵͳ�еĵ��

	if(l_DoUpInfo >= (YK_NUM_MAX + MAX_DO_PER_BITS) )
		return 0xFFFF;
	else
		return l_DoUpInfo;
}


//����1��nCommNo:�˿ں�
//����2��DiNo:ӳ��ң�ŵ�˳����,Ҫ��ȥ����ַ��0x01
//����3��bDouble:�Ƿ�˫��ң��
//����ֵ��ң�ŵ���ֵ
u16 GetDiMapPtValue(u8 nCommNo, u16 DiNo, u16 bDouble)
{
	u16 l_DiUpInfo;
	
	//�����ܵĸ���ʱ��������Ч����
	if(DiNo >= *g_Comm[nCommNo].m_pDIUpInfo_Num)
		return 0x80;
	
	l_DiUpInfo = g_Comm[nCommNo].m_pDIUpInfo[DiNo];	//��ȡӳ�����,��ϵͳ�еĵ��
	
	if( bDouble)
	{//˫��

		if(l_DiUpInfo != YX_BIT_KGWZ)
			return 0x80;

		if(GET_YXVALUE_NO(YX_BIT_KGHW))
		{//��λΪ1
			if(GET_YXVALUE_NO(YX_BIT_KGFW))
				return 3;	//��Чλ��
			else
				return 2;	//��λ
		}
		else
		{
			if(GET_YXVALUE_NO(YX_BIT_KGFW))
				return 1;	//��λ
			else
				return 0;	//��Чλ��
		}
	}
	else
	{//����
		if(l_DiUpInfo < (YX_NUM_MAX + g_PerDevicePointNumber[COMM_DI_TYPE]))
		{
			return GET_YXVALUE_NO(l_DiUpInfo);
		}
	}

	return 0x80;
}

//ʹ�÷���ֵ 0xFFFF��ʾ��������ڣ� 100��ʾ�Լ��㣬0-3��ʾ��Ӧ�˿�
//DoSysNo--ң�ص�ţ�ϵͳ�еĵ��
//����2��nComNo--��ǰ��ͨѶ�˿ں�
u16 Get1InfoDoCom(s32 DoSysNo,u8 nComNo)
{
	u16 l_StratInfo, l_InfoNoNUmbers;
	u16 l_nCommandCount, l_nSetType;
	u16* l_pData;
	
	if(DoSysNo >= (YK_NUM_MAX + MAX_DO_PER_BITS) )
	{// ��ַ���ڷ�Χ
		return 0xFFFF;
	}
	
	if( DoSysNo < YK_NUM_MAX )
	{//ң�ذ�
		return 100;
	}
	else
	{//���ⲿ�豸�ķ�Χ��MAX_COMNUM
		u16 uuu, k;
		for(uuu = 0; uuu < MAX_PORT_DEVICE; uuu++)//���ǵ��豸ֻ��2��3�������ڿ��Թ��ⲿ���豸
		{
			if( (g_Comm[uuu].ProtocolType != PROTOCOL_MASTER_MODBUS) && (g_Comm[uuu].ProtocolType != PROTOCOL_MASTER_101) )
			{
				continue;
			}
			if(PerDeviceData_Len[uuu] == 0)
			{
				continue;
			}
			l_nCommandCount = (PerDeviceData_Len[uuu] - 1) / PERDEVICE_COMMANDLEN;//ÿ��������12���ֽ�
			l_pData = &PerDeviceData[uuu][1];
			for(k = 0; k < l_nCommandCount; k++)
			{
				l_nSetType = l_pData[k * PERDEVICE_COMMANDLEN + 7];
				switch(l_nSetType)//��ʾ���õ�����
				{
				case COMM_AI_TYPE:
					{
					}
					break;
				case COMM_DI_TYPE:
					{
					}
					break;
				case COMM_DO_TYPE:
					{
						l_StratInfo = (l_pData[k * PERDEVICE_COMMANDLEN + 4] & 0xFF)  + ((l_pData[k * PERDEVICE_COMMANDLEN + 5] & 0xFF)*0x100);
						l_InfoNoNUmbers = (l_pData[k * PERDEVICE_COMMANDLEN + 6] & 0xFF);
						if((DoSysNo >= l_StratInfo) && (DoSysNo < (l_StratInfo + l_InfoNoNUmbers)))
						{
							if(nComNo == uuu)
								return 0xFFFF;
							else
								return (uuu);
						}			
					}
					break;
				default:
					break;
				}
			}
		}
	}

	return 0xFFFF;//û�����101Info��
}

//����1��nCommNo--��ǰ����˿�
//����2��l_nRet--ң�ص�ŷ��صĶ˿�
//����3����ǰң�ز�����
//����ֵ��0--�ޣ�1--��
u16 FindPerdeviceYk(u8 nCommNo, u16 l_nRet)
{//�ⲿң��Ҫת������Э�鴦��
	
	int i,j;
	u16 SysInfoNo;
	
	g_Comm[nCommNo].m_DoUpInfo.m_nCurStep = 0;
	g_Comm[nCommNo].m_DoUpInfo.m_nSubReturn = 0;
	SysInfoNo = g_Comm[nCommNo].m_DoUpInfo.m_nSystemDoInfo;
	//�ⲿ�豸Ҫת�����Э�鴦��,ִ��ң��ʱ�Ŵ���������ⲿ�豸
	for(j = 0; j < g_101DeviceList[nCommNo].nDeviceCount; j++)
	{
		for(i=0; i<g_I101M_Dev[nCommNo][j].ItemNumDo; i++)
		{//��ң�ص��
			if((SysInfoNo >= g_I101M_Dev[nCommNo][j].plDo[i].SysInfoNo)
				&& (SysInfoNo < (g_I101M_Dev[nCommNo][j].plDo[i].SysInfoNo + g_I101M_Dev[nCommNo][j].plDo[i].Numbers)))
			{//�ҵ���
				if(g_101DeviceInfo[nCommNo][j].nDeviceAddr == g_I101M_Dev[nCommNo][j].addr)
				{//�豸��ַһ��ʱ
					g_Comm[l_nRet].m_DOinfo.m_nCurStep = 0;
					g_Comm[l_nRet].m_DOinfo.m_RetCom = nCommNo;	//���ض˿ں�
					g_Comm[l_nRet].m_DOinfo.m_ExeTick = g_TickCount;	//��ʱ
					
					//�ҵ�ң�ص�ַ
					g_Comm[l_nRet].m_DOinfo.m_nControlDoInfo = (SysInfoNo  - g_I101M_Dev[nCommNo][j].plDo[i].SysInfoNo)	+ g_I101M_Dev[nCommNo][j].plDo[i].StartInfoNo;
					
					g_Comm[l_nRet].m_DOinfo.m_nSystemDoInfo = SysInfoNo ;	//ϵͳ���
					g_Comm[l_nRet].m_DOinfo.m_bDoClose = g_Comm[nCommNo].m_DoUpInfo.m_bDoClose;		//�ֺ�
					g_Comm[l_nRet].m_DOinfo.m_bSelect = g_Comm[nCommNo].m_DoUpInfo.m_bSelect;		//ѡ��/ִ��
					g_Comm[l_nRet].m_DOinfo.m_DoRetStatus = g_Comm[nCommNo].m_DoUpInfo.m_DoRetStatus;	//ѡ��/ִ��/ȡ��
					
					g_Comm[l_nRet].m_DOinfo.m_bDoubleType = g_Comm[nCommNo].m_DoUpInfo.m_bDoubleType;		//˫������
					g_Comm[l_nRet].m_DOinfo.m_nCause = g_Comm[nCommNo].m_DoUpInfo.m_nCause;		//����ԭ��
					g_Comm[l_nRet].m_DOinfo.m_nQU = g_Comm[nCommNo].m_DoUpInfo.m_nQU;		//����
					
					return 1;
				}
			}
		}
	}
	
	return 0;
}

void Slaver_YK_Init(u8 CommNo)
{
	g_Comm[CommNo].m_DoUpInfo.m_nCurStep = 0;

	g_Comm[CommNo].m_DoUpInfo.m_nCause.Cause = 0;
	g_Comm[CommNo].m_DoUpInfo.m_nCause.P_N = 0;
	g_Comm[CommNo].m_DoUpInfo.m_nCause.T = 0;
	g_Comm[CommNo].m_DoUpInfo.m_nCause.SRC = 0;

	g_Comm[CommNo].m_DoUpInfo.m_DoSelectTick = g_TickCount;
	g_Comm[CommNo].m_DoUpInfo.m_nControlDoInfo = INVALID_DOINFOADDR;
	g_Comm[CommNo].m_DoUpInfo.m_SelectDO = INVALID_DOINFOADDR;
	g_Comm[CommNo].m_DoUpInfo.m_nSystemDoInfo = INVALID_DOINFOADDR;
	g_Comm[CommNo].m_DoUpInfo.m_DoRetStatus = YK_RETURN_STATUS_NULL;
	g_Comm[CommNo].m_DoUpInfo.m_nSubReturn = YK_RETURN_STATUS_NULL;
}

void Master_YK_Init(u8 CommNo)
{
	g_Comm[CommNo].m_DOinfo.m_nCurStep = 0;
	g_Comm[CommNo].m_DOinfo.m_RetCom = 0xFF;

	g_Comm[CommNo].m_DOinfo.m_nCause.Cause = 0;
	g_Comm[CommNo].m_DOinfo.m_nCause.P_N = 0;
	g_Comm[CommNo].m_DOinfo.m_nCause.T = 0;
	g_Comm[CommNo].m_DOinfo.m_nCause.SRC = 0;

	g_Comm[CommNo].m_DOinfo.m_nControlDoInfo = INVALID_DOINFOADDR;
	g_Comm[CommNo].m_DOinfo.m_nSystemDoInfo = INVALID_DOINFOADDR;
	g_Comm[CommNo].m_DOinfo.m_DoRetStatus = DO_STATUS_NULL;
	g_Comm[CommNo].m_DOinfo.m_nSubReturn = YK_RETURN_STATUS_NULL;
}

//5msִ��һ��
void Comm_module(void)
{
	COM_STRUCT *pComm;
	static Uint8 kk=1;

#if 0
	{//for test
		u8* l_pBuf=0;
		u16 l_Len;
		
		GWPDAQ_ProFrame(0,&l_Len);
		return;
	}
#endif

	if(kk)
		kk = 0;
	else
		kk = 1;

	//����ң��ѡ��ʱ

	if(g_yk.Select)
	{
		if (g_yk.CountTime++ > 6000)
		{//5*6��000=30��000=30s
			g_yk.Select = 0;
			g_yk.CountTime = 0;
			g_yk.yk_from = 0xff; //��ʾû��ң��
		}
	}



	switch(kk)
	{
	case 0:
		{

//			SLAVER_104_Run(0);
//			SLAVER101_ProFrame(0);
			POISED_SLAVER101_ProFrame(0);
			return;
			pComm = &g_Comm[0];
			u8 *pBuf = &(pComm->m_RXBuffer->l_Buffer[pComm->m_RXBuffer->l_OutPos]);
			if( IsTimeOut(pComm->m_nLastSendTickCount, (u16)180*1000) )
			{	//180s�޷��������Զ���ʼ��һ��
				uart1_init(9600);
				pComm->m_nLastSendTickCount = g_TickCount;
				pComm->m_RXBuffer->l_OutPos = pComm->m_RXBuffer->l_InPos;
			}
			
			//ά�������жϣ�
#ifdef CONFIGURTION_XX
			if(CheckWh(1))
			{
				return;
			}
#endif			
			{//����COM1,����Э�鴦��,������
//			switch(pComm->ProtocolType)
//			{
//			case PROTOCOL_MASTER_MODBUS:
//					ProModbusFrame_Master(0);
//				break;
//			case PROTOCOL_MASTER_101:
					ProIEC101Frame_Master(0);
//					break;
//				default:
//					pComm->m_RXBuffer->l_OutPos = pComm->m_RXBuffer->l_InPos;
//					break;
//			}
			}//com1

			{//com2 101��
				pComm = &g_Comm[1];
				if( IsTimeOut(pComm->m_nLastSendTickCount, (u16)180*1000) )
				{	//180s�޷��������Զ���ʼ��һ��
					uart1_init(9600);
					pComm->m_nLastSendTickCount = g_TickCount;
					pComm->m_RXBuffer->l_OutPos = pComm->m_RXBuffer->l_InPos;
				}
				if( CheckGSMHead(&(pComm->m_RXBuffer->l_Buffer[pComm->m_RXBuffer->l_OutPos])) )
				{//���ݴ�GSM������
					GSM_REC_DATA_CON_SUB(pComm);
					pComm->m_RXBuffer->l_OutPos = pComm->m_RXBuffer->l_InPos;
				}
				else if( CheckPDAHead(pBuf) )
				{//PDA
					PL2_WORK_SUB(pComm);
					pComm->m_RXBuffer->l_OutPos = pComm->m_RXBuffer->l_InPos;
				}
				else if( Check232Head(pBuf) )
				{//232
					PL2_WORK_SUB(pComm);
					pComm->m_RXBuffer->l_OutPos = pComm->m_RXBuffer->l_InPos;
				}
				else if( CheckLCDHead(pBuf) )
				{//LCD
					PL2_WORK_SUB(pComm);
					pComm->m_RXBuffer->l_OutPos = pComm->m_RXBuffer->l_InPos;
				}
				else if( ((pBuf[0]==PDAHead0)&& (pBuf[1]==(PDAHead1&0x7f)) &&(pBuf[2]==PDAHead2)&&(pBuf[3]==PDAHead3))
					||((pBuf[0]==COM232Head0)&&(pBuf[1]==(COM232Head1&0x7f))&&(pBuf[2]==COM232Head2)&&(pBuf[3]==COM232Head3))
					||((pBuf[0]==LCDHead0)&&(pBuf[1]==(LCDHead1&0x7f))&&(pBuf[2]==LCDHead2)&&(pBuf[3]==LCDHead3))
					)
				{//��GSM�ȹ�����Ӧ��,Ҫת����ȥ
					u16 temp;
					switch( pBuf[7] )
					{
					case FunXGHM://�޸ĸ߼�����  
					case FunDQHM://��ȡ�߼�����   
					case FunDQJRD://��ȡ�����
					case FunXGJR://�޸Ľ����
					case FunCXGSMST://��ѯGSM״̬ 
					case FunGSMRST://��λGSMģ��
					case FunGSMDEV://��ѯGSMģ���豸���ͺ����ݵ�
					case FunGSMVER://��ȡGSMģ��汾��
						temp = pBuf[6];
						temp <<= 8;
						temp += pBuf[5];
						if(temp != LianluAddrBuf)
							return;
						GPL2_OWER_return_sub(pComm);
						pComm->m_RXBuffer->l_OutPos = pComm->m_RXBuffer->l_InPos;
					default :
						break;
					}
				}
				else
				{//101Э��
					switch(pComm->ProtocolType)
					{
					case PROTOCOL_SLAVER_101:
						SLAVER101_ProFrame(1);
						break;
// 					case PROTOCOL_SLAVER_MODBUS:
// 						Modbus_S_Run(1);
// 						break;
					case PROTOCOL_SLAVER_101_POISED:
						POISED_SLAVER101_ProFrame(1);
						break;
// 					case PROTOCOL_SLAVER_104:
// 						SLAVER_104_Run(CommNo);
// 						break;
					default:
						pComm->m_RXBuffer->l_OutPos = pComm->m_RXBuffer->l_InPos;
						break;
					}
				}
			}
		}
		break;
	case 1:
		//������̫��1&2
		{//��̫��
			CheckIPFrame(3);

			SLAVER_104_Run(2);
			SLAVER_104_Run(3);

			//ά�������жϣ�
// 			if(CheckWh(3))
// 			{
// 				break;;
// 			}
		}
		break;
	}//switch (kk)
}

