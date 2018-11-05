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

uint16 	mc52itimebuf = 	0; //用来发送缓冲数据给网络的定时器用。
uint16 	mc52gprseerobuf = 	0;
/*********************************************/
//以下为各种标志位

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
uint8  gsmduanxinnum;   // 短信进来的时候有个提示是第几条短信位置
uint8  gsmdelectduanxinnum;  //要删除的短信位置
uint8  gsmMsgNumbufGroup; //短信号码所在短信号码数组的位置

uint8  teleToalnumbuf;	//当前要发送的号码编号
uint8  teletolnumbuf;	//号码总数量=设置的号码个数+超级号码个数
uint8  telebaohunumbuf;	//当前发送的号码组




uint8  Gsmmodeselect;   //GSM短信模式的选择，0为TEXT模式，1为中文模式

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

	WDR();//清看门狗
	if(	gprs_ok_test_sub(pComm))	//测试模块是否联机
	{//模块应答OK
		Gprserrnumbuf=0;
		switch(Gprsststuretypebuf)
		{//根据状态类型决定下一步要干嘛
		case gprsATstaturef:{Mc52i_CMD_flag1.bit.attestflag = 1;}break;		//下一个指令 AT&F
		case gprsATTESTf:{Mc52i_CMD_flag1.bit.atcpinflag = 1;}break;		//下一个指令 CPIN
		case gprscpinstaturef:
			{
				Mc52i_CMD_flag1.bit.clipflag = 1;		//下一个指令 CLIP
				for(s=0; s<pComm->RXIndex; s++)
				{
					if(CheckCmdStr(&pComm->RXdata[s],"READY"))
					{
						statusflag0.bit.noSim = 0;
					} 
				}
							  
			}break;
		case gprsCLIPstaturef:{Mc52i_CMD_flag1.bit.cnmiflag = 1;}break;		//下一个指令 CNMI
		case gprsCNMIstaturef:{Mc52i_CMD_flag1.bit.cmgfflag = 1;}break;		//下一个指令 CMGF
		case gprsCMGFstaturef:{Mc52i_CMD_flag1.bit.csmpflag = 1;}break;		//下一个指令 CSMP
		case gprsCSMPf:       {Mc52i_CMD_flag1.bit.at_wflag = 1;}break;		//下一个指令 AT&W
		case gprsATWstaturef: {Mc52i_CMD_flag1.bit.atgsnflag = 1;}break;	//下一个指令 AT+GSN
		case gprsGSNstaturef:
			{
				Mc52i_CMD_flag1.bit.atscidflag = 1;						//下一个指令 AT^SCID
				if(pComm->RXIndex > 10)    //这保证是同时来的
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
								statusflag0.bit.cantest232flag = 1;//可以对模块的设置和测试标志
								return;
							}
						}
					}
				}
			}
			break;
		case gprsSCIDstaturef:
			{
				Mc52i_CMD_flag1.bit.atcscaflag = 1;			//下一个指令 AT+CSCA?\r\n
				if(pComm->RXIndex > 10)    //这保证是同时来的
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
				Mc52i_CMD_flag1.bit.atcsqflag = 1;				//下一个指令 AT+CSQ\r\n
			}
			break;
		case gprsCSCAstaturef:
			{//读取短信中心号码
				statusflag0.bit.noSim = 0;

				Mc52i_CMD_flag1.bit.atSetSaveMode = 1;				//下一个指令 AT+CSQ\r\n
//				Mc52i_CMD_flag1.bit.atcsqflag = 1;				//下一个指令 AT+CSQ\r\n
				if(pComm->RXIndex > 10)    //这保证是同时来的
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
			{//读取信号强度
				if(pComm->RXIndex > 10)    //这保证是同时来的
				{
					for(s=0; s<pComm->RXIndex; s++)
					{
						if(CheckCmdStr(&pComm->RXdata[s],"+CSQ: 99,"))
						{//表示无信号
							SETBIT(MC52istaflag,mc52iNoSignal);	//无信号
							CLRBIT(mc52iworkflag,mc52gsmworkingf);	//清除连接
							return;
						}
						if(CheckCmdStr(&pComm->RXdata[s],"+CSQ:99,"))
						{//表示无信号
							SETBIT(MC52istaflag,mc52iNoSignal);	//无信号
							CLRBIT(mc52iworkflag,mc52gsmworkingf);	//清除连接
							return;
						}
					}
				}
				CLRBIT(MC52istaflag,mc52iNoSignal);	//无信号

				if(MC52iworkstatusbuf==mc52workgsmf)
				{
					SETBIT(mc52iworkflag,mc52gsmworkingf); //不用往下设置了
					statusflag0.bit.telconconnect = 1; //告诉控制器连接上了
					//初始化发送数据
					Mc52i_CMD_flag1.bit.first_connect = 1;
				}
				eepbufflag.bit.baohuworkf = 0;			//清保护标志
				eepbufflag.bit.Diwunworkingf1 = 0;		//清
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
		case gprsraddressstaturef: {SETBIT(Mc52igprsipsetflag,closemgprsf);}    break; //到这来的话就是已经完成设置了。后面的工作就是连接问题了
		case gprsmCLOSENETstaturef:{SETBIT(Mc52igprssetflag,closergprsf);}     break; //先关闭
		case gprsrCLOSENETstaturef:{SETBIT(funcflag3,mc52icxlianjieopenf);}break;
		case gprscxljopenf:
			{
				CLRBIT(funcflag5,mc52icxnjtdelayf);  //重新连接
				Gprsststuretypebuf=0; //结束对命令的发送
				SETBIT(mc52igprsflag,mc52igprsworking); //有网络，但不一定是跟后台连接上的
				SETBIT(funcflag7,gprsnettotestf); //开始做记时处理，避免太长时间没反应就挂了
			}
			break;
		}
	return;
	}
	gprs_eer_test_sub(pComm);
	if(CHKBITSET(mc52igprsflag,mc52ierrf))
	{//回答ERR
		CLRBIT(mc52igprsflag,mc52ierrf);
		Gprserrnumbuf++;
		Gprssendatdelaytime=0;
		if(Gprserrnumbuf > 20)
		{
			Gprserrnumbuf = 0;   //连接错误计数
			Mc52i_CMD_flag1.bit.atsendf = 1;	//下一条指令发送AT
			if(eepbufflag.bit.Diwunworkingf1 == 0)
			{
				if((Gprsststuretypebuf==gprsCNMIstaturef)||(Gprsststuretypebuf==gprsCSCAstaturef))
				{
					eepbufflag.bit.baohuworkf = 0;
				}
			}
			return;
		}
		//重发指令
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
		{//收到"START"，则开始发送AT指令
			Mc52i_CMD_flag1.bit.atsendf = 1;
			return;
		}
		if(CheckCmdStr(&pComm->RXdata[s],"^SIS:"))
		{//收到"^SIS:","Remote host has rejected the connection"
			Gprstesttimedelaybuf=0;
			Gprstesttime2delaybuf=0;
			SETBIT(funcflag3,mc52iqhnjtdelayf); //切换连接
			CLRBIT(mc52igprsflag,mc52igprsnetoking);//后台主动断线，后面就要不停的去连接
			CLRBIT(mc52igprsflag,mc52igprsworking);
			SETBIT(mc52iworkflag,gprsSISMf); //主要是屏蔽后面用
			return; //考虑没有后台的时候有别的指令返回。
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
				{//空格
					if( pComm->RXdata[s+9] == 0x0D )
					{//回车
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
功能:基本的判断通信连接情况。主要看模块或波特率是否正确
返回:OK TEST OK
*******************************/
void GPRS_AT_SEND_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT\r\n");
	Uart_Send_start(pComm);
}

/*******************************
AT+CLIP=1
功能:基本的判断通信连接情况。主要看模块或波特率是否正确
返回:OK TEST OK
*******************************/
void GPRS_AT_CLIP_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CLIP=1\r\n");
	Uart_Send_start(pComm);
}	
/**********************
测试指令用,该指令应该为恢复出厂设置 hyk
***********************/
void AT_Send_TEST_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT&F\r\n");
	Uart_Send_start(pComm);
}	
/**********************
挂机令用
***********************/
void GPRS_AT_ATH_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"ATH\r\n");
	Uart_Send_start(pComm);
}	
/*******************************
功能:SCID,并保存 //读ID号码 
返回:^SCID: 89860075190930002379
AT^SCID
*******************************/
void GPRS_AT_SCID_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT^SCID\r\n");
	Uart_Send_start(pComm);
}
/*******************************
功能:AT+CSCA?  //读中心号码
返回:+CSCA: "+8613800756500",145
AT+CSCA?
*******************************/
void AT_Send_CSCA_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CSCA?\r\n");
	Uart_Send_start(pComm);
}

