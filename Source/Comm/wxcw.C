/********************************************************
File name: wxcw.c
Function: communication
Editor: hyk
Date: 08/09/2018
Version: V10
Description:
*********************************************************/
#include "define.h"
#include "iic.h"
#include "gsm.h"
#include <string.h>
#include <stdio.h>
#include "timer.h"
#include "comm.h"
#include "modbusm.h"
#include "work.h"

#include "gprs.h"
#include "ucs2.h"
#include "wxcw.h"


char sssbuf[100];
char sssbuf1[100];
#define TX_ERR_COUNT	180	//通讯故障次数判断，需小于256.默认为180S
void SleepMSec(uint8 ms);
extern uint32 dTCounter;
//通信配置数据类型
enum
{
	COMM_AI_TYPE = 0,
	COMM_DI_TYPE,
	COMM_DO_TYPE,
	COMM_CI_TYPE,
	COMM_PTTYPE_NUM
};

enum{
	STA_OFFLINE=0,		//离线
	STA_FRESH_TEMP,		//刷新温度
	STA_UNFRESH_TEMP1,	//不刷新温度
	STA_UNFRESH_TEMP2,	//不刷新温度
	STA_FAIL1,			//测量失败
	STA_FAIL2			//测量失败
};

enum{
	ADDR_TEMP=0,	//温度地址
	ADDR_REV1,
	ADDR_REV2,
	ADDR_DB,		//db地址
	ADDR_REV3,
	ADDR_STA		//状态地址
};

enum{
	CMD_STATUS = 0,	//状态
	CMD_TEMP,		//温度
	CMD_DB			//DB
};

#define TEMP_RISER	800	//定义5分钟温升5°	
void AddSoe(SOE_STRUCT soe);
//基数从7开始

uint8 lastPort;
uint8 g_bPauseWarning;
uint8 writeEepDevps_flag;
uint32 pw_count;	//暂停计时
uint8 g_soe_pos,g_soe_num;

SOE_STRUCT g_soe;
DEVICE_DATA dev[PORT_NUM][LINE_DEV_NUMS];
DEV_PARAM_STRUCT devPS;
// BASE_PARAM_STRUCT devBase;
// PORT_PARAM_STRUCT devPort;

const DEV_PARAM_STRUCT defParam = {
	{0x73AF,0x7F51,0x67DC,0,0,0,0,0,0,0},	//"环网柜",
	60,			//故障报警间隔,分钟
	8000,120,	//报警数值1,报警间隔
	9000,60,	//报警数值2,报警间隔.报警数值2需要大于数值1超过5度。
	1,1,		//地址,通讯端口
	1000,	//相间温差报警设定值
	800,	//10分钟温升报警设定值
	//端口名称
	{{'P','O','R','T','1'},{'P','O','R','T','2'},{'P','O','R','T','3'},
	{'P','O','R','T','4'},{'P','O','R','T','5'},{'P','O','R','T','6'}},
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,	//有效性
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,	//报警使能
};

DEV_BJ devBj1[PORT_NUM][LINE_DEV_NUMS];
DEV_BJ devBj2[PORT_NUM][LINE_DEV_NUMS];
DEV_ERR devErr[PORT_NUM][LINE_DEV_NUMS];
PORT_COMM_ERR portCommErr;	//端口通讯故障
uint16 BjEnalbe[PORT_NUM];	//报警使能，每位对应1个设备


uint8 Read_temp_Param(DEV_PARAM_STRUCT *dps)
{
	uint16 i,addr;
	uint8 *pdata,crc,crc1;
	
	crc = 0;
	addr = EEP_ADDR_TEMP;
	pdata = (uint8 *)dps;

	WDR();
	for(i=0; i<sizeof(DEV_PARAM_STRUCT); i++)
	{
		*pdata = EEPROM_Read(addr);
		crc += *pdata;
		pdata++;
		addr++;
	}
	WDR();
	//读CRC
	crc1 = EEPROM_Read(addr);
	if(crc == crc1 )
		return 1;
	return 0;
}

void Write_temp_Param(DEV_PARAM_STRUCT dps)
{
	uint16 i,addr;
	uint8 *pdata,crc,err;
	DEV_PARAM_STRUCT tmpdevps;

	err = 0;
BEGIN:	
	WDR();
	crc = 0;
	addr = EEP_ADDR_TEMP;
	pdata = (uint8 *)&dps;

	for(i=0; i<sizeof(DEV_PARAM_STRUCT); i++)
	{
		EEPROM_Write(*pdata, addr);
		crc += *pdata;
		pdata++;
		addr++;
	}
	//写CRC
	EEPROM_Write(crc, addr);
	WDR();
	if(Read_temp_Param(&tmpdevps)==0)
	{
		err++;
		if(err >= 2)
		{
			return;
		}
		goto BEGIN;
	}
}


void init_wxcw_run()
{
	uint8 i,j;
	
	memset(&dev[0][0],0,sizeof(dev));

	writeEepDevps_flag = 0;
	g_bPauseWarning = 0;
	pw_count = 0;
	WDR();
	for(i=0; i<PORT_NUM; i++)
	{
		for(j=0; j<LINE_DEV_NUMS; j++)
		{
			devErr[i][j].err = 0;
			devErr[i][j].errold = 0;
			devErr[i][j].errCount = 0;
			
			devBj1[i][j].bj = 0;
			devBj1[i][j].bjold = 0;
			devBj1[i][j].bjValue = 0;
			devBj1[i][j].bjCount = 0;
			
			devBj2[i][j].bj = 0;
			devBj2[i][j].bjold = 0;
			devBj2[i][j].bjValue = 0;
			devBj2[i][j].bjCount = 0;

			dev[i][j].oldtemp = 0xFFFF;

			dev[i][j].status = 1;	//默认为正常
		}
		BjEnalbe[i] = 0xFFFF;
	}
	portCommErr.err = 1;		//初始化不要为0
	portCommErr.errold = 0;
	lastPort = 0;
}

