
#include <math.h>
#include "work.h"
#include "gprs.h"
#include "gsm.h"
#include "iic.h"
#include "comm.h"


extern void Mc52i_init(void);
static uint16 senddatabaohutime;
static uint16 senddataGSMbaohutime;
static uint16 eepwritedelaybuf;
uint32 Nodatatratbuf;	//用来做没有数据通讯计时用。
static uint32 mokbaohutimebuf;
static uint16 Baudsetbuf;
static uint32 GPRS_cyreaddatatime;
static uint32 GPRS_cydatatimebuf;
uint16 ATHtimebuf;
uint16 netlestimebuf;
uint16 datalestimebuf;
uint16 GPRS_POWEROFFTIME;
uint16 GPRS_IGTTIME;
uint8 g_stoprun;



void sys_timeonems_sub(void)
{
	COM_STRUCT *pComm = &g_Comm1;

	if(statusflag0.bit.mc52iintf)
	{//初始化标志
		eepbufflag.bit.baohuworkf = 1;	// 初始化时设定保护标志
		mokbaohutimebuf = 0;
	}
	if(eepbufflag.bit.baohuworkf)
	{//保护标志
		mokbaohutimebuf++;
		if(mokbaohutimebuf > 600000) //600秒
		{
			mokbaohutimebuf = 0;
			eepbufflag.bit.baohuworkf = 0;
			funcflag5 = 0;
			SETBIT(funcflag3,mc52RESTf);
			if(eepbufflag.bit.Diwunworkingf2 == 0)
			{
				eepbufflag.bit.Diwunworkingf1 = 0;
			}
		}
	}
	else
		mokbaohutimebuf = 0;

	if(CHKBITSET(funcflag7,EEpwritedelay))
	{//写eeprom标志
		eepwritedelaybuf++;
		if(eepwritedelaybuf > 5000)
		{
			eepwritedelaybuf = 0;
			CLRBIT(funcflag7,EEpwritedelay);
		}
	}
	else
		eepwritedelaybuf = 0;

	if(CHKBITSET(funcflag3,mc52RESTf))
	{//重启标志
		MC52reSETTIMEBUF++;
		if(MC52reSETTIMEBUF > 2000)
		{//2秒后清除正常标志，设置初始化标志
			MC52reSETTIMEBUF=0;
			statusflag0.Word = 0;	//hyk
			statusflag0.bit.mc52iintf = 1;
			eepbufflag.bit.GsmFuweisendf = 0;
			CLRBIT(funcflag3,mc52RESTf);
			CLRBIT(mc52igprsflag,mc52igprsnetoking);//后台主动断线，后面就要不停的去连接
			CLRBIT(mc52igprsflag,mc52igprsworking);
			CLRBIT(mc52iworkflag,mc52gsmworkingf);
			CLRBIT(mc52iworkflag,mc52gprsworkingf);
			MC52IPOWEROFF;	 
		}
	}
	else
		MC52reSETTIMEBUF = 0;

	if(CHKBITSET(Gsmstatureflag,gsmguajif))
	{//挂机命令 计时
		ATHtimebuf++;
		if(ATHtimebuf > 0)
		{
			CLRBIT(Gsmstatureflag,gsmguajif);
			GPRS_AT_ATH_SUB(pComm);
		}
	}
	else
		ATHtimebuf = 0;
	
	net_led_work_sub();	// 指示灯处理

	Nodatatratbuf++;
	if(Nodatatratbuf > 360000000ul)
	{
		Nodatatratbuf = 0;
		Mc52i_init();	//重新初始化
	}
	if(statusflag0.bit.mc52iintf)
	{
		GPRS_POWEROFFTIME++;
		if(GPRS_POWEROFFTIME > GPRS_POWEROFFTIME_SET)
		{
			GPRS_POWEROFFTIME = 0;
			mc52igprsflag = 0;
			mc52iworkflag = 0;
			funcflag5 = 0;
			Nodatatratbuf = 0;
			funcflag7 = 0;
			Mc52masterNetf = 0;
			statusflag0.Word = 0;
			CLRBIT(mc52iworkflag,mc52gsmworkingf);
			CLRBIT(mc52iworkflag,mc52gprsworkingf);
			MC52IPOWERON;			// 打开电源，低电平要足够长，大于3.2S
			statusflag0.bit.mc52iigtf = 1;	//初始化标志2 igt 上电处理
			MC52IRESTON;			// IGT先高电平
		}
	}
	if(statusflag0.bit.mc52iigtf)
	{
		GPRS_IGTTIME++;    //点火保持低电平时间
		if(GPRS_IGTTIME > GPRS_DIANHUO_TIME_SET)
		{
			GPRS_IGTTIME = 0;
			MC52IRESTOFF;		//IGT至少要低电平100ms,恢复高电平
			statusflag0.bit.mc52iigtf = 0;
			SETBIT(mc52igprsflag,mc52isenddelay);	// MC52I 点火后第一个标志
			//此时启动完成，下面判断是否正常启动
		}
	}
	if(CHKBITSET(funcflag5,mc52iqhnjtdelayf))
	{
		CLRBIT(funcflag5,mc52iqhnjtdelayf);
		if(CHKBITSET(Mc52masterNetf,Mc52masterconnect))
		{
			CLRBIT(Mc52masterNetf,Mc52masterconnect);
			SETBIT(Mc52prepareNetf,Mc52prepareconnect);
		}
		else
		{
			CLRBIT(Mc52prepareNetf,Mc52prepareconnect);
			SETBIT(Mc52masterNetf,Mc52masterconnect);
		}
		SETBIT(funcflag5,mc52icxnjtdelayf); //重新连接
	}
	if(CHKBITSET(funcflag5,mc52icxnjtdelayf))
	{
		CLRBIT(funcflag5,mc52icxnjtdelayf); //重新连接
		CLRBIT(mc52igprsflag,mc52igprsnetoking);//后台主动断线，后面就要不停的去连接
		CLRBIT(mc52igprsflag,mc52igprsworking);
		Gprsststuretypebuf = 0; //结束对命令的发送
		SETBIT(Mc52igprsipsetflag,closemgprsf);//主备一起关闭后再打开
	}
	if(CHKBITSET(funcflag7,gprsnettotestf)) //开始做记时处理，避免太长时间没反应就挂了
	{
		Gprstesttimedelaybuf++;            //这主要考虑的是为了避免设置错误后连不上网
		if(Gprstesttimedelaybuf > 1000)
		{
			Gprstesttimedelaybuf = 0;
			Gprstesttime2delaybuf++;
			if(Gprstesttime2delaybuf > 120)
			{
				Gprstesttime2delaybuf = 0;
				CLRBIT(funcflag7,gprsnettotestf); //开始做记时处理，避免太长时间没反应就挂了
				   SETBIT(funcflag5,mc52iqhnjtdelayf);//只能两个地方切换
				Gprsbaohutimebuf++;
				if(Gprsbaohutimebuf > 720)   //24小时=1天
				{
					Gprsbaohutimebuf = 0;
					SETBIT(funcflag3,mc52RESTf);
				}
			}
		}
	}
	else
	{
		Gprstesttimedelaybuf = 0;
		Gprstesttime2delaybuf = 0;
	}
	/*************************空暇时候周期性去读数据************/
	if(CHKBITSET(mc52igprsflag,mc52igprsnetoking))
	{
		Gprstesttimedelaybuf = 0;
		Gprstesttime2delaybuf = 0;
		Gprsbaohutimebuf = 0;
		GPRS_cyreaddatatime++; //周期去读数据
		if(GPRS_cyreaddatatime > 10000) //10秒钟测试
		{
			GPRS_cyreaddatatime = 0;
			SETBIT(MC52istaflag,readmc52idataf);//周期读数据，怕漏数据。
		}
		GPRS_cydatatimebuf++; //在网络连接好后还要看3分种内有没有数据流通
		if(GPRS_cydatatimebuf > 300000ul) //300秒钟测试
		{
			GPRS_cydatatimebuf = 0;   //在网络连接好后还要看3分种内有没有数据流通
			SETBIT(funcflag5,mc52icxnjtdelayf); //重新连接
		}
	}
	else
	{
		GPRS_cyreaddatatime = 0;
		GPRS_cydatatimebuf = 0;
	}
/*	if(CHKBITSET(Gprsworkingflag,gprsSISRMf))
	{
		SISRtimebuf++;
		if(SISRtimebuf > 6000)
		{
			SISRtimebuf = 0;
			CLRBIT(Gprsworkingflag,gprsSISRMf);
		}
	}
	else
	{
		SISRtimebuf = 0;
	}
	if(CHKBITSET(Gprsworkingflag,gprsSISWMf))
	{
		SISWtimebuf++;
		if(SISWtimebuf > 6000)
		{
			SISWtimebuf = 0;
			CLRBIT(Gprsworkingflag,gprsSISWMf);
		}
	}
	else
	{
		SISWtimebuf = 0;
	}*/
	if(CHKBITSET(mc52iworkflag,mc52sendbaohuf))
	{
		senddatabaohutime++;
		if(senddatabaohutime > 10000)
		{
			senddatabaohutime = 0;
			CLRBIT(mc52iworkflag,mc52sendbaohuf);
		}
	}
	else
	{
		senddatabaohutime = 0;
	}
	if(CHKBITSET(mc52iworkflag,mc52gsmsendbaohuf))
	{
		senddataGSMbaohutime++;
		if(senddataGSMbaohutime > 10000)
		{
			senddataGSMbaohutime = 0;
			CLRBIT(mc52iworkflag,mc52gsmsendbaohuf);
			if(Gsmstature4flag.bit.gsmbaohuworkingf==0)
			{
				Gsmstature4flag.bit.gsmbaohuworkingf = 1;
				SETBIT(mc52iworkflag,gsmchongxingdata); //重新发送

				GSM_Send_num_sub(pComm,GsmEORRcfnumbuf);
				SETBIT(mc52iworkflag,mc52gsmsendbaohuf);//保护指令或数据
			}
			else
			{//两次发送无回应，则重新初始化网络
				Gsmstature4flag.bit.gsmbaohuworkingf = 0;
				CLRBIT(mc52iworkflag,gsmchongxingdata); //重新发送
				mc52iworkflag = 0;
				funcflag5 = 0;
				GsmEORRtimebuf=0;
				Mc52i_CMD_flag1.bit.atsendf = 1;	//下一条指令发送AT
				Mc52i_CMD_flag1.bit.first_connect = 0;
			}
		}
	}
	else
	{
		senddataGSMbaohutime = 0;
	}
}

