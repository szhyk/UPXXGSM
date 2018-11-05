/***********************************************

************************************************/

#include <math.h>
#include <string.h>
#include "gprs.h"
#include "gsm.h"
#include "comm.h"
#include "uart.h"
#include "work.h"

uint8  zsqbuf0 = 	0;
uint8  zsqbuf2 =	0;
uint8  simIDnumbuf[20];
uint8  Mc52xilienumbuf[15];
char sSCA[13];


uint16 MC52reSETTIMEBUF;

uint8  Gprsststuretypebuf;
uint8  Gprserrnumbuf;
uint16 Gprssendatdelaytime;
uint16 Gprstesttimedelaybuf;
uint16 Gprstesttime2delaybuf;
uint16 Gprsbaohutimebuf;

uint16 	mc52itimebuf = 	0; //�������ͻ������ݸ�����Ķ�ʱ���á�
uint16 	mc52gprseerobuf = 	0;
/*********************************************/
//����Ϊ���ֱ�־λ

uint8 funcflag3;
uint8 funcflag5;
uint8 funcflag7;
uint8 mc52igprsflag;
uint8 Mc52igprssetflag;
uint8 Mc52i2gprssetflag;
uint8 mc52iworkflag;
uint8 MC52istaflag;
uint8 Mc52igprsipsetflag;
uint8 Mc52masterNetf;
uint8 Mc52prepareNetf;
uint8 Gsmstatureflag;
uint8 MC52iworkstatusbuf;

uint8  telenumoldbuf;
uint8  telenumnewbuf;

uint8  gsmtelnumbuf[teletaolnumbuf][11];
uint16 gsmtelsuper;
uint8  gsmduanxinnum;   // ���Ž�����ʱ���и���ʾ�ǵڼ�������λ��
uint8  gsmdelectduanxinnum;  //Ҫɾ���Ķ���λ��
uint8  gsmMsgNumbufGroup; //���ź������ڶ��ź��������λ��

uint8  teleToalnumbuf;	//��ǰҪ���͵ĺ�����
uint8  teletolnumbuf;	//����������=���õĺ������+�����������
uint8  telebaohunumbuf;	//��ǰ���͵ĺ�����




uint8  Gsmmodeselect;   //GSM����ģʽ��ѡ��0ΪTEXTģʽ��1Ϊ����ģʽ

uint8  GsmOKfunbuf=0;
uint16 GsmEorrefunbuf=0;

extern uint8 g_bCandeleteMsg;
extern uint16 g_deleteMsg_Rst_Count;
uint8 CheckCmdStr(uint8 * pbuf, char *str);

uint8 gprs_ok_test_sub(COM_STRUCT *pComm);
void gprs_eer_test_sub(COM_STRUCT *pComm);

extern void test_Send_data_to_uart(COM_STRUCT *pCommT, COM_STRUCT *pCommS);
const uint8  NUMDATA_TO_ASC[]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,0x45,0x46};