#define DEVPS_FIRST_FLAG	0x5A
void Get_Init_temp_data()
{
	DEV_PARAM_STRUCT devpstmp;
	uint8 err=0;

	g_soe_pos = 0;
	memset(&g_soe,0,sizeof(SOE_STRUCT));

	WDR();
	if(EEPROM_Read(EEP_ADDR_TEMP-1) != DEVPS_FIRST_FLAG)
	{//未初始化过
		EEPROM_Write(DEVPS_FIRST_FLAG,EEP_ADDR_TEMP-1);
		Write_temp_Param(defParam);
		EEPROM_Write(0,EEP_ADDR_SOE-2);
		EEPROM_Write(0,EEP_ADDR_SOE-1);
	  SleepMSec(5);
	  WDR();
	}
	memset(&devPS,0,sizeof(DEV_PARAM_STRUCT));
BEGIN:
	if(Read_temp_Param(&devpstmp)==0)
	{//参数有错误
		uint8 i,k,len;
		err++;
		if(err < 3)
			goto BEGIN;
		devPS = defParam;
		//记录参数错误，并发送数据给用户
		//项目名称
		k = 0;
		for(i=0; i<10; i++)
		{
			if(devPS.strPrjName[i]==0)
				break;
			sCurSMS[k++] = (devPS.strPrjName[i]>>8)&0xFF;	//高8位
			sCurSMS[k++] = devPS.strPrjName[i]&0xFF;	//低8位
		}
		sprintf(sssbuf,"配置参数错误!请重新配置!!");
		len = Str2UCS2((const char *)&sssbuf[0],&sCurSMS[k]);
		len += k;
		AddUCS2Group((uint8 *)sCurSMS,len,0,1);
	}
	else
	{
		devPS = devpstmp;
	}

	WDR();
	g_soe_pos = EEPROM_Read(EEP_ADDR_SOE-2);
	g_soe_num = EEPROM_Read(EEP_ADDR_SOE-1);
	if(g_soe_num==0xFF)
	{
		g_soe_pos = 0;
		g_soe_num = 0;
	}
	else
	{
		if(g_soe_pos > g_soe_num)
		{
			g_soe_pos = g_soe_num;
		}
	}

	init_wxcw_run();
}

uint8 getCommNO()
{
	return devPS.m_CommNo;
}

//获取当前的命令地址
uint16 getCurCmdPort()
{
	if(lastPort >= PORT_NUM-1)
		lastPort = 0;
	else
		lastPort ++;

	return lastPort;
}


int GetModbusStruct(MODBUSINFO* ModbusInfo)
{
	uint16 addr;

	addr = getCurCmdPort();

	ModbusInfo->DeviceAddr = devPS.m_Addr;
	ModbusInfo->FunCode = 4;
	ModbusInfo->Regiseter = addr*LINE_DEV_NUMS* DEV_BLOCK_SIZE;
	ModbusInfo->RegNumbers = LINE_DEV_NUMS* DEV_BLOCK_SIZE;	//一次采集一个端口

	ModbusInfo->nType = COMM_AI_TYPE;

	return addr;
}

//读取端口数据
uint8 ReadPortData(uint16 addr,uint8 *data)
{
	uint16 i,j,k;
	uint16 *pdat;

	k = 0;
	for(i=0; i<PORT_NUM; i++)
	{
		if(addr == k)
			break;
		k += DEV_BLOCK_SIZE*LINE_DEV_NUMS;
	}
	if(i==PORT_NUM)
		return 0;

	k = i;	//请求端口
	j = 0;
	for(i=0; i<LINE_DEV_NUMS; i++)
	{
		pdat = (uint16 *)&(dev[k][i].temp);
		data[j++] = ((*pdat)>>8)&0xFF;
		data[j++] = (*pdat)&0xFF;

		data[j++] = 0;
		data[j++] = 0;
		data[j++] = 0;
		data[j++] = 0;

		data[j++] = (dev[k][i].db>>8)&0xFF;
		data[j++] = dev[k][i].db&0xFF;

		data[j++] = 0;
		data[j++] = 0;

		data[j++] = (dev[k][i].status>>8)&0xFF;
		data[j++] = dev[k][i].status&0xFF;
	}

	return j;
}

