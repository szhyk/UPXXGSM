
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
MC55i���������̼�ATָ���˳��

	1��  ����GPRS��Դ��PWC   ��PWON��PWOFF��

	2��  ����EMERGOFF�ܽţ�Ҫ�����ṩһ������3.2��ĵ͵�ƽ�źţ�PDON��PDOFF��

	3��  �ж�ģ���Ƿ�������������VDDΪ�͵�ƽʱ��ģ����������MC55��IGTOFF��

	4��  ����IGT�ܽţ�Ҫ�����ṩһ������100����ĵ͵�ƽ�źţ�IGTON��IGTOFF��

	5��  �ж�ģ���Ƿ�������������VDDΪ�ߵ�ƽʱ��ģ������������
	��ATָ�
	1��  AT  ��ȷ�ϴ����Ƿ�������ģ�鴮�ڳ�ʼ��Ϊ����Ӧ�ģ�
	2��  AT+IPR=57600
	3��  AT^SSYNC=1
	4��  AT&C2

	6��  GETCSQ �� 
	1��AT+CSQ  ��ѯ�����ź�ǿ�� 
	�������ĳ�ʼ������֮����Ҫʹ��AT^SICSָ��������ģ������Internet�Ľű�
	AT^SICSָ� 
	1��AT^SICS=0,CONTYPE,GPRS0    0��ʾ�ű��ı�ţ������6�����ֱ�Ϊ0~5
	conType��ʾ����GPRS0�����ӷ�ʽ��GPRS
	ע������GPRS0�⣬������ѡ��CSD���ӷ�ʽ
	2��AT^SICS=0,APN        ����APN
	3��AT^SICS=0,USER       ����APN�û���
	4��AT^SICS=0,PASSWD     ����APN����

	���Internet���ӽű����ú���Ҫ����Internet����ű����������
	5��AT^SISS=0,SRVTYPE,SOCKET     ��0 ������ƽ̨����������Ϊsocket
	6��AT^SISS=0,CONID,0               ��0 ������ƽ̨��ʹ�õ�����ƽ̨Ϊ0

	7������IP���˿ںţ� AT^SISS=0,address,socktcp  �ȵ�
	8��AT^SISC=0   �ر�GPRS0 ����  

	7��LINKING ��AT^SISO=0    ����GPRS0����

	8��LINKEND ��^SISW: 0, 1  ����������Ӧ���յ����Ѿ����ͳɹ�  
	��¼�ɹ���Mc55WorkStatus = MC55_LINKOK
	ChannelStatus = CHANNEL_STATUS_OK
*/
extern void init_app(void);
extern uint8 Mc52i_rst_count;	// ��λ����
extern uint32 Mc52i_rst_time;	// ��λʱ��
void Mc52i_init()
{
	init_app();
	CLRBIT(mc52iworkflag,mc52gsmworkingf);
	CLRBIT(mc52iworkflag,mc52gprsworkingf);
	statusflag0.Word = 0;
	Mc52i_CMD_flag1.Word = 0;
	eepbufflag.Word = 0;
	statusflag0.bit.mc52iintf = 1;	//�趨��ʼ����־ 1
	Mc52i_rst_time = 0;
	MC52IPOWEROFF;	// �رյ�Դ
	WDR();
    SleepSec(5);
    
    MC52IPOWERON;	//MC52�ϵ�
    
    SleepSec(5);
    WDR();
    
    //����,����
    MC52IRESTON;
    SleepSec(5);
    MC52IRESTOFF;
    
    //�ȴ�����5S
    SleepSec(10);
	Mc52i_CMD_flag1.bit.atsendf = 1;	//��һ��ָ���AT
	
}

/****************************************************
*****************************************************/
void Get_int_data_sub(void)
{
	uint16 i;
	if(EEPROM_Read(0) != 0x3A)
	{//δ��ʼ����
		EEPROM_Write(0x3A,0);
		for(i=1; i<700; i++)
			EEPROM_Write(0,i);
	}
    eeprom_read_sub();
	//��ȡ�û��趨�ĺ���
	for(i=YongfuNumStart; i<teletaolnumbuf; i++)
		EEPROM_tel_Read(i);

//	Gsmmodeselect = MC52GSMTEXT; //ѡ��ΪGSM��TEXT�ı�ģʽ
	Gsmmodeselect = MC52GSMPDU; //ѡ��ΪGSM��TEXT�ı�ģʽ hyk
	SendStrToBuf((char*)&gsmtelnumbuf[0][0],"00000000000");// 
	SendStrToBuf((char*)&gsmtelnumbuf[1][0],"13510908672");//����1���� hyk
	SendStrToBuf((char*)&gsmtelnumbuf[2][0],"15820994887");//����2���� hcf
	SendStrToBuf((char*)&gsmtelnumbuf[3][0],"18899741276");//����3���� test
	SendStrToBuf((char*)&gsmtelnumbuf[4][0],"13923789584");//����4���� ����
	SendStrToBuf((char*)&gsmtelnumbuf[5][0],"13423484035");//����5���� lmj

}

//�����ĵĺ��룬һ��һ��д��EEPROM��
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
		{//δ����ʼ���������ú���Ϊ'0'
			gsmtelnumbuf[telnum][i] = 0;
			Gsmstature4flag.Word |= (1<<(telnum-YongfuNumStart));
		}
		else
		{
			gsmtelnumbuf[telnum][i] = j;
		}
	}
}

//д����
void EEPROM_tel_write (uint8 telnum)
{
    uint8 i=0;
	for(i=0;i<11;i++)
	{
		WDR();//�忴�Ź�
		EEPROM_Write(gsmtelnumbuf[telnum][i],telenumstartaddr+(telnum-YongfuNumStart)*11+i) ;
	}
	
}

