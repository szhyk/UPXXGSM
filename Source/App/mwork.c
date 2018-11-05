
#include <stdio.h> 
#include <math.h>
#include <string.h>
#include "mwork.h"
#include "work.h"
#include "gprs.h"
#include "gsm.h"
#include "iic.h"
#include "wxcw.h"

/*
MC55i的启动流程及AT指令的顺序

	1、  启动GPRS电源：PWC   （PWON—PWOFF）

	2、  启动EMERGOFF管脚：要给他提供一个大于3.2秒的低电平信号（PDON—PDOFF）

	3、  判断模块是否正常启动？当VDD为低电平时，模块正常（—MC55—IGTOFF）

	4、  启动IGT管脚：要给他提供一个大于100毫秒的低电平信号（IGTON—IGTOFF）

	5、  判断模块是否正常启动？当VDD为高电平时，模块正常启动：
	发AT指令：
	1）  AT  （确认串口是否正常，模块串口初始化为自适应的）
	2）  AT+IPR=57600
	3）  AT^SSYNC=1
	4）  AT&C2

	6、  GETCSQ ： 
	1）AT+CSQ  查询网络信号强度 
	完成上面的初始化操作之后，需要使用AT^SICS指令来设置模块连接Internet的脚本
	AT^SICS指令： 
	1）AT^SICS=0,CONTYPE,GPRS0    0表示脚本的编号，最多有6个，分别为0~5
	conType表示后面GPRS0的连接方式是GPRS
	注：除了GPRS0外，还可以选择CSD连接方式
	2）AT^SICS=0,APN        设置APN
	3）AT^SICS=0,USER       设置APN用户名
	4）AT^SICS=0,PASSWD     设置APN密码

	完成Internet连接脚本设置后，需要设置Internet服务脚本的相关设置
	5）AT^SISS=0,SRVTYPE,SOCKET     第0 个服务平台，服务类型为socket
	6）AT^SISS=0,CONID,0               第0 个服务平台，使用的连接平台为0

	7）设置IP及端口号： AT^SISS=0,address,socktcp  等等
	8）AT^SISC=0   关闭GPRS0 连接  

	7、LINKING ：AT^SISO=0    启动GPRS0连接

	8、LINKEND ：^SISW: 0, 1  接收网络响应，收到则已经发送成功  
	登录成功：Mc55WorkStatus = MC55_LINKOK
	ChannelStatus = CHANNEL_STATUS_OK
*/
extern void init_app(void);
extern uint8 Mc52i_rst_count;	// 复位次数
extern uint32 Mc52i_rst_time;	// 复位时间
void Mc52i_init()
{
	init_app();
	CLRBIT(mc52iworkflag,mc52gsmworkingf);
	CLRBIT(mc52iworkflag,mc52gprsworkingf);
	statusflag0.Word = 0;
	Mc52i_CMD_flag1.Word = 0;
	eepbufflag.Word = 0;
	statusflag0.bit.mc52iintf = 1;	//设定初始化标志 1
	Mc52i_rst_time = 0;
	MC52IPOWEROFF;	// 关闭电源
	WDR();
    SleepSec(5);
    
    MC52IPOWERON;	//MC52上电
    
    SleepSec(5);
    WDR();
    
    //开机,脉冲
    MC52IRESTON;
    SleepSec(5);
    MC52IRESTOFF;
    
    //等待超过5S
    SleepSec(10);
	Mc52i_CMD_flag1.bit.atsendf = 1;	//下一条指令发送AT
	
}

/****************************************************
*****************************************************/
void Get_int_data_sub(void)
{
	uint16 i;
	if(EEPROM_Read(0) != 0x3A)
	{//未初始化过
		EEPROM_Write(0x3A,0);
		for(i=1; i<700; i++)
			EEPROM_Write(0,i);
	}
    eeprom_read_sub();
	//读取用户设定的号码
	for(i=YongfuNumStart; i<teletaolnumbuf; i++)
		EEPROM_tel_Read(i);

//	Gsmmodeselect = MC52GSMTEXT; //选择为GSM的TEXT文本模式
	Gsmmodeselect = MC52GSMPDU; //选择为GSM的TEXT文本模式 hyk
	SendStrToBuf((char*)&gsmtelnumbuf[0][0],"00000000000");// 
	SendStrToBuf((char*)&gsmtelnumbuf[1][0],"13510908672");//超级1号码 hyk
	SendStrToBuf((char*)&gsmtelnumbuf[2][0],"15820994887");//超级2号码 hcf
	SendStrToBuf((char*)&gsmtelnumbuf[3][0],"18899741276");//超级3号码 test
	SendStrToBuf((char*)&gsmtelnumbuf[4][0],"13923789584");//超级4号码 兰总
	SendStrToBuf((char*)&gsmtelnumbuf[5][0],"13423484035");//超级5号码 lmj

}

//将更改的号码，一个一个写入EEPROM中
void tele_write_sub(void)
{
	uint8 i;
	if(CHKBITCLR(funcflag7,EEpwritedelay))
	{
		for(i=0; i<teleUserNumMax; i++)
		{
			if(Gsmstature4flag.Word & (1<<i))
			{
				Gsmstature4flag.Word &= ~(1<<i);
				EEPROM_tel_write(YongfuNumStart+i);
				return;
			}
		}
	}
}


void EEPROM_tel_Read (uint8 telnum)
{
    uint8 i=0,j=0;
	for(i=0; i<11; i++)
	{
		j = EEPROM_Read(telenumstartaddr+(telnum-YongfuNumStart)*11+i);
		
		if(j==0xff)
		{//未曾初始化，则设置号码为'0'
			gsmtelnumbuf[telnum][i] = 0;
			Gsmstature4flag.Word |= (1<<(telnum-YongfuNumStart));
		}
		else
		{
			gsmtelnumbuf[telnum][i] = j;
		}
	}
}

//写号码
void EEPROM_tel_write (uint8 telnum)
{
    uint8 i=0;
	for(i=0;i<11;i++)
	{
		WDR();//清看门狗
		EEPROM_Write(gsmtelnumbuf[telnum][i],telenumstartaddr+(telnum-YongfuNumStart)*11+i) ;
	}
	
}