//读取公共参数
uint8 ReadBaseParam(uint8 *data)
{
	uint16 i,j;
	uint16 dat;
	j = 0;
	//项目名称
	for(i=0; i<10; i++)
	{
		data[j++] = (devPS.strPrjName[i]>>8)&0xFF;
		data[j++] = devPS.strPrjName[i]&0xFF;
	}
	data[j++] = (devPS.ErrInterval>>8)&0xFF;
	data[j++] = devPS.ErrInterval&0xFF;

	data[j++] = (devPS.BJ1Interval>>8)&0xFF;
	data[j++] = devPS.BJ1Interval&0xFF;

	dat = devPS.tempSet1;
	data[j++] = (dat>>8)&0xFF;
	data[j++] = dat&0xFF;

	data[j++] = (devPS.BJ2Interval>>8)&0xFF;
	data[j++] = devPS.BJ2Interval&0xFF;

	dat = devPS.tempSet2;
	data[j++] = (dat>>8)&0xFF;
	data[j++] = dat&0xFF;
	
	data[j++] = 0;
	data[j++] = devPS.m_Addr;

	data[j++] = 0;
	data[j++] = devPS.m_CommNo;
	
	data[j++] = (devPS.diffTemp>>8)&0xFF;
	data[j++] = devPS.diffTemp&0xFF;
	
	data[j++] = (devPS.deltaTemp>>8)&0xFF;
	data[j++] = devPS.deltaTemp&0xFF;

	return j;
}

uint8 WriteBaseParam(uint8 *buffer)
{
	uint16 i,j,dat;

	j = 0;
	//项目名称
	for(i=0; i<10; i++)
	{
		dat = buffer[j++];
		dat <<= 8;
		dat += buffer[j++];
		devPS.strPrjName[i] = dat;
	}
	dat = buffer[j++];
	dat <<= 8;
	dat += buffer[j++];
	devPS.ErrInterval = dat;
	
	dat = buffer[j++];
	dat <<= 8;
	dat += buffer[j++];
	devPS.BJ1Interval = dat;
	
	dat = buffer[j++];
	dat <<= 8;
	dat += buffer[j++];
	devPS.tempSet1 = dat;
	
	dat = buffer[j++];
	dat <<= 8;
	dat += buffer[j++];
	devPS.BJ2Interval = dat;
	
	dat = buffer[j++];
	dat <<= 8;
	dat += buffer[j++];
	devPS.tempSet2 = dat;
	
	j++;
	devPS.m_Addr = buffer[j++];

	j++;
	devPS.m_CommNo = buffer[j++];
	
	dat = buffer[j++];
	dat <<= 8;
	dat += buffer[j++];
	devPS.diffTemp = dat;
	
	dat = buffer[j++];
	dat <<= 8;
	dat += buffer[j++];
	devPS.deltaTemp = dat;

	writeEepDevps_flag = 1;

	return j;
}

//读取端口参数
uint8 ReadPortParam(uint8 *data)
{
	uint8 i,j,k;
	j = 0;
	//有效性
	for(i=0; i<PORT_NUM; i++)
	{
		data[j++] = (devPS.Valid[i]>>8)&0xFF;
		data[j++] = devPS.Valid[i]&0xFF;
	}
	//报警
	for(i=0; i<PORT_NUM; i++)
	{
		data[j++] = (devPS.BjEnalbe[i]>>8)&0xFF;
		data[j++] = devPS.BjEnalbe[i]&0xFF;
	}
	
	//端口名称
	for(i=0; i<PORT_NUM; i++)
	{
		for(k=0; k<5; k++)
		{
			data[j++] = (devPS.strPortName[i][k]>>8)&0xFF;
			data[j++] = devPS.strPortName[i][k]&0xFF;
		}
	}
	
	return j;
}

uint8 WritePortParam(uint8 *buffer)
{
	uint16 i,j,k,dat;

	j = 0;
	//有效性
	for(i=0; i<PORT_NUM; i++)
	{
		dat = buffer[j++];
		dat <<= 8;
		dat += buffer[j++];
		devPS.Valid[i] = dat;
	}
	//报警
	for(i=0; i<PORT_NUM; i++)
	{
		dat = buffer[j++];
		dat <<= 8;
		dat += buffer[j++];
		devPS.BjEnalbe[i] = dat;
	}
	
	//端口名称
	for(i=0; i<PORT_NUM; i++)
	{
		for(k=0; k<5; k++)
		{
			dat = buffer[j++];
			dat <<= 8;
			dat += buffer[j++];
			devPS.strPortName[i][k] = dat;
		}
	}

	writeEepDevps_flag = 2;
	return j;
}

//读取电话号码
uint8 ReadPhoneParam(uint8 *data)
{
	uint8 i,j,k;
	j = 0;
	//号码个数
	data[j++] = 0;
	data[j++] = telenumoldbuf;
	
	//号码
	for(i=0; i<10; i++)
	{
		for(k=0; k<11; k++)
		{
			data[j++] = gsmtelnumbuf[YongfuNumStart+i][k];
		}
	}
	
	//号码权限
	data[j++] = (gsmtelsuper>>8)&0xFF;
	data[j++] = gsmtelsuper&0xFF;

	return j;
}

//写电话号码
uint8 WritePhoneParam(uint8 *buffer)
{
	uint16 i,j,k,dat;

	j = 1;
	//号码个数
	telenumnewbuf = buffer[j++];
	
	//号码
	for(i=0; i<teleUserNumMax; i++)
	{
		WDR();//清看门狗
		for(k=0; k<11; k++)
		{
			gsmtelnumbuf[YongfuNumStart+i][k] = buffer[j++];
		}
	}

	//号码权限
	dat = buffer[j++];
	dat <<= 8;
	dat += buffer[j];
	gsmtelsuper = dat;
	
	//
	telenumoldbuf = telenumnewbuf;
	teletolnumbuf = telenumoldbuf+YongfuNumStart;
	eepbufflag.bit.telenumf = 1;
	eepbufflag.bit.telesuper = 1;
	SETBIT(funcflag7,EEpwritedelay);
	for(j = 0; j < telenumnewbuf; j++)
	{
		Gsmstature4flag.Word |= (1 << j);
	}
	WDR();//清看门狗

	eepbufflag.bit.telesuper = 1;

	return j;
}

