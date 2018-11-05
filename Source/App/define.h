#ifndef _defineh 
#define _defineh

#ifdef _MAIN_INIT_
	#define _EXT
#else
	#define _EXT extern
#endif
#include "lpc17xx.h"

#define DEVICE_NAME "UP116"

//芯片为LPC1768 ARM Cortex-M3
#define ISOFT_VERSION 10
#define FSOFT_VERSION "1.0"


#define	uint8   unsigned char 
#define	uchar   unsigned char

#ifdef uint16
	#undef uint16
#endif

#ifdef int16
	#undef int16
#endif

typedef   signed short     int int16_t;
typedef unsigned short     int uint16_t;
#define  uint16  uint16_t
#define  int16   int16_t
// #define  uint  unsigned int

#define  uint32  unsigned long
#define  int8    signed   char
#define  int32   signed   long 
/***************************************************/
#define	 SETBIT(x,y)	(x|=(1<<y))		//Set bit y in x
#define	 CLRBIT(x,y)	(x&=(~(1<<y)))	//Clear bit y in x
#define  CHKBITSET(x,y)	(x&(1<<y))		//Check bit y in x is 1?
#define  CHKBITCLR(x,y) ((x&(1<<y))^(1<<y))	//Check bit y in x is 0?


#define hibyte(x) (unsigned char)(x>>8)
#define lobyte(x) (unsigned char)(x & 0xFF)

#define NOP() ;
#define WDR() {LPC_WDT->WDFEED=0xaa;LPC_WDT->WDFEED=0x55;}      

//
#define FOSC	12000000                            /*  振荡器频率                  */
#define FCCLK	(FOSC  * 8)                         /*  主时钟频率<=100Mhz          */
/*  FOSC的整数倍                */
#define FCCO	(FCCLK * 3)                         /*  PLL频率(275Mhz~550Mhz)      */
/*  与FCCLK相同，或是其的偶数倍 */
#define FPCLK	(FCCLK / 4)                         /*  外设时钟频率,FCCLK的1/2、1/4*/
/*  或与FCCLK相同               */



/***********************************/
#define UART0TXSETCounter  11
#define datamaxlen         250
/***********************************/
#define mokuaiqidongshijian     360000000 //10小时
/***********************************/
#define MC52WORKgprsTCP          0
#define MC52WORKgprsUDP          1
#define MC52WORKGSMTEXT          2
#define MC52WORKGSMPDU           3
#define MC52WORKTCPTEXT          4
#define MC52WORKUDPTEXT          5
#define MC52WORKTCPPDU           6
#define MC52WORKUDPPDU           7
/************************************/
#define MC52GSMTEXT         0x31
#define MC52GSMPDU          0x30
/***********************************/
#define gprsATstaturef            1
#define gprsCLIPstaturef          2
#define gprsCNMIstaturef          3
#define gprsCMGFstaturef          4
#define gprsATWstaturef           5

#define gprsSCIDstaturef          6
#define gprsCSCAstaturef          7
#define gprsGSNstaturef           8
#define gprsCSQstaturef           9
#define gprsconTypestaturef       10
#define gprspasswdstaturef        11
#define gprstimestaturef          12
#define gprsuserstaturef          13
#define gprsapnstaturef           14

#define gprs2conTypestaturef       15
#define gprs2passwdstaturef        16
#define gprs2timestaturef          17
#define gprs2userstaturef          18
#define gprs2apnstaturef           19

#define gprsmsrvTypestaturef       20
#define gprsmconIdstaturef         21
#define gprsmaddressstaturef       22
#define gprsrsrvTypestaturef       23
#define gprsrconIdstaturef         24
#define gprsraddressstaturef       25

#define gprsmSISRstaturef          26
#define gprsmCLOSENETstaturef      27
#define gprsmSTARTSENDstaturef     28

#define gprsrSISRstaturef          29
#define gprsrCONNECTNETstaturef    30
#define gprsrCLOSENETstaturef      31
#define gprsrSTARTSENDstaturef     32
#define gprsralonclose             33

#define gprscxljopenf              34
#define gprsATTESTf                35
#define gprsCSMPf                  36
#define GSMSAVEMODE                37

#define gprscpinstaturef           38


