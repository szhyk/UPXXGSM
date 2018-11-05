#include "define.h"
#include "iic.h"

uint8  	ACKEEP;    
void Delay(uint8 n)
{
	while(n--);
}
void Start_I2cA( void )
{
	//WP = 0;
	
	OUTPUT;
	
	SDA1;    	/*发送起始条件的数据信号*/
	
	Delay( delaytime );
	
	SCL1;
	
	Delay( delaytime );		/*起始条件建立时间大于4.7us,延时*/
	
	SDA0;    	/*发送起始信号*/
	
	Delay( delaytime );   	/* 起始条件锁定时间大于4μs*/
    
	SCL0;    	/*钳住I2C总线，准备发送或接收数据 */
	
	Delay( delaytime );
}



/*******************************************************************
结束总线函数

  函数原型:	void Stop_I2cA();
  
	功能:	结束I2C总线,即发送I2C结束条件.
********************************************************************/
void Stop_I2cA( void )
{
	//	WP = 1;
	OUTPUT;
	
	SDA0; 		/*发送结束条件的数据信号*/
	
	Delay( delaytime );   	/*发送结束条件的时钟信号*/
	
	SCL1; 		/*结束条件建立时间大于4μs*/
	
	Delay( delaytime );
	
	SDA1; 		/*发送I2C总线结束信号*/
	
	Delay( delaytime );
	
	SCL0;
	
	Delay( delaytime );
}
/*******************************************************************
字节数据传送函数

  函数原型:	void SendByteA(uchar uData);
  
	功能:	将数据c发送出去,可以是地址,也可以是数据,发完后等待应答,并对
  		此状态位进行操作.(不应答或非应答都使ack=0 假)发送数据正常，
		ack=1; ack=0表示被控器无应答或损坏。 
********************************************************************/
void SendByteA( uint8 uData )
{
	uchar BitCnt;
	OUTPUT;
	for( BitCnt = 0; BitCnt < 8 ; BitCnt ++ ) /*要传送的数据长度为8位*/
	{
		if(( uData << BitCnt ) & 0x80 )
		{
			SDA1;   	/*判断发送位*/
		}
		else 
		{
			SDA0;
        }
		
		Delay( delaytime );
		
		SCL1;           	/*置时钟线为高，通知被控器开始接收数据位*/         
		
		Delay( delaytime );		  	/*保证时钟高电平周期大于4μs*/
		
		SCL0; 
		Delay( delaytime );
	}
	
	Delay( delaytime );
	INPUT;
	
	SDA1;           /*8位发送完后释放数据线，准备接收应答位*/
	
	Delay( delaytime ); 
	
	SCL1;
	
	Delay( delaytime ); 
	
	if(A_SDA)
	{
		ACKEEP= 0; 
	}
    else 
	{
		ACKEEP= 1;     	/*判断是否接收到应答信号*/
	}
	
	SCL0;
	
	Delay( delaytime );
}	


/*******************************************************************
字节数据传送函数 

  函数原型: 	uchar RcvByte();
  
	功能: 	用来接收从器件传来的数据,并判断总线错误(不发应答信号)，
  		发完后请用应答函数。 
********************************************************************/    
uchar RcvByteA( void )
{
	uchar uData;
	
	uchar BitCnt;
	
	uData = 0; 
	INPUT;
	
    SDA1;          	/*置数据线为输入方式*/
	
	for( BitCnt = 0 ; BitCnt < 8 ; BitCnt ++ )
    {
		Delay( delaytime );
		
		SCL0;      	/*置时钟线为低，准备接收数据位*/
		
		Delay( delaytime );  	/*时钟低电平周期大于4.7μs*/
		
		SCL1;        	/*置时钟线为高使数据线上数据有效*/
		
		Delay( delaytime );
		
		uData = uData << 1;
		
		if(A_SDA)
		{
			uData = uData + 1; /*读数据位,接收的数据位放入retc中 */
		}
		
		Delay( delaytime );
    }
	
	SCL0;  
	
	Delay( delaytime );
	
	return uData;
}




/********************************************************************
应答子函数

  原型: 	void Ack_I2cA(bit a);
  
	功能:	主控器进行应答信号,(可以是应答或非应答信号)
********************************************************************/
void Ack_I2cA( uint8 BITAck )
{
	OUTPUT;
	if( BITAck == 0 )
	{
		SDA0;   		/*在此发出应答或非应答信号 */
	}
    else 
	{
		SDA1;
	}
	
	Delay( delaytime ); 
    
	SCL1;       
	
	Delay( delaytime ); 		/*时钟低电平周期大于4μs*/
	
	SCL0;           	/*清时钟线，钳住I2C总线以便继续接收*/
	
	Delay( delaytime ); 
}

void EEPROM_Write ( uint8 uData ,uint16 Address )
{	
	unsigned short k;
	
	Start_I2cA();           		/*启动总线*/
	do
	{
		SendByteA( 0xA0 );		/*发送器件地址*/
	}while( ACKEEP== 0);
	
	do
	{
		SendByteA( Address>>8 );         
	}while( ACKEEP== 0);
	do
	{
		SendByteA( Address );         
	}while( ACKEEP== 0);
	do
	{
		SendByteA( uData );    	/*发送数据*/
	}while( ACKEEP== 0);
	
	Stop_I2cA();           		/*结束总线*/ 
	
	Delay(250);
	for(k=1;k;k++);
	for(k=1;k;k++);
}

uint8 EEPROM_Read (uint16 Address)
{
	uint8 uData	;
	unsigned short k;
	Start_I2cA();           		/*启动总线*/
	
	do
	{
		SendByteA( 0xA0 );		/*发送器件地址*/
	}while( ACKEEP== 0);
	
	do
	{
		SendByteA( Address>>8 );       
	}while( ACKEEP== 0);
	do
	{
		SendByteA( Address );       
	}while( ACKEEP== 0);
	Start_I2cA();           		/*启动总线*/
	
	do
	{
		SendByteA( 0xA1 );		/*发送器件地址*/
	}while( ACKEEP== 0);
	
	uData = RcvByteA();         /*读取数据*/
	
	Ack_I2cA(1);           		/*发送非就答位*/
	
	Stop_I2cA();             	/*结束总线*/ 
	Delay(250);
	for(k=1;k;k++);
	for(k=1;k;k++);
	
	return  uData;
}



