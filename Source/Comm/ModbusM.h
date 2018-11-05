// ModbusM.h

#ifndef __MODBUSM_H__001
#define __MODBUSM_H__001
#define MAX_STRUCT_COUNT  0x200


//modbus���ݣ����ֽ���ǰ�����ֽ��ں�CRC--���ֽ���ǰ�����ֽ��ں�

typedef struct  
{
	uint16  nType;//�����Ai Di,Do
	uint16  DeviceAddr;//�豸��ַ
	uint16  FunCode;//������
	uint16  Regiseter;//�Ĵ���
	uint16  SysInfoNo;//��ϵͳ�е����
	uint16  RegNumbers;//�Ĵ�������
	uint16  CloseCode;//ң��ʹ�õĺ���
	uint16  OpenCode;//ң��ʹ�õ�ң����
	uint32  PreTick;//��һ�η��͵�Tick,ͨ�������ȷ����ǰ�Ƿ���Ҫ����
	uint16  ErrorNumbers;//����û�л�Ӧ�Ĵ���
}MODBUSINFO;//����ṹ��Ҫ�Ǵ��ݲ����ķ���
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

int  GetNextSendIndex(uint8 nComNo);//�õ���һ����Ҫ���͵Ľṹ, �������-1��ʾû����Ҫ���͵Ľṹ
void FormatSendFrame(uint8* Data, uint16* DataLen, MODBUSINFO ModbusInfo);

#endif	//#define __MODBUSM_H__001

