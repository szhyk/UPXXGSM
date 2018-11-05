/***********************************************

************************************************/

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "gsm.h"
#include "gprs.h"
#include "comm.h"
#include "uart.h"
#include "ucs2.h"
#include "gsmcomm.h"
#include "wxcw.h"
#include "timer.h"
#include "work.h"

extern uint8 g_bCandeleteMsg;
extern uint16 g_deleteMsg_Rst_Count;
static uint16 GsmRingtimebuf;
static uint8  GsmEORRnumbuf;
uint8  GsmEORRTOnumbuf;
uint8  GsmEORRcfnumbuf;
uint16 GsmEORRtimebuf;
static uint8  GsmRingnumbuf;
uint8  Soesearchnumbuf;

char sCurSMS[700];
uint8 g_error_count;
GSM_SEND_DATA_STRUCT gsmSendData;
static uint8 ucs2_flag;
static uint8  gsmMsgNumbuf[11];	//�յ����ŵķ����ź���
static uint8  gsmMsgtimebuf[7]; //�Ӷ�������ȡ��ʱ�䣬��-��-��-ʱ-��-��

//GSM���ţ���ASCII���160���ַ����������70����


const char *strMsgInfo[msgErrNum]={"����������!","��ѯ��Ŵ���,��С��20!","������ʱ!","��������!","�������!","����Ȩ�޲���!","��λ�ɹ�!"};

//GSM��ATָ��ʱ�ԣ�\r��β�������û�����ʱ����\r\n��β��
void reset(void);
void CMGR_PDU_Decode(uint8 *l_pRxData, COM_STRUCT * pCommGSM, COM_STRUCT * pCommKZQ);


uint8 CheckCmdStr(uint8 * pbuf, char *str)
{
	uint8 len,i;
	len = strlen(str);
	for(i=0; i<len; i++)
	{
		if(pbuf[i] != str[i])
			return 0;
	}
	return 1;
}


void GSM_Send_HELLO_sub(COM_STRUCT *pComm,uint8 type,uint8 groupno)
{
	if(type == 1)
		AddMsgGroup("���!",groupno);
	else
		AddMsgGroup("Hello!",groupno);
}

//GSMģ�鸴λӦ��
void GSM_Send_RST_sub(COM_STRUCT *pComm, uint8 groupno)
{   
	AddMsgGroup("��λ GSM OK \r\n",groupno);
}

//GSMģ��汾��Ӧ��
void GSM_Send_VER_sub(COM_STRUCT *pComm, uint8 groupno)
{   
	uint8 i,len;

	i = SendStrToBuf(&sCurSMS[0],"�豸=");
	len = i;
	i = SendStrToBuf(&sCurSMS[len],DEVICE_NAME);
	len += i;
	i = SendStrToBuf(&sCurSMS[len],",�汾=");
	len += i;
	i = SendStrToBuf(&sCurSMS[len],FSOFT_VERSION);
	len += i;
	
	AddMsgGroup(sCurSMS,groupno);
}

/**************************
ERROR  OK
**************************/
void GSM_Send_ERROR_sub(COM_STRUCT *pComm, uint8 groupno)
{
	if(Gsmmodeselect == MC52GSMPDU)
		AddMsgGroup("��Ϣ����!",groupno);
	else
		AddMsgGroup("GS ERROR!",groupno);
}

/***************************
XGHM=1XXXX,1XXXX,1XXXX,1XXXX
****************************/
uint8 gsm_xghm_sub(uint8 *pBuf,uint8 len)
{
	uint8 t=0,i=0,datanum=0;
	
	for(t=0; t<len; t++)
	{
		WDR();//�忴�Ź�
		if((pBuf[t]=='=')||(pBuf[t]==','))
		{
			datanum++;
			for (i=0;i<11;i++)
			{
				if((pBuf[t+i+1]<0x30)||(pBuf[t+i+1]>0x39))
				{
					return 0;
				}
				gsmtelnumbuf[YongfuNumStart-1+datanum][i] = pBuf[t+i+1];
			}
		}
	}
	if((datanum<1)||(datanum>(teletaolnumbuf-YongfuNumStart)))
	{
		return 0;
	}
	telenumnewbuf=datanum;
	telenumoldbuf=telenumnewbuf; //�޸ĳɹ�
	teletolnumbuf=telenumoldbuf+YongfuNumStart;
	eepbufflag.bit.telenumf = 1;//��������
	SETBIT(funcflag7,EEpwritedelay);
	for(i=0; i<telenumnewbuf; i++)
	{
		Gsmstature4flag.Word |= (1<<i);
	}

	return 1;
}




//��������
uint8 gsm_xgcjhm_sub(uint8 *pBuf,uint8 len)
{
	uint8 t=0,i=0,datanum=0;
	uint8 kk=0;
	
	for(t=0; t<len; t++)
	{
		WDR();//�忴�Ź�
		if((pBuf[t]=='=')||(pBuf[t]==','))
		{
			datanum++;
			if((pBuf[t+i+1]<0x31)||(pBuf[t+i+1]>0x33))
			{//1,2,3
				return 0;
			}
			kk |= (1<<pBuf[t+i+1]);
		}
	}
	if((datanum<1)||(datanum>3))
	{
		return 0;
	}

	gsmtelsuper = kk;	//��������
	eepbufflag.bit.telesuper = 1;
	
	return 1;
}