/***********************************/
#define GsmSendokbuf     1
#define GsmSendSCDXbuf     2
/**********************************/
/***********************************/
//以下为存储地址
//存储器为FM24CL64，8k字节大小
#define STOP_RUN_ADDR		0x01
// #define maiipadd0           0x01
// #define maiipadd1           0x02
// #define maiipadd2           0x03
// #define maiipadd3           0x04
// #define preperipadd0        0x05
// #define preperipadd1        0x06
// #define preperipadd2        0x07
// #define preperipadd3        0x08
// #define maiportaddh         0x09
// #define maiportaddl         0x0a
// #define preperportaddh      0x0b
// #define preperportaddl      0x0c
#define baudaddh            0x0d
#define baudaddl            0x0e
#define heartbeataddl       0x0f
#define mc52workmodeladd    0x10
#define telenumdataadd      0x11
#define telenumsuperadd     0x12
/************************************/
#define telenumstartaddr        telenumdataadd+2
#define YongfuNumStart          6   //0没用到，1-5
#define teleUserNumMax			10
#define teletaolnumbuf          (YongfuNumStart+teleUserNumMax)

#define EEP_ADDR_TEMP	1024
#define EEP_ADDR_SOE	(EEP_ADDR_TEMP+200)


/************出厂设置********************/
#define oldmc52model     0   //30为GSM模式，31为GPRS
#define oldbauddata      9600

#define UartreceDelayTime 20
#define Uart0sendjiangetime 200   //10MS*10=1s

#define	  UartRXminnum    0   //考虑>

#define	  UartRXmaxnum    255
#define	 GPRS_POWEROFFTIME_SET  3000
#define	 GPRS_DIANHUO_TIME_SET  3000
/*******************************************/
#define	  SimIDnum        20 
#define	  Mc52xilienum    15 
#define	  simcardnum      13

#define	  mc52workgsmf      0  //GSM模式连接
#define	  mc52workgprsf     1  //GPRS模式连接
/***************************************/

#define _0   0x30
#define _1   0x31
#define _2   0x32
#define _3   0x33
#define _4   0x34
#define _5   0x35
#define _6   0x36
#define _7   0x37
#define _8   0x38
#define _9   0x39

#define _A   0x41
#define _B   0x42
#define _C   0x43
#define _D   0x44
#define _E   0x45
#define _F   0x46
#define _G   0x47
#define _H   0x48
#define _I   0x49
#define _J   0x4A
#define _K   0x4B
#define _L   0x4C
#define _M  0x4D
#define _N   0x4E
#define _O   0x4F
#define _P   0x50
#define _Q   0x51
#define _R   0x52
#define _S   0x53
#define _T   0x54
#define _U   0x55
#define _V   0x56
#define _W   0x57
#define _X   0x58
#define _Y   0x59
#define _Z   0x5A

#define _LF		0x0A		//换行 \n
#define _CR		0x0D		//回车 \r
#define _CTRLZ  0x1A		//CTRL+Z
#define _SPACE  0x20		//sp
#define _QUOTE	0x22		//"
#define _AND	0x26		//&
#define _PLUS   0x2B		//+
#define _COMMA	0x2C		//,
#define _DOTP	0x2E		//.
#define _SLASH	0x2F		///
#define _COLON  0x3A		//:
#define _EQUAL  0x3D		//=
#define _YOUJ   0x3E		//>
#define _QustionMark   0x3F	//?
#define _FEN   0x3B	//;
#define _AND   0x26	//&


/******************************************/
//P0.22 左2绿灯，数据传输时
#define  NETLEDON          LPC_GPIO0->FIOSET |= 0x00400000		// 接收发送时点亮 300ms
#define  NETLEDOFF         LPC_GPIO0->FIOCLR |= 0x00400000		//
//P0.20 红灯,SIM模块电源指示
#define  POWERLEDON         LPC_GPIO0->FIOSET |= 0x00100000 // 接收发送时点亮 300ms
#define  POWERLEDOFF        LPC_GPIO0->FIOCLR |= 0x00100000
//P0.21 左1绿灯，已注册到网络
#define  DATALEDON          LPC_GPIO0->FIOSET |= 0x00200000 // GPRS GSM 进入工作时点亮 --长亮 
#define  DATALEDOFF         LPC_GPIO0->FIOCLR |= 0x00200000

