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
static uint8  gsmMsgNumbuf[11];	//收到短信的发短信号码
static uint8  gsmMsgtimebuf[7]; //从短信中提取的时间，年-月-日-时-分-秒

//GSM短信，纯ASCII最多160个字符，汉字最多70个。


const char *strMsgInfo[msgErrNum]={"参数已设置!","查询编号错误,需小于20!","操作超时!","参数错误!","号码错误!","号码权限不足!","复位成功!"};

//GSM发AT指令时以，\r结尾，发送用户数据时，以\r\n结尾。
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
		AddMsgGroup("你好!",groupno);
	else
		AddMsgGroup("Hello!",groupno);
}

//GSM模块复位应答
void GSM_Send_RST_sub(COM_STRUCT *pComm, uint8 groupno)
{   
	AddMsgGroup("复位 GSM OK \r\n",groupno);
}

//GSM模块版本号应答
void GSM_Send_VER_sub(COM_STRUCT *pComm, uint8 groupno)
{   
	uint8 i,len;

	i = SendStrToBuf(&sCurSMS[0],"设备=");
	len = i;
	i = SendStrToBuf(&sCurSMS[len],DEVICE_NAME);
	len += i;
	i = SendStrToBuf(&sCurSMS[len],",版本=");
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
		AddMsgGroup("信息错误!",groupno);
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
		WDR();//清看门狗
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
	telenumoldbuf=telenumnewbuf; //修改成功
	teletolnumbuf=telenumoldbuf+YongfuNumStart;
	eepbufflag.bit.telenumf = 1;//保存起来
	SETBIT(funcflag7,EEpwritedelay);
	for(i=0; i<telenumnewbuf; i++)
	{
		Gsmstature4flag.Word |= (1<<i);
	}

	return 1;
}