//指示灯处理
void net_led_work_sub(void)
{
	if(CHKBITSET(mc52igprsflag,mc52igprsnetoking)||CHKBITSET(mc52iworkflag,mc52gsmworkingf))
	{
		DATALEDON;	//左1绿灯点亮,注册
		if(CHKBITSET(funcflag5,mc52havetrancedataled))
		{
			netlestimebuf++;
			datalestimebuf=0;
			NETLEDON;	//左2绿灯点亮
			if(netlestimebuf>150)
			{
				netlestimebuf=0;
				CLRBIT(funcflag5,mc52havetrancedataled);
				CLRBIT(mc52igprsflag,mc52ileddataf);
			}
			return;
		}
		/*********************/
		datalestimebuf++;
		if(CHKBITCLR(mc52igprsflag,mc52ileddataf))
		{
			if(datalestimebuf>2000)
			{
				datalestimebuf=0;
				SETBIT(mc52igprsflag,mc52ileddataf);
			}
			if(Gsmstature4flag.bit.gsmsenddusef==0)
			{
				NETLEDOFF;
			}
			datalestimebuf=0;
		}
		else
		{
			if(datalestimebuf>100)
			{
				datalestimebuf=0;
				CLRBIT(mc52igprsflag,mc52ileddataf);
			}
			NETLEDON;
		}
		return;
		/************************/
	}
	else
	{
		netlestimebuf=0;
		datalestimebuf=0;
		NETLEDOFF;
		DATALEDOFF;
	}
}