////电源控制 P2.0
#define  MC52IPOWEROFF     LPC_GPIO2->FIOSET |= 0x00000001
#define  MC52IPOWERON      LPC_GPIO2->FIOCLR |= 0x00000001 
#define  MC52IPOWERstat    LPC_GPIO2->FIOPIN & 0x00000001 

//点火控制 打开MC52I P2.12 GSMRST
#define  MC52IRESTON      LPC_GPIO2->FIOCLR |= 0x00001000
#define  MC52IRESTOFF     LPC_GPIO2->FIOSET |= 0x00001000
//P1.30
#define  TEMTHOTOFF     LPC_GPIO1->FIOCLR |= 0x40000000 // CLRBIT(PORTC, 7)
#define  TEMTHOTON      LPC_GPIO1->FIOCLR |= 0x40000000 // SETBIT(PORTC, 7)

//#define  MOKUAIDOGHIGH    SETBIT(PORTE, 4)
///#define  MOKUAIDOGLOW     CLRBIT(PORTE, 4)

//#define  TESTKEY        (PIND&0x20)
//#define  TESTNETLED     (PINC&0x20)
/*
#define  TEMTHOTOFF       CLRBIT(PORTC, 7)
#define  TEMTHOTON        SETBIT(PORTC, 7)
*/
extern uint8 flag_sys_1ms,flag_sys_10ms,flag_sys_100ms;

//statusflag0 bit definition
_EXT union
{
	struct
	{
		unsigned   SYSWORKINGF:1;		//系统正常工作(包括LCD初始化完成和跟CPU通讯正常
		unsigned   mc52iintf:1; 
		unsigned   mc52iigtf:1;
		unsigned   heartbeatf:1;
		unsigned   mc52istarttest:1;
		unsigned   mc52testdelayf:1;
		unsigned   :1;
		unsigned   noSim:1;

		unsigned   telconconnect:1;
		unsigned   atcomsendf:1;
		unsigned   atnetsendf:1;
		unsigned   getsinglenumf:1;
		unsigned   :1;
		unsigned   cantest232flag:1;
		unsigned   test232delayf:1;
		unsigned   uart2workmode:1;//高为测试模式,低为通讯模式
	}bit;
	uint16 Word;
}statusflag0;

//
_EXT union
{
	struct
	{
		unsigned   atsendf:1;
		unsigned   attestflag:1;
		unsigned   clipflag:1;
		unsigned   cnmiflag:1;
		unsigned   cmgfflag:1;
		unsigned   csmpflag:1;
		unsigned   at_wflag:1;
		unsigned   atgsnflag:1;

		unsigned   atscidflag:1;
		unsigned   atcscaflag:1;
		unsigned   atcsqflag:1;
		unsigned   atSetSaveMode:1;
		unsigned   :1;
		unsigned   first_connect:1;
		unsigned   atcpinflag:1;
		unsigned   :1;
	}bit;
	uint16 Word;
}Mc52i_CMD_flag1;	//整个初始化过程中使用。

_EXT union
{
	struct
	{
		unsigned   mainipeepf:1;
		unsigned   preperipeepf:1;
		unsigned   hearteepf:1;
		unsigned   mainporteepf:1;
		unsigned   preperporteepf:1;
		unsigned   baudeepf:1;
		unsigned   Modeleepf:1;
		unsigned   apneepf:1;
		
		unsigned   telenumf:1;
		unsigned   baohuworkf:1;
		unsigned   wangluookflag:1;
		unsigned   Diwunworkingf1:1;
		unsigned   Diwunworkingf2:1;
		unsigned   GsmFuweisendf:1;
		unsigned   gsmds:1;
		unsigned   telesuper:1;
	}bit;
	uint16 Word;
}eepbufflag;

//funcflag3 bit definition
#define mc52RESTf			  0
#define mc52icxlianjieopenf		   4
//funcflag5 bit definition
#define mc52igprsdatasendf	   1
#define mc52havetrancedataled   5
#define mc52icxnjtdelayf        6   //重新连接
#define mc52iqhnjtdelayf        7   //重新连接

//mc52igprsflag bit definition
#define mc52igprsworking		0
//#define mc52iokf		        3
#define mc52ierrf		        4
#define mc52igprsnetoking		5
#define mc52isenddelay		    6
#define mc52ileddataf		    7
//funcflag7 bit definition
#define gprsnettotestf	        2
#define EEpwritedelay	        6