void RX_gprs_set_SUB(COM_STRUCT *pComm)
{
	uint8 s=0;

	WDR();//�忴�Ź�
	if(	gprs_ok_test_sub(pComm))	//����ģ���Ƿ�����
	{//ģ��Ӧ��OK
		Gprserrnumbuf=0;
		switch(Gprsststuretypebuf)
		{//����״̬���;�����һ��Ҫ����
		case gprsATstaturef:{Mc52i_CMD_flag1.bit.attestflag = 1;}break;		//��һ��ָ�� AT&F
		case gprsATTESTf:{Mc52i_CMD_flag1.bit.atcpinflag = 1;}break;		//��һ��ָ�� CPIN
		case gprscpinstaturef:
			{
				Mc52i_CMD_flag1.bit.clipflag = 1;		//��һ��ָ�� CLIP
				for(s=0; s<pComm->RXIndex; s++)
				{
					if(CheckCmdStr(&pComm->RXdata[s],"READY"))
					{
						statusflag0.bit.noSim = 0;
					} 
				}
							  
			}break;
		case gprsCLIPstaturef:{Mc52i_CMD_flag1.bit.cnmiflag = 1;}break;		//��һ��ָ�� CNMI
		case gprsCNMIstaturef:{Mc52i_CMD_flag1.bit.cmgfflag = 1;}break;		//��һ��ָ�� CMGF
		case gprsCMGFstaturef:{Mc52i_CMD_flag1.bit.csmpflag = 1;}break;		//��һ��ָ�� CSMP
		case gprsCSMPf:       {Mc52i_CMD_flag1.bit.at_wflag = 1;}break;		//��һ��ָ�� AT&W
		case gprsATWstaturef: {Mc52i_CMD_flag1.bit.atgsnflag = 1;}break;	//��һ��ָ�� AT+GSN
		case gprsGSNstaturef:
			{
				Mc52i_CMD_flag1.bit.atscidflag = 1;						//��һ��ָ�� AT^SCID
				if(pComm->RXIndex > 10)    //�Ᵽ֤��ͬʱ����
				{
					for(s=0; s<pComm->RXIndex; s++)
					{
						if(CheckCmdStr(&pComm->RXdata[s],"GSN"))
						{
							if(CheckCmdStr(&pComm->RXdata[pComm->RXIndex-4],"OK"))
							{
								zsqbuf0 = s+6;
								for(zsqbuf2=0; zsqbuf2<Mc52xilienum; zsqbuf2++)
								{
									Mc52xilienumbuf[zsqbuf2] = pComm->RXdata[zsqbuf0+zsqbuf2];
								}
								statusflag0.bit.cantest232flag = 1;//���Զ�ģ������úͲ��Ա�־
								return;
							}
						}
					}
				}
			}
			break;
		case gprsSCIDstaturef:
			{
				Mc52i_CMD_flag1.bit.atcscaflag = 1;			//��һ��ָ�� AT+CSCA?\r\n
				if(pComm->RXIndex > 10)    //�Ᵽ֤��ͬʱ����
				{
					for(s=0; s<pComm->RXIndex; s++)
					{
						if(CheckCmdStr(&pComm->RXdata[s],"SCID:"))
						{
							zsqbuf0 = s+6;
							for(zsqbuf2=0; zsqbuf2<SimIDnum; zsqbuf2++)
							{
								simIDnumbuf[zsqbuf2] = pComm->RXdata[zsqbuf0+zsqbuf2];
							}
							SETBIT(MC52istaflag,mc52igetidnum);
							return;
						} 
					}
				}
			}
			break;
		case GSMSAVEMODE:
			{
				Mc52i_CMD_flag1.bit.atcsqflag = 1;				//��һ��ָ�� AT+CSQ\r\n
			}
			break;
		case gprsCSCAstaturef:
			{//��ȡ�������ĺ���
				statusflag0.bit.noSim = 0;

				Mc52i_CMD_flag1.bit.atSetSaveMode = 1;				//��һ��ָ�� AT+CSQ\r\n
//				Mc52i_CMD_flag1.bit.atcsqflag = 1;				//��һ��ָ�� AT+CSQ\r\n
				if(pComm->RXIndex > 10)    //�Ᵽ֤��ͬʱ����
				{
					for(s=0; s<pComm->RXIndex; s++)
					{
						if(CheckCmdStr(&pComm->RXdata[s],"CSCA:"))
						{
							if(	(pComm->RXdata[pComm->RXIndex-4]=='O')&&(pComm->RXdata[pComm->RXIndex-3]=='K') )
							{//^SIS
								zsqbuf0 = s+8;
								for(zsqbuf2=0; zsqbuf2<simcardnum; zsqbuf2++)
								{
									sSCA[zsqbuf2] = pComm->RXdata[zsqbuf0+zsqbuf2];
								}
								return;
							}
						}
					}
				}
			}
			break;
			/**************************************/
		case gprsCSQstaturef:
			{//��ȡ�ź�ǿ��
				if(pComm->RXIndex > 10)    //�Ᵽ֤��ͬʱ����
				{
					for(s=0; s<pComm->RXIndex; s++)
					{
						if(CheckCmdStr(&pComm->RXdata[s],"+CSQ: 99,"))
						{//��ʾ���ź�
							SETBIT(MC52istaflag,mc52iNoSignal);	//���ź�
							CLRBIT(mc52iworkflag,mc52gsmworkingf);	//�������
							return;
						}
						if(CheckCmdStr(&pComm->RXdata[s],"+CSQ:99,"))
						{//��ʾ���ź�
							SETBIT(MC52istaflag,mc52iNoSignal);	//���ź�
							CLRBIT(mc52iworkflag,mc52gsmworkingf);	//�������
							return;
						}
					}
				}
				CLRBIT(MC52istaflag,mc52iNoSignal);	//���ź�

				if(MC52iworkstatusbuf==mc52workgsmf)
				{
					SETBIT(mc52iworkflag,mc52gsmworkingf); //��������������
					statusflag0.bit.telconconnect = 1; //���߿�������������
					//��ʼ����������
					Mc52i_CMD_flag1.bit.first_connect = 1;
				}
				eepbufflag.bit.baohuworkf = 0;			//�屣����־
				eepbufflag.bit.Diwunworkingf1 = 0;		//��
			}
			break;  
			/********************************************************/
		case gprsconTypestaturef:  {SETBIT(Mc52igprssetflag,mc52ipasswdsf);}   break;
		case gprspasswdstaturef:   {SETBIT(Mc52igprssetflag,mc52itimef);}    break;
		case gprstimestaturef:     {SETBIT(Mc52igprssetflag,mc52iuseresf);}    break;
		case gprsuserstaturef:     {SETBIT(Mc52igprssetflag,mc52iapnsf);}      break;
//		case gprsapnstaturef:      {SETBIT(Mc52i2gprssetflag,mc52i2conTypesf);}break;

		case gprsapnstaturef:      {SETBIT(Mc52igprsipsetflag,mc52imsrvTypesf);}break;
			/********************************************************/
		case gprs2conTypestaturef:  {SETBIT(Mc52i2gprssetflag,mc52i2passwdsf);}   break;
		case gprs2passwdstaturef:   {SETBIT(Mc52i2gprssetflag,mc52i2timef);}    break;
		case gprs2timestaturef:     {SETBIT(Mc52i2gprssetflag,mc52i2useresf);}    break;
		case gprs2userstaturef:     {SETBIT(Mc52i2gprssetflag,mc52i2apnsf);}
			/********************************************************/
		case gprs2apnstaturef:      {SETBIT(Mc52igprsipsetflag,mc52imsrvTypesf);}break;
		case gprsmsrvTypestaturef: {SETBIT(Mc52igprsipsetflag,mc52imconIdsf);}  break;	
		case gprsmconIdstaturef:   {SETBIT(Mc52igprsipsetflag,mc52imaddresssf);}break;
			
		case gprsmaddressstaturef: {SETBIT(Mc52igprsipsetflag,mc52irsrvTypesf);}break;
		case gprsrsrvTypestaturef: {SETBIT(Mc52igprsipsetflag,mc52irconIdsf);}  break;	
		case gprsrconIdstaturef:   {SETBIT(Mc52igprsipsetflag,mc52iraddresssf);}break;
		case gprsraddressstaturef: {SETBIT(Mc52igprsipsetflag,closemgprsf);}    break; //�������Ļ������Ѿ���������ˡ�����Ĺ�����������������
		case gprsmCLOSENETstaturef:{SETBIT(Mc52igprssetflag,closergprsf);}     break; //�ȹر�
		case gprsrCLOSENETstaturef:{SETBIT(funcflag3,mc52icxlianjieopenf);}break;
		case gprscxljopenf:
			{
				CLRBIT(funcflag5,mc52icxnjtdelayf);  //��������
				Gprsststuretypebuf=0; //����������ķ���
				SETBIT(mc52igprsflag,mc52igprsworking); //�����磬����һ���Ǹ���̨�����ϵ�
				SETBIT(funcflag7,gprsnettotestf); //��ʼ����ʱ��������̫��ʱ��û��Ӧ�͹���
			}
			break;
		}
	return;
	}
	gprs_eer_test_sub(pComm);
	if(CHKBITSET(mc52igprsflag,mc52ierrf))
	{//�ش�ERR
		CLRBIT(mc52igprsflag,mc52ierrf);
		Gprserrnumbuf++;
		Gprssendatdelaytime=0;
		if(Gprserrnumbuf > 20)
		{
			Gprserrnumbuf = 0;   //���Ӵ������
			Mc52i_CMD_flag1.bit.atsendf = 1;	//��һ��ָ���AT
			if(eepbufflag.bit.Diwunworkingf1 == 0)
			{
				if((Gprsststuretypebuf==gprsCNMIstaturef)||(Gprsststuretypebuf==gprsCSCAstaturef))
				{
					eepbufflag.bit.baohuworkf = 0;
				}
			}
			return;
		}
		//�ط�ָ��
		switch(Gprsststuretypebuf)
		{
		case gprsATstaturef: {Mc52i_CMD_flag1.bit.atsendf = 1;}break; 
//		case gprsATTESTf:{Mc52i_CMD_flag1.bit.attestflag = 1;}break; 
		case gprsATTESTf:{Mc52i_CMD_flag1.bit.attestflag = 1;}break; 
		case gprscpinstaturef:
			{
				Mc52i_CMD_flag1.bit.atcpinflag = 1;
				statusflag0.bit.noSim = 1;
			}
			break; 
		case gprsCLIPstaturef: {Mc52i_CMD_flag1.bit.clipflag = 1;}break;
		case gprsCNMIstaturef:{Mc52i_CMD_flag1.bit.cnmiflag = 1;}break;
		case gprsCMGFstaturef:{Mc52i_CMD_flag1.bit.cmgfflag = 1;}break;
		case gprsCSMPf:{Mc52i_CMD_flag1.bit.csmpflag = 1;}break;
		case gprsATWstaturef:{Mc52i_CMD_flag1.bit.at_wflag = 1;}break; 
		case gprsGSNstaturef:{Mc52i_CMD_flag1.bit.atgsnflag = 1;}break;
		case gprsSCIDstaturef:{Mc52i_CMD_flag1.bit.atscidflag = 1;}break;
		case gprsCSCAstaturef:{Mc52i_CMD_flag1.bit.atcscaflag = 1;}break;
		case gprsCSQstaturef:{Mc52i_CMD_flag1.bit.atcsqflag = 1;}break;
		case gprsconTypestaturef:  {SETBIT(Mc52igprssetflag,mc52iconTypesf);}break;
		case gprspasswdstaturef:   {SETBIT(Mc52igprssetflag,mc52ipasswdsf);}break;
		case gprstimestaturef:     {SETBIT(Mc52igprssetflag,mc52itimef);}break;
		case gprsuserstaturef:     {SETBIT(Mc52igprssetflag,mc52iuseresf);}break;
		case gprsapnstaturef:      {SETBIT(Mc52igprssetflag,mc52iapnsf);}break;
			
		case gprs2conTypestaturef:  {SETBIT(Mc52i2gprssetflag,mc52i2conTypesf);}break;
		case gprs2passwdstaturef:   {SETBIT(Mc52i2gprssetflag,mc52i2passwdsf);}break;
		case gprs2timestaturef:     {SETBIT(Mc52i2gprssetflag,mc52i2timef);}break;
		case gprs2userstaturef:     {SETBIT(Mc52i2gprssetflag,mc52i2useresf);}break;
		case gprs2apnstaturef:      {SETBIT(Mc52i2gprssetflag,mc52i2apnsf);}break;
			
		case gprsmsrvTypestaturef: {SETBIT(Mc52igprsipsetflag,mc52imsrvTypesf);}break;
		case gprsmconIdstaturef:   {SETBIT(Mc52igprsipsetflag,mc52imconIdsf);}break;
		case gprsmaddressstaturef: {SETBIT(Mc52igprsipsetflag,mc52imaddresssf);}break;
		case gprsrsrvTypestaturef: {SETBIT(Mc52igprssetflag,mc52irsrvTypesf);}break;
		case gprsrconIdstaturef:   {SETBIT(Mc52igprssetflag,mc52irconIdsf);}break;
		case gprsraddressstaturef: {SETBIT(Mc52igprssetflag,mc52iraddresssf);}break;
		case gprsmCLOSENETstaturef:{SETBIT(Mc52igprsipsetflag,closemgprsf);}break;
		case gprsrCLOSENETstaturef:{SETBIT(Mc52igprssetflag,closergprsf);}break;
		case gprscxljopenf:{SETBIT(funcflag3,mc52icxlianjieopenf);}break;
		}
		return;
	}
	for(s=0; s<pComm->RXIndex; s++)
	{
		if(CheckCmdStr(&pComm->RXdata[s],"START"))
		{//�յ�"START"����ʼ����ATָ��
			Mc52i_CMD_flag1.bit.atsendf = 1;
			return;
		}
		if(CheckCmdStr(&pComm->RXdata[s],"^SIS:"))
		{//�յ�"^SIS:","Remote host has rejected the connection"
			Gprstesttimedelaybuf=0;
			Gprstesttime2delaybuf=0;
			SETBIT(funcflag3,mc52iqhnjtdelayf); //�л�����
			CLRBIT(mc52igprsflag,mc52igprsnetoking);//��̨�������ߣ������Ҫ��ͣ��ȥ����
			CLRBIT(mc52igprsflag,mc52igprsworking);
			SETBIT(mc52iworkflag,gprsSISMf); //��Ҫ�����κ�����
			return; //����û�к�̨��ʱ���б��ָ��ء�
		}
	}
}


