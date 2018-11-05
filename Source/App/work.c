
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
uint32 Nodatatratbuf;	//������û������ͨѶ��ʱ�á�
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
	{//��ʼ����־
		eepbufflag.bit.baohuworkf = 1;	// ��ʼ��ʱ�趨������־
		mokbaohutimebuf = 0;
	}
	if(eepbufflag.bit.baohuworkf)
	{//������־
		mokbaohutimebuf++;
		if(mokbaohutimebuf > 600000) //600��
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
	{//дeeprom��־
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
	{//������־
		MC52reSETTIMEBUF++;
		if(MC52reSETTIMEBUF > 2000)
		{//2������������־�����ó�ʼ����־
			MC52reSETTIMEBUF=0;
			statusflag0.Word = 0;	//hyk
			statusflag0.bit.mc52iintf = 1;
			eepbufflag.bit.GsmFuweisendf = 0;
			CLRBIT(funcflag3,mc52RESTf);
			CLRBIT(mc52igprsflag,mc52igprsnetoking);//��̨�������ߣ������Ҫ��ͣ��ȥ����
			CLRBIT(mc52igprsflag,mc52igprsworking);
			CLRBIT(mc52iworkflag,mc52gsmworkingf);
			CLRBIT(mc52iworkflag,mc52gprsworkingf);
			MC52IPOWEROFF;	 
		}
	}
	else
		MC52reSETTIMEBUF = 0;

	if(CHKBITSET(Gsmstatureflag,gsmguajif))
	{//�һ����� ��ʱ
		ATHtimebuf++;
		if(ATHtimebuf > 0)
		{
			CLRBIT(Gsmstatureflag,gsmguajif);
			GPRS_AT_ATH_SUB(pComm);
		}
	}
	else
		ATHtimebuf = 0;
	
	net_led_work_sub();	// ָʾ�ƴ���

	Nodatatratbuf++;
	if(Nodatatratbuf > 360000000ul)
	{
		Nodatatratbuf = 0;
		Mc52i_init();	//���³�ʼ��
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
			MC52IPOWERON;			// �򿪵�Դ���͵�ƽҪ�㹻��������3.2S
			statusflag0.bit.mc52iigtf = 1;	//��ʼ����־2 igt �ϵ紦��
			MC52IRESTON;			// IGT�ȸߵ�ƽ
		}
	}
	if(statusflag0.bit.mc52iigtf)
	{
		GPRS_IGTTIME++;    //��𱣳ֵ͵�ƽʱ��
		if(GPRS_IGTTIME > GPRS_DIANHUO_TIME_SET)
		{
			GPRS_IGTTIME = 0;
			MC52IRESTOFF;		//IGT����Ҫ�͵�ƽ100ms,�ָ��ߵ�ƽ
			statusflag0.bit.mc52iigtf = 0;
			SETBIT(mc52igprsflag,mc52isenddelay);	// MC52I �����һ����־
			//��ʱ������ɣ������ж��Ƿ���������
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
		SETBIT(funcflag5,mc52icxnjtdelayf); //��������
	}
	if(CHKBITSET(funcflag5,mc52icxnjtdelayf))
	{
		CLRBIT(funcflag5,mc52icxnjtdelayf); //��������
		CLRBIT(mc52igprsflag,mc52igprsnetoking);//��̨�������ߣ������Ҫ��ͣ��ȥ����
		CLRBIT(mc52igprsflag,mc52igprsworking);
		Gprsststuretypebuf = 0; //����������ķ���
		SETBIT(Mc52igprsipsetflag,closemgprsf);//����һ��رպ��ٴ�
	}
	if(CHKBITSET(funcflag7,gprsnettotestf)) //��ʼ����ʱ��������̫��ʱ��û��Ӧ�͹���
	{
		Gprstesttimedelaybuf++;            //����Ҫ���ǵ���Ϊ�˱������ô������������
		if(Gprstesttimedelaybuf > 1000)
		{
			Gprstesttimedelaybuf = 0;
			Gprstesttime2delaybuf++;
			if(Gprstesttime2delaybuf > 120)
			{
				Gprstesttime2delaybuf = 0;
				CLRBIT(funcflag7,gprsnettotestf); //��ʼ����ʱ��������̫��ʱ��û��Ӧ�͹���
				   SETBIT(funcflag5,mc52iqhnjtdelayf);//ֻ�������ط��л�
				Gprsbaohutimebuf++;
				if(Gprsbaohutimebuf > 720)   //24Сʱ=1��
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
	/*************************��Ͼʱ��������ȥ������************/
	if(CHKBITSET(mc52igprsflag,mc52igprsnetoking))
	{
		Gprstesttimedelaybuf = 0;
		Gprstesttime2delaybuf = 0;
		Gprsbaohutimebuf = 0;
		GPRS_cyreaddatatime++; //����ȥ������
		if(GPRS_cyreaddatatime > 10000) //10���Ӳ���
		{
			GPRS_cyreaddatatime = 0;
			SETBIT(MC52istaflag,readmc52idataf);//���ڶ����ݣ���©���ݡ�
		}
		GPRS_cydatatimebuf++; //���������Ӻú�Ҫ��3��������û��������ͨ
		if(GPRS_cydatatimebuf > 300000ul) //300���Ӳ���
		{
			GPRS_cydatatimebuf = 0;   //���������Ӻú�Ҫ��3��������û��������ͨ
			SETBIT(funcflag5,mc52icxnjtdelayf); //��������
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
				SETBIT(mc52iworkflag,gsmchongxingdata); //���·���

				GSM_Send_num_sub(pComm,GsmEORRcfnumbuf);
				SETBIT(mc52iworkflag,mc52gsmsendbaohuf);//����ָ�������
			}
			else
			{//���η����޻�Ӧ�������³�ʼ������
				Gsmstature4flag.bit.gsmbaohuworkingf = 0;
				CLRBIT(mc52iworkflag,gsmchongxingdata); //���·���
				mc52iworkflag = 0;
				funcflag5 = 0;
				GsmEORRtimebuf=0;
				Mc52i_CMD_flag1.bit.atsendf = 1;	//��һ��ָ���AT
				Mc52i_CMD_flag1.bit.first_connect = 0;
			}
		}
	}
	else
	{
		senddataGSMbaohutime = 0;
	}
}

//ָʾ�ƴ���
void net_led_work_sub(void)
{
	if(CHKBITSET(mc52igprsflag,mc52igprsnetoking)||CHKBITSET(mc52iworkflag,mc52gsmworkingf))
	{
		DATALEDON;	//��1�̵Ƶ���,ע��
		if(CHKBITSET(funcflag5,mc52havetrancedataled))
		{
			netlestimebuf++;
			datalestimebuf=0;
			NETLEDON;	//��2�̵Ƶ���
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
	//��ȡ������
	Baudsetbuf = EEPROM_Read(baudaddh);
	Baudsetbuf <<= 8;
	Baudsetbuf += EEPROM_Read(baudaddl);

	if(Baudsetbuf>65530)
	{
		Baudsetbuf=oldbauddata;
		eepbufflag.bit.baudeepf = 1;
	}

	//��ȡ����ģʽ0--GSM,1--GPRS
	MC52iworkstatusbuf=EEPROM_Read(mc52workmodeladd);
	if(MC52iworkstatusbuf > 250)
	{
		MC52iworkstatusbuf=oldmc52model;
		eepbufflag.bit.Modeleepf = 1;
	}
	//��ȡ���������
	telenumoldbuf = EEPROM_Read(telenumdataadd);	// �û��������
	teletolnumbuf = telenumoldbuf+YongfuNumStart;	// �û��������+�����������
	if(telenumoldbuf > 20)
	{//Ҳ����˵�û��������14��
		telenumoldbuf = 0; //���д漸�����
		teletolnumbuf =  YongfuNumStart;
		eepbufflag.bit.telenumf = 1;
	}
	//��ȡ����Ȩ��
	gsmtelsuper = EEPROM_Read(telenumsuperadd);

}