void GPRS_AT_CPNS_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n");//设置命令
	Uart_Send_start(pComm);
}

/*******************************
功能:AT+GSN    //读取序列号
返回:357224021125195
AT+GSN
*******************************/
void AT_Send_GSN_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+GSN\r\n");
	Uart_Send_start(pComm);
}
/*******************************
功能:AT+CSQ    读取信号强度
返回:+CSQ: 21,99
AT+CSQ
*******************************/
void AT_Send_CSQ_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CSQ\r\n");
	Uart_Send_start(pComm);
}
/*******************************
功能:AT+CNMI=2,1,0,0,1 设置短信显示方式
<mode>2——数据线空闲时，直接通知TE；否则先将通知缓存起来，待数据线空闲时再行发送。 
<mt>1——接收的短消息储存到默认的内存位置，并且向TE发出通知（包括class 3）。通知的形式为： ＋CMTI：”SM”，＜index＞ 
<bm>0-
<ds>0-
<bfr>1-
返回:OK
AT+CNMI
*******************************/
void AT_Send_CNMI_SUB(COM_STRUCT *pComm)
{
//	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CNMI=2,1,0,0,1\r\n");//这个是原来的命令
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CNMI=2,1,0,0,1\r");
	Uart_Send_start(pComm);
}
/*******************************
功能:at+cmgf=1 设置短信模式
返回:OK
*******************************/
void AT_Send_cmgf_SUB(COM_STRUCT *pComm)
{
	uint8 z=0;

	z = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CMGF=");
	pComm->TXdata[z++] = Gsmmodeselect;  //0-pdu,1-text模式
	pComm->TXdata[z++] =_CR;//回车
	pComm->TXdata[z++] =_LF;//换行
	pComm->Senddatanum = z;
	Uart_Send_start(pComm);
}
/*******************************
功能:AT+CSMP=81 设置为长短信方式
返回:OK
AT+CMEE=1
*******************************/
void GPRS_AT_CSMP_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CMEE=1\r\n");
	Uart_Send_start(pComm);
}
/*******************************
功能:AT&W 保存设置
返回:OK
*******************************/
void AT_Send_W_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"AT&W\r\n");
	Uart_Send_start(pComm);
}
/*******************************
功能:A/ 重新发送
返回:OK
*******************************/
void GPRS_A_W_SUB(COM_STRUCT *pComm)
{
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],"A/\r\n");
	Uart_Send_start(pComm);
}
/*******************************
功能:at+cmgr=? 读短信内容 
返回:OK
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
	pComm->TXdata[z++] = _CR;//回车
	pComm->TXdata[z++] = _LF;//换行
	pComm->Senddatanum = z;
	Uart_Send_start(pComm);
}
/*******************************
功能:at+cmgd=x 删除所有信息
返回:OK
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
	pComm->TXdata[z++] = _CR;//回车
	pComm->TXdata[z++] = _LF;//换行
	pComm->Senddatanum = z;
	Uart_Send_start(pComm);
}

//设置接口工作情况 1--表示触发LED模式
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
功能:对GPRS接受处理过后的数据进行处理
返回:
备注:每换一种工作模式必须重新启动电源管理。
能进来就说明模块是存在的.
设置的时候都有OK返回
*******************************/

