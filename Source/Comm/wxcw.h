// wxcw.h

#ifndef __WXCW_H__001
#define __WXCW_H__001

#include "modbusm.h"

#define MAX_STRUCT_COUNT  0x200
#define DEV_BLOCK_SIZE	6	//6����=12���ֽ�
#define LINE_DEV_NUMS	12	//ÿ�����߽�12��
#define PORT_NUM		6	//�˿���

enum
{
	SOE_TYPE_COMM = 0,
	SOE_TYPE_COMMX,
	SOE_TYPE_ERR,
	SOE_TYPE_ERRX,
	SOE_TYPE_BJ1,
	SOE_TYPE_BJ1X,
	SOE_TYPE_BJ2,
	SOE_TYPE_BJ2X,
	SOE_TYPE_RISE,
	SOE_TYPE_RISEX,
	SOE_TYPE_DELTA,
	SOE_TYPE_DELTAX,
	SOE_TYPE_NUM
};

typedef struct  
{
	uint8 year;
	uint8 mon;
	uint8 day;
	uint8 hour;
	uint8 minu;
	uint8 sec;
	uint8 port;		//�˿�
	uint8 sensor;	//������
	uint8 type;		//��������
	int16 value1;	//������ֵ1
	int16 value2;	//������ֵ2
}SOE_STRUCT;
extern SOE_STRUCT g_soe;
#define SOE_REC_SIZE	(sizeof(SOE_STRUCT)+1)
#define SOE_NUM_MAX		200

typedef struct  
{
	uint16 valid;			//��Ч��Ч
	int16 temp;			//�¶ȣ�0.01��
	uint16 db;			//dbֵ
	uint16 status;		
//	������״̬.״̬0ʱ��ˢ�´�����״̬������ʾ�����������ߡ�
// 	״̬1ʱ��ˢ�´�����״̬������ʾ����������������ˢ���¶���ʾֵ��
// 	״̬2ʱ��ˢ�´�����״̬������ʾ��������������������ˢ���¶���ʾֵ��
// 	״̬3ʱ��ˢ�´�����״̬������ʾ��������������������ˢ���¶���ʾֵ��
//	״̬4/5ʱ��ˢ�´�����״̬������ʾ����������ʧ�ܣ�ˢ���¶�Ϊ��Чֵ��
	int16 oldtemp;			//�¶ȣ�0.01��
	int16  deltaTemp;	//�¶ȱ仯
	uint32 deltaCount;	//��ʱ
	uint8 rise:1;		//�����쳣״̬
	uint8 diff:1;		//�²��쳣״̬
}DEVICE_DATA;
extern DEVICE_DATA dev[PORT_NUM][LINE_DEV_NUMS];

typedef struct  
{
	uint16 temp1;		//�¶ȶ�ֵ1����λ0.01�ȡ�0--���ж�
	uint16 temp2;		//�¶ȶ�ֵ2����λ0.01�ȡ�0--���ж�
}DEV_SET_TEMP;

//�����ṹ
typedef struct  
{
	//��������
	uint16 strPrjName[10];		//��Ŀ���ƣ��10���֡��磺XX������,318֧��,UCS2��ʽ
	uint16 ErrInterval;			//�����ϱ��������λ�����ӡ���������ÿ������ٱ���һ��.Ϊ0ʱ��ֻ����һ�Ρ�
	uint16 tempSet1;			//�¶ȶ�ֵ1����λ0.01�ȡ�0--���ж�
	uint16 BJ1Interval;			//�ϱ��������λ�����ӡ���������ÿ������ٱ���һ��.Ϊ0ʱ��ֻ����һ�Ρ�
	uint16 tempSet2;			//�¶ȶ�ֵ2����λ0.01�ȡ�0--���ж�
	uint16 BJ2Interval;			//�ϱ��������λ�����ӡ���������ÿ������ٱ���һ��.Ϊ0ʱ��ֻ����һ�Ρ�
	uint8  m_Addr;				//װ�õ�ַ
	uint8 m_CommNo;				//��ɼ�����ͨѶ�˿ڣ�0--232,1--485
	uint16 diffTemp;			//����²���趨ֵ
	uint16 deltaTemp;			//10�������������趨ֵ
}BASE_PARAM_STRUCT;
// extern BASE_PARAM_STRUCT devBase;

typedef struct  
{
	//�˿ڲ���
	uint16 strPortName[PORT_NUM][5];		//�������ر������,��703���5����,UCS2��ʽ
	uint16 Valid[PORT_NUM];		//��Ч��,ÿ��Ԫ�ش���ö˿ڵ������豸�Ƿ�����,ÿһλ����һ���豸��0--��Ч��1--��Ч
	uint16 BjEnalbe[PORT_NUM];	//�Ƿ񱨾�,ÿ��Ԫ�ش���ö˿ڵ������豸�Ƿ񱨾�,ÿһλ����һ���豸��0--��������1--����
}PORT_PARAM_STRUCT;
// extern PORT_PARAM_STRUCT devPort;


