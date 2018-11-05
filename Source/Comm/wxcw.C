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
#define TX_ERR_COUNT	180	//ͨѶ���ϴ����жϣ���С��256.Ĭ��Ϊ180S
void SleepMSec(uint8 ms);
extern uint32 dTCounter;
//ͨ��������������
enum
{
	COMM_AI_TYPE = 0,
	COMM_DI_TYPE,
	COMM_DO_TYPE,
	COMM_CI_TYPE,
	COMM_PTTYPE_NUM
};

enum{
	STA_OFFLINE=0,		//����
	STA_FRESH_TEMP,		//ˢ���¶�
	STA_UNFRESH_TEMP1,	//��ˢ���¶�
	STA_UNFRESH_TEMP2,	//��ˢ���¶�
	STA_FAIL1,			//����ʧ��
	STA_FAIL2			//����ʧ��
};

enum{
	ADDR_TEMP=0,	//�¶ȵ�ַ
	ADDR_REV1,
	ADDR_REV2,
	ADDR_DB,		//db��ַ
	ADDR_REV3,
	ADDR_STA		//״̬��ַ
};

enum{
	CMD_STATUS = 0,	//״̬
	CMD_TEMP,		//�¶�
	CMD_DB			//DB
};

#define TEMP_RISER	800	//����5��������5��	
void AddSoe(SOE_STRUCT soe);
//������7��ʼ

uint8 lastPort;
uint8 g_bPauseWarning;
uint8 writeEepDevps_flag;
uint32 pw_count;	//��ͣ��ʱ
uint8 g_soe_pos,g_soe_num;

SOE_STRUCT g_soe;
DEVICE_DATA dev[PORT_NUM][LINE_DEV_NUMS];
DEV_PARAM_STRUCT devPS;
// BASE_PARAM_STRUCT devBase;
// PORT_PARAM_STRUCT devPort;

const DEV_PARAM_STRUCT defParam = {
	{0x73AF,0x7F51,0x67DC,0,0,0,0,0,0,0},	//"������",
	60,			//���ϱ������,����
	8000,120,	//������ֵ1,�������
	9000,60,	//������ֵ2,�������.������ֵ2��Ҫ������ֵ1����5�ȡ�
	1,1,		//��ַ,ͨѶ�˿�
	1000,	//����²���趨ֵ
	800,	//10�������������趨ֵ
	//�˿�����
	{{'P','O','R','T','1'},{'P','O','R','T','2'},{'P','O','R','T','3'},
	{'P','O','R','T','4'},{'P','O','R','T','5'},{'P','O','R','T','6'}},
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,	//��Ч��
	0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,	//����ʹ��
};