uint8 gsm_get_var_PDU_sub(uint8 *pBuf,uint8 len, uint8 num,uint16* pdes)
{
	uint8 t=0,datastart=0,datanum=0,onedatanum=0;//,bufdatanum;
	uint16 DATA;
	
	for(t=0; t<len; t++)
	{
		if((pBuf[t]=='=')||(pBuf[t]==','))  //�ҳ����ݵ���ʼλ��
		{
			if(datastart==0)
			{
				datastart = 1;
			}
			else
			{
				if(onedatanum==1)
				{
					DATA = pBuf[t-1]-0x30;
				}
				else if(onedatanum==2)
				{
					DATA = (pBuf[t-2]-0x30)*10+(pBuf[t-1]-0x30);
				}
				else if(onedatanum==3)
				{
					DATA=(pBuf[t-3]-0x30)*100+(pBuf[t-2]-0x30)*10+(pBuf[t-1]-0x30);
				}
				else if(onedatanum==4)
				{
					DATA=(pBuf[t-4]-0x30)*1000+(pBuf[t-3]-0x30)*100+(pBuf[t-2]-0x30)*10+(pBuf[t-1]-0x30);
				}
				else if(onedatanum==5)
				{
					DATA = (pBuf[t-5]-0x30)*10000+(pBuf[t-4]-0x30)*1000+(pBuf[t-3]-0x30)*100+(pBuf[t-2]-0x30)*10+(pBuf[t-1]-0x30);
				}
				else
				{//���ڷ�Χ����ʾ����
					return 0;
				} 
				onedatanum=0;
				pdes[datanum] = DATA; //������ȡ�󱣴�����������
				datanum +=1;
			}
		}
		else
		{
			if(datastart==1)
			{
				if((pBuf[t]<0x30)||(pBuf[t]>0x39))
				{
					return 0;
				}
				onedatanum +=1;
				if( (t==len-1) && (onedatanum>0) )
				{//���һλ,��������
					if(onedatanum==1)
					{
						DATA = pBuf[t-1]-0x30;
					}
					else if(onedatanum==2)
					{
						DATA = (pBuf[t-2]-0x30)*10+(pBuf[t-1]-0x30);
					}
					else if(onedatanum==3)
					{
						DATA=(pBuf[t-3]-0x30)*100+(pBuf[t-2]-0x30)*10+(pBuf[t-1]-0x30);
					}
					else if(onedatanum==4)
					{
						DATA=(pBuf[t-4]-0x30)*1000+(pBuf[t-3]-0x30)*100+(pBuf[t-2]-0x30)*10+(pBuf[t-1]-0x30);
					}
					else if(onedatanum==5)
					{
						DATA = (pBuf[t-5]-0x30)*10000+(pBuf[t-4]-0x30)*1000+(pBuf[t-3]-0x30)*100+(pBuf[t-2]-0x30)*10+(pBuf[t-1]-0x30);
					}
					else
					{//���ڷ�Χ����ʾ����
						return 0;
					} 
					onedatanum=0;
					pdes[datanum] = DATA; //������ȡ�󱣴�����������
					datanum +=1;
				}
			}
		}
		if(datanum > num)
		{//��������
			return 0;
		}
	}
	
	if(datanum != num)
	{//�������˻�������
		return 0;
	}
	
	return 1;
}

uint8 gsm_szwd_sub(uint8 *pBuf,uint8 len)
{
	uint8 t=0,i=0,datanum=0;
	uint8 kk=0;
	uint16 temp[2];

	if(gsm_get_var_PDU_sub(pBuf,len,2,temp)==0)
		return 0;	//��������

	if(temp[1] <= temp[0])
	{//���ر����¶������һ�㱨���¶�
		return 0;
	}
	if(temp[1] - temp[0] < 5)
	{//�¶Ȳ���>=5��
		return 0;
	}

	for(t=0; t<len; t++)
	{
		WDR();//�忴�Ź�
		if((pBuf[t]=='=')||(pBuf[t]==','))
		{
			datanum++;
			if((pBuf[t+i+1]<0x30)||(pBuf[t+i+1]>0x39))
			{//1,2,3
				return 0;
			}
			kk |= (1<<pBuf[t+i+1]);
		}
	}

	//2,3
	if((datanum<2)||(datanum>3))
	{
		return 0;
	}
	
	gsmtelsuper = kk;	//��������
	eepbufflag.bit.telesuper = 1;

	return 1;
}

//������Ч��
uint8 gsm_szyx_sub(uint8 *pBuf,uint8 len)
{
	uint8 t=0,i=0,datanum=0;
	uint8 kk=0;
	uint16 temp[6];
	
	if(gsm_get_var_PDU_sub(pBuf,len,6,temp)==0)
		return 0;	//��������
	
	for(t=0; t<len; t++)
	{
		WDR();//�忴�Ź�
		if((pBuf[t]=='=')||(pBuf[t]==','))
		{
			datanum++;
			if((pBuf[t+i+1]<0x30)||(pBuf[t+i+1]>0x39))
			{//1,2,3
				return 0;
			}
			kk |= (1<<pBuf[t+i+1]);
		}
	}
	
	if((datanum<6)||(datanum>7))
	{
		return 0;
	}

	GSM_XGYX(temp);
	
	return 1;
}

uint8 gsm_Msg_Number_test_sub(uint8 group,uint8 *pGSMtelno)
{
	uint8 y=0;
	for(y=0; y<11; y++) 
	{
		if(pGSMtelno[y] != gsmtelnumbuf[group][y])
		{
			return 0;
		}
	}
	return 1;
}

uint8 gsm_Number_Group_test_sub(uint8 *pGSMtelno)
{
	uint8 x=0;
	
	WDR();//�忴�Ź�
	for(x=1; x<teletaolnumbuf; x++)  //��0���ã�1-5Ϊ�����û�6Ϊ�߼��û���7-9����Ϊ�߼��û�������Ϊ��ͨ�û�
	{
		if( gsm_Msg_Number_test_sub(x,pGSMtelno) )
		{//�������OK
			return x;
		}
	}
	return 0;
}
/***************************
//�ҳ����ݵĿ�ʼλ�ã��������ݳ��Ⱥͳ��ֵ�λ��
//�������س��ͻ���֮�������������Ҫ���������
****************************/
uint8 gsm_get_datastart_test_sub(uint8 *pbuf,uint8 *len)
{
	uint8 x,pos;
	
	pos = 0;
	for(x=0; x<(*len); x++)
	{
		if((pbuf[x]==_CR)&&(pbuf[x+1]==_LF))
		{
			if(pos==0)
			{//��һ���س�����
				pos = x+2;
			} 
			else
			{//�ڶ����س�����
				*len = pos;	// ����һ�γ��ֵ�λ�ô���ȥ
				return (x-pos);
			}
		}
	}
	return 0;
}