typedef struct  
{
	//��������
	uint16 strPrjName[10];		//��Ŀ���ƣ��10���֡��磺XX������,318֧��,UCS2��ʽ
	uint16 ErrInterval;			//�����ϱ��������λ�����ӡ���������ÿ������ٱ���һ��.Ϊ0ʱ��ֻ����һ�Ρ�
	uint16 tempSet1;			//�¶ȶ�ֵ1����λ0.01�ȡ�0--���ж�
	uint16 BJ1Interval;			//�ϱ��������λ�����ӡ���������ÿ������ٱ���һ��.Ϊ0ʱ��ֻ����һ�Ρ�
	uint16 tempSet2;			//�¶ȶ�ֵ2����λ0.01�ȡ�0--���ж�
	uint16 BJ2Interval;			//�ϱ��������λ�����ӡ���������ÿ������ٱ���һ��.Ϊ0ʱ��ֻ����һ�Ρ�
	uint8  m_Addr;				//װ�õ�ַ
	uint8 m_CommNo;				//��ɼ�����ͨѶ�˿ڣ�0--232,1--485
	uint16 diffTemp;			//����²���趨ֵ
	uint16 deltaTemp;			//10�������������趨ֵ

	//�˿ڲ���
	uint16 strPortName[PORT_NUM][5];		//�������ر������,��703���5����,UCS2��ʽ
	uint16 Valid[PORT_NUM];		//��Ч��,ÿ��Ԫ�ش���ö˿ڵ������豸�Ƿ�����,ÿһλ����һ���豸��0--��Ч��1--��Ч
	uint16 BjEnalbe[PORT_NUM];	//�Ƿ񱨾�,ÿ��Ԫ�ش���ö˿ڵ������豸�Ƿ񱨾�,ÿһλ����һ���豸��0--��������1--����
// 	//�绰����
// 	uint8 teleNum;				//�������
// 	uint8 telenumber[10][11];	//�绰����
// 	uint16 teleSuperNo;		//������������
}DEV_PARAM_STRUCT;


extern DEV_PARAM_STRUCT devPS;

typedef struct  
{
	uint8 err;
	uint8 errold;
}PORT_COMM_ERR;

typedef struct  
{
	uint8 err:1;
	uint8 errold:1;
	uint32 errCount;	//�ڴ������ݲ��ȵ�ʱ�򣬿�ʼ��ʱ����ʱʱ�䵽���򷢱���
}DEV_ERR;

extern DEV_ERR devErr[PORT_NUM][LINE_DEV_NUMS];

typedef struct  
{
	uint16 bjValue;	//��ǰ�¶���ֵ����λ��0.01��
	uint8 bj:1;		//����״̬,0--�ޱ�����1--�б���
	uint8 bjold:1;		//����
	uint32 bjCount;	//�ڴ������ݲ��ȵ�ʱ�򣬿�ʼ��ʱ����ʱʱ�䵽���򷢱���
}DEV_BJ;

void WXCW_Run(void);

void init_setTemp(DEV_SET_TEMP st);
void Get_Init_temp_data(void);
void ProcessData(uint8 *Data, uint8 len);
int GetModbusStruct(MODBUSINFO* ModbusInfo);
void SetCommStatusErr(uint8 err);
void GSM_CXYX(uint8 qx,uint8 groupno);
void GSM_XGYX(uint16 *pBuf);

void GSM_CXSZ(uint8 qx,uint8 groupno);
void GSM_SCJL(uint8 groupno);
void GSM_CXZT(uint8 groupno);
void GSM_CXJL(uint8 nRec,uint8 groupno);
uint8 ReadSoe(uint8 soe_no,SOE_STRUCT *pSoe);
void GSM_CXMC(uint8 groupno);
void GSM_ZTBJ(uint8 groupno);
void GSM_XGXMMC(uint8 *pBuf, uint8 len,uint8 groupno);
void GSM_XGDKMC(uint8 port,uint8 *pBuf, uint8 len,uint8 groupno);
uint8 getCommNO(void);

uint8 ReadPortData(uint16 addr,uint8 *data);
uint8 ReadBaseParam(uint8 *data);
uint8 WriteBaseParam(uint8 *data);
uint8 ReadPortParam(uint8 *data);
uint8 WritePortParam(uint8 *data);
uint8 ReadPhoneParam(uint8 *data);
uint8 WritePhoneParam(uint8 *data);

#endif	//#define __WXCW_H__001