void SetCommStatusErr(uint8 err)
{
	if(err)
	{
		if(portCommErr.err < TX_ERR_COUNT)
			portCommErr.err++;
	}
	else
	{
		portCommErr.err = 0;
	}
}

//设置当前设备数据,整个端口12个设备的数据
void ProcessData(uint8 *Data, uint8 len)
{//一次请求12个设备
	uint8 i,j;
	uint16 tmp;

	j = 0;
	//当前采集的端口有效
	if(devPS.Valid[lastPort] & 0x0FFF)
	{//端口有效

		for(i=0; i<LINE_DEV_NUMS; i++)
		{
			//温度
			tmp = Data[j++];	//0
			tmp <<= 8;
			tmp += Data[j++];	//1
			dev[lastPort][i].temp = (int16)tmp;
			//db
			j += 4;
			tmp = Data[j++];	//6
			tmp <<= 8;
			tmp += Data[j++];	//7
			dev[lastPort][i].db = tmp;
			//状态
			j += 2;
			tmp = Data[j++];	//10
			tmp <<= 8;
			tmp += Data[j++];	//11
			dev[lastPort][i].status = tmp;
		}
	}
}

uint8 FormatMsgPrjName(uint8 *buf)
{
	uint16 i,k;
	uint8 *p1;
	
	p1 = buf;
	//项目名称
	for(i=0; i<10; i++)
	{
		if(devPS.strPrjName[i]==0)
			break;
		*p1++ = (devPS.strPrjName[i]>>8)&0xFF;	//高8位
		*p1++ = devPS.strPrjName[i]&0xFF;	//低8位
	}
	k = i*2;

	return k;
}

uint8 FormatMsgPortName(uint16 *src, uint8 *buf)
{
	uint16 i,k;
	uint8 *p1;

	p1 = buf;
	//端口名称
	for(i=0; i<5; i++)
	{
		if(src[i]==0)
			break;
		*p1++ = (src[i]>>8)&0xFF;	//高8位
		*p1++ = src[i]&0xFF;	//低8位
	}
	k = i*2;	//总长度，字节数
	
	return k;
}

uint8 FormatMsgHead(uint16 *src, uint8 *buf)
{
	uint16 k;
	k = FormatMsgPrjName(buf);

	buf += k;
	*buf++ = 0;
	*buf++ = '_';

	k += FormatMsgPortName(src, buf);

//	uint16 i,k;
// 	uint8 *p1;
// 	
// 	p1 = buf;
// 	//项目名称
// 	for(i=0; i<10; i++)
// 	{
// 		if(devPS.strPrjName[i]==0)
// 			break;
// 		*p1++ = (devPS.strPrjName[i]>>8)&0xFF;	//高8位
// 		*p1++ = devPS.strPrjName[i]&0xFF;	//低8位
// 	}
// 	k = i*2;
// 	//'_'
// 	*p1++ = 0;
// 	*p1++ = 0x5F;	//_
// 	k += 2;
// 	//端口名称
// 	for(i=0; i<5; i++)
// 	{
// 		if(src[i]==0)
// 			break;
// 		*p1++ = (src[i]>>8)&0xFF;	//高8位
// 		*p1++ = src[i]&0xFF;	//低8位
// 	}
// 	k += i*2;	//总长度，字节数
	
	return k;
}

void FormatMsg(SOE_STRUCT soe)
{
	uint8 k,len;
	
	switch(soe.type)
	{
	case SOE_TYPE_COMM:
		{//通信故障
			k = FormatMsgPrjName((uint8 *)sCurSMS);
			sprintf(sssbuf,"(地址:%d):通讯故障!",soe.value1);
		}
		break;
	case SOE_TYPE_COMMX:
		{//通信故障恢复
			k = FormatMsgPrjName((uint8 *)sCurSMS);
			sprintf(sssbuf,"(地址:%d):通讯恢复!",soe.value1);
		}
		break;
	case SOE_TYPE_ERR:
		{//传感器异常
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(端口%d):传感器%d异常(%d)",soe.port+1,soe.sensor+7,soe.value1);
		}
		break;
	case SOE_TYPE_ERRX:
		{//传感器异常
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(端口%d):传感器%d恢复(%d)",soe.port+1,soe.sensor+7,soe.value1);
		}
		break;
	case SOE_TYPE_BJ1:
		{//温度报警1
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(端口%d):传感器%d温度:%d,一般报警.",soe.port+1,soe.sensor+7,soe.value1/100);
		}
		break;
	case SOE_TYPE_BJ1X:
		{//温度报警1
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(端口%d):传感器%d温度:%d,退出一般报警.",soe.port+1,soe.sensor+7,soe.value1/100);
		}
		break;
	case SOE_TYPE_BJ2:
		{//温度报警1
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(端口%d):传感器%d温度:%d,严重报警.",soe.port+1,soe.sensor+7,soe.value1/100);
		}
		break;
	case SOE_TYPE_BJ2X:
		{//温度报警1
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(端口%d):传感器%d温度:%d,退出严重报警.",soe.port+1,soe.sensor+7,soe.value1/100);
		}
		break;
	case SOE_TYPE_RISE:
		{//温升异常
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(端口%d):传感器%d十分钟温升%d度,异常报警!温度:%d.",soe.port+1,soe.sensor+7,soe.value2/100,soe.value1/100);
		}
		break;
	case SOE_TYPE_RISEX:
		{//温升异常
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(端口%d):传感器%d十分钟温升%d度,恢复正常!温度:%d.",soe.port+1,soe.sensor+7,soe.value2/100,soe.value1/100);
		}
		break;
	case SOE_TYPE_DELTA:
		{//温差异常
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(端口%d):温差(%d)异常报警。最高温传感器%d,温度:%d.",soe.port+1,soe.value2/100,soe.sensor+7,soe.value1/100);
		}
		break;
	case SOE_TYPE_DELTAX:
		{//温差异常
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(端口%d):温差(%d)恢复正常。最高温传感器%d,温度:%d.",soe.port+1,soe.value2/100,soe.sensor+7,soe.value1/100);
		}
		break;
	default:
		return;
	}

	len = Str2UCS2((const char *)&sssbuf[0],&sCurSMS[k]);
	len += k;
	AddUCS2Group((uint8 *)sCurSMS,len,0,1);
}