/***************************
+CMGR: "REC UNREAD","+8613926988790",,"10/03/17,14:32:16+32"
//�ҳ�ʱ��Ŀ�ʼλ��
****************************/
uint8 gsm_get_Msgtime_sub(uint8 *pRecvbuf,uint8 len,uint8 *timebuf)
{
	uint8 x=0;
	
	for(x=0; x<len; x++)
	{
		if((pRecvbuf[x]==',')&&(pRecvbuf[x+1]==','))
		{
			timebuf[0] = (pRecvbuf[x+3]-0x30)*10+(pRecvbuf[x+4]-0x30); //��
			timebuf[1] = (pRecvbuf[x+6]-0x30)*10+(pRecvbuf[x+7]-0x30); //��
			timebuf[2] = (pRecvbuf[x+9]-0x30)*10+(pRecvbuf[x+10]-0x30); //��
			timebuf[3] = (pRecvbuf[x+12]-0x30)*10+(pRecvbuf[x+13]-0x30); //ʱ
			timebuf[4] = (pRecvbuf[x+15]-0x30)*10+(pRecvbuf[x+16]-0x30); //��
			timebuf[5] = (pRecvbuf[x+18]-0x30)*10+(pRecvbuf[x+19]-0x30); //��
			return 1;
		}
	}
	return 0;
}
/**********************
telzhu:�·�������������Ǹ���ĺ���
telnumf;Ŀǰ�����˼�����룬�п������㣬�տ�ʼ���õ�ʱ��
***********************/
void GSM_Send_tel_queren_sub(COM_STRUCT *pComm,uint8 groupno,uint8 telnumf)
{
	uint8 z, j, i;
	uint8* l_pRXData;
	z = 0;
	l_pRXData = (uint8 *)&sCurSMS[0];
	
	l_pRXData[z++]='D';
	l_pRXData[z++]='H';
	l_pRXData[z++]='=';
	l_pRXData[z++]=telnumf+0x30;
	l_pRXData[z++]=' ';
	for (j=0;j<telnumf;j++)
	{ 
		for (i=0;i<11;i++)
		{
			l_pRXData[z++]=gsmtelnumbuf[YongfuNumStart+j][i];
		}
		if(j==0)
		{
			l_pRXData[z++]='(';
			l_pRXData[z++]='S';
			l_pRXData[z++]=')';
		}
		else if(j<4)
		{
			if(gsmtelsuper&(1<<j))
			{
				l_pRXData[z++]='(';
				l_pRXData[z++]='S';
				l_pRXData[z++]=')';
			}
		}
		l_pRXData[z++]=',';
	}
	l_pRXData[z] = '\0';//ȥ�����һ������
	AddMsgGroup(sCurSMS,groupno);
}

