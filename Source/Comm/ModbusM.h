// ModbusM.h

#ifndef __MODBUSM_H__001
#define __MODBUSM_H__001
#define MAX_STRUCT_COUNT  0x200


//modbus数据，高字节在前，低字节在后。CRC--低字节在前，高字节在后。

typedef struct  
{
	uint16  nType;//这个是Ai Di,Do
	uint16  DeviceAddr;//设备地址
	uint16  FunCode;//功能码
	uint16  Regiseter;//寄存器
	uint16  SysInfoNo;//在系统中的序号
	uint16  RegNumbers;//寄存器数量
	uint16  CloseCode;//遥控使用的合码
	uint16  OpenCode;//遥控使用的遥分码
	uint32  PreTick;//上一次发送的Tick,通过这个来确定当前是否需要发送
	uint16  ErrorNumbers;//发送没有回应的次数
}MODBUSINFO;//这个结构主要是传递参数的方便
typedef struct  
{
	MODBUSINFO MbStruct;
	int8 Index;
	uint8 bInit;
}MODBUS_STRUCT;

extern MODBUS_STRUCT MODBS[];
uint16 Crc16Code(uint8 *pInFrame, uint16 dwInLen);

void InitModbus_Master_Com(MODBUS_STRUCT *pModbus);

void FormatAiFrame(uint8* Data, uint16* DataLen, MODBUSINFO ModbusInfo);
void FormatDiFrame(uint8* Data, uint16* DataLen, MODBUSINFO ModbusInfo);
void FormatDoFrame(uint8* Data, uint16* DataLen, MODBUSINFO ModbusInfo, uint16 DOInfoNo, uint16 bClose);

void DealFrame(uint8 nComNo, uint8* Data, uint16 DataLen, MODBUSINFO ModbusInfo);
void DealAiFrame(uint8 nComNo, uint8* Data, uint16 DataLen, MODBUSINFO ModbusInfo);
void DealDiFrame(uint8 nComNo, uint8* Data, uint16 DataLen, MODBUSINFO ModbusInfo);
void DealDoFrame(uint8 nComNo, uint8* Data, uint16 DataLen, MODBUSINFO ModbusInfo);

void ProModbusFrame_Master(COM_STRUCT *l_pCom,MODBUS_STRUCT *pModbus,uint8 recflag);

int  GetNextSendIndex(uint8 nComNo);//得到下一个需要发送的结构, 如果返回-1表示没有需要发送的结构
void FormatSendFrame(uint8* Data, uint16* DataLen, MODBUSINFO ModbusInfo);

#endif	//#define __MODBUSM_H__001