void FormatSoe(uint8 type, uint8 port, uint8 sensor, int16 value1, int16 value2,uint8 bRec)
{

	SOE_STRUCT soe;
	
	if(type >= SOE_TYPE_NUM)
		return;

	soe.year = g_CLK.YEAR;
	soe.year = g_CLK.YEAR;
	soe.year = g_CLK.YEAR;
	soe.year = g_CLK.YEAR;
	soe.year = g_CLK.YEAR;
	soe.year = g_CLK.YEAR;
	soe.port = port;
	soe.type = type;
	soe.value1 = value1;
	soe.value2 = value2;
	soe.sensor = sensor;

	if(bRec)
	{
		AddSoe(soe);
	}

	FormatMsg(soe);
}

//查询记录
void GSM_CXJL(uint8 nRec,uint8 groupno)
{
	SOE_STRUCT soe;
	
	if(ReadSoe(Soesearchnumbuf,&soe))
	{
		FormatMsg(soe);
	}
	else
	{
		AddMsgGroup("无记录",groupno);
	}
}

void TX_run()
{
	//通讯判断
	if(portCommErr.err==TX_ERR_COUNT)
	{//通讯故障处理
		if(portCommErr.errold==0)
		{
			portCommErr.errold = 1;
			FormatSoe(SOE_TYPE_COMM, 0, 0, devPS.m_Addr, 0, 1);
		}
	}
	else if(portCommErr.err==0)
	{//通讯正常时
		if(portCommErr.errold)
		{
			portCommErr.errold = 0;
			FormatSoe(SOE_TYPE_COMMX, 0, 0, devPS.m_Addr, 0, 1);
		}
	}
}

void Status_run()
{
	uint8 i,j;

	if(portCommErr.err)
		return;

	//1、判断状态
	for(i=0; i<PORT_NUM; i++)
	{

		for(j=0; j<LINE_DEV_NUMS; j++)
		{
			if(devPS.Valid[i]&(1<<j))
			{//有效设备
				if(dev[i][j].status != 1)
				{//温度未刷新，
					devErr[i][j].err = 1;
				}
				else
				{
					devErr[i][j].err = 0;
					
					if(dev[i][j].temp > devPS.tempSet2)
					{
						devBj2[i][j].bj = 1;
					}
					else if(dev[i][j].temp < devPS.tempSet2-500)
					{
						devBj2[i][j].bj = 0;
					}
					
					if(dev[i][j].temp > devPS.tempSet1)
					{
						devBj1[i][j].bj = 1;
					}
					else if(dev[i][j].temp < devPS.tempSet1-500)
					{
						devBj1[i][j].bj = 0;
					}
				}
				//故障处理
				if(devErr[i][j].err)
				{
					if(devErr[i][j].err != devErr[i][j].errold)
					{
						devErr[i][j].errold = devErr[i][j].err;
						devErr[i][j].errCount = dTCounter;
						FormatSoe(SOE_TYPE_ERR, i, j, dev[i][j].status, 0, 1);
					}
					if(devPS.ErrInterval != 0)
					{
						if(g_bPauseWarning==0)
						{
							devErr[i][j].errCount++;
//							if(BjEnalbe[i] & (1<<j))
							{//报警使能
								uint32 ul = 60ul * 1000 * devPS.ErrInterval;
								if (CalcTickWidth(dTCounter, devErr[i][j].errCount) > ul )
								{
									devErr[i][j].errCount = dTCounter;
									FormatSoe(SOE_TYPE_ERR, i, j, dev[i][j].status, 0, 0);
								}
							}
						}
					}
				}
				else if(devErr[i][j].err != devErr[i][j].errold)
				{
					devErr[i][j].errold = devErr[i][j].err;
					BjEnalbe[i] |= (1<<j);	//恢复报警使能
					FormatSoe(SOE_TYPE_ERRX, i, j, dev[i][j].status, 0, 1);
				}

				//报警处理
				if(devBj2[i][j].bj)
				{
					if(devBj2[i][j].bjold != devBj2[i][j].bj)
					{
						devBj2[i][j].bjold = devBj2[i][j].bj;
						devBj2[i][j].bjCount = dTCounter;
						FormatSoe(SOE_TYPE_BJ2, i, j, dev[i][j].temp, 0, 1);
					}
					if(devPS.BJ2Interval != 0)
					{//间隔多久再发一次报警
						if(g_bPauseWarning==0)
						{
							devBj2[i][j].bjCount++;
//							if(BjEnalbe[i] & (1<<j))
							{//报警使能
								uint32 ul = 60ul * 1000 * devPS.BJ2Interval;
								if (CalcTickWidth(dTCounter, devBj2[i][j].bjCount) > ul )
								{
									devBj2[i][j].bjCount = dTCounter;
									FormatSoe(SOE_TYPE_BJ2, i, j, dev[i][j].temp, 0, 0);
								}
							}
						}
					}
				}
				else if(devBj1[i][j].bj)
				{//严重报警时，这个不报警
					if(devBj2[i][j].bjold != devBj2[i][j].bj)
					{//严重报警恢复
						devBj2[i][j].bjold = devBj2[i][j].bj;
						FormatSoe(SOE_TYPE_BJ2X, i, j, dev[i][j].temp, 0, 1);
					}

					if(devBj1[i][j].bjold != devBj1[i][j].bj)
					{
						devBj1[i][j].bjold = devBj1[i][j].bj;
						devBj1[i][j].bjCount = dTCounter;
						//记录到事件中
						FormatSoe(SOE_TYPE_BJ1, i, j, dev[i][j].temp, 0, 1);
					}
					if( devPS.BJ1Interval != 0 )
					{//间隔多久再发一次报警
						if(g_bPauseWarning==0)
						{
							uint32 ul = 60ul * 1000 * devPS.BJ1Interval;

							if (CalcTickWidth(dTCounter, devBj1[i][j].bjCount) > ul )
							{
								devBj1[i][j].bjCount = dTCounter;
//								if(BjEnalbe[i] & (1<<j))
								{//报警使能
									FormatSoe(SOE_TYPE_BJ1, i, j, dev[i][j].temp, 0, 0);
								}
							}
						}
					}
				}
				if(devBj1[i][j].bjold != devBj1[i][j].bj)
				{
					devBj1[i][j].bjold = devBj1[i][j].bj;
					if(devBj1[i][j].bjold==0)
					{
						BjEnalbe[i] |= (1<<j);	//恢复报警使能
						//记录到事件中
						FormatSoe(SOE_TYPE_BJ1X, i, j, dev[i][j].temp, 0, 1);
					}
				}
			}
		}
	}
}