void GSM_RX_Commun_SUB(COM_STRUCT *pCommKZQ,COM_STRUCT *pCommGSM)
{
	Nodatatratbuf=0;//可以考虑有没有AT
	if(CHKBITSET(mc52iworkflag,mc52gsmworkingf)) //GSM模式
	{
		RX_gsmwork_SUB(pCommKZQ,pCommGSM);
	}
	else          //还在设置状态
	{
		RX_gprs_set_SUB(pCommGSM);
	}
}


void mc52_work_sub(void)
{
	COM_STRUCT *pComm = &g_Comm1;
	if(CHKBITSET(pComm->funcflag,uartsenddata))
	{
		return;//在发数据的时候不能进入
	}
	if(CHKBITSET(mc52iworkflag,mc52gsmsendbaohuf))
	{//保护期间不能进入
		return;
	}
// 	if(Gprsworkingflag > 0)
// 	{
// 		return;//在发数据的时候不能进入
// 	}

	/***********GSM********************/
	if(CHKBITSET(Gsmstatureflag,gsmreaddataf))
	{//有新的短信要读
		CLRBIT(Gsmstatureflag,gsmreaddataf);
		AT_Send_cmgr_SUB(pComm,gsmduanxinnum); //读短信
		return;
	}
	/**************************************/
	if(CHKBITSET(Gsmstatureflag,gsmdelectduanxf))
	{//删除短信
		CLRBIT(Gsmstatureflag,gsmdelectduanxf);
		AT_Send_CMGD_SUB(pComm,gsmdelectduanxinnum); //读完后才能删除
		GsmOKfunbuf=GsmSendSCDXbuf;
		g_bCandeleteMsg = 1;//我们能定时的开始删除短信了
		return;
	}
	/***********GSM********************/
	//这里面的命令，正常时是不会发的，初始化或者设置时才使用，所以可以延时处理
	if(CHKBITSET(mc52igprsflag,mc52isenddelay))
	{//MC52I点火后，一直保持1
		Gprssendatdelaytime++;
		if(Gprssendatdelaytime > 200)
		{
			Gprssendatdelaytime=0;
			if(Mc52i_CMD_flag1.bit.atsendf)
			{
				Mc52i_CMD_flag1.bit.atsendf = 0;
				CLRBIT(MC52istaflag,mc52iNoSignal);	//重新开始，清除无信号标志
				GPRS_AT_SEND_SUB(pComm);  //AT测试命令 AT\r
				Gprsststuretypebuf=gprsATstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.attestflag)
			{
				Mc52i_CMD_flag1.bit.attestflag = 0;
				AT_Send_TEST_SUB(pComm);  //AT测试命令,调试用，用来看一些没用到的指令
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
				GPRS_AT_CLIP_SUB(pComm); //来电显示功能
				Gprsststuretypebuf=gprsCLIPstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.cnmiflag)
			{
				Mc52i_CMD_flag1.bit.cnmiflag = 0;
				AT_Send_CNMI_SUB(pComm); //新短信提示方式
				Gprsststuretypebuf=gprsCNMIstaturef;
				return;
			}	
			else if(Mc52i_CMD_flag1.bit.cmgfflag)
			{
				Mc52i_CMD_flag1.bit.cmgfflag = 0;
				AT_Send_cmgf_SUB(pComm); //设置TEXT短信模式
				Gprsststuretypebuf=gprsCMGFstaturef;
				return;
			}	
			else if(Mc52i_CMD_flag1.bit.csmpflag)
			{
				Mc52i_CMD_flag1.bit.csmpflag = 0;
				GPRS_AT_CSMP_SUB(pComm); //合成超长短信/用来做扩展用也可以
				Gprsststuretypebuf=gprsCSMPf;
				return;
			}	
			else if(Mc52i_CMD_flag1.bit.at_wflag)
			{
				Mc52i_CMD_flag1.bit.at_wflag = 0;
				AT_Send_W_SUB(pComm); //保存设置命令
				Gprsststuretypebuf=gprsATWstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.atgsnflag)
			{
				Mc52i_CMD_flag1.bit.atgsnflag = 0;
				AT_Send_GSN_SUB(pComm);  //读取序列号
				Gprsststuretypebuf=gprsGSNstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.atscidflag)
			{
				Mc52i_CMD_flag1.bit.atscidflag = 0;
				GPRS_AT_SCID_SUB(pComm); //读ID号码 
				Gprsststuretypebuf=gprsSCIDstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.atcscaflag)
			{
				Mc52i_CMD_flag1.bit.atcscaflag = 0;
				AT_Send_CSCA_SUB(pComm); //读中心号码
				Gprsststuretypebuf=gprsCSCAstaturef;
				return;
			}
			else if(Mc52i_CMD_flag1.bit.atSetSaveMode)
			{
 				Mc52i_CMD_flag1.bit.atSetSaveMode = 0;
  				GPRS_AT_CPNS_SUB(pComm); //设置存储模式
  				Gprsststuretypebuf=GSMSAVEMODE;
 				return;
			}
			else if(Mc52i_CMD_flag1.bit.atcsqflag)
			{
				Mc52i_CMD_flag1.bit.atcsqflag = 0;
				AT_Send_CSQ_SUB(pComm);  // 读取信号强度
				Gprsststuretypebuf = gprsCSQstaturef;
				return;
			}
			/********************************/
			//以下为用户命令
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
			// 主网连接设置
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
			// 备网连接设置
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
			// 主网连接
			else if(CHKBITSET(Mc52igprsipsetflag,closemgprsf))
			{
				CLRBIT(Mc52igprsipsetflag,closemgprsf);
// 				GPRS_AT_CLOSENET_SUB(pComm,mastermode);
// 				Gprsststuretypebuf=gprsmCLOSENETstaturef;
				return;
			}
			// 备网连接
			else if(CHKBITSET(Mc52igprssetflag,closergprsf))
			{
				CLRBIT(Mc52igprssetflag,closergprsf);
// 				GPRS_AT_CLOSENET_SUB(pComm,preparemode);
// 				Gprsststuretypebuf=gprsrCLOSENETstaturef;
				return;
			}
			// 打开连接
			else if(CHKBITSET(funcflag3,mc52icxlianjieopenf))
			{
				CLRBIT(funcflag3,mc52icxlianjieopenf);
// 				Gprsststuretypebuf = gprscxljopenf;
				return;
			}
		}
	}
}


