#ifndef COMPROTOCOL_H
#define COMPROTOCOL_H


//���Ͷ���
typedef unsigned char	u8
typedef int				s16
typedef unsigned int	u16
typedef unsigned long	u32
typedef unsigned char	BYTE


#define MAX_COMNUM  3
#define NORMALBUF_SIZE  256
#define MAX_DATALEN  255
#define PERDEVICE_COMMANDLEN  12

//ͨ��������������
enum
{
	COMM_AI_TYPE = 0,
	COMM_DI_TYPE,
	COMM_DO_TYPE,
	COMM_CI_TYPE,
	COMM_PTTYPE_NUM
};


#define PARAM_START_BASE_ADDR 0xa000
#define MAX_PARAMNUM     0x3000
#define MAX_ASDUDATALEN  246
#define MAX_ONEFRAME_PARAMSIZE   0x50
#ifndef SUPERPASSWORD
	#define  SUPERPASSWORD  12315 
#endif 

#define MAX_SENDFRAMELEN 210
#define MAX_ONEFRAME_COUNT  20//���͵�Frame��������Frame������

#define MONITOR_TX_TYPE 0
#define MONITOR_RX_TYPE 1

#undef NULL
#define NULL   0

#define MAX_STEP_DATA  10000
#define CHANGE_STEP    10
#define INVALID_AI_VALUE 0xFFFF
#define INVALID_CI_VALUE 0xFFFFFFFF

#define ONE_PAGE_BYTE_SIZE  252

#define ONE_FRAME_MAX_PARAM_SIZE 123


#define MAX_GROUP 5

typedef struct //���ڽ��շ������ݶ�ʹ�����������
{
	u8* l_Buffer;
	u16  l_InPos;
	u16  l_OutPos;
	u16  l_Buffer_size;
}BUFFERINFO;

typedef struct 
{
	u16 m_Addr;//��ַ
	u8 ComNo;
	u8 ProtocolType;//Э������
	
	u8 *m_bRecvOverFlag;	//���ս�����־
	
	BUFFERINFO   *m_RXBuffer;
	BUFFERINFO   *m_TXBuffer;
	
	BYTE* m_pRecvProBuffer;
	BYTE* m_pSendProBuffer;	// ����֡������
	u16  m_nSendLen;		//�ϴη��͵ĳ��ȣ�101Э��ʹ��
	u32 m_TickCount;//����յ����ݵ�TickCount
	u32 m_CheckTickCount;//�����������ʱ��ʱ���־
	
	u8 m_bCanAddChangeDi:1;
	u8 busy:1;
	u8 m_bCheckTime:1;//�Ƿ���Ҫ��ʱ�Ĺ���
	u8 m_bxxaq:1;	//�Ƿ���Ҫ������Ϣ��ȫ�ж�
	
	COMDOINFO   m_DOinfo;	//
	UP_DOINFO   m_DoUpInfo;	//
	u16* m_pAIUpInfo;
	u16* m_pDIUpInfo;
	u16* m_pDOUpInfo;
	u16* m_pCIUpInfo;
	u16* m_pAIUpInfo_Num;
	u16* m_pDIUpInfo_Num;
	u16* m_pDOUpInfo_Num;
	u16* m_pCIUpInfo_Num;
	u32 m_nLastSendTickCount;//����յ����ݵ�TickCount
	u8  m_nSendTimes;//�ظ����͵Ĵ���
}COM_STRUCT;


#define GET_YXVALUE_NO(YX_NO) ( (g_yx_Word_value[(YX_NO)/ONE_WORD_BITS] >> ((YX_NO) % ONE_WORD_BITS)) & 0x0001  )

u16  ComSendData(u8* Data, u16 SendLen, u8 nComNo);//��������,���ط��ͳɹ�������
void InitCommunicate(void);//��ʼ������ͨ����Ϣ
void initComm(u8 nComNo);

u16 GetPerDevPointNum(u8 pttype);
u16 GetPerDevPtInit(u16 ptIdx);
void SetPerDevPtInit(u16 ptIdx, u16 val);
u16 IsTimeOut(u32 preTickCount, u32 OutMisSec);
u16 IsLeap(u16 Year);


u32 GetTimeInterval(u32 preTickCount);
u16 MoveRxBufferOutPoint(u8 nComNo, u16 nLen);
void Comm_module(void);
void Protocol_Init();

extern COM_STRUCT g_Comm[];
extern union PROTOCOL_STRUCT  g_ProtoclInfo[];

extern COM_STRUCT * pComModbusM;		//MODBUS�˿�
extern COM_STRUCT * pComGsm;		//���Ŷ˿�
extern COM_STRUCT * pComConfig;		//ά���˿�


extern s16 g_Communicate_yc_value[][YC_ALL_NUM];	

#endif
