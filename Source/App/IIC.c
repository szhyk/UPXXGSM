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
	
	SDA1;    	/*������ʼ�����������ź�*/
	
	Delay( delaytime );
	
	SCL1;
	
	Delay( delaytime );		/*��ʼ��������ʱ�����4.7us,��ʱ*/
	
	SDA0;    	/*������ʼ�ź�*/
	
	Delay( delaytime );   	/* ��ʼ��������ʱ�����4��s*/
    
	SCL0;    	/*ǯסI2C���ߣ�׼�����ͻ�������� */
	
	Delay( delaytime );
}



/*******************************************************************
�������ߺ���

  ����ԭ��:	void Stop_I2cA();
  
	����:	����I2C����,������I2C��������.
********************************************************************/
void Stop_I2cA( void )
{
	//	WP = 1;
	OUTPUT;
	
	SDA0; 		/*���ͽ��������������ź�*/
	
	Delay( delaytime );   	/*���ͽ���������ʱ���ź�*/
	
	SCL1; 		/*������������ʱ�����4��s*/
	
	Delay( delaytime );
	
	SDA1; 		/*����I2C���߽����ź�*/
	
	Delay( delaytime );
	
	SCL0;
	
	Delay( delaytime );
}
/*******************************************************************
�ֽ����ݴ��ͺ���

  ����ԭ��:	void SendByteA(uchar uData);
  
	����:	������c���ͳ�ȥ,�����ǵ�ַ,Ҳ����������,�����ȴ�Ӧ��,����
  		��״̬λ���в���.(��Ӧ����Ӧ��ʹack=0 ��)��������������
		ack=1; ack=0��ʾ��������Ӧ����𻵡� 
********************************************************************/
void SendByteA( uint8 uData )
{
	uchar BitCnt;
	OUTPUT;
	for( BitCnt = 0; BitCnt < 8 ; BitCnt ++ ) /*Ҫ���͵����ݳ���Ϊ8λ*/
	{
		if(( uData << BitCnt ) & 0x80 )
		{
			SDA1;   	/*�жϷ���λ*/
		}
		else 
		{
			SDA0;
        }
		
		Delay( delaytime );
		
		SCL1;           	/*��ʱ����Ϊ�ߣ�֪ͨ��������ʼ��������λ*/         
		
		Delay( delaytime );		  	/*��֤ʱ�Ӹߵ�ƽ���ڴ���4��s*/
		
		SCL0; 
		Delay( delaytime );
	}
	
	Delay( delaytime );
	INPUT;
	
	SDA1;           /*8λ��������ͷ������ߣ�׼������Ӧ��λ*/
	
	Delay( delaytime ); 
	
	SCL1;
	
	Delay( delaytime ); 
	
	if(A_SDA)
	{
		ACKEEP= 0; 
	}
    else 
	{
		ACKEEP= 1;     	/*�ж��Ƿ���յ�Ӧ���ź�*/
	}
	
	SCL0;
	
	Delay( delaytime );
}	


/*******************************************************************
�ֽ����ݴ��ͺ��� 

  ����ԭ��: 	uchar RcvByte();
  
	����: 	�������մ���������������,���ж����ߴ���(����Ӧ���ź�)��
  		���������Ӧ������ 
********************************************************************/    
uchar RcvByteA( void )
{
	uchar uData;
	
	uchar BitCnt;
	
	uData = 0; 
	INPUT;
	
    SDA1;          	/*��������Ϊ���뷽ʽ*/
	
	for( BitCnt = 0 ; BitCnt < 8 ; BitCnt ++ )
    {
		Delay( delaytime );
		
		SCL0;      	/*��ʱ����Ϊ�ͣ�׼����������λ*/
		
		Delay( delaytime );  	/*ʱ�ӵ͵�ƽ���ڴ���4.7��s*/
		
		SCL1;        	/*��ʱ����Ϊ��ʹ��������������Ч*/
		
		Delay( delaytime );
		
		uData = uData << 1;
		
		if(A_SDA)
		{
			uData = uData + 1; /*������λ,���յ�����λ����retc�� */
		}
		
		Delay( delaytime );
    }
	
	SCL0;  
	
	Delay( delaytime );
	
	return uData;
}




/********************************************************************
Ӧ���Ӻ���

  ԭ��: 	void Ack_I2cA(bit a);
  
	����:	����������Ӧ���ź�,(������Ӧ����Ӧ���ź�)
********************************************************************/
void Ack_I2cA( uint8 BITAck )
{
	OUTPUT;
	if( BITAck == 0 )
	{
		SDA0;   		/*�ڴ˷���Ӧ����Ӧ���ź� */
	}
    else 
	{
		SDA1;
	}
	
	Delay( delaytime ); 
    
	SCL1;       
	
	Delay( delaytime ); 		/*ʱ�ӵ͵�ƽ���ڴ���4��s*/
	
	SCL0;           	/*��ʱ���ߣ�ǯסI2C�����Ա��������*/
	
	Delay( delaytime ); 
}

void EEPROM_Write ( uint8 uData ,uint16 Address )
{	
	unsigned short k;
	
	Start_I2cA();           		/*��������*/
	do
	{
		SendByteA( 0xA0 );		/*����������ַ*/
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
		SendByteA( uData );    	/*��������*/
	}while( ACKEEP== 0);
	
	Stop_I2cA();           		/*��������*/ 
	
	Delay(250);
	for(k=1;k;k++);
	for(k=1;k;k++);
}

uint8 EEPROM_Read (uint16 Address)
{
	uint8 uData	;
	unsigned short k;
	Start_I2cA();           		/*��������*/
	
	do
	{
		SendByteA( 0xA0 );		/*����������ַ*/
	}while( ACKEEP== 0);
	
	do
	{
		SendByteA( Address>>8 );       
	}while( ACKEEP== 0);
	do
	{
		SendByteA( Address );       
	}while( ACKEEP== 0);
	Start_I2cA();           		/*��������*/
	
	do
	{
		SendByteA( 0xA1 );		/*����������ַ*/
	}while( ACKEEP== 0);
	
	uData = RcvByteA();         /*��ȡ����*/
	
	Ack_I2cA(1);           		/*���ͷǾʹ�λ*/
	
	Stop_I2cA();             	/*��������*/ 
	Delay(250);
	for(k=1;k;k++);
	for(k=1;k;k++);
	
	return  uData;
}



