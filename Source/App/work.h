#ifndef _WORK_H_
#define _WORK_H_

#include "define.h"


extern uint8  gsmtelnumbuf[teletaolnumbuf][11]; // 1-5Ϊ�����û���6Ϊ�߼��û��������Ϊ��ͨ�û�
extern uint16 gsmtelsuper;
extern uint8  gsmduanxinnum;  // ���Ž�����ʱ���и���ʾ�ǵڼ�������λ��
extern uint8  gsmdelectduanxinnum;  //Ҫɾ���Ķ���λ��
extern uint8  gsmMsgNumbufGroup;      //���ź������ڶ��ź��������λ��

extern uint8  telenumoldbuf;//���������Ͷ��ŵ�ʱ����
extern uint8  telenumnewbuf;

extern uint8  teleToalnumbuf;
extern uint8  teletolnumbuf;
extern uint8  telebaohunumbuf;


extern uint8 g_stoprun;

extern uint16 netlestimebuf;
extern uint32 Nodatatratbuf;//������û������ͨѶ��ʱ�á�



extern void sys_timeonems_sub(void);
extern void net_led_work_sub(void);
extern void eeprom_write_sub(void);
extern void eeprom_read_sub(void);
extern uint8 EEPROM_Read ( uint16 Address  );
extern void EEPROM_Write ( uint8  ,uint16  );

uint8 SendStrToBuf(char *dest, const char *src);



#endif