void temp_run()
{
	uint8 i,j,k;
	uint16 max,min,dat;

	if(portCommErr.err)
	{//通讯故障，则重置上次测量温度
		for(i=0; i<PORT_NUM; i++)
		{
			for(j=0; j<LINE_DEV_NUMS; j++)
				dev[i][j].deltaCount = 0;
// 				dev[i][j].oldtemp = 0xFFFF;
		}
		return;
	}

	for(i=0; i<PORT_NUM; i++)
	{
		for(j=0; j<LINE_DEV_NUMS; j++)
		{
			if(dev[i][j].deltaCount==0)
			{
				dev[i][j].oldtemp = dev[i][j].temp;;
			}
		}

		max = 0;
		min = 30000;
		k = 0;
		for(j=0; j<LINE_DEV_NUMS; j++)
		{
			if(devPS.Valid[i]&(1<<j))
			{//有效设备
				if(dev[i][j].status == 1)
				{//温度有效
					if(dev[i][j].temp > max)
					{
						max = dev[i][j].temp;
						k = j;
					}
					if(dev[i][j].temp < min)
						min = dev[i][j].temp;

					dev[i][j].deltaCount++;
					if(dev[i][j].deltaCount > 5L*60*10)
					{//五分钟
						dev[i][j].deltaCount = 0;
						if(dev[i][j].oldtemp != 0xFFFF)
						{
							if(dev[i][j].temp > dev[i][j].oldtemp)
							{
								dat = dev[i][j].temp-dev[i][j].oldtemp;
								if( dat > TEMP_RISER)
								{//五分钟升温超过8度以上
									if(dev[i][j].rise==0)
									{
										dev[i][j].rise = 1;
										FormatSoe(SOE_TYPE_RISE, i, j, dev[i][j].temp, dat, 1);
									}
								}
								else if( dat < TEMP_RISER*8/10)
								{
									if(dev[i][j].rise)
									{
										dev[i][j].rise = 0;
										FormatSoe(SOE_TYPE_RISEX, i, j, dev[i][j].temp, dat, 1);
									}
								}
							}
						}
						dev[i][j].oldtemp = dev[i][j].temp;
					}
				}
			}
			else
			{
				dev[i][j].oldtemp = 0xFFFF;
			}
		}
		if(max > min)
		{
			dat = max-min;
			if(dat > devPS.deltaTemp)
			{//相间温度不平衡
				if(dev[i][k].diff==0)
				{
					dev[i][k].diff = 1;
					FormatSoe(SOE_TYPE_DELTA, i, j, dev[i][k].temp, dat, 1);
				}
			}
			else if(dat < (devPS.deltaTemp*8/10))
			{
				if(dev[i][k].diff)
				{
					dev[i][k].diff = 0;
					FormatSoe(SOE_TYPE_DELTAX, i, j, dev[i][k].temp, dat, 1);
				}
			}
		}
	}
}

#define PWTIME	(2L*60*60*10)	//两个小时
//日常运行模块,100ms执行一次
void WXCW_Run()
{
	if(g_stoprun==0x5A)
		return;

	WDR();
	if(g_bPauseWarning)
	{
		pw_count++;
		if(pw_count > PWTIME)
		{
			g_bPauseWarning = 0;
			pw_count = 0;
		}
	}

	TX_run();

	Status_run();

	temp_run();
	if(writeEepDevps_flag)
	{
		writeEepDevps_flag = 0;
		Write_temp_Param(devPS);
	}
}

