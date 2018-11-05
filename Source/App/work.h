#ifndef _WORK_H_
#define _WORK_H_

#include "define.h"


extern uint8  gsmtelnumbuf[teletaolnumbuf][11]; // 1-5为超级用户，6为高级用户，后面的为普通用户
extern uint16 gsmtelsuper;
extern uint8  gsmduanxinnum;  // 短信进来的时候有个提示是第几条短信位置
extern uint8  gsmdelectduanxinnum;  //要删除的短信位置
extern uint8  gsmMsgNumbufGroup;      //短信号码所在短信号码数组的位置

extern uint8  telenumoldbuf;//用来做发送短信的时候用
extern uint8  telenumnewbuf;

extern uint8  teleToalnumbuf;
extern uint8  teletolnumbuf;
extern uint8  telebaohunumbuf;


extern uint8 g_stoprun;

extern uint16 netlestimebuf;
extern uint32 Nodatatratbuf;//用来做没有数据通讯计时用。



extern void sys_timeonems_sub(void);
extern void net_led_work_sub(void);
extern void eeprom_write_sub(void);
extern void eeprom_read_sub(void);
extern uint8 EEPROM_Read ( uint16 Address  );
extern void EEPROM_Write ( uint8  ,uint16  );

uint8 SendStrToBuf(char *dest, const char *src);



#endif