/********************/
void eeprom_write_sub(void)
{
	if(eepbufflag.bit.baudeepf)
	{
		eepbufflag.bit.baudeepf = 0;
		EEPROM_Write(hibyte(Baudsetbuf),baudaddh);
		EEPROM_Write(lobyte(Baudsetbuf),baudaddl);
		return;
	}
	
	if(eepbufflag.bit.Modeleepf)
	{
		eepbufflag.bit.Modeleepf = 0;
		EEPROM_Write(MC52iworkstatusbuf,mc52workmodeladd);
		
		return;
	}
	if(eepbufflag.bit.telenumf)
	{
		eepbufflag.bit.telenumf = 0;
		EEPROM_Write(telenumnewbuf,telenumdataadd);
		return;
	}
	if(eepbufflag.bit.telesuper)
	{
		eepbufflag.bit.telesuper = 0;
		EEPROM_Write(gsmtelsuper,telenumsuperadd);
		return;
	}
}

void eeprom_read_sub(void)
{
	g_stoprun = EEPROM_Read(STOP_RUN_ADDR);
	//读取波特率
	Baudsetbuf = EEPROM_Read(baudaddh);
	Baudsetbuf <<= 8;
	Baudsetbuf += EEPROM_Read(baudaddl);

	if(Baudsetbuf>65530)
	{
		Baudsetbuf=oldbauddata;
		eepbufflag.bit.baudeepf = 1;
	}

	//读取工作模式0--GSM,1--GPRS
	MC52iworkstatusbuf=EEPROM_Read(mc52workmodeladd);
	if(MC52iworkstatusbuf > 250)
	{
		MC52iworkstatusbuf=oldmc52model;
		eepbufflag.bit.Modeleepf = 1;
	}
	//读取号码组个数
	telenumoldbuf = EEPROM_Read(telenumdataadd);	// 用户号码个数
	teletolnumbuf = telenumoldbuf+YongfuNumStart;	// 用户号码个数+超级号码个数
	if(telenumoldbuf > 20)
	{//也就是说用户号码最多14个
		telenumoldbuf = 0; //共有存几组号码
		teletolnumbuf =  YongfuNumStart;
		eepbufflag.bit.telenumf = 1;
	}
	//读取号码权限
	gsmtelsuper = EEPROM_Read(telenumsuperadd);

}