//记录SOE内容
void AddSoe(SOE_STRUCT soe)
{
	uint8 i;
	uint8 *pBuf;
	uint16 addr;
	
	pBuf = (uint8 *)&soe;
	addr = EEP_ADDR_SOE + SOE_REC_SIZE*g_soe_pos;
	for(i=0; i<sizeof(SOE_STRUCT); i++)
	{
		EEPROM_Write(*pBuf,addr);
		pBuf++;
	}
	
	if(g_soe_num < SOE_NUM_MAX)
	{
		g_soe_num++;
		g_soe_pos++;
	}
	else
	{
		g_soe_pos++;
		if(g_soe_pos >= SOE_NUM_MAX)
			g_soe_pos = 0;
	}
	EEPROM_Write(g_soe_pos,EEP_ADDR_SOE-2);
	EEPROM_Write(g_soe_num,EEP_ADDR_SOE-1);
}

//读取soe
uint8 ReadSoe(uint8 rec_no,SOE_STRUCT *pSoe)
{
	uint8 i;
	uint8 *pBuf;
	uint16 addr;
	
	//判断序号
	if(rec_no > g_soe_num)
		rec_no = g_soe_num;

	if(g_soe_num==SOE_NUM_MAX)
	{//已经保存满了
		if(g_soe_pos >= rec_no)
		{
			i = g_soe_pos - rec_no;
		}
		else
		{
			i = g_soe_num + g_soe_pos - rec_no;
		}
	}
	else
	{//没有满的时候,g_soe_pos=g_soe_num
		if(g_soe_num==0)
		{
			return 0;
		}
		i = g_soe_pos - rec_no;
	}

	pBuf = (uint8 *)&pSoe;
	//取得要读取的序号位置
	addr = EEP_ADDR_SOE + SOE_REC_SIZE*i;
	for(i=0; i<sizeof(SOE_STRUCT); i++)
	{
		*pBuf = EEPROM_Read(addr++);
		pBuf++;
	}
	return 1;
}

//查询状态
void GSM_CXZT(uint8 groupno)
{
	uint16 i,j,k,m,len;
	char *pbuf;
	
	pbuf = sCurSMS;

	//项目名称
	k = FormatMsgPrjName((uint8 *)pbuf);
	pbuf += k;
	len = k;
	*pbuf++ = 0;
	*pbuf++ = ':';
	*pbuf++ = 0;
	*pbuf++ = '\r';
	len += 4;

	for(i=0; i<PORT_NUM; i++)
	{
		k = FormatMsgPortName(devPS.strPortName[i], (uint8 *)sssbuf1);
		m = 140 - len;
		if(k < m)
		{
			memcpy(pbuf,sssbuf1,k);
			pbuf += k;
			len += k;
		}
		else
		{
			memcpy(pbuf,sssbuf1,m);
			len += m;
			AddUCS2Group((uint8 *)sCurSMS,len,groupno,1);
			
			pbuf = sCurSMS;
			len = k-m;
			memcpy(pbuf, &sssbuf1[m], len);
		}

		for(j=0; j<LINE_DEV_NUMS; j++)
		{
			if(devPS.Valid[i]&(1<<j))
			{//设备有效性
// 				k = FormatMsgPortName(devPS.strPortName[i], (uint8 *)pbuf);

				if(dev[i][j].status==1)
					sprintf(sssbuf,":S%d_温度:%d,)",j+7,dev[i][j].temp/100);
				else
					sprintf(sssbuf,":S%d_异常:%d,)",j+7,dev[i][j].status);

				k = Str2UCS2((const char *)&sssbuf[0],sssbuf1);
				m = 140 - len;
				if(k < m)
				{
					memcpy(pbuf,sssbuf1,k);
					pbuf += k;
					len += k;
				}
				else
				{
					memcpy(pbuf,sssbuf1,m);
					len += m;
					AddUCS2Group((uint8 *)sCurSMS,len,groupno,1);
					
					pbuf = sCurSMS;
					len = k-m;
					memcpy(pbuf, &sssbuf1[m], len);
				}
			}
		}
	}
	if(len)
		AddUCS2Group((uint8 *)sCurSMS,len,groupno,1);
}

//查询有效
void GSM_CXYX(uint8 qx,uint8 groupno)
{
	uint8 k;
	
	k = 0;
	k += sprintf(&sCurSMS[k],"故障上报间隔:%d分钟;",devPS.ErrInterval);
	k += sprintf(&sCurSMS[k],"一般报警温度:%d度,报警间隔:%d分钟;",devPS.tempSet1/100,devPS.BJ1Interval);
	k += sprintf(&sCurSMS[k],"严重报警温度:%d度,报警间隔:%d分钟.",devPS.tempSet2/100,devPS.BJ2Interval);
	AddMsgGroup(sCurSMS,groupno);
}

//修改有效
void GSM_XGYX(uint16 *pBuf)
{
	uint8 k;
	for(k=0; k<PORT_NUM; k++)
		devPS.Valid[k] = pBuf[k];
}