DEV_BJ devBj1[PORT_NUM][LINE_DEV_NUMS];
DEV_BJ devBj2[PORT_NUM][LINE_DEV_NUMS];
DEV_ERR devErr[PORT_NUM][LINE_DEV_NUMS];
PORT_COMM_ERR portCommErr;	//�˿�ͨѶ����
uint16 BjEnalbe[PORT_NUM];	//����ʹ�ܣ�ÿλ��Ӧ1���豸


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
	//��CRC
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
	//дCRC
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

			dev[i][j].status = 1;	//Ĭ��Ϊ����
		}
		BjEnalbe[i] = 0xFFFF;
	}
	portCommErr.err = 1;		//��ʼ����ҪΪ0
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
	{//δ��ʼ����
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
	{//�����д���
		uint8 i,k,len;
		err++;
		if(err < 3)
			goto BEGIN;
		devPS = defParam;
		//��¼�������󣬲��������ݸ��û�
		//��Ŀ����
		k = 0;
		for(i=0; i<10; i++)
		{
			if(devPS.strPrjName[i]==0)
				break;
			sCurSMS[k++] = (devPS.strPrjName[i]>>8)&0xFF;	//��8λ
			sCurSMS[k++] = devPS.strPrjName[i]&0xFF;	//��8λ
		}
		sprintf(sssbuf,"���ò�������!����������!!");
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

//��ȡ��ǰ�������ַ
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
	ModbusInfo->RegNumbers = LINE_DEV_NUMS* DEV_BLOCK_SIZE;	//һ�βɼ�һ���˿�

	ModbusInfo->nType = COMM_AI_TYPE;

	return addr;
}

//��ȡ�˿�����
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

	k = i;	//����˿�
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

//��ȡ��������
uint8 ReadBaseParam(uint8 *data)
{
	uint16 i,j;
	uint16 dat;
	j = 0;
	//��Ŀ����
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
	//��Ŀ����
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

//��ȡ�˿ڲ���
uint8 ReadPortParam(uint8 *data)
{
	uint8 i,j,k;
	j = 0;
	//��Ч��
	for(i=0; i<PORT_NUM; i++)
	{
		data[j++] = (devPS.Valid[i]>>8)&0xFF;
		data[j++] = devPS.Valid[i]&0xFF;
	}
	//����
	for(i=0; i<PORT_NUM; i++)
	{
		data[j++] = (devPS.BjEnalbe[i]>>8)&0xFF;
		data[j++] = devPS.BjEnalbe[i]&0xFF;
	}
	
	//�˿�����
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
	//��Ч��
	for(i=0; i<PORT_NUM; i++)
	{
		dat = buffer[j++];
		dat <<= 8;
		dat += buffer[j++];
		devPS.Valid[i] = dat;
	}
	//����
	for(i=0; i<PORT_NUM; i++)
	{
		dat = buffer[j++];
		dat <<= 8;
		dat += buffer[j++];
		devPS.BjEnalbe[i] = dat;
	}
	
	//�˿�����
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

//��ȡ�绰����
uint8 ReadPhoneParam(uint8 *data)
{
	uint8 i,j,k;
	j = 0;
	//�������
	data[j++] = 0;
	data[j++] = telenumoldbuf;
	
	//����
	for(i=0; i<10; i++)
	{
		for(k=0; k<11; k++)
		{
			data[j++] = gsmtelnumbuf[YongfuNumStart+i][k];
		}
	}
	
	//����Ȩ��
	data[j++] = (gsmtelsuper>>8)&0xFF;
	data[j++] = gsmtelsuper&0xFF;

	return j;
}

//д�绰����
uint8 WritePhoneParam(uint8 *buffer)
{
	uint16 i,j,k,dat;

	j = 1;
	//�������
	telenumnewbuf = buffer[j++];
	
	//����
	for(i=0; i<teleUserNumMax; i++)
	{
		WDR();//�忴�Ź�
		for(k=0; k<11; k++)
		{
			gsmtelnumbuf[YongfuNumStart+i][k] = buffer[j++];
		}
	}

	//����Ȩ��
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
	WDR();//�忴�Ź�

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

//���õ�ǰ�豸����,�����˿�12���豸������
void ProcessData(uint8 *Data, uint8 len)
{//һ������12���豸
	uint8 i,j;
	uint16 tmp;

	j = 0;
	//��ǰ�ɼ��Ķ˿���Ч
	if(devPS.Valid[lastPort] & 0x0FFF)
	{//�˿���Ч

		for(i=0; i<LINE_DEV_NUMS; i++)
		{
			//�¶�
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
			//״̬
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
	//��Ŀ����
	for(i=0; i<10; i++)
	{
		if(devPS.strPrjName[i]==0)
			break;
		*p1++ = (devPS.strPrjName[i]>>8)&0xFF;	//��8λ
		*p1++ = devPS.strPrjName[i]&0xFF;	//��8λ
	}
	k = i*2;

	return k;
}

uint8 FormatMsgPortName(uint16 *src, uint8 *buf)
{
	uint16 i,k;
	uint8 *p1;

	p1 = buf;
	//�˿�����
	for(i=0; i<5; i++)
	{
		if(src[i]==0)
			break;
		*p1++ = (src[i]>>8)&0xFF;	//��8λ
		*p1++ = src[i]&0xFF;	//��8λ
	}
	k = i*2;	//�ܳ��ȣ��ֽ���
	
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
// 	//��Ŀ����
// 	for(i=0; i<10; i++)
// 	{
// 		if(devPS.strPrjName[i]==0)
// 			break;
// 		*p1++ = (devPS.strPrjName[i]>>8)&0xFF;	//��8λ
// 		*p1++ = devPS.strPrjName[i]&0xFF;	//��8λ
// 	}
// 	k = i*2;
// 	//'_'
// 	*p1++ = 0;
// 	*p1++ = 0x5F;	//_
// 	k += 2;
// 	//�˿�����
// 	for(i=0; i<5; i++)
// 	{
// 		if(src[i]==0)
// 			break;
// 		*p1++ = (src[i]>>8)&0xFF;	//��8λ
// 		*p1++ = src[i]&0xFF;	//��8λ
// 	}
// 	k += i*2;	//�ܳ��ȣ��ֽ���
	
	return k;
}

void FormatMsg(SOE_STRUCT soe)
{
	uint8 k,len;
	
	switch(soe.type)
	{
	case SOE_TYPE_COMM:
		{//ͨ�Ź���
			k = FormatMsgPrjName((uint8 *)sCurSMS);
			sprintf(sssbuf,"(��ַ:%d):ͨѶ����!",soe.value1);
		}
		break;
	case SOE_TYPE_COMMX:
		{//ͨ�Ź��ϻָ�
			k = FormatMsgPrjName((uint8 *)sCurSMS);
			sprintf(sssbuf,"(��ַ:%d):ͨѶ�ָ�!",soe.value1);
		}
		break;
	case SOE_TYPE_ERR:
		{//�������쳣
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(�˿�%d):������%d�쳣(%d)",soe.port+1,soe.sensor+7,soe.value1);
		}
		break;
	case SOE_TYPE_ERRX:
		{//�������쳣
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(�˿�%d):������%d�ָ�(%d)",soe.port+1,soe.sensor+7,soe.value1);
		}
		break;
	case SOE_TYPE_BJ1:
		{//�¶ȱ���1
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(�˿�%d):������%d�¶�:%d,һ�㱨��.",soe.port+1,soe.sensor+7,soe.value1/100);
		}
		break;
	case SOE_TYPE_BJ1X:
		{//�¶ȱ���1
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(�˿�%d):������%d�¶�:%d,�˳�һ�㱨��.",soe.port+1,soe.sensor+7,soe.value1/100);
		}
		break;
	case SOE_TYPE_BJ2:
		{//�¶ȱ���1
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(�˿�%d):������%d�¶�:%d,���ر���.",soe.port+1,soe.sensor+7,soe.value1/100);
		}
		break;
	case SOE_TYPE_BJ2X:
		{//�¶ȱ���1
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(�˿�%d):������%d�¶�:%d,�˳����ر���.",soe.port+1,soe.sensor+7,soe.value1/100);
		}
		break;
	case SOE_TYPE_RISE:
		{//�����쳣
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(�˿�%d):������%dʮ��������%d��,�쳣����!�¶�:%d.",soe.port+1,soe.sensor+7,soe.value2/100,soe.value1/100);
		}
		break;
	case SOE_TYPE_RISEX:
		{//�����쳣
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(�˿�%d):������%dʮ��������%d��,�ָ�����!�¶�:%d.",soe.port+1,soe.sensor+7,soe.value2/100,soe.value1/100);
		}
		break;
	case SOE_TYPE_DELTA:
		{//�²��쳣
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(�˿�%d):�²�(%d)�쳣����������´�����%d,�¶�:%d.",soe.port+1,soe.value2/100,soe.sensor+7,soe.value1/100);
		}
		break;
	case SOE_TYPE_DELTAX:
		{//�²��쳣
			k = FormatMsgHead(devPS.strPortName[soe.port],(uint8 *)sCurSMS);
			sprintf(sssbuf,"(�˿�%d):�²�(%d)�ָ�����������´�����%d,�¶�:%d.",soe.port+1,soe.value2/100,soe.sensor+7,soe.value1/100);
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

//��ѯ��¼
void GSM_CXJL(uint8 nRec,uint8 groupno)
{
	SOE_STRUCT soe;
	
	if(ReadSoe(Soesearchnumbuf,&soe))
	{
		FormatMsg(soe);
	}
	else
	{
		AddMsgGroup("�޼�¼",groupno);
	}
}

void TX_run()
{
	//ͨѶ�ж�
	if(portCommErr.err==TX_ERR_COUNT)
	{//ͨѶ���ϴ���
		if(portCommErr.errold==0)
		{
			portCommErr.errold = 1;
			FormatSoe(SOE_TYPE_COMM, 0, 0, devPS.m_Addr, 0, 1);
		}
	}
	else if(portCommErr.err==0)
	{//ͨѶ����ʱ
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

	//1���ж�״̬
	for(i=0; i<PORT_NUM; i++)
	{

		for(j=0; j<LINE_DEV_NUMS; j++)
		{
			if(devPS.Valid[i]&(1<<j))
			{//��Ч�豸
				if(dev[i][j].status != 1)
				{//�¶�δˢ�£�
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
				//���ϴ���
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
							{//����ʹ��
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
					BjEnalbe[i] |= (1<<j);	//�ָ�����ʹ��
					FormatSoe(SOE_TYPE_ERRX, i, j, dev[i][j].status, 0, 1);
				}

				//��������
				if(devBj2[i][j].bj)
				{
					if(devBj2[i][j].bjold != devBj2[i][j].bj)
					{
						devBj2[i][j].bjold = devBj2[i][j].bj;
						devBj2[i][j].bjCount = dTCounter;
						FormatSoe(SOE_TYPE_BJ2, i, j, dev[i][j].temp, 0, 1);
					}
					if(devPS.BJ2Interval != 0)
					{//�������ٷ�һ�α���
						if(g_bPauseWarning==0)
						{
							devBj2[i][j].bjCount++;
//							if(BjEnalbe[i] & (1<<j))
							{//����ʹ��
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
				{//���ر���ʱ�����������
					if(devBj2[i][j].bjold != devBj2[i][j].bj)
					{//���ر����ָ�
						devBj2[i][j].bjold = devBj2[i][j].bj;
						FormatSoe(SOE_TYPE_BJ2X, i, j, dev[i][j].temp, 0, 1);
					}

					if(devBj1[i][j].bjold != devBj1[i][j].bj)
					{
						devBj1[i][j].bjold = devBj1[i][j].bj;
						devBj1[i][j].bjCount = dTCounter;
						//��¼���¼���
						FormatSoe(SOE_TYPE_BJ1, i, j, dev[i][j].temp, 0, 1);
					}
					if( devPS.BJ1Interval != 0 )
					{//�������ٷ�һ�α���
						if(g_bPauseWarning==0)
						{
							uint32 ul = 60ul * 1000 * devPS.BJ1Interval;

							if (CalcTickWidth(dTCounter, devBj1[i][j].bjCount) > ul )
							{
								devBj1[i][j].bjCount = dTCounter;
//								if(BjEnalbe[i] & (1<<j))
								{//����ʹ��
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
						BjEnalbe[i] |= (1<<j);	//�ָ�����ʹ��
						//��¼���¼���
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
	{//ͨѶ���ϣ��������ϴβ����¶�
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
			{//��Ч�豸
				if(dev[i][j].status == 1)
				{//�¶���Ч
					if(dev[i][j].temp > max)
					{
						max = dev[i][j].temp;
						k = j;
					}
					if(dev[i][j].temp < min)
						min = dev[i][j].temp;

					dev[i][j].deltaCount++;
					if(dev[i][j].deltaCount > 5L*60*10)
					{//�����
						dev[i][j].deltaCount = 0;
						if(dev[i][j].oldtemp != 0xFFFF)
						{
							if(dev[i][j].temp > dev[i][j].oldtemp)
							{
								dat = dev[i][j].temp-dev[i][j].oldtemp;
								if( dat > TEMP_RISER)
								{//��������³���8������
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
			{//����¶Ȳ�ƽ��
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

#define PWTIME	(2L*60*60*10)	//����Сʱ
//�ճ�����ģ��,100msִ��һ��
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

//��¼SOE����
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

//��ȡsoe
uint8 ReadSoe(uint8 rec_no,SOE_STRUCT *pSoe)
{
	uint8 i;
	uint8 *pBuf;
	uint16 addr;
	
	//�ж����
	if(rec_no > g_soe_num)
		rec_no = g_soe_num;

	if(g_soe_num==SOE_NUM_MAX)
	{//�Ѿ���������
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
	{//û������ʱ��,g_soe_pos=g_soe_num
		if(g_soe_num==0)
		{
			return 0;
		}
		i = g_soe_pos - rec_no;
	}

	pBuf = (uint8 *)&pSoe;
	//ȡ��Ҫ��ȡ�����λ��
	addr = EEP_ADDR_SOE + SOE_REC_SIZE*i;
	for(i=0; i<sizeof(SOE_STRUCT); i++)
	{
		*pBuf = EEPROM_Read(addr++);
		pBuf++;
	}
	return 1;
}

//��ѯ״̬
void GSM_CXZT(uint8 groupno)
{
	uint16 i,j,k,m,len;
	char *pbuf;
	
	pbuf = sCurSMS;

	//��Ŀ����
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
			{//�豸��Ч��
// 				k = FormatMsgPortName(devPS.strPortName[i], (uint8 *)pbuf);

				if(dev[i][j].status==1)
					sprintf(sssbuf,":S%d_�¶�:%d,)",j+7,dev[i][j].temp/100);
				else
					sprintf(sssbuf,":S%d_�쳣:%d,)",j+7,dev[i][j].status);

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

//��ѯ��Ч
void GSM_CXYX(uint8 qx,uint8 groupno)
{
	uint8 k;
	
	k = 0;
	k += sprintf(&sCurSMS[k],"�����ϱ����:%d����;",devPS.ErrInterval);
	k += sprintf(&sCurSMS[k],"һ�㱨���¶�:%d��,�������:%d����;",devPS.tempSet1/100,devPS.BJ1Interval);
	k += sprintf(&sCurSMS[k],"���ر����¶�:%d��,�������:%d����.",devPS.tempSet2/100,devPS.BJ2Interval);
	AddMsgGroup(sCurSMS,groupno);
}

//�޸���Ч
void GSM_XGYX(uint16 *pBuf)
{
	uint8 k;
	for(k=0; k<PORT_NUM; k++)
		devPS.Valid[k] = pBuf[k];
}

//��ѯ����
void GSM_CXSZ(uint8 qx,uint8 groupno)
{
	uint8 k;

	k = 0;
	k += sprintf(&sCurSMS[k],"�����ϱ����:%d����;",devPS.ErrInterval);
	k += sprintf(&sCurSMS[k],"һ�㱨���¶�:%d��,�������:%d����;",devPS.tempSet1/100,devPS.BJ1Interval);
	k += sprintf(&sCurSMS[k],"���ر����¶�:%d��,�������:%d����.",devPS.tempSet2/100,devPS.BJ2Interval);
	AddMsgGroup(sCurSMS,groupno);
}

//ɾ����¼
void GSM_SCJL(uint8 groupno)
{
	g_soe_num = 0;
	g_soe_pos = 0;
	EEPROM_Write(g_soe_pos,EEP_ADDR_SOE-2);
	EEPROM_Write(g_soe_num,EEP_ADDR_SOE-1);

	AddMsgGroup("ɾ�����м�¼�ɹ�!",groupno);
}

const uint8 UCS2_XMMC[]={0x98,0x79,0x76,0xEE,0x54,0x0D,0x79,0xF0,0,':'};	//��Ŀ����:
//��ѯ����
void GSM_CXMC(uint8 groupno)
{
	uint8 i,k,len;
	uint8 *p1;
	
	p1 = (uint8 *)&sCurSMS[0];
	for(i=0; i<sizeof(UCS2_XMMC)/sizeof(UCS2_XMMC[0]); i++)
	{
		*p1++ = UCS2_XMMC[i];
	}
	//��Ŀ����
	for(i=0; i<10; i++)
	{
		if(devPS.strPrjName[i]==0)
			break;
		*p1++ = (devPS.strPrjName[i]>>8)&0xFF;	//��8λ
		*p1++ = devPS.strPrjName[i]&0xFF;	//��8λ
	}
	*p1++ = 0;
	*p1++ = ';' ;	//; 

	for(i=0; i<6; i++)
	{
		*p1++ = 0x7A;	//��
		*p1++ = 0xEF;
		*p1++ = 0x53;	//��
		*p1++ = 0xE3;
		*p1++ = 0;
		*p1++ = 0x31 + i;
		*p1++ = 0;
		*p1++ = ':';
		for(k=0; k<5; k++)
		{
			if(devPS.strPortName[i][k]==0)
				break;
			*p1++ = (devPS.strPortName[i][k]>>8)&0xFF;	//��8λ
			*p1++ = devPS.strPortName[i][k]&0xFF;	//��8λ
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

//�޸���Ŀ����
void GSM_XGXMMC(uint8 *pBuf, uint8 len,uint8 groupno)
{
	uint8 j,t=0,i=0;
	uint8 datastart=0;//,bufdatanum;
	uint16 DATA;
	uint8 dat[20];

	WDR();//�忴�Ź�

	for(t=0; t<len; t++)
	{
		if(datastart==0)
		{
			if((pBuf[t]=='='))  //�ҳ����ݵ���ʼλ��
				datastart = 1;
		}
		else
		{
			dat[i] = pBuf[t];
			i++;
			if(i > 20)
			{
				AddMsgGroup("��Ŀ��������ʧ��!���ƹ���!���10����.",groupno);
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
	AddMsgGroup("��Ŀ�������óɹ�!",groupno);
}

//�޸Ķ˿�����
void GSM_XGDKMC(uint8 port,uint8 *pBuf, uint8 len,uint8 groupno)
{
	uint8 j,t=0,i=0;
	uint8 datastart=0;//,bufdatanum;
	uint16 DATA;
	uint8 dat[20];
	
	WDR();//�忴�Ź�
	
	for(t=0; t<len; t++)
	{
		if(datastart==0)
		{
			if((pBuf[t]=='='))  //�ҳ����ݵ���ʼλ��
				datastart = 1;
		}
		else
		{
			dat[i] = pBuf[t];
			i++;
			if(i > 10)
			{
				sprintf(sssbuf,"�˿�%d��������ʧ��!���ƹ���!���5����.",port+1);
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

	sprintf(sssbuf,"�˿�%d�������óɹ�!",port+1);
	AddMsgGroup(sssbuf,groupno);
}

//��ͣ����
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
		AddMsgGroup("��ͣ�����ɹ�!2Сʱ��ָ���������!",groupno);
	}
	else
	{
		AddMsgGroup("��ǰû�б���,��ͣ����ʧ��!",groupno);
	}
}