/****************************/
uint8 gprs_ok_test_sub(COM_STRUCT *pComm)
{
	uint8 s;

	for(s=0; s<pComm->RXIndex-1; s++)
	{
		if(CheckCmdStr(&pComm->RXdata[s],"OK"))
		{
			return 1;
		}
	}
	return 0;
}
/****************************
+CME ERROR: 256,262
****************************/
void gprs_eer_test_sub(COM_STRUCT *pComm)
{
	uint8 s;

	for(s=0;s<pComm->RXIndex-2;s++)
	{
		if((pComm->RXdata[s]=='E')&&(pComm->RXdata[s+1]=='R')&&(pComm->RXdata[s+2]=='R'))
		{
			if( s < pComm->RXIndex-9 )
			{
				if( pComm->RXdata[s+6] == 0x20 )
				{//�ո�
					if( pComm->RXdata[s+9] == 0x0D )
					{//�س�
						mc52gprseerobuf = (pComm->RXdata[s+7]-0x30)*10 + (pComm->RXdata[s+8]-0x30);
					}
					else if( s < pComm->RXIndex-10 )
					{
						if( pComm->RXdata[s+10] == 0x0D )
							mc52gprseerobuf = (pComm->RXdata[s+7]-0x30)*100 + (pComm->RXdata[s+8]-0x30)*10 + (pComm->RXdata[s+9]-0x30);
					}
				}
			}
			SETBIT(mc52igprsflag,mc52ierrf);
			return;
		}
	}
}