//超级号码
uint8 gsm_xgcjhm_sub(uint8 *pBuf,uint8 len)
{
	uint8 t=0,i=0,datanum=0;
	uint8 kk=0;
	
	for(t=0; t<len; t++)
	{
		WDR();//清看门狗
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

	gsmtelsuper = kk;	//超级号码
	eepbufflag.bit.telesuper = 1;
	
	return 1;
}

uint8 gsm_get_var_PDU_sub(uint8 *pBuf,uint8 len, uint8 num,uint16* pdes)
{
	uint8 t=0,datastart=0,datanum=0,onedatanum=0;//,bufdatanum;
	uint16 DATA;
	
	for(t=0; t<len; t++)
	{
		if((pBuf[t]=='=')||(pBuf[t]==','))  //找出数据的起始位置
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
				{//不在范围就提示错误
					return 0;
				} 
				onedatanum=0;
				pdes[datanum] = DATA; //变量提取后保存在数组里面
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
				{//最后一位,且有数据
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
					{//不在范围就提示错误
						return 0;
					} 
					onedatanum=0;
					pdes[datanum] = DATA; //变量提取后保存在数组里面
					datanum +=1;
				}
			}
		}
		if(datanum > num)
		{//参数多了
			return 0;
		}
	}
	
	if(datanum != num)
	{//参数多了或者少了
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
		return 0;	//参数错误

	if(temp[1] <= temp[0])
	{//严重报警温度需大于一般报警温度
		return 0;
	}
	if(temp[1] - temp[0] < 5)
	{//温度差需>=5度
		return 0;
	}

	for(t=0; t<len; t++)
	{
		WDR();//清看门狗
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
	
	gsmtelsuper = kk;	//超级号码
	eepbufflag.bit.telesuper = 1;

	return 1;
}

//设置有效性
uint8 gsm_szyx_sub(uint8 *pBuf,uint8 len)
{
	uint8 t=0,i=0,datanum=0;
	uint8 kk=0;
	uint16 temp[6];
	
	if(gsm_get_var_PDU_sub(pBuf,len,6,temp)==0)
		return 0;	//参数错误
	
	for(t=0; t<len; t++)
	{
		WDR();//清看门狗
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
	
	WDR();//清看门狗
	for(x=1; x<teletaolnumbuf; x++)  //第0不用，1-5为超级用户6为高级用户，7-9可设为高级用户，其余为普通用户
	{
		if( gsm_Msg_Number_test_sub(x,pGSMtelno) )
		{//号码测试OK
			return x;
		}
	}
	return 0;
}
/***************************
//找出数据的开始位置，返回数据长度和出现的位置
//在两个回车和换行之间的数据是我们要处理的内容
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
			{//第一个回车换行
				pos = x+2;
			} 
			else
			{//第二个回车换行
				*len = pos;	// 将第一次出现的位置带回去
				return (x-pos);
			}
		}
	}
	return 0;
}

/***************************
+CMGR: "REC UNREAD","+8613926988790",,"10/03/17,14:32:16+32"
//找出时间的开始位置
****************************/
uint8 gsm_get_Msgtime_sub(uint8 *pRecvbuf,uint8 len,uint8 *timebuf)
{
	uint8 x=0;
	
	for(x=0; x<len; x++)
	{
		if((pRecvbuf[x]==',')&&(pRecvbuf[x+1]==','))
		{
			timebuf[0] = (pRecvbuf[x+3]-0x30)*10+(pRecvbuf[x+4]-0x30); //年
			timebuf[1] = (pRecvbuf[x+6]-0x30)*10+(pRecvbuf[x+7]-0x30); //月
			timebuf[2] = (pRecvbuf[x+9]-0x30)*10+(pRecvbuf[x+10]-0x30); //日
			timebuf[3] = (pRecvbuf[x+12]-0x30)*10+(pRecvbuf[x+13]-0x30); //时
			timebuf[4] = (pRecvbuf[x+15]-0x30)*10+(pRecvbuf[x+16]-0x30); //分
			timebuf[5] = (pRecvbuf[x+18]-0x30)*10+(pRecvbuf[x+19]-0x30); //秒
			return 1;
		}
	}
	return 0;
}
/**********************
telzhu:下发命令的所属于那个组的号码
telnumf;目前设置了几组号码，有可能是零，刚开始设置的时候
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
	l_pRXData[z] = '\0';//去掉最后一个，号
	AddMsgGroup(sCurSMS,groupno);
}

//短信处理
void SMS_Process(COM_STRUCT *pCommKZQ,COM_STRUCT *pCommGSM,uint8 *pBuf,uint8 len)
{
	uint8 t=0,w=0;
	
	for(t=0; t<len; t++)
	{
		// 读取GSM模块版本号
		if(CheckCmdStr(&pBuf[t],"版本"))
		{
			GSM_Send_VER_sub(pCommGSM,gsmMsgNumbufGroup);
			return; 
		}
		// GSM模块复位
		if(CheckCmdStr(&pBuf[t],"复位"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{//有权限的才可以
				GSM_Send_RST_sub(pCommGSM,gsmMsgNumbufGroup);
				reset();
			}
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"HELLO"))
		{
			GSM_Send_HELLO_sub(pCommGSM,0,gsmMsgNumbufGroup);			//发送分闸NG短信
			return;
		}
		if(CheckCmdStr(&pBuf[t],"你好"))
		{
			GSM_Send_HELLO_sub(pCommGSM,1,gsmMsgNumbufGroup);			//发送分闸NG短信
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
		/************查询电话号码***********/
		if(CheckCmdStr(&pBuf[t],"查询号码"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{
				GSM_Send_tel_queren_sub(pCommGSM,gsmMsgNumbufGroup,telenumoldbuf);
			}
			return; 
		}
		/************修改电话号码***********/
		if(CheckCmdStr(&pBuf[t],"修改号码="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{
				if(gsm_xghm_sub(pBuf,len)==0)
				{
					AddMsgGroup("修改号码失败!",gsmMsgNumbufGroup);
				}
				else
				{
					AddMsgGroup("修改号码成功!",gsmMsgNumbufGroup);
				}
			}
			else
			{//权限不足
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			}
			
			return; 
		}
		/************修改号码权限***********/
		if(CheckCmdStr(&pBuf[t],"修改号码权限="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{//超级号码才有权限
				if(gsm_xgcjhm_sub(pBuf,len)==0)
				{
					AddMsgGroup("修改号码权限失败!",gsmMsgNumbufGroup);
				}
				else
				{
					AddMsgGroup("修改号码权限成功!",gsmMsgNumbufGroup);
				}
			}
			else
			{//权限不足
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			}
			
			return; 
		}
		/************删除历史记录***返回删除OK********/
		if(CheckCmdStr(&pBuf[t],"删除所有记录"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{
				GSM_SCJL(gsmMsgNumbufGroup);
			}
			else
			{//权限不足
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			}
			return; 
		}
		
		if(CheckCmdStr(&pBuf[t],"查询状态"))
		{
			GSM_CXZT(gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"查询名称"))
		{
			GSM_CXMC(gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"暂停报警"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_ZTBJ(gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"项目名称="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGXMMC(pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"端口1名称="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(0,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"端口2名称="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(1,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"端口3名称="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(2,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"端口4名称="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(3,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"端口5名称="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(4,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"端口6名称="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_XGDKMC(5,pBuf,len,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		/************查询参数***********/
		if(CheckCmdStr(&pBuf[t],"查询参数"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_CXSZ(1,gsmMsgNumbufGroup);
			else
				GSM_CXSZ(0,gsmMsgNumbufGroup);

			return; 
		}
		if(CheckCmdStr(&pBuf[t],"查询有效"))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
				GSM_CXYX(1,gsmMsgNumbufGroup);
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}
		if(CheckCmdStr(&pBuf[t],"修改有效="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{
				if(gsm_szyx_sub(pBuf,len))
				{
					AddMsgGroup("有效设置成功!",gsmMsgNumbufGroup);
				}
				else
				{
					AddMsgGroup("有效设置失败!",gsmMsgNumbufGroup);
				}
			}
			else
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			return; 
		}

		/************修改定值***********/
		if(CheckCmdStr(&pBuf[t],"设置温度="))
		{
			if( (gsmMsgNumbufGroup<=YongfuNumStart) || (gsmtelsuper&(1<<gsmMsgNumbufGroup)) )
			{
				if( gsm_szwd_sub(pBuf,len)==0 )
				{
					AddMsgGroup("温度设置失败!",gsmMsgNumbufGroup);
				}
				else
				{
					AddMsgGroup("温度设置成功!",gsmMsgNumbufGroup);
				}
			}
			else
			{//权限不足
				AddMsgGroup(strMsgInfo[msgNRERR],gsmMsgNumbufGroup);
			}
			return; 
		}
	
		/************查询事件记录***********
		最大200条
		*************************************/
		if(CheckCmdStr(&pBuf[t],"查询记录="))
		{//查询指定记录
			if( len >= (t+12) )//保证为CXJL=1---CXJL=200
			{//不在范围内提示错误,<100
				AddMsgGroup(strMsgInfo[msgBHERR],gsmMsgNumbufGroup);
				return; 
			}
			w = len-(t+9);//判断查询编号
			if(w==0)
			{//后面没有跟数字,则默认为查询最新信息
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
					AddMsgGroup("最多支持200条记录，编号为0-199!",gsmMsgNumbufGroup);
					return;
				}
			}
			else
			{
				AddMsgGroup("最多支持200条记录!",gsmMsgNumbufGroup);
				return;
			}
			GSM_CXJL(Soesearchnumbuf,gsmMsgNumbufGroup);
			return; 
		}
		
		if(CheckCmdStr(&pBuf[t],"查询记录"))
		{//查询最新的一条记录
			Soesearchnumbuf = 0;
			GSM_CXJL(Soesearchnumbuf,gsmMsgNumbufGroup);
			return; 
		}
	}//for(t=0; t<len; t++)
}

/*****************
//对收到的数据进行处理和分析
**************/
// 指令 ：YF.FZ,YH.HZ,CXDZ,DZ=,DZQR,CXZT,FW,RST,DS,CXSJ,CXJL=,CXHM,XGHM=,SCSJJL,WAPN=,RAPN=,DZX=,DZCX,VER,
void gsm_data_con_sub(COM_STRUCT *pCommKZQ,COM_STRUCT *pCommGSM,uint8 *pBuf,uint8 len)
{
	//	uint16  l_nLoop;
	uint8 t=0;
	
	WDR();//清看门狗
	if((pBuf[0]==0x30)&&(pBuf[1]==0x30))
	{//连续两个0,错误.
		if(gsmMsgNumbufGroup != 0)
			GSM_Send_ERROR_sub(pCommGSM,gsmMsgNumbufGroup);
		return; 
	}
	
	if(gsmMsgNumbufGroup==0)
	{//此情况只在未设置号码时发生，这时只能是做设置号码用，其它功能不支持。
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
	{//短信处理
		SMS_Process(pCommKZQ,pCommGSM,pBuf,len);
	}
}

//读取短信的内容 +CMGR:
//+CMGR: "REC READ","+8613694121067",,"04/05/31,11:21:18+00"_CR_LFxxxxxx_CR_LF
//+CMGR: "REC UNREAD","+8613926988790",,"10/03/17,14:32:16+32"_CR_LFxxxxxx_CR_LF
//只接受"+8613616532300"格式,号码长度为11的号码，其它格式的号码不接受.
void CMGR_TXT_Decode(uint8 *l_pRxData, COM_STRUCT * pCommGSM, COM_STRUCT * pCommKZQ)
{
	uint8 s=0;
	uint8 q=0,i=0,len;
	uint8 *pMsg;
	
	pMsg = l_pRxData;
	for(q=0; q<40; q++)
	{
		//查找","的位置
		if( (pMsg[0]=='"') && (pMsg[1]==',') && (pMsg[2]=='"') )
		{
			pMsg += 3;
			if((pMsg[0]=='+')&&(pMsg[1]=='8')&&(pMsg[2]=='6'))
			{//去掉+86
				pMsg += 3;
				for(i=0; i<11; i++)
				{
					gsmMsgNumbuf[i] = *pMsg++;  //读取短信来的电话号码
					if((gsmMsgNumbuf[i]>0x39)||(gsmMsgNumbuf[i]<0x30)) 
					{
						return;
					} //要保证号码是11位数字的
				}
				break;
			}
		}
		pMsg++;
	}

	//获取短信的时间，为后面数据处理的时候做对时间用
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

	//如果号码不对，
	if(gsmMsgNumbuf[0]==0)
		return;
	/*********************/
	//判断收到信息的号码属于哪个组
	gsmMsgNumbufGroup = gsm_Number_Group_test_sub(gsmMsgNumbuf);
	
	if(gsmMsgNumbufGroup == 0 )
	{//号码不在设置里时
		if(telenumoldbuf==0)
		{//还未设置号码时,后面会只处理设置号码命令，其余命令不执行。
		}
		else
		{//非授权号码，则不处理
			return;
		}
	}
	
	/***********可以对数据的读取了**********/
	pMsg += 22;
	len = pMsg - l_pRxData;
	len = pCommGSM->RXIndex - len;
	
	//找出数据的开始位置len,返回数据的数据长度q
	q = gsm_get_datastart_test_sub(pMsg,&len);
	pMsg += len;
	gsm_data_con_sub(pCommKZQ,pCommGSM,pMsg,q);   //对收到的数据进行处理和分析
	SETBIT(funcflag5,mc52havetrancedataled);//有数据流通指示LED
	netlestimebuf = 0;	// 指示灯时间清0
}

void GSM_Send_start_data_sub(COM_STRUCT *pComm)
{
	uint8 z=0,k=0;
	//	uint8 *pBuffer = &Mc52senddatatonetbuf[0];
	uint8 *pBuffer = &gsmSendData.buffer[0];
	if(Gsmmodeselect == MC52GSMPDU)
	{//PDZ模式下，发送格式不一样
		int nPduLength;
		char *pPdubuf = (char *)&pComm->TXdata[0];
		
		if(ucs2_flag)
		{//ucs2编码
			// 			nPduLength = GsmEncodePdu_UCS2((const char *)&sSCA[0], (const char *)&gsmtelnumbuf[telebaohunumbuf][0], (const char *)pBuffer, pPdubuf, mc52isenddatanumbuf);
			nPduLength = GsmEncodePdu_UCS2((const char *)&sSCA[0], (const char *)&gsmtelnumbuf[telebaohunumbuf][0], (const char *)pBuffer, pPdubuf, gsmSendData.len);
		}
		else
		{//7bit编码
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
		pComm->TXdata[z++] = 0x1A; //末尾以0x1A CTRL+Z结束短信
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

	WDR();//清看门狗

//	if(Gsmmodeselect==MC52GSMTEXT)
	{
		for(s = 0; s < pCommGSM->RXIndex; s++)
		{
			l_pRxData = &pCommGSM->RXdata[s];
			//收到新短消息+CMTI: "MT",*...*[CR]
			if(CheckCmdStr(l_pRxData, "+CMTI:"))	//有短信进来
			{
				l_tmpNumber = 0;
				l_bFind = 0;
				g_bCandeleteMsg = 0;//在这里不能删除短信
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
					SETBIT(Gsmstatureflag,gsmreaddataf);//只要有短信.,就需要读取,删除
					gsmdelectduanxinnum = gsmduanxinnum; //从这个数一直删到0。
				}
				//有可能结束的就是0,不是0x0d,那样这个就不回的

				//找回车,[CR]
				return;
			}
			/************************************/
			if(l_pRxData[0] == '>') //可以发信息了
			{//第一个字符是'>'
				GSM_Send_start_data_sub(pCommGSM);//可以发数据了。
				SETBIT(funcflag5,mc52havetrancedataled);//有数据流通指示LED
				netlestimebuf = 0;
				GsmOKfunbuf = GsmSendokbuf;
				return;
			}
			/************* +CMGS: 120  OK***发送成功标志*****************/
			if(CheckCmdStr(l_pRxData, "+CMGS:"))
			{
				CLRBIT(mc52iworkflag,mc52gsmsendbaohuf);//保护指令或数据
				CLRBIT(mc52iworkflag,gsmchongxingdata); //重新发送

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
			// 有电话进来 RING
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
				SETBIT(funcflag5,mc52havetrancedataled);//有数据流通指示LED
				netlestimebuf = 0;
				return;
			}
			/********************************/
			//读取短信的内容 +CMGR:
			//+CMGR: "REC READ","+8613694121067",,"04/05/31,11:21:18+00"_CR_LFxxxxxx_CR_LF
			//+CMGR: "REC UNREAD","+8613926988790",,"10/03/17,14:32:16+32"_CR_LFxxxxxx_CR_LF
			//只接受"+8613616532300"格式,号码长度为11的号码，其它格式的号码不接受.
			//+CMGR: 1,,111 
			//　　0891683108501505F0240BA13190433074F30008501157 
			if(CheckCmdStr(l_pRxData,"+CMGR:"))  //读取短信
			{
				SETBIT(Gsmstatureflag,gsmdelectduanxf); //删除短信
				gsmMsgNumbuf[0] = 0;	// 防止上一次正确号码后，这次的号码非法时的处理。
				//根据收到的短信格式，调用相应的解码程序
//				if(l_pRxData[7]=='"')
				if(Gsmmodeselect == MC52GSMPDU)
				{//PDU解码
					CMGR_PDU_Decode(l_pRxData,pCommGSM,pCommKZQ);
				}
				else
				{//TEXT解码
					CMGR_TXT_Decode(l_pRxData,pCommGSM,pCommKZQ);
				}
				return;
			}
			/***********************************/
			// CMGD=OK //删除短信成功
			if(CheckCmdStr(l_pRxData,"CMGD"))  //删除短信成功
			{
				for(kkk = s + 4; kkk < pCommGSM->RXIndex; kkk++)
				{
					if(CheckCmdStr(&pCommGSM->RXdata[s],"OK"))  //删除短信成功
					{
						if(gsmdelectduanxinnum > 1)
						{
							gsmdelectduanxinnum--;
							SETBIT(Gsmstatureflag,gsmdelectduanxf);//继续删除短信
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
						SETBIT(Gsmstatureflag,gsmdelectduanxf);//继续删除短信
					}
				}
				else if(GsmOKfunbuf==GsmSendokbuf)
				{//发送成功
					GsmOKfunbuf = 0;
					CLRBIT(mc52iworkflag,mc52gsmsendbaohuf);//保护指令或数据
					CLRBIT(mc52iworkflag,gsmchongxingdata); //重新发送

					Gsmstature4flag.bit.gsmbaohuworkingf = 0;
					GsmEORRnumbuf = 0;
					GsmEORRTOnumbuf = 0;
				}
				NOP();
				return;
			}
			/***************+CMS ERROR: xxx ************************/
			if(CheckCmdStr(l_pRxData, "ERROR"))  //返回错误
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
				if(GsmEorrefunbuf==28)  //号码设置错误，无效号码
				{
					GsmOKfunbuf=0;
					CLRBIT(mc52iworkflag,mc52gsmsendbaohuf);//保护指令或数据
					CLRBIT(mc52iworkflag,gsmchongxingdata); //重新发送

					Gsmstature4flag.bit.gsmbaohuworkingf = 0;
				}
				else if(GsmEorrefunbuf==38)  //网络出秩序
				{
					funcflag5 = 0;
					SETBIT(funcflag3,mc52RESTf);
				}
				else if(GsmEorrefunbuf==42)  //挤塞
				{
					Gsmstature4flag.bit.gsmsenddusef = 1;
				}
				else if(GsmEorrefunbuf==208)  //做SIM短信存储满
				{
					gsmdelectduanxinnum=100;
					SETBIT(Gsmstatureflag,gsmdelectduanxf);//继续删除短信
				}
				else if(GsmEorrefunbuf==310)  //SIM卡未插入
				{
					statusflag0.bit.noSim = 1;
				}
				else if(GsmEorrefunbuf==500)  //无线没网络 
				{
					GsmEORRnumbuf++;
					if(GsmEORRnumbuf>2)
					{
						mc52iworkflag = 0;
						funcflag5 = 0;
						Mc52i_CMD_flag1.bit.atsendf = 1;	//下一条指令发送AT
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
//						SETBIT(mc52iworkflag,mc52gsmsendbaohuf);//保护指令或数据
					}
				}
				else if(GsmEorrefunbuf==512)//在发送短消息的同时它收到了新的短消息，发送短消息被拒绝
				{
					GPRS_A_W_SUB(pCommGSM);
				}
				else if(GsmEorrefunbuf==513)  //失去了无线链路连接； 
				{
					;
				}
				else if(GsmEorrefunbuf==514) //您设置的短消息中心的号码是错误的或短消息将要送达的目的号码是错误的
				{
					;
				}
				else if(GsmEorrefunbuf==515)//您必须等到初始化完成或指令执行完毕。
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
//发送给短信模块时，发送缓冲区组号
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

//找UCS2中的汉字
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
设置目的手机号码
at+cmgs="13926988790"
//tel_group_no号码组编号0-10
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
		{//不是重发信息才处理
//			Mc52senddatatonetbuf[mc52isenddatanumbuf] = '\0';
		//if(Find_Str_Hz(pBuffer))
// 			if(Find_Str_UCS2Hz(pBuffer,mc52isenddatanumbuf))
			if(Find_Str_UCS2Hz(pBuffer,gsmSendData.len))
			{//有汉字,UCS2编码
				ucs2_flag = 1;
//				mc52isenddatanumbuf = Str2UCS2((const char *)&Mc52senddatatonetbuf[0],pPdubuf);
			}
			else
			{//全部为字符,7位编码
				ucs2_flag = 0;
//				mc52isenddatanumbuf = gsmEncode7bit((const char *)&Mc52senddatatonetbuf[0],pPdubuf,mc52isenddatanumbuf);
			}
			//把内容转换到Mc52senddatatonetbuf中
//			memcpy((char *)&Mc52senddatatonetbuf[0], (const char *)pPdubuf, mc52isenddatanumbuf);
		}
// 		nPduLength = GsmEncodePdu_UCS2((const char *)&sSCA[0], (const char *)&gsmtelnumbuf[tel_group_no][0], (const char *)pBuffer, pPdubuf, mc52isenddatanumbuf);
		nPduLength = GsmEncodePdu_UCS2((const char *)&sSCA[0], (const char *)&gsmtelnumbuf[tel_group_no][0], (const char *)pBuffer, pPdubuf, gsmSendData.len);

		GsmString2Bytes(pPdubuf, &nSmscLength, 2);    // 取PDU串中的SMSC信息长度
		nSmscLength++;        							// 加上长度字节本身
		z = sprintf(pPdubuf, "AT+CMGS=%d\r", nPduLength / 2 - nSmscLength);
	}
	else
	{
		z = SendStrToBuf((char*)&pComm->TXdata[0],"AT+CMGS=\"");
		l_pTelNumber = &gsmtelnumbuf[tel_group_no][0];
		for(i=0; i<11; i++)
			pComm->TXdata[z++]=l_pTelNumber[i];
		pComm->TXdata[z++]='"';
		pComm->TXdata[z++]=_CR;//回车
	}
	pComm->Senddatanum=z;
	Uart_Send_start(pComm);
}

/***********************
开始发送数据
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
					SETBIT(mc52iworkflag,mc52gsmsendbaohuf);//保护指令或数据
				}
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////
//	PDU模式的各个GSM函数
//
////////////////////////////////////////////////////////////////////////

static char sSmst[141];
//从GSM短信收到的命令
void gsm_data_con_PDU_sub(COM_STRUCT *pCommKZQ,COM_STRUCT *pCommGSM,uint8 *pBuf,uint8 len)
{
	uint8 t;
	WDR();//清看门狗
	if((pBuf[0]==0x30)&&(pBuf[1]==0x30))
	{//连续两个0,错误.
		if(gsmMsgNumbufGroup != 0)
			GSM_Send_ERROR_sub(pCommGSM,gsmMsgNumbufGroup);
		return; 
	}
	
	//短信处理
	if(gsmMsgNumbufGroup==0)
	{//此情况只在未设置号码时发生，这时只能是做设置号码用，其它功能不支持。
		for(t=0; t<len; t++)
		{
			if(CheckCmdStr(&pBuf[t],"XGHM="))
			{
				pCommGSM->TXTELBufNUM[pCommGSM->TXBUFNumber] = gsmMsgNumbufGroup;//返回给电话的号码
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

//读取短信的内容 +CMGR:
//只接受"+8613616532300"格式,号码长度为11的号码，其它格式的号码不接受.
//AT+CMGR=1\r\r\n
//+CMGR: 0,,24\r\n
//08 91 683108705505F0 24 0D 91 683115908076F2 0008 71 11 01 41 10 30 23 04 542F 52A8 \r\n\r\n 'O' 'K' \r\n
//08 91 683108705505F0 04 0D 91 683115908076F2 0000 81 11 20 91 24 95 23 05 C82293F904 0D 0A 0D 0A 4F 4B 0D 0A 00\r\n
void CMGR_PDU_Decode(uint8 *l_pRxData, COM_STRUCT * pCommGSM, COM_STRUCT * pCommKZQ)
{
	uint8 q=0;
	char * pstr;

	pstr = strstr((const char*)l_pRxData, "\r\n");	//查找"\r\n"，并返回出现的首地址
	
	q = GsmDecodePdu(pstr+2,(char *)&gsmMsgNumbuf[0],(char *)&gsmMsgtimebuf[0],sCurSMS);	//返回信息长度
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
	
	//对时
	{
		g_CLK.YEAR = gsmMsgtimebuf[0];
		g_CLK.MON = gsmMsgtimebuf[1];
		g_CLK.DAY = gsmMsgtimebuf[2];
		g_CLK.HOUR = gsmMsgtimebuf[3];
		g_CLK.MIN = gsmMsgtimebuf[4];
		g_CLK.SEC = gsmMsgtimebuf[5];
		g_CLK.wDAY = GetWeek(g_CLK.YEAR, g_CLK.MON,g_CLK.DAY);
	}

	//如果号码不对，
	if(gsmMsgNumbuf[0]==0)
		return;
	/*********************/
	//判断收到信息的号码属于哪个组
	gsmMsgNumbufGroup = gsm_Number_Group_test_sub(gsmMsgNumbuf);
	
	if(gsmMsgNumbufGroup == 0 )
	{//号码不在设置里时
		if(telenumoldbuf==0)
		{//还未设置号码时,后面会只处理设置号码命令，其余命令不执行。
		}
		else
		{//非授权号码，则不处理
			return;
		}
	}
	
	gsm_data_con_PDU_sub(pCommKZQ,pCommGSM,(uint8 *)&sCurSMS[0],q);   //对收到的数据进行处理和分析
	SETBIT(funcflag5,mc52havetrancedataled);//有数据流通指示LED
	netlestimebuf = 0;	// 指示灯时间清0
}


//返回带汉字的时候的第50个字符位置，缓冲区不够长，超过50个就超过缓冲区了
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
		{//满50个，则返回第70个字符的位置
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

	//没有设置号码，不发信息
	if(telenumoldbuf==0)
		return;
	pComm->TXTELBufNUM[pComm->TXBUFNumber] = YongfuNumStart;//返回给电话的号码权限组号
	if(Gsmmodeselect == MC52GSMPDU)
		AddMsgGroup("已上线!",YongfuNumStart);
	else
		AddMsgGroup("On line!",YongfuNumStart);
}

//删除短信
void DeleteOldMsg(COM_STRUCT *pComm,uint8 msgNo)
{
	char  uuuu[64];
	sprintf(uuuu, "AT+CMGD=%d\r\n", msgNo);
	pComm->Senddatanum = SendStrToBuf((char*)&pComm->TXdata[0],uuuu);
	Uart_Send_start(pComm);
}