//���Ŵ���
void SMS_Process(COM_STRUCT *pCommKZQ,COM_STRUCT *pCommGSM,uint8 *pBuf,uint8 len)
{
	uint8 t=0,w=0;
	
	for(t=0; t<len; t++)
	{
		// ��ȡGSMģ��汾��
		if(CheckCmdStr(&pBuf[t],"�汾"))
		{
			GSM_Send_VER_sub(pCommGSM,gsmMsgNumbufGroup);
			return; 
		}
		// GSMģ�鸴λ
		if(CheckCmdStr(&pBuf[t],"��λ"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{//��Ȩ�޵Ĳſ���
				GSM_Send_RST_sub(pCommGSM,gsmMsgNumbufGroup);
				reset();
			}
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"HELLO"))
		{
			GSM_Send_HELLO_sub(pCommGSM,0,gsmMsgNumbufGroup);			//���ͷ�բNG����
			return;
		}
		if(CheckCmdStr(&pBuf[t],"���"))
		{
			GSM_Send_HELLO_sub(pCommGSM,1,gsmMsgNumbufGroup);			//���ͷ�բNG����
			return;
		}
		if(CheckCmdStr(&pBuf[t],"STOPRUN>1"))
		{
			if( gsmMsgNumbufGroup==1 )
			{
				g_stoprun = 0x5A;
				EEPROM_Write(g_stoprun,STOP_RUN_ADDR);
				AddMsgGroup("STOPRUNed!",gsmMsgNumbufGroup);
			}
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"STOPRUN>0"))
		{
			if( gsmMsgNumbufGroup==1 )
			{
				g_stoprun = 0;
				EEPROM_Write(g_stoprun,STOP_RUN_ADDR);
				AddMsgGroup("no STOPRUN!",gsmMsgNumbufGroup);
			}
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"STOPRUN?"))
		{
			if( gsmMsgNumbufGroup==1 )
			{
				if(g_stoprun==0x5A)
					AddMsgGroup("STOPRUNed!",gsmMsgNumbufGroup);
				else
					AddMsgGroup("no STOPRUN!",gsmMsgNumbufGroup);
			}
			return; 
		}
		/************��ѯ�绰����***********/
		if(CheckCmdStr(&pBuf[t],"��ѯ����"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{
				GSM_Send_tel_queren_sub(pCommGSM,gsmMsgNumbufGroup,telenumoldbuf);
			}
			return; 
		}
		/************�޸ĵ绰����***********/
		if(CheckCmdStr(&pBuf[t],"�޸ĺ���="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{
				if(gsm_xghm_sub(pBuf,len)==0)
				{
					AddMsgGroup("�޸ĺ���ʧ��!",gsmMsgNumbufGroup);
				}
				else
				{
					AddMsgGroup("�޸ĺ���ɹ�!",gsmMsgNumbufGroup);
				}
			}
			else
			{//Ȩ�޲���
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			}
			
			return; 
		}
		/************�޸ĺ���Ȩ��***********/
		if(CheckCmdStr(&pBuf[t],"�޸ĺ���Ȩ��="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{//�����������Ȩ��
				if(gsm_xgcjhm_sub(pBuf,len)==0)
				{
					AddMsgGroup("�޸ĺ���Ȩ��ʧ��!",gsmMsgNumbufGroup);
				}
				else
				{
					AddMsgGroup("�޸ĺ���Ȩ�޳ɹ�!",gsmMsgNumbufGroup);
				}
			}
			else
			{//Ȩ�޲���
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			}
			
			return; 
		}
		/************ɾ����ʷ��¼***����ɾ��OK********/
		if(CheckCmdStr(&pBuf[t],"ɾ�����м�¼"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{
				GSM_SCJL(gsmMsgNumbufGroup);
			}
			else
			{//Ȩ�޲���
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			}
			return; 
		}
		
		if(CheckCmdStr(&pBuf[t],"��ѯ״̬"))
		{
			GSM_CXZT(gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"��ѯ����"))
		{
			GSM_CXMC(gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"��ͣ����"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_ZTBJ(gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"��Ŀ����="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGXMMC(pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"�˿�1����="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(0,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"�˿�2����="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(1,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"�˿�3����="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(2,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"�˿�4����="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(3,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"�˿�5����="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(4,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"�˿�6����="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(5,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		/************��ѯ����***********/
		if(CheckCmdStr(&pBuf[t],"��ѯ����"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_CXSZ(1,gsmMsgNumbufGroup);
			else
				GSM_CXSZ(0,gsmMsgNumbufGroup);

			return; 
		}
		if(CheckCmdStr(&pBuf[t],"��ѯ��Ч"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_CXYX(1,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"�޸���Ч="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{
				if(gsm_szyx_sub(pBuf,len))
				{
					AddMsgGroup("��Ч���óɹ�!",gsmMsgNumbufGroup);
				}
				else
				{
					AddMsgGroup("��Ч����ʧ��!",gsmMsgNumbufGroup);
				}
			}
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}

		/************�޸Ķ�ֵ***********/
		if(CheckCmdStr(&pBuf[t],"�����¶�="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{
				if( gsm_szwd_sub(pBuf,len)==0 )
				{
					AddMsgGroup("�¶�����ʧ��!",gsmMsgNumbufGroup);
				}
				else
				{
					AddMsgGroup("�¶����óɹ�!",gsmMsgNumbufGroup);
				}
			}
			else
			{//Ȩ�޲���
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			}
			return; 
		}
	
		/************��ѯ�¼���¼***********
		���200��
		*************************************/
		if(CheckCmdStr(&pBuf[t],"��ѯ��¼="))
		{//��ѯָ����¼
			if( len >= (t+12) )//��֤ΪCXJL=1---CXJL=200
			{//���ڷ�Χ����ʾ����,<100
				AddMsgGroup(strMsgInfo[msgBHERR],gsmMsgNumbufGroup);
				return; 
			}
			w = len-(t+9);//�жϲ�ѯ���
			if(w==0)
			{//����û�и�����,��Ĭ��Ϊ��ѯ������Ϣ
				Soesearchnumbuf = 0;
			}
			else if(w==1)
			{
				if((pBuf[t+5]<0x30)||(pBuf[t+5]>0x39))
				{
					AddMsgGroup(strMsgInfo[msgBHERR],gsmMsgNumbufGroup);
					return;
				}
				Soesearchnumbuf = pBuf[t+5]-0x30;
			}
			else if(w==2)
			{
				if((pBuf[t+5]<0x30)||(pBuf[t+5]>0x39)||(pBuf[t+6]<0x30)||(pBuf[t+6]>0x39))
				{
					AddMsgGroup(strMsgInfo[msgBHERR],gsmMsgNumbufGroup);
					return;
				}
				Soesearchnumbuf = (pBuf[t+5]-0x30)*10+(pBuf[t+6]-0x30);
			}
			else if(w==3)
			{
				if((pBuf[t+5]<0x30)||(pBuf[t+5]>0x39)||(pBuf[t+6]<0x30)||(pBuf[t+6]>0x39)||(pBuf[t+7]<0x30)||(pBuf[t+7]>0x39))
				{
					AddMsgGroup(strMsgInfo[msgBHERR],gsmMsgNumbufGroup);
					return;
				}
				Soesearchnumbuf = (pBuf[t+5]-0x30)*100 + (pBuf[t+6]-0x30)*10+(pBuf[t+7]-0x30);
				if(Soesearchnumbuf >= SOE_NUM_MAX)
				{
					AddMsgGroup("���֧��200����¼�����Ϊ0-199!",gsmMsgNumbufGroup);
					return;
				}
			}
			else
			{
				AddMsgGroup("���֧��200����¼!",gsmMsgNumbufGroup);
				return;
			}
			GSM_CXJL(Soesearchnumbuf,gsmMsgNumbufGroup);
			return; 
		}
		
		if(CheckCmdStr(&pBuf[t],"��ѯ��¼"))
		{//��ѯ���µ�һ����¼
			Soesearchnumbuf = 0;
			GSM_CXJL(Soesearchnumbuf,gsmMsgNumbufGroup);
			return; 
		}
	}//for(t=0; t<len; t++)
}

/*****************
//���յ������ݽ��д���ͷ���
**************/
// ָ�� ��YF.FZ,YH.HZ,CXDZ,DZ=,DZQR,CXZT,FW,RST,DS,CXSJ,CXJL=,CXHM,XGHM=,SCSJJL,WAPN=,RAPN=,DZX=,DZCX,VER,
void gsm_data_con_sub(COM_STRUCT *pCommKZQ,COM_STRUCT *pCommGSM,uint8 *pBuf,uint8 len)
{
	//	uint16  l_nLoop;
	uint8 t=0;
	
	WDR();//�忴�Ź�
	if((pBuf[0]==0x30)&&(pBuf[1]==0x30))
	{//��������0,����.
		if(gsmMsgNumbufGroup != 0)
			GSM_Send_ERROR_sub(pCommGSM,gsmMsgNumbufGroup);
		return; 
	}
	
	if(gsmMsgNumbufGroup==0)
	{//�����ֻ��δ���ú���ʱ��������ʱֻ���������ú����ã��������ܲ�֧�֡�
		for(t=0; t<len; t++)
		{
			if(CheckCmdStr(&pBuf[t],"XGHM="))
			{
				if(gsm_xghm_sub(pBuf,len)==0)
				{
					AddMsgGroup("XGHM ERROR!",1);
				}
				else
				{
					AddMsgGroup("XGHM OK!",YongfuNumStart);
				}
			}
		}
		return; 
	}
	else
	{//���Ŵ���
		SMS_Process(pCommKZQ,pCommGSM,pBuf,len);
	}
}

//��ȡ���ŵ����� +CMGR:
//+CMGR: "REC READ","+8613694121067",,"04/05/31,11:21:18+00"_CR_LFxxxxxx_CR_LF
//+CMGR: "REC UNREAD","+8613926988790",,"10/03/17,14:32:16+32"_CR_LFxxxxxx_CR_LF
//ֻ����"+8613616532300"��ʽ,���볤��Ϊ11�ĺ��룬������ʽ�ĺ��벻����.
void CMGR_TXT_Decode(uint8 *l_pRxData, COM_STRUCT * pCommGSM, COM_STRUCT * pCommKZQ)
{
	uint8 s=0;
	uint8 q=0,i=0,len;
	uint8 *pMsg;
	
	pMsg = l_pRxData;
	for(q=0; q<40; q++)
	{
		//����","��λ��
		if( (pMsg[0]=='"') && (pMsg[1]==',') && (pMsg[2]=='"') )
		{
			pMsg += 3;
			if((pMsg[0]=='+')&&(pMsg[1]=='8')&&(pMsg[2]=='6'))
			{//ȥ��+86
				pMsg += 3;
				for(i=0; i<11; i++)
				{
					gsmMsgNumbuf[i] = *pMsg++;  //��ȡ�������ĵ绰����
					if((gsmMsgNumbuf[i]>0x39)||(gsmMsgNumbuf[i]<0x30)) 
					{
						return;
					} //Ҫ��֤������11λ���ֵ�
				}
				break;
			}
		}
		pMsg++;
	}

	//��ȡ���ŵ�ʱ�䣬Ϊ�������ݴ����ʱ������ʱ����
	len = pMsg - &pCommGSM->RXdata[s];
	len = pCommGSM->RXIndex-len;
	if(gsm_get_Msgtime_sub(pMsg,len,gsmMsgtimebuf))
	{
		g_CLK.YEAR = gsmMsgtimebuf[0];
		g_CLK.MON = gsmMsgtimebuf[1];
		g_CLK.DAY = gsmMsgtimebuf[2];
		g_CLK.HOUR = gsmMsgtimebuf[3];
		g_CLK.MIN = gsmMsgtimebuf[4];
		g_CLK.SEC = gsmMsgtimebuf[5];
		g_CLK.wDAY = GetWeek(g_CLK.YEAR, g_CLK.MON,g_CLK.DAY);
	}

	//������벻�ԣ�
	if(gsmMsgNumbuf[0]==0)
		return;
	/*********************/
	//�ж��յ���Ϣ�ĺ��������ĸ���
	gsmMsgNumbufGroup = gsm_Number_Group_test_sub(gsmMsgNumbuf);
	
	if(gsmMsgNumbufGroup == 0 )
	{//���벻��������ʱ
		if(telenumoldbuf==0)
		{//��δ���ú���ʱ,�����ֻ�������ú�������������ִ�С�
		}
		else
		{//����Ȩ���룬�򲻴���
			return;
		}
	}
	
	/***********���Զ����ݵĶ�ȡ��**********/
	pMsg += 22;
	len = pMsg - l_pRxData;
	len = pCommGSM->RXIndex - len;
	
	//�ҳ����ݵĿ�ʼλ��len,�������ݵ����ݳ���q
	q = gsm_get_datastart_test_sub(pMsg,&len);
	pMsg += len;
	gsm_data_con_sub(pCommKZQ,pCommGSM,pMsg,q);   //���յ������ݽ��д���ͷ���
	SETBIT(funcflag5,mc52havetrancedataled);//��������ָͨʾLED
	netlestimebuf = 0;	// ָʾ��ʱ����0
}

void GSM_Send_start_data_sub(COM_STRUCT *pComm)
{
	uint8 z=0,k=0;
	//	uint8 *pBuffer = &Mc52senddatatonetbuf[0];
	uint8 *pBuffer = &gsmSendData.buffer[0];
	if(Gsmmodeselect == MC52GSMPDU)
	{//PDZģʽ�£����͸�ʽ��һ��
		int nPduLength;
		char *pPdubuf = (char *)&pComm->TXdata[0];
		
		if(ucs2_flag)
		{//ucs2����
			// 			nPduLength = GsmEncodePdu_UCS2((const char *)&sSCA[0], (const char *)&gsmtelnumbuf[telebaohunumbuf][0], (const char *)pBuffer, pPdubuf, mc52isenddatanumbuf);
			nPduLength = GsmEncodePdu_UCS2((const char *)&sSCA[0], (const char *)&gsmtelnumbuf[telebaohunumbuf][0], (const char *)pBuffer, pPdubuf, gsmSendData.len);
		}
		else
		{//7bit����
			// 			nPduLength = GsmEncodePdu_7bit((const char *)&sSCA[0], (const char *)&gsmtelnumbuf[telebaohunumbuf][0], (const char *)pBuffer, pPdubuf, mc52isenddatanumbuf);
			nPduLength = GsmEncodePdu_7bit((const char *)&sSCA[0], (const char *)&gsmtelnumbuf[telebaohunumbuf][0], (const char *)pBuffer, pPdubuf, gsmSendData.len);
		}
		
		pPdubuf[nPduLength] = 0x1A;
		pPdubuf[nPduLength+1] = '\0';
		
		z = nPduLength+1;
	}
	else
	{
		// 		for(k=0; k<mc52isenddatanumbuf; k++)
		for(k=0; k<gsmSendData.len; k++)
		{
			pComm->TXdata[z++] = pBuffer[k];
		} 
		pComm->TXdata[z++] = 0x1A; //ĩβ��0x1A CTRL+Z��������
	}
	pComm->Senddatanum = z;
	Uart_Send_start(pComm);
}

void RX_gsmwork_SUB(COM_STRUCT *pCommKZQ,COM_STRUCT *pCommGSM)
{
	uint16 l_tmpNumber;
	uint8 s=0, l_bFind;
	uint8 kkk;
	uint8* l_pRxData;

	WDR();//�忴�Ź�

//	if(Gsmmodeselect==MC52GSMTEXT)
	{
		for(s = 0; s < pCommGSM->RXIndex; s++)
		{
			l_pRxData = &pCommGSM->RXdata[s];
			//�յ��¶���Ϣ+CMTI: "MT",*...*[CR]
			if(CheckCmdStr(l_pRxData, "+CMTI:"))	//�ж��Ž���
			{
				l_tmpNumber = 0;
				l_bFind = 0;
				g_bCandeleteMsg = 0;//�����ﲻ��ɾ������
				g_deleteMsg_Rst_Count = 0;
				if(s+12 < pCommGSM->RXIndex)
				{
					if((l_pRxData[12] >= '0') && (l_pRxData[12] <= '9'))
					{
						l_tmpNumber = l_pRxData[12] - 0x30;
						l_bFind = 1;
					}
				}


				if(s+13 < pCommGSM->RXIndex && l_bFind)
				{
					if((l_pRxData[13] >= '0') && (l_pRxData[13] <= '9'))
					{
						l_tmpNumber = l_tmpNumber * 10 + l_pRxData[13] - 0x30;
					}
				}

				if(s+14 < pCommGSM->RXIndex && l_bFind)
				{
					if((l_pRxData[14] >= '0') && (l_pRxData[14] <= '9'))
					{
						l_tmpNumber = l_tmpNumber * 10 + l_pRxData[14] - 0x30;
					}
				}

				if(l_bFind)
				{
					gsmduanxinnum = l_tmpNumber;
					SETBIT(Gsmstatureflag,gsmreaddataf);//ֻҪ�ж���.,����Ҫ��ȡ,ɾ��
					gsmdelectduanxinnum = gsmduanxinnum; //�������һֱɾ��0��
				}
				//�п��ܽ����ľ���0,����0x0d,��������Ͳ��ص�

				//�һس�,[CR]
				return;
			}
			/************************************/
			if(l_pRxData[0] == '>') //���Է���Ϣ��
			{//��һ���ַ���'>'
				GSM_Send_start_data_sub(pCommGSM);//���Է������ˡ�
				SETBIT(funcflag5,mc52havetrancedataled);//��������ָͨʾLED
				netlestimebuf = 0;
				GsmOKfunbuf = GsmSendokbuf;
				return;
			}
			/************* +CMGS: 120  OK***���ͳɹ���־*****************/
			if(CheckCmdStr(l_pRxData, "+CMGS:"))
			{
				CLRBIT(mc52iworkflag,mc52gsmsendbaohuf);//����ָ�������
				CLRBIT(mc52iworkflag,gsmchongxingdata); //���·���

				Gsmstature4flag.bit.gsmbaohuworkingf = 0;
				Gsmstature4flag.bit.gsmsenddusef = 0;
				GsmEORRnumbuf = 0;
				GsmEORRTOnumbuf = 0;
				if(GsmOKfunbuf==GsmSendokbuf)
				{
					GsmOKfunbuf = 0;
				}
				return;
			}
			/********************************************/
			// �е绰���� RING
			if(CheckCmdStr(l_pRxData,"RING"))
			{
				GsmRingtimebuf = 0;
				Gsmstature4flag.bit.gsmringworkf = 1;
				GsmRingnumbuf++;
				if(GsmRingnumbuf > 2)
				{
					GsmRingnumbuf=0;
					Gsmstature4flag.bit.gsmringworkf = 0;
				}
				SETBIT(funcflag5,mc52havetrancedataled);//��������ָͨʾLED
				netlestimebuf = 0;
				return;
			}
			/********************************/
			//��ȡ���ŵ����� +CMGR:
			//+CMGR: "REC READ","+8613694121067",,"04/05/31,11:21:18+00"_CR_LFxxxxxx_CR_LF
			//+CMGR: "REC UNREAD","+8613926988790",,"10/03/17,14:32:16+32"_CR_LFxxxxxx_CR_LF
			//ֻ����"+8613616532300"��ʽ,���볤��Ϊ11�ĺ��룬������ʽ�ĺ��벻����.
			//+CMGR: 1,,111 
			//����0891683108501505F0240BA13190433074F30008501157 
			if(CheckCmdStr(l_pRxData,"+CMGR:"))  //��ȡ����
			{
				SETBIT(Gsmstatureflag,gsmdelectduanxf); //ɾ������
				gsmMsgNumbuf[0] = 0;	// ��ֹ��һ����ȷ�������εĺ���Ƿ�ʱ�Ĵ���
				//�����յ��Ķ��Ÿ�ʽ��������Ӧ�Ľ������
//				if(l_pRxData[7]=='"')
				if(Gsmmodeselect == MC52GSMPDU)
				{//PDU����
					CMGR_PDU_Decode(l_pRxData,pCommGSM,pCommKZQ);
				}
				else
				{//TEXT����
					CMGR_TXT_Decode(l_pRxData,pCommGSM,pCommKZQ);
				}
				return;
			}
			/***********************************/
			// CMGD=OK //ɾ�����ųɹ�
			if(CheckCmdStr(l_pRxData,"CMGD"))  //ɾ�����ųɹ�
			{
				for(kkk = s + 4; kkk < pCommGSM->RXIndex; kkk++)
				{
					if(CheckCmdStr(&pCommGSM->RXdata[s],"OK"))  //ɾ�����ųɹ�
					{
						if(gsmdelectduanxinnum > 1)
						{
							gsmdelectduanxinnum--;
							SETBIT(Gsmstatureflag,gsmdelectduanxf);//����ɾ������
						}
						if(GsmOKfunbuf==GsmSendSCDXbuf)
						{
							GsmOKfunbuf = 0;
						}
						return;
					}
				}
			}
		}
		/******************************/
		for(s=0; s<pCommGSM->RXIndex; s++)
		{
			l_pRxData = &pCommGSM->RXdata[s];
			if(CheckCmdStr(l_pRxData, "OK"))
			{
				if(GsmOKfunbuf==GsmSendSCDXbuf)
				{
					GsmOKfunbuf = 0;
					if(gsmdelectduanxinnum > 1)
					{
						gsmdelectduanxinnum--;
						SETBIT(Gsmstatureflag,gsmdelectduanxf);//����ɾ������
					}
				}
				else if(GsmOKfunbuf==GsmSendokbuf)
				{//���ͳɹ�
					GsmOKfunbuf = 0;
					CLRBIT(mc52iworkflag,mc52gsmsendbaohuf);//����ָ�������
					CLRBIT(mc52iworkflag,gsmchongxingdata); //���·���

					Gsmstature4flag.bit.gsmbaohuworkingf = 0;
					GsmEORRnumbuf = 0;
					GsmEORRTOnumbuf = 0;
				}
				NOP();
				return;
			}
			/***************+CMS ERROR: xxx ************************/
			if(CheckCmdStr(l_pRxData, "ERROR"))  //���ش���
			{
				if((l_pRxData[6]==0x20)&&(pCommGSM->RXdata[s+9]==0x0D))
				{
					GsmEorrefunbuf=(pCommGSM->RXdata[s+7]-0x30)*10+(pCommGSM->RXdata[s+8]-0x30);
				}
				else if((pCommGSM->RXdata[s+6]==0x20)&&(pCommGSM->RXdata[s+10]==0x0D))
				{
					GsmEorrefunbuf=(pCommGSM->RXdata[s+7]-0x30)*100+(pCommGSM->RXdata[s+8]-0x30)*10+(pCommGSM->RXdata[s+9]-0x30);
				}
				/******************************************/
				if(GsmEorrefunbuf==28)  //�������ô�����Ч����
				{
					GsmOKfunbuf=0;
					CLRBIT(mc52iworkflag,mc52gsmsendbaohuf);//����ָ�������
					CLRBIT(mc52iworkflag,gsmchongxingdata); //���·���

					Gsmstature4flag.bit.gsmbaohuworkingf = 0;
				}
				else if(GsmEorrefunbuf==38)  //���������
				{
					funcflag5 = 0;
					SETBIT(funcflag3,mc52RESTf);
				}
				else if(GsmEorrefunbuf==42)  //����
				{
					Gsmstature4flag.bit.gsmsenddusef = 1;
				}
				else if(GsmEorrefunbuf==208)  //��SIM���Ŵ洢��
				{
					gsmdelectduanxinnum=100;
					SETBIT(Gsmstatureflag,gsmdelectduanxf);//����ɾ������
				}
				else if(GsmEorrefunbuf==310)  //SIM��δ����
				{
					statusflag0.bit.noSim = 1;
				}
				else if(GsmEorrefunbuf==500)  //����û���� 
				{
					GsmEORRnumbuf++;
					if(GsmEORRnumbuf>2)
					{
						mc52iworkflag = 0;
						funcflag5 = 0;
						Mc52i_CMD_flag1.bit.atsendf = 1;	//��һ��ָ���AT
						eepbufflag.bit.GsmFuweisendf = 1;
						GsmEORRtimebuf=0;
						Mc52i_CMD_flag1.bit.first_connect = 0;
						g_error_count++;
						if(g_error_count >= 20)
							reset();
							
					}
					else
					{
//						GSM_Send_num_sub(pCommGSM,GsmEORRcfnumbuf);
//						SETBIT(mc52iworkflag,mc52gsmsendbaohuf);//����ָ�������
					}
				}
				else if(GsmEorrefunbuf==512)//�ڷ��Ͷ���Ϣ��ͬʱ���յ����µĶ���Ϣ�����Ͷ���Ϣ���ܾ�
				{
					GPRS_A_W_SUB(pCommGSM);
				}
				else if(GsmEorrefunbuf==513)  //ʧȥ��������·���ӣ� 
				{
					;
				}
				else if(GsmEorrefunbuf==514) //�����õĶ���Ϣ���ĵĺ����Ǵ���Ļ����Ϣ��Ҫ�ʹ��Ŀ�ĺ����Ǵ����
				{
					;
				}
				else if(GsmEorrefunbuf==515)//������ȵ���ʼ����ɻ�ָ��ִ����ϡ�
				{
					;
				}
				GsmEorrefunbuf = 0;
				return;
			}
		}
	}
}

/******************************
*******************************/
//���͸�����ģ��ʱ�����ͻ��������
void UART_com_sub(COM_STRUCT *pComm)
{
	pComm->TXBUFNumber++;
	if (pComm->TXBUFNumber>=UART0TXSETCounter)
	{
		pComm->TXBUFC_flag = 1;
		pComm->TXBUFNumber = 0;
	}
}

uint8 Find_Str_Hz(char *pStr)
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

//��UCS2�еĺ���
uint8 Find_Str_UCS2Hz(uint8 *pStr,uint8 len)
{
	uint8 i;
	for(i=0; i<len; i++)
	{
		if(*pStr > 0)
			return 1;
		pStr++;
		pStr++;
	}

	return 0;
}
/************************
����Ŀ���ֻ�����
at+cmgs="13926988790"
//tel_group_no��������0-10
*************************/
void GSM_Send_num_sub(COM_STRUCT *pComm,uint8 tel_group_no)
{
	uint8* l_pTelNumber;
	uint8 i,z=0;

	GsmEORRcfnumbuf = tel_group_no;
	
	if(Gsmmodeselect == MC52GSMPDU)
	{//mc52isenddatanumbuf
		int nPduLength;
		unsigned char nSmscLength;
		uint8 *pBuffer = &(gsmSendData.buffer[0]);
		char *pPdubuf = (char *)&pComm->TXdata[0];
		
		if(CHKBITCLR(mc52iworkflag,gsmchongxingdata))
		{//�����ط���Ϣ�Ŵ���
//			Mc52senddatatonetbuf[mc52isenddatanumbuf] = '\0';
		//if(Find_Str_Hz(pBuffer))
// 			if(Find_Str_UCS2Hz(pBuffer,mc52isenddatanumbuf))
			if(Find_Str_UCS2Hz(pBuffer,gsmSendData.len))
			{//�к���,UCS2����
				ucs2_flag = 1;
//				mc52isenddatanumbuf = Str2UCS2((const char *)&Mc52senddatatonetbuf[0],pPdubuf);
			}
			else
			{//ȫ��Ϊ�ַ�,7λ����
				ucs2_flag = 0;
//				mc52isenddatanumbuf = gsmEncode7bit((const char *)&Mc52senddatatonetbuf[0],pPdubuf,mc52isenddatanumbuf);
			}
			//������ת����Mc52senddatatonetbuf��
//			memcpy((char *)&Mc52senddatatonetbuf[0], (const char *)pPdubuf, mc52isenddatanumbuf);
		}
// 		nPduLength = GsmEncodePdu_UCS2((const char *)&sSCA[0], (const char *)&gsmtelnumbuf[tel_group_no][0], (const char *)pBuffer, pPdubuf, mc52isenddatanumbuf);
		nPduLength = GsmEncodePdu_UCS2((const char *)&sSCA[0], (const char *)&gsmtelnumbuf[tel_group_no][0], (const char *)pBuffer, pPdubuf, gsmSendData.len);

		GsmString2Bytes(pPdubuf, &nSmscLength, 2);    // ȡPDU���е�SMSC��Ϣ����
		nSmscLength++;        							// ���ϳ����ֽڱ���
		z = sprintf(pPdubuf, "AT+CMGS=%d\r", nPduLength / 2 - nSmscLength);
	}
	else
	{
		z = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CMGS=\"");
		l_pTelNumber = &gsmtelnumbuf[tel_group_no][0];
		for(i=0; i<11; i++)
			pComm->TXdata[z++]=l_pTelNumber[i];
		pComm->TXdata[z++]='"';
		pComm->TXdata[z++]=_CR;//�س�
	}
	pComm->Senddatanum=z;
	Uart_Send_start(pComm);
}

/***********************
��ʼ��������
*****************************/
uint8 getAscii(uint8 dat)
{
	if(dat<=9)
	{
		return (0x30+dat);
	}
	else
	{
		return ('A'+(dat-10));
	}
}

/***********************************
************************************/
void GSM_Send_onems_sub(COM_STRUCT *pComm)
{
	if(Gsmstature4flag.bit.gsmringworkf)
	{
		GsmRingtimebuf++;
		if(GsmRingtimebuf>6000)
		{
			GsmRingtimebuf=0;
			GsmRingnumbuf=0;
			Gsmstature4flag.bit.gsmringworkf = 0;
		}
	}
	/****************************************/
	if(eepbufflag.bit.GsmFuweisendf)
	{
		GsmEORRtimebuf++;
		if(GsmEORRtimebuf>20000)
		{
			eepbufflag.bit.GsmFuweisendf = 0;
			GsmEORRtimebuf=0;
			if(CHKBITSET(mc52iworkflag,mc52gsmworkingf))
			{
				GsmEORRTOnumbuf++;
				if(GsmEORRTOnumbuf>30)
				{
					GsmEORRTOnumbuf=0;
					funcflag5=0;
					SETBIT(funcflag3,mc52RESTf);
				}
				else
				{
					GSM_Send_num_sub(pComm,GsmEORRcfnumbuf);
					SETBIT(mc52iworkflag,mc52gsmsendbaohuf);//����ָ�������
				}
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////
//	PDUģʽ�ĸ���GSM����
//
////////////////////////////////////////////////////////////////////////

static char sSmst[141];
//��GSM�����յ�������
void gsm_data_con_PDU_sub(COM_STRUCT *pCommKZQ,COM_STRUCT *pCommGSM,uint8 *pBuf,uint8 len)
{
	uint8 t;
	WDR();//�忴�Ź�
	if((pBuf[0]==0x30)&&(pBuf[1]==0x30))
	{//��������0,����.
		if(gsmMsgNumbufGroup != 0)
			GSM_Send_ERROR_sub(pCommGSM,gsmMsgNumbufGroup);
		return; 
	}
	
	//���Ŵ���
	if(gsmMsgNumbufGroup==0)
	{//�����ֻ��δ���ú���ʱ��������ʱֻ���������ú����ã��������ܲ�֧�֡�
		for(t=0; t<len; t++)
		{
			if(CheckCmdStr(&pBuf[t],"XGHM="))
			{
				pCommGSM->TXTELBufNUM[pCommGSM->TXBUFNumber] = gsmMsgNumbufGroup;//���ظ��绰�ĺ���
				if(gsm_xghm_sub(pBuf,len)==0)
				{
					AddMsgGroup("XGHM ERROR!",1);
				}
				else
				{
					AddMsgGroup("XGHM OK!",YongfuNumStart);
				}
			}
		}
		return; 
	}
	else
	{
		SMS_Process(pCommKZQ,pCommGSM,pBuf,len);
	}
	g_error_count = 0;
}

//��ȡ���ŵ����� +CMGR:
//ֻ����"+8613616532300"��ʽ,���볤��Ϊ11�ĺ��룬������ʽ�ĺ��벻����.
//AT+CMGR=1\r\r\n
//+CMGR: 0,,24\r\n
//08 91 683108705505F0 24 0D 91 683115908076F2 0008 71 11 01 41 10 30 23 04 542F 52A8 \r\n\r\n 'O' 'K' \r\n
//08 91 683108705505F0 04 0D 91 683115908076F2 0000 81 11 20 91 24 95 23 05 C82293F904 0D 0A 0D 0A 4F 4B 0D 0A 00\r\n
void CMGR_PDU_Decode(uint8 *l_pRxData, COM_STRUCT * pCommGSM, COM_STRUCT * pCommKZQ)
{
	uint8 q=0;
	char * pstr;

	pstr = strstr((const char*)l_pRxData, "\r\n");	//����"\r\n"�������س��ֵ��׵�ַ
	
	q = GsmDecodePdu(pstr+2,(char *)&gsmMsgNumbuf[0],(char *)&gsmMsgtimebuf[0],sCurSMS);	//������Ϣ����
	if(*(pstr+2)==0)
	{//7bit
		q = gsmDecode7bit(sCurSMS, sSmst, q);
		memcpy(sCurSMS,sSmst, q);
	}
	else if(*(pstr+2)==8)
	{//ucs2
		q = UCS2ToStr(sCurSMS, sSmst, q);
		memcpy(sCurSMS,sSmst, q);
	}
	else
		return;
	
	//��ʱ
	{
		g_CLK.YEAR = gsmMsgtimebuf[0];
		g_CLK.MON = gsmMsgtimebuf[1];
		g_CLK.DAY = gsmMsgtimebuf[2];
		g_CLK.HOUR = gsmMsgtimebuf[3];
		g_CLK.MIN = gsmMsgtimebuf[4];
		g_CLK.SEC = gsmMsgtimebuf[5];
		g_CLK.wDAY = GetWeek(g_CLK.YEAR, g_CLK.MON,g_CLK.DAY);
	}

	//������벻�ԣ�
	if(gsmMsgNumbuf[0]==0)
		return;
	/*********************/
	//�ж��յ���Ϣ�ĺ��������ĸ���
	gsmMsgNumbufGroup = gsm_Number_Group_test_sub(gsmMsgNumbuf);
	
	if(gsmMsgNumbufGroup == 0 )
	{//���벻��������ʱ
		if(telenumoldbuf==0)
		{//��δ���ú���ʱ,�����ֻ�������ú�������������ִ�С�
		}
		else
		{//����Ȩ���룬�򲻴���
			return;
		}
	}
	
	gsm_data_con_PDU_sub(pCommKZQ,pCommGSM,(uint8 *)&sCurSMS[0],q);   //���յ������ݽ��д���ͷ���
	SETBIT(funcflag5,mc52havetrancedataled);//��������ָͨʾLED
	netlestimebuf = 0;	// ָʾ��ʱ����0
}


//���ش����ֵ�ʱ��ĵ�50���ַ�λ�ã�������������������50���ͳ�����������
uint8 Find_UCS2_50_pos(char *pStr)
{
	uint8 i=0,k=0,hz=0;

	while(*pStr != '\0')
	{
        if (*pStr > 0x80)
		{
			i ++;
			pStr++;
			pStr++;
			k++;
			k++;
			hz = 1;
		}
		else
		{
			i++;
			pStr++;
			k++;
		}

		if(i==140)
			return i;
		if(i==50)
		{//��50�����򷵻ص�70���ַ���λ��
			if(hz)
				return k;
		}
		else if(i>50)
		{
			if(hz)
				return k-2;
		}
	}
	return k;
}

void gsmsendteststr(void)
{
	COM_STRUCT *pComm = &g_Comm1;

	//û�����ú��룬������Ϣ
	if(telenumoldbuf==0)
		return;
	pComm->TXTELBufNUM[pComm->TXBUFNumber] = YongfuNumStart;//���ظ��绰�ĺ���Ȩ�����
	if(Gsmmodeselect == MC52GSMPDU)
		AddMsgGroup("������!",YongfuNumStart);
	else
		AddMsgGroup("On line!",YongfuNumStart);
}

//ɾ������
void DeleteOldMsg(COM_STRUCT *pComm,uint8 msgNo)
{
	char  uuuu[64];
	sprintf(uuuu, "AT+CMGD=%d\r\n", msgNo);
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],uuuu);
	Uart_Send_start(pComm);
}