/*******************************
AT
����:�������ж�ͨ�������������Ҫ��ģ��������Ƿ���ȷ
����:OK TEST OK
*******************************/
void GPRS_AT_SEND_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT\r\n");
	Uart_Send_start(pComm);
}

/*******************************
AT+CLIP=1
����:�������ж�ͨ�������������Ҫ��ģ��������Ƿ���ȷ
����:OK TEST OK
*******************************/
void GPRS_AT_CLIP_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CLIP=1\r\n");
	Uart_Send_start(pComm);
}	
/**********************
����ָ����,��ָ��Ӧ��Ϊ�ָ��������� hyk
***********************/
void AT_Send_TEST_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT&F\r\n");
	Uart_Send_start(pComm);
}	
/**********************
�һ�����
***********************/
void GPRS_AT_ATH_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"ATH\r\n");
	Uart_Send_start(pComm);
}	
/*******************************
����:SCID,������ //��ID���� 
����:^SCID: 89860075190930002379
AT^SCID
*******************************/
void GPRS_AT_SCID_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT^SCID\r\n");
	Uart_Send_start(pComm);
}
/*******************************
����:AT+CSCA?  //�����ĺ���
����:+CSCA: "+8613800756500",145
AT+CSCA?
*******************************/
void AT_Send_CSCA_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CSCA?\r\n");
	Uart_Send_start(pComm);
}