//uartfuncflag bit definition
#define uartdelayf		 0
#define uartsenddata	 1
#define uartrecflag	     2
#define uartsendfirstf	 3 //握手信息
#define uartsendfirstokf  4
#define uartaskshishionf  5
#define uartaskshishiofff  6
#define uartatsendf  7

//mc52iworkflag bit definition
#define gsmchongxingdata    0
#define mc52gprsworkingf	    2
#define mc52gsmworkingf	    3
#define gprsSISMf		    4 //保护后面有相关的没必要的数据发生
#define mc52sendbaohuf       5
#define mc52gsmsendbaohuf    6

//MC52istaflag  bit definition
#define  readmc52idataf			2
#define  mc52igetidnum			5
#define  mc52iNoSignal		7

//Mc52igprssetflag bit definition
#define mc52iconTypesf			0
#define mc52ipasswdsf			1
#define mc52itimef			    2
#define mc52iuseresf			3
#define mc52iapnsf			    4
#define closergprsf              7
//Mc52i2gprssetflag bit definition
#define mc52i2conTypesf			0
#define mc52i2passwdsf			1
#define mc52i2timef			    2
#define mc52i2useresf			3
#define mc52i2apnsf			    4
//Mc52igprsipsetflag  bit definition
#define mc52imsrvTypesf			0
#define mc52imconIdsf			1
#define mc52imaddresssf			2
#define mc52irsrvTypesf			3
#define mc52irconIdsf			4
#define mc52iraddresssf			5
#define closemgprsf             7
//Mc52masterNetf  bit definition
#define     Mc52masterconnect   3
//Mc52prepareNetf  bit definition
#define     Mc52prepareconnect	3
/****************************/
//Gsmstatureflag bit definition
#define     gsmreaddataf 	    0
#define     gsmdelectduanxf 	1
#define     gsmguajif  	        7

//Gsmstature4flag bit definition
_EXT union
{
	struct
	{
		unsigned   gsmwrite1numf:1;
		unsigned   gsmwrite2numf:1;
		unsigned   gsmwrite3numf:1;
		unsigned   gsmwrite4numf:1;
		unsigned   gsmwrite5numf:1;
		unsigned   gsmwrite6numf:1;
		unsigned   gsmwrite7numf:1;
		unsigned   gsmwrite8numf:1;
		
		unsigned   gsmwrite9numf:1;
		unsigned   gsmwrite10numf:1;
		unsigned   :1;
		unsigned   :1;
		unsigned   :1;
		unsigned   gsmbaohuworkingf:1;
		unsigned   gsmringworkf:1;
		unsigned   gsmsenddusef:1;
	}bit;
	uint16 Word;
}Gsmstature4flag;


typedef struct 
{
	uint8	COMM_ID;	//0~2,UART0~UART2
	
	uint8	RXIndex;
	uint8	RxOut;
	uint8	TXIndex;
	uint8	TxOut;
	uint8	RXtime;
	uint8	RXdata[datamaxlen];
	uint8	TXdata[datamaxlen];
	
	uint8	SendIndex;
	uint8	Senddatanum;
	uint8	Senddatanumold;
	uint8	funcflag;
	
	uint8   TXBUFNumber;
	uint8   TXCounter;
	uint8   TXBUFC_flag;
	uint8   *TxDataBuf[UART0TXSETCounter];
	uint8   TXBufLen[UART0TXSETCounter];
	uint8   TXBufUcs[UART0TXSETCounter];	//0--7BIT,1--UCS
	uint8   TXTELBufNUM[UART0TXSETCounter];	//每个发送缓冲区的电话号码序号
}COM_STRUCT;

#define UART0_BPS     9600                          /* 串口0通信波特率             */
#define UART2_BPS     9600  

typedef struct {
	uint32  sec :6;
	uint32  secnc  :2;
	uint32  min :6;
	uint32  minnc:2;
	uint32 hour:5;
	uint32 hournc:3;
		
}RTC;
typedef union {
	uint32 rtctime;
	RTC     st_rtc;
}RTCtime;


#define joyceGPRStest   0


#endif		//#ifndef _defineh 



