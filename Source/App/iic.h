
#ifndef IIC_H
#define IIC_H

#include "lpc17xx.h"
 

#define IIC_READ 0XA1    //定义读指令
#define IIC_WRITE 0XA0   //定义写指令

#define  A_SCL   LPC_GPIO0->FIOPIN&0x10000000
#define  A_SDA   LPC_GPIO0->FIOPIN&0x08000000
#define  SCL1    LPC_GPIO0->FIOPIN|=0x10000000
#define  SCL0    LPC_GPIO0->FIOPIN&=0xefffffff
#define  SDA1    LPC_GPIO0->FIOPIN|=0x08000000
#define  SDA0    LPC_GPIO0->FIOPIN&=0xf7ffffff
#define  INPUT   LPC_GPIO0->FIODIR&=0xf7ffffff
#define  OUTPUT  LPC_GPIO0->FIODIR|=0x08000000

#define  delaytime   100

#endif		//#ifndef IIC_H