void GPRS_AT_CPNS_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n");//��������
	Uart_Send_start(pComm);
}

/*******************************
����:AT+GSN    //��ȡ���к�
����:357224021125195
AT+GSN
*******************************/
void AT_Send_GSN_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+GSN\r\n");
	Uart_Send_start(pComm);
}
/*******************************
����:AT+CSQ    ��ȡ�ź�ǿ��
����:+CSQ: 21,99
AT+CSQ
*******************************/
void AT_Send_CSQ_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CSQ\r\n");
	Uart_Send_start(pComm);
}
/*******************************
����:AT+CNMI=2,1,0,0,1 ���ö�����ʾ��ʽ
<mode>2���������߿���ʱ��ֱ��֪ͨTE�������Ƚ�֪ͨ�����������������߿���ʱ���з��͡� 
<mt>1�������յĶ���Ϣ���浽Ĭ�ϵ��ڴ�λ�ã�������TE����֪ͨ������class 3����֪ͨ����ʽΪ�� ��CMTI����SM������index�� 
<bm>0-
<ds>0-
<bfr>1-
����:OK
AT+CNMI
*******************************/
void AT_Send_CNMI_SUB(COM_STRUCT *pComm)
{
//	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CNMI=2,1,0,0,1\r\n");//�����ԭ��������
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CNMI=2,1,0,0,1\r");
	Uart_Send_start(pComm);
}
/*******************************
����:at+cmgf=1 ���ö���ģʽ
����:OK
*******************************/
void AT_Send_cmgf_SUB(COM_STRUCT *pComm)
{
	uint8 z=0;

	z = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CMGF=");
	pComm->TXdata[z++] = Gsmmodeselect;  //0-pdu,1-textģʽ
	pComm->TXdata[z++] =_CR;//�س�
	pComm->TXdata[z++] =_LF;//����
	pComm->Senddatanum = z;
	Uart_Send_start(pComm);
}
/*******************************
����:AT+CSMP=81 ����Ϊ�����ŷ�ʽ
����:OK
AT+CMEE=1
*******************************/
void GPRS_AT_CSMP_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CMEE=1\r\n");
	Uart_Send_start(pComm);
}
/*******************************
����:AT&W ��������
����:OK
*******************************/
void AT_Send_W_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT&W\r\n");
	Uart_Send_start(pComm);
}
/*******************************
����:A/ ���·���
����:OK
*******************************/
void GPRS_A_W_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"A/\r\n");
	Uart_Send_start(pComm);
}
/*******************************
����:at+cmgr=? ���������� 
����:OK
void byte_disp(uchar pag,
*******************************/
void AT_Send_cmgr_SUB(COM_STRUCT *pComm,uint8 Readnumdata)
{
	uint8 z=0;

	z = pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CMGR=");
	if(Readnumdata>99)
		pComm->TXdata[z++] = NUMDATA_TO_ASC[Readnumdata/100];
	if(Readnumdata>9)
		pComm->TXdata[z++] = NUMDATA_TO_ASC[(Readnumdata%100)/10];
	pComm->TXdata[z++] = NUMDATA_TO_ASC[Readnumdata%10];
	pComm->TXdata[z++] = _CR;//�س�
	pComm->TXdata[z++] = _LF;//����
	pComm->Senddatanum = z;
	Uart_Send_start(pComm);
}
/*******************************
����:at+cmgd=x ɾ��������Ϣ
����:OK
*******************************/
void AT_Send_CMGD_SUB(COM_STRUCT *pComm,uint8 delectnumdata)
{
	uint8 z = 0;

	z = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CMGD=");
	if(delectnumdata>99)
		pComm->TXdata[z++] = NUMDATA_TO_ASC[delectnumdata/100];
	if(delectnumdata>9)
		pComm->TXdata[z++] = NUMDATA_TO_ASC[(delectnumdata%100)/10];
	pComm->TXdata[z++] = NUMDATA_TO_ASC[delectnumdata%10];
	pComm->TXdata[z++] = _CR;//�س�
	pComm->TXdata[z++] = _LF;//����
	pComm->Senddatanum = z;
	Uart_Send_start(pComm);
}