//查询设置
void GSM_CXSZ(uint8 qx,uint8 groupno)
{
	uint8 k;

	k = 0;
	k += sprintf(&sCurSMS[k],"故障上报间隔:%d分钟;",devPS.ErrInterval);
	k += sprintf(&sCurSMS[k],"一般报警温度:%d度,报警间隔:%d分钟;",devPS.tempSet1/100,devPS.BJ1Interval);
	k += sprintf(&sCurSMS[k],"严重报警温度:%d度,报警间隔:%d分钟.",devPS.tempSet2/100,devPS.BJ2Interval);
	AddMsgGroup(sCurSMS,groupno);
}

//删除记录
void GSM_SCJL(uint8 groupno)
{
	g_soe_num = 0;
	g_soe_pos = 0;
	EEPROM_Write(g_soe_pos,EEP_ADDR_SOE-2);
	EEPROM_Write(g_soe_num,EEP_ADDR_SOE-1);

	AddMsgGroup("删除所有记录成功!",groupno);
}

const uint8 UCS2_XMMC[]={0x98,0x79,0x76,0xEE,0x54,0x0D,0x79,0xF0,0,':'};	//项目名称:
//查询名称
void GSM_CXMC(uint8 groupno)
{
	uint8 i,k,len;
	uint8 *p1;
	
	p1 = (uint8 *)&sCurSMS[0];
	for(i=0; i<sizeof(UCS2_XMMC)/sizeof(UCS2_XMMC[0]); i++)
	{
		*p1++ = UCS2_XMMC[i];
	}
	//项目名称
	for(i=0; i<10; i++)
	{
		if(devPS.strPrjName[i]==0)
			break;
		*p1++ = (devPS.strPrjName[i]>>8)&0xFF;	//高8位
		*p1++ = devPS.strPrjName[i]&0xFF;	//低8位
	}
	*p1++ = 0;
	*p1++ = ';' ;	//; 

	for(i=0; i<6; i++)
	{
		*p1++ = 0x7A;	//端
		*p1++ = 0xEF;
		*p1++ = 0x53;	//口
		*p1++ = 0xE3;
		*p1++ = 0;
		*p1++ = 0x31 + i;
		*p1++ = 0;
		*p1++ = ':';
		for(k=0; k<5; k++)
		{
			if(devPS.strPortName[i][k]==0)
				break;
			*p1++ = (devPS.strPortName[i][k]>>8)&0xFF;	//高8位
			*p1++ = devPS.strPortName[i][k]&0xFF;	//低8位
		}
		if(i<5)
		{
			*p1++ = 0;
			*p1++ = ';' ;	//; 
		}
	}
	len = p1 - (uint8 *)&sCurSMS[0];
	AddUCS2Group((uint8 *)sCurSMS,len,groupno,1);
}

//修改项目名称
void GSM_XGXMMC(uint8 *pBuf, uint8 len,uint8 groupno)
{
	uint8 j,t=0,i=0;
	uint8 datastart=0;//,bufdatanum;
	uint16 DATA;
	uint8 dat[20];

	WDR();//清看门狗

	for(t=0; t<len; t++)
	{
		if(datastart==0)
		{
			if((pBuf[t]=='='))  //找出数据的起始位置
				datastart = 1;
		}
		else
		{
			dat[i] = pBuf[t];
			i++;
			if(i > 20)
			{
				AddMsgGroup("项目名称设置失败!名称过长!最多10个字.",groupno);
				return;
			}
		}
	}
	j = 0;
	for(i=0; i<20; i++,i++)
	{
		DATA = dat[i];
		DATA <<= 8;
		DATA = dat[i+1];
		if(DATA == 0)
			break;
		devPS.strPrjName[j++] = DATA;
	}
	AddMsgGroup("项目名称设置成功!",groupno);
}

//修改端口名称
void GSM_XGDKMC(uint8 port,uint8 *pBuf, uint8 len,uint8 groupno)
{
	uint8 j,t=0,i=0;
	uint8 datastart=0;//,bufdatanum;
	uint16 DATA;
	uint8 dat[20];
	
	WDR();//清看门狗
	
	for(t=0; t<len; t++)
	{
		if(datastart==0)
		{
			if((pBuf[t]=='='))  //找出数据的起始位置
				datastart = 1;
		}
		else
		{
			dat[i] = pBuf[t];
			i++;
			if(i > 10)
			{
				sprintf(sssbuf,"端口%d名称设置失败!名称过长!最多5个字.",port+1);
				AddMsgGroup(sssbuf,groupno);
				return;
			}
		}
	}
	j = 0;
	for(i=0; i<10; i++,i++)
	{
		DATA = dat[i];
		DATA <<= 8;
		DATA = dat[i+1];
		if(DATA == 0)
			break;
		devPS.strPortName[port][j++] = DATA;
	}

	sprintf(sssbuf,"端口%d名称设置成功!",port+1);
	AddMsgGroup(sssbuf,groupno);
}

//暂停报警
void GSM_ZTBJ(uint8 groupno)
{
	uint8 i,j,k;
	k = 0;
	for(i=0; i<PORT_NUM; i++)
	{
		for(j=0; j<LINE_DEV_NUMS; j++)
		{
			if(devErr[i][j].err)
			{
				k = 1;
			}
			if(devBj1[i][j].bj)
			{
				k = 1;
			}
			if(devBj2[i][j].bj)
			{
				k = 1;
			}
		}
	}
	if(k==1)
	{
		g_bPauseWarning = 1;
		AddMsgGroup("暂停报警成功!2小时后恢复报警功能!",groupno);
	}
	else
	{
		AddMsgGroup("当前没有报警,暂停报警失败!",groupno);
	}
}






