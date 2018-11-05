// Configuration.h

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define CONFIGURTION_XX

typedef struct  
{
	uint8 whflag:1;
	uint8 whflagold:1;
	uint8 whPause:1;
	uint8 whInitFlag;
	uint32 l_lt1;
}WH_STRUCT;

extern WH_STRUCT wh_com[];
#define MODBUS_RX_VACANCY	0x00
#define MODBUS_TX_VACANCY	0x00
#define MODBUS_TX_ON		0x01


void Init_Wh(WH_STRUCT *pWh);
uint8 CheckWh(COM_STRUCT*  l_pCom,uint8 whno,uint8 recflag);


#endif	//#define CONFIGURATION_H