//���ýӿڹ������ 1--��ʾ����LEDģʽ
void AT_Send_SSYNC_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT^SSYNC=1\r\n");
	Uart_Send_start(pComm);
}


void AT_Send_CPIN_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CPIN?\r\n");
	Uart_Send_start(pComm);
}

/*******************************
����:��GPRS���ܴ����������ݽ��д���
����:
��ע:ÿ��һ�ֹ���ģʽ��������������Դ����
�ܽ�����˵��ģ���Ǵ��ڵ�.
���õ�ʱ����OK����
*******************************/

void GSM_RX_Commun_SUB(COM_STRUCT *pCommKZQ,COM_STRUCT *pCommGSM)
{
	Nodatatratbuf=0;//���Կ�����û��AT
	if(CHKBITSET(mc52iworkflag,mc52gsmworkingf)) //GSMģʽ
	{
		RX_gsmwork_SUB(pCommKZQ,pCommGSM);
	}
	else          //��������״̬
	{
		RX_gprs_set_SUB(pCommGSM);
	}
}


void mc52_work_sub(void)
{
	COM_STRUCT *pComm = &g_Comm1;
	if(CHKBITSET(pComm->funcflag,uartsenddata))
	{
		return;//�ڷ����ݵ�ʱ���ܽ���
	}
	if(CHKBITSET(mc52iworkflag,mc52gsmsendbaohuf))
	{//�����ڼ䲻�ܽ���
		return;
	}
// 	if(Gprsworkingflag > 0)
// 	{
// 		return;//�ڷ����ݵ�ʱ���ܽ���
// 	}

	/***********GSM********************/
	if(CHKBITSET(Gsmstatureflag,gsmreaddataf))
	{//���µĶ���Ҫ��
		CLRBIT(Gsmstatureflag,gsmreaddataf);
		AT_Send_cmgr_SUB(pComm,gsmduanxinnum); //������
		return;
	}
	/**************************************/
	if(CHKBITSET(Gsmstatureflag,gsmdelectduanxf))
	{//ɾ������
		CLRBIT(Gsmstatureflag,gsmdelectduanxf);
		AT_Send_CMGD_SUB(pComm,gsmdelectduanxinnum); //��������ɾ��
		GsmOKfunbuf=GsmSendSCDXbuf;
		g_bCandeleteMsg = 1;//�����ܶ�ʱ�Ŀ�ʼɾ��������
		return;
	}
	/***********GSM********************/
	//��������������ʱ�ǲ��ᷢ�ģ���ʼ����������ʱ��ʹ�ã����Կ�����ʱ����
	if(CHKBITSET(mc52igprsflag,mc52isenddelay))
	{//MC52I����һֱ����1
		Gprssendatdelaytime++;
		if(Gprssendatdelaytime > 200)
		{
			Gprssendatdelaytime=0;
			if(Mc52i_CMD_flag1.bit.atsendf)
			{
				Mc52i_CMD_flag1.bit.atsendf = 0;
				CLRBIT(MC52istaflag,mc52iNoSignal);	//���¿�ʼ��������źű�־
				GPRS_AT_SEND_SUB(pComm);  //AT�������� AT\r
				Gprsststuretypebuf=gprsATstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.attestflag)
			{
				Mc52i_CMD_flag1.bit.attestflag = 0;
				AT_Send_TEST_SUB(pComm);  //AT��������,�����ã�������һЩû�õ���ָ��
				Gprsststuretypebuf = gprsATTESTf;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.atcpinflag)
			{
				Mc52i_CMD_flag1.bit.atcpinflag = 0;
				AT_Send_CPIN_SUB(pComm);
				Gprsststuretypebuf = gprscpinstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.clipflag)
			{
				Mc52i_CMD_flag1.bit.clipflag = 0;
				GPRS_AT_CLIP_SUB(pComm); //������ʾ����
				Gprsststuretypebuf=gprsCLIPstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.cnmiflag)
			{
				Mc52i_CMD_flag1.bit.cnmiflag = 0;
				AT_Send_CNMI_SUB(pComm); //�¶�����ʾ��ʽ
				Gprsststuretypebuf=gprsCNMIstaturef;
				return;
			}	
			else if(Mc52i_CMD_flag1.bit.cmgfflag)
			{
				Mc52i_CMD_flag1.bit.cmgfflag = 0;
				AT_Send_cmgf_SUB(pComm); //����TEXT����ģʽ
				Gprsststuretypebuf=gprsCMGFstaturef;
				return;
			}	
			else if(Mc52i_CMD_flag1.bit.csmpflag)
			{
				Mc52i_CMD_flag1.bit.csmpflag = 0;
				GPRS_AT_CSMP_SUB(pComm); //�ϳɳ�������/��������չ��Ҳ����
				Gprsststuretypebuf=gprsCSMPf;
				return;
			}	
			else if(Mc52i_CMD_flag1.bit.at_wflag)
			{
				Mc52i_CMD_flag1.bit.at_wflag = 0;
				AT_Send_W_SUB(pComm); //������������
				Gprsststuretypebuf=gprsATWstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.atgsnflag)
			{
				Mc52i_CMD_flag1.bit.atgsnflag = 0;
				AT_Send_GSN_SUB(pComm);  //��ȡ���к�
				Gprsststuretypebuf=gprsGSNstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.atscidflag)
			{
				Mc52i_CMD_flag1.bit.atscidflag = 0;
				GPRS_AT_SCID_SUB(pComm); //��ID���� 
				Gprsststuretypebuf=gprsSCIDstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.atcscaflag)
			{
				Mc52i_CMD_flag1.bit.atcscaflag = 0;
				AT_Send_CSCA_SUB(pComm); //�����ĺ���
				Gprsststuretypebuf=gprsCSCAstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.atSetSaveMode)
			{
 				Mc52i_CMD_flag1.bit.atSetSaveMode = 0;
  				GPRS_AT_CPNS_SUB(pComm); //���ô洢ģʽ
  				Gprsststuretypebuf=GSMSAVEMODE;
 				return;
			}
			else if(Mc52i_CMD_flag1.bit.atcsqflag)
			{
				Mc52i_CMD_flag1.bit.atcsqflag = 0;
				AT_Send_CSQ_SUB(pComm);  // ��ȡ�ź�ǿ��
				Gprsststuretypebuf = gprsCSQstaturef;
				return;
			}
			/********************************/
			//����Ϊ�û�����
			else if(CHKBITSET(Mc52igprssetflag,mc52iconTypesf))
			{
				CLRBIT(Mc52igprssetflag,mc52iconTypesf);
// 				GPRS_SET_conType_SUB(pComm,mastermode);
// 				Gprsststuretypebuf=gprsconTypestaturef;
				return;
			}
			else if(CHKBITSET(Mc52igprssetflag,mc52ipasswdsf))
			{
				CLRBIT(Mc52igprssetflag,mc52ipasswdsf);
// 				GPRS_SET_passwd_SUB(pComm,mastermode);
// 				Gprsststuretypebuf=gprspasswdstaturef;
				return;
			}
			else if(CHKBITSET(Mc52igprssetflag,mc52itimef))
			{
				CLRBIT(Mc52igprssetflag,mc52itimef);
// 				GPRS_SET_time_SUB(pComm,mastermode);
// 				Gprsststuretypebuf=gprstimestaturef;
				return;
			}
			else if(CHKBITSET(Mc52igprssetflag,mc52iuseresf))
			{
				CLRBIT(Mc52igprssetflag,mc52iuseresf);
// 				GPRS_SET_user_SUB(pComm,mastermode);
// 				Gprsststuretypebuf=gprsuserstaturef;
				return;
			}
			else if(CHKBITSET(Mc52igprssetflag,mc52iapnsf))
			{
				CLRBIT(Mc52igprssetflag,mc52iapnsf);
// 				GPRS_SET_apn_SUB(pComm,mastermode);
// 				Gprsststuretypebuf=gprsapnstaturef;
				return;
			}
			else if(CHKBITSET(Mc52i2gprssetflag,mc52i2conTypesf))
			{
				CLRBIT(Mc52i2gprssetflag,mc52i2conTypesf);
// 				GPRS_SET_conType_SUB(pComm,preparemode);
// 				Gprsststuretypebuf=gprs2conTypestaturef;
				return;
			}
			else if(CHKBITSET(Mc52i2gprssetflag,mc52i2passwdsf))
			{
				CLRBIT(Mc52i2gprssetflag,mc52i2passwdsf);
// 				GPRS_SET_passwd_SUB(pComm,preparemode);
// 				Gprsststuretypebuf=gprs2passwdstaturef;
				return;
			}
			else if(CHKBITSET(Mc52i2gprssetflag,mc52i2timef))
			{
				CLRBIT(Mc52i2gprssetflag,mc52i2timef);
// 				GPRS_SET_time_SUB(pComm,preparemode);
// 				Gprsststuretypebuf=gprs2timestaturef;
				return;
			}
			if(CHKBITSET(Mc52i2gprssetflag,mc52i2useresf))
			{
				CLRBIT(Mc52i2gprssetflag,mc52i2useresf);
// 				GPRS_SET_user_SUB(pComm,preparemode);
// 				Gprsststuretypebuf=gprs2userstaturef;
				return;
			}
			else if(CHKBITSET(Mc52i2gprssetflag,mc52i2apnsf))
			{
				CLRBIT(Mc52i2gprssetflag,mc52i2apnsf);
// 				GPRS_SET_apn_SUB(pComm,preparemode);
// 				Gprsststuretypebuf=gprs2apnstaturef;
				return;
			}
			// ������������
			else if(CHKBITSET(Mc52igprsipsetflag,mc52imsrvTypesf))
			{
				CLRBIT(Mc52igprsipsetflag,mc52imsrvTypesf);
// 				GPRS_SET_srvType_SUB(pComm,mastermode);
// 				Gprsststuretypebuf=gprsmsrvTypestaturef;
				return;
			}
			else if(CHKBITSET(Mc52igprsipsetflag,mc52imconIdsf))
			{
				CLRBIT(Mc52igprsipsetflag,mc52imconIdsf);
// 				GPRS_SET_conId_SUB(pComm,mastermode);
// 				Gprsststuretypebuf=gprsmconIdstaturef;
				return;
			}
			else if(CHKBITSET(Mc52igprsipsetflag,mc52imaddresssf))
			{
				CLRBIT(Mc52igprsipsetflag,mc52imaddresssf);
// 				GPRS_SET_address_SUB(pComm,mastermode);
// 				Gprsststuretypebuf=gprsmaddressstaturef;
				return;
			}
			// ������������
			else if(CHKBITSET(Mc52igprsipsetflag,mc52irsrvTypesf))
			{
				CLRBIT(Mc52igprsipsetflag,mc52irsrvTypesf);
// 				GPRS_SET_srvType_SUB(pComm,preparemode);
// 				Gprsststuretypebuf=gprsrsrvTypestaturef;
				return;
			}
			else if(CHKBITSET(Mc52igprsipsetflag,mc52irconIdsf))
			{
				CLRBIT(Mc52igprsipsetflag,mc52irconIdsf);
// 				GPRS_SET_conId_SUB(pComm,preparemode);
// 				Gprsststuretypebuf=gprsrconIdstaturef;
				return;
			}
			else if(CHKBITSET(Mc52igprsipsetflag,mc52iraddresssf))
			{
				CLRBIT(Mc52igprsipsetflag,mc52iraddresssf);
// 				GPRS_SET_address_SUB(pComm,preparemode);
// 				Gprsststuretypebuf=gprsraddressstaturef;
				return;
			}
			// ��������
			else if(CHKBITSET(Mc52igprsipsetflag,closemgprsf))
			{
				CLRBIT(Mc52igprsipsetflag,closemgprsf);
// 				GPRS_AT_CLOSENET_SUB(pComm,mastermode);
// 				Gprsststuretypebuf=gprsmCLOSENETstaturef;
				return;
			}
			// ��������
			else if(CHKBITSET(Mc52igprssetflag,closergprsf))
			{
				CLRBIT(Mc52igprssetflag,closergprsf);
// 				GPRS_AT_CLOSENET_SUB(pComm,preparemode);
// 				Gprsststuretypebuf=gprsrCLOSENETstaturef;
				return;
			}
			// ������
			else if(CHKBITSET(funcflag3,mc52icxlianjieopenf))
			{
				CLRBIT(funcflag3,mc52icxlianjieopenf);
// 				Gprsststuretypebuf = gprscxljopenf;
				return;
			}
		}
	}
}


