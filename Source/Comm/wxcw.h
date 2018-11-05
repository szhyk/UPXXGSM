// wxcw.h

#ifndef __WXCW_H__001
#define __WXCW_H__001

#include "modbusm.h"

#define MAX_STRUCT_COUNT  0x200
#define DEV_BLOCK_SIZE	6	//6个字=12个字节
#define LINE_DEV_NUMS	12	//每根天线接12个
#define PORT_NUM		6	//端口数

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
	uint8 port;		//端口
	uint8 sensor;	//传感器
	uint8 type;		//故障类型
	int16 value1;	//故障数值1
	int16 value2;	//故障数值2
}SOE_STRUCT;
extern SOE_STRUCT g_soe;
#define SOE_REC_SIZE	(sizeof(SOE_STRUCT)+1)
#define SOE_NUM_MAX		200

typedef struct  
{
	uint16 valid;			//有效无效
	int16 temp;			//温度，0.01度
	uint16 db;			//db值
	uint16 status;		
//	传感器状态.状态0时，刷新传感器状态，且显示传感器不在线。
// 	状态1时，刷新传感器状态，且显示传感器测量正常，刷新温度显示值。
// 	状态2时，刷新传感器状态，且显示传感器测量正常，但不刷新温度显示值。
// 	状态3时，刷新传感器状态，且显示传感器测量正常，但不刷新温度显示值。
//	状态4/5时，刷新传感器状态，且显示传感器测量失败，刷新温度为无效值。
	int16 oldtemp;			//温度，0.01度
	int16  deltaTemp;	//温度变化
	uint32 deltaCount;	//计时
	uint8 rise:1;		//升温异常状态
	uint8 diff:1;		//温差异常状态
}DEVICE_DATA;
extern DEVICE_DATA dev[PORT_NUM][LINE_DEV_NUMS];

typedef struct  
{
	uint16 temp1;		//温度定值1，单位0.01度。0--不判断
	uint16 temp2;		//温度定值2，单位0.01度。0--不判断
}DEV_SET_TEMP;

//参数结构
typedef struct  
{
	//公共参数
	uint16 strPrjName[10];		//项目名称，最长10个字。如：XX环网柜,318支线,UCS2格式
	uint16 ErrInterval;			//故障上报间隔，单位：分钟。即报警后每隔多久再报下一次.为0时，只报警一次。
	uint16 tempSet1;			//温度定值1，单位0.01度。0--不判断
	uint16 BJ1Interval;			//上报间隔，单位：分钟。即报警后每隔多久再报下一次.为0时，只报警一次。
	uint16 tempSet2;			//温度定值2，单位0.01度。0--不判断
	uint16 BJ2Interval;			//上报间隔，单位：分钟。即报警后每隔多久再报下一次.为0时，只报警一次。
	uint8  m_Addr;				//装置地址
	uint8 m_CommNo;				//与采集器的通讯端口，0--232,1--485
	uint16 diffTemp;			//相间温差报警设定值
	uint16 deltaTemp;			//10分钟温升报警设定值
}BASE_PARAM_STRUCT;
// extern BASE_PARAM_STRUCT devBase;

typedef struct  
{
	//端口参数
	uint16 strPortName[PORT_NUM][5];		//各个开关编号名称,如703，最长5个字,UCS2格式
	uint16 Valid[PORT_NUM];		//有效性,每个元素代表该端口的所有设备是否配置,每一位代表一个设备。0--无效，1--有效
	uint16 BjEnalbe[PORT_NUM];	//是否报警,每个元素代表该端口的所有设备是否报警,每一位代表一个设备。0--不报警，1--报警
}PORT_PARAM_STRUCT;
// extern PORT_PARAM_STRUCT devPort;


typedef struct  
{
	//公共参数
	uint16 strPrjName[10];		//项目名称，最长10个字。如：XX环网柜,318支线,UCS2格式
	uint16 ErrInterval;			//故障上报间隔，单位：分钟。即报警后每隔多久再报下一次.为0时，只报警一次。
	uint16 tempSet1;			//温度定值1，单位0.01度。0--不判断
	uint16 BJ1Interval;			//上报间隔，单位：分钟。即报警后每隔多久再报下一次.为0时，只报警一次。
	uint16 tempSet2;			//温度定值2，单位0.01度。0--不判断
	uint16 BJ2Interval;			//上报间隔，单位：分钟。即报警后每隔多久再报下一次.为0时，只报警一次。
	uint8  m_Addr;				//装置地址
	uint8 m_CommNo;				//与采集器的通讯端口，0--232,1--485
	uint16 diffTemp;			//相间温差报警设定值
	uint16 deltaTemp;			//10分钟温升报警设定值

	//端口参数
	uint16 strPortName[PORT_NUM][5];		//各个开关编号名称,如703，最长5个字,UCS2格式
	uint16 Valid[PORT_NUM];		//有效性,每个元素代表该端口的所有设备是否配置,每一位代表一个设备。0--无效，1--有效
	uint16 BjEnalbe[PORT_NUM];	//是否报警,每个元素代表该端口的所有设备是否报警,每一位代表一个设备。0--不报警，1--报警
// 	//电话号码
// 	uint8 teleNum;				//号码个数
// 	uint8 telenumber[10][11];	//电话号码
// 	uint16 teleSuperNo;		//超级号码设置
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
	uint32 errCount;	//在错误数据不等的时候，开始计时，计时时间到，则发报警
}DEV_ERR;

extern DEV_ERR devErr[PORT_NUM][LINE_DEV_NUMS];

typedef struct  
{
	uint16 bjValue;	//当前温度数值，单位：0.01度
	uint8 bj:1;		//报警状态,0--无报警，1--有报警
	uint8 bjold:1;		//报警
	uint32 bjCount;	//在错误数据不等的时候，开始计时，计时时间到，则发报警
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

