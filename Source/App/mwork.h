#include "define.h"
#ifndef _MAIN_H_
#define _MAIN_H_
/*********************************************/
extern uint32 dTCounter;

/****************************/
void SleepSec(uint8 s);
void SleepMSec(uint8 ms);

extern void Mc52i_init(void);
extern  void Get_int_data_sub(void);
extern void tele_write_sub(void);
extern void EEPROM_tel_write (uint8 telnum);
extern void EEPROM_tel_Read (uint8 telnum);

#endif

