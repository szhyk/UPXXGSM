/************************************************************************* 
* ��Ȩ���У� �㶫���������ܵ������޹�˾
* �ļ��汾�� V1.00
* �ļ����ƣ� gsmcomm.c
* �������ڣ� 2014.04.29
* ��	�ߣ� hjy
* ��	�ܣ� GSMͨ��ͨ�ú���
* ������Ϣ��
*	  ������־1��
*		  ���ڣ�	  
*		  �޸��ߣ�	  
*		  �޸����ݣ�  
*		  �޸�ԭ��  
*	  ������־2��
**************************************************************************/
#include "gsmcomm.h"

#include <string.h>

/**************************************************************************
�������ƣ�GsmString2Bytes
�����汾��1.00 
���ߣ�   hjy
�������ڣ�20140429
��������˵�����ɴ�ӡ�ַ���ת��Ϊ�ֽ�����
���������pSrc:Դ����; pDst:Ŀ������; nSrcLength:����
��������:
���������
����ֵ��
***************************************************************************/
int GsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	int i;
    for(i=0; i<nSrcLength; i += 2)
    {
        // �����4λ
        if(*pSrc>='0' && *pSrc<='9')
        {
            *pDst = (*pSrc - '0') << 4;
        }
        else
        {
            *pDst = (*pSrc - 'A' + 10) << 4;
        }
        
        pSrc++;
        
        // �����4λ
        if(*pSrc>='0' && *pSrc<='9')
        {
            *pDst |= *pSrc - '0';
        }
        else
        {
            *pDst |= *pSrc - 'A' + 10;
        }
        pSrc++;
        pDst++;
    }
    
    // ����Ŀ�����ݳ���
    return nSrcLength / 2;
}


/**************************************************************************
�������ƣ�GsmBytes2String
�����汾��1.00 
���ߣ�   hjy
�������ڣ�20140429
��������˵�����ֽ�����ת��Ϊ�ɴ�ӡ�ַ���
�磺{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
���������pSrc:Դ����; pDst:Ŀ������; nSrcLength:Դ���ݳ���
��������:
���������
����ֵ��Ŀ���ַ�������
***************************************************************************/
int GsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    const char tab[]="0123456789ABCDEF";    // 0x0-0xf���ַ����ұ�
    int i;
    for( i=0; i<nSrcLength; i++)
    {
        // �����4λ
        *pDst++ = tab[*pSrc >> 4];
        
        // �����4λ
        *pDst++ = tab[*pSrc & 0x0f];
        
        pSrc++;
    }
    
    // ����ַ����Ӹ�������
    *pDst = '\0';
    
    // ����Ŀ���ַ�������
    return nSrcLength * 2;
}


/**************************************************************************
�������ƣ�GsmSerializeNumbers
�����汾��1.00 
���ߣ�   hjy
�������ڣ�20140429
��������˵���� �����ߵ����ַ���ת��Ϊ����˳����ַ���
�磺"683158812764F8" --> "8613851872468"
���������pSrc:Դ����; pDst:Ŀ������; nSrcLength:����
��������:
���������
����ֵ��
***************************************************************************/
int GsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength;   // Ŀ���ַ�������
    char ch;          // ���ڱ���һ���ַ�
		int i;
    
    // ���ƴ�����
    nDstLength = nSrcLength;
    
    if (nSrcLength & 01)
	{
        nSrcLength = nSrcLength >> 1 + 1;
    }
	else
	{
        nSrcLength = nSrcLength >> 1;
    }
    
    // �����ߵ�
    for(i=0; i<nSrcLength; i++)
    {
        ch = *pSrc++;        // �����ȳ��ֵ��ַ�
        *pDst++ = *pSrc++;   // ���ƺ���ֵ��ַ�
        *pDst++ = ch;        // �����ȳ��ֵ��ַ�
    }
    
    // �����ַ���'F'��
    if(*(pDst-1) == 'F')
    {
        pDst--;
        nDstLength--;        // Ŀ���ַ������ȼ�1
    }
    
    // ����ַ����Ӹ�������
    *pDst = '\0';
    
    // ����Ŀ���ַ�������
    return nDstLength;
}

/**************************************************************************
�������ƣ�GsmSerializeNumbers
�����汾��1.00 
���ߣ�   hjy
�������ڣ�20140429
��������˵��������˳����ַ���ת��Ϊ�����ߵ����ַ�����������Ϊ��������'F'�ճ�ż��
�磺"8613851872468" --> "683158812764F8"
���������pSrc:Դ����; pDst:Ŀ������; nSrcLength:����
��������:
���������
����ֵ��Ŀ���ַ�������
***************************************************************************/
int GsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength,i;   // Ŀ���ַ�������
    char ch;          // ���ڱ���һ���ַ�
    
    // ���ƴ�����
    nDstLength = nSrcLength;
    
    // �����ߵ�
    for(i=0; i < nSrcLength; i+=2)
    {
        ch = *pSrc++;        // �����ȳ��ֵ��ַ�
        *pDst++ = *pSrc++;   // ���ƺ���ֵ��ַ�
        *pDst++ = ch;        // �����ȳ��ֵ��ַ�
    }
    
    // Դ��������������
    if(nSrcLength & 1)
    {
        *(pDst-2) = 'F';     // ��'F'
        nDstLength++;        // Ŀ�괮���ȼ�1
    }
    
    // ����ַ����Ӹ�������
    *pDst = '\0';
    
    // ����Ŀ���ַ�������
    return nDstLength;
}



/**************************************************************************
�������ƣ�GsmDecodePdu
�����汾��1.00 
���ߣ�   hjy
�������ڣ�20140429
��������˵����GSM����PDU��
���������rxlen:�ֽ��ճ���
��������:pPdu:PDU����
���������pOa:���ͷ�����;  pUdata:���ݰ�
����ֵ����Ϣ�����ֽ� �յ�"����"
//08 91 683108705505F0 24 0D 91 683115908076F2 0008 71 11 01 41 10 30 23 04 542F 52A8 \r\n\r\n 'O' 'K' \r\n
//91Ϊ���ʣ��������Ҫ��86
//08 91 683108705505F0 --SMSC��ַ��Ϣ 08�ܳ��ȣ��ֽڳ���
//24 
//0D 91 683115908076F2 
//00 
//08 --����
//71 11 01 41 10 30 23 
//04 542F 52A8 \r\n\r\n 'O' 'K' \r\n
***************************************************************************/
int GsmDecodePdu(char* pPdu, char *pOa, char *pScts, char* pUdata)
{
    int nDstLength;
    unsigned char tmp;
	char* p1=pPdu;

    char scts[12];
    
    // SMSC��ַ��Ϣ��08
    GsmString2Bytes(pPdu, &tmp, 2);    		/* ȡSCA����,�ֽڳ���*/
    tmp = tmp * 2;
    pPdu += (2+tmp+2);						/*����SCA,PUD(24)ƫ����OA*/
    
	//TPDU
    //OA��ַ��Ϣ��0D 91 68 13510908672F
	//00 -- ��ͨGSM ���ͣ��㵽�㷽ʽ
    GsmString2Bytes(pPdu, &tmp, 2);			/*ȡOA���ȣ�������ַ�����,��������������91*/
    if(tmp & 1) tmp += 1;    				/* ������ż��*/
    pPdu += 6;
    GsmSerializeNumbers(pPdu, pOa, tmp-2);	/*ȡOA*/
    pPdu += tmp;							/*����OA,PID,ƫ����DCS*/

	//ȡ����08
    GsmString2Bytes(pPdu, &tmp, 2);			/*ȡ�����ַ�*/
	*p1 = tmp;								//�������ݱ��뷽����Դ�ĵ�һ���ֽ�08--UCS2,04-8bit,00-7bit
    pPdu += 2;								/*ƫ����SCTS*/
    
    //SCTS��Ϣ��71 11 01 41 10 30
	//23 ʱ��
    GsmSerializeNumbers(pPdu, scts, 12);	/*ȡSCTS,ʱ��*/
    GsmString2Bytes(scts, (unsigned char *)pScts, 12);		/*ת����ʽ*/
    pPdu += 7*2;							/*ƫ����UDL*/
    
    //UDL,UD��Ϣ��04 542F 52A8
    GsmString2Bytes(pPdu, &tmp, 2);			/*ȡUDL ��Ϣ���ȣ��ֽ�*/
    nDstLength = tmp;
    pPdu += 2;
    GsmString2Bytes(pPdu, (unsigned char *)pUdata, nDstLength*2);	/*ȡUD*/
	*(pUdata+nDstLength) = '\0';
	*(pUdata+nDstLength+1) = '\0';
	
    
    return nDstLength;
}


/**************************************************************************
�������ƣ�GsmEncodePdu
�����汾��1.00 
���ߣ�   hjy
�������ڣ�20140429
��������˵����GSM����PDU��
���������rxlen:�ֽ��ճ���
��������:pPdu:PDU����
���������pSca:���ŷ������ĺ���; pDa:Ŀ�����; pSrc:Դ����(UCS16);
pDst:Ŀ������; srclen:Դ���ݳ���;
����ֵ��
���ͷ�PDU��ʽ��
SCA	--�������ģ������ַ�ʽ��һ��Ϊ00��һ��13 08 91 68 31 08 70 55 05 F0 13
PDU-Type Э�����ݵ�Ԫ���ͣ�11
MR	     ���ŷ��Ͳο���Ŀ��00
DA		���շ���ַ���ֻ����룩��0D 91 68 31 15 90 80 76 F2
PID		������ʾ��Ϣ�����Ժ��ַ�ʽ������Ϣ����:00
DCS		������ʾ�û����ݱ��뷽��:08
VP		������ʾ��Ϣ��Ч��:00
UDL		�û����ݳ���:
UD		�û�����:

***************************************************************************/
int GsmEncodePdu_UCS2(const char *pSca, const char *pDa, const char* pSrc, char* pDst, int srclen)
{
    int nLength;             // �ڲ��õĴ�����
    int nDstLength;          // Ŀ��PDU������
    unsigned char buf[32]; // �ڲ��õĻ�����
    
    // SMSC��ַ��Ϣ��
    nLength = 13;//strlen(pSca);    // SMSC��ַ�ַ����ĳ���   
    buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;    // SMSC��ַ��Ϣ����
    buf[1] = 0x91;        // �̶�: �ù��ʸ�ʽ����
    nDstLength = GsmBytes2String(buf, pDst, 2);        // ת��2���ֽڵ�Ŀ��PDU��
    nDstLength += GsmInvertNumbers(pSca, &pDst[nDstLength], nLength);    // ת��SMSC��Ŀ��PDU��
    
    // TPDU�λ���������Ŀ���ַ��
    nLength = 13;//strlen(pDa);    // TP-DA��ַ�ַ����ĳ���
    buf[0] = 0x11;            // �Ƿ��Ͷ���(TP-MTI=01)��TP-VP����Ը�ʽ(TP-VPF=10)
    buf[1] = 0;               // TP-MR=0
    buf[2] = (char)nLength;   // Ŀ���ַ���ָ���(TP-DA��ַ�ַ�����ʵ����)
    buf[3] = 0x91;            // �̶�: �ù��ʸ�ʽ����
    buf[4] = 0x68;            // �̶�: �й����� 86�ߵ�
    nDstLength += GsmBytes2String(buf, &pDst[nDstLength], 5); // ת��5���ֽڵ�Ŀ��PDU��
    nDstLength += GsmInvertNumbers(pDa, &pDst[nDstLength], nLength-2); // ת��TP-DA��Ŀ��PDU�����绰���벻����86

    
    // TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��
    buf[0] = 0x00;        // Э���ʶ(TP-PID)
    buf[1] = 0x08;        // �û���Ϣ���뷽ʽ(TP-DCS)
    buf[2] = 0x00;            // ��Ч��(TP-VP)Ϊ5����
    
    // UCS2���뷽ʽ
    buf[3] = srclen;    // ת��TP-DA��Ŀ��PDU��
    nDstLength += GsmBytes2String(buf, &pDst[nDstLength], 4);
    nDstLength += GsmBytes2String((unsigned char *)pSrc, &pDst[nDstLength], srclen);
    
    // ����Ŀ���ַ�������
    return nDstLength;
}

int GsmEncodePdu_7bit(const char *pSca, const char *pDa, const char* pSrc, char* pDst, int srclen)
{
	  int nLength;             // �ڲ��õĴ�����
    int nDstLength;          // Ŀ��PDU������
    unsigned char buf[32]; // �ڲ��õĻ�����
    
    // SMSC��ַ��Ϣ��
    nLength = 13;//strlen(pSca);    // SMSC��ַ�ַ����ĳ���   
    buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;    // SMSC��ַ��Ϣ����
    buf[1] = 0x91;        // �̶�: �ù��ʸ�ʽ����
    nDstLength = GsmBytes2String(buf, pDst, 2);        // ת��2���ֽڵ�Ŀ��PDU��
    nDstLength += GsmInvertNumbers(pSca, &pDst[nDstLength], nLength);    // ת��SMSC��Ŀ��PDU��
    
    // TPDU�λ���������Ŀ���ַ��
    nLength = 13;//strlen(pDa);    // TP-DA��ַ�ַ����ĳ���
    buf[0] = 0x11;            // �Ƿ��Ͷ���(TP-MTI=01)��TP-VP����Ը�ʽ(TP-VPF=10)
    buf[1] = 0;               // TP-MR=0
    buf[2] = (char)nLength;   // Ŀ���ַ���ָ���(TP-DA��ַ�ַ�����ʵ����)
    buf[3] = 0x91;            // �̶�: �ù��ʸ�ʽ����
    buf[4] = 0x68;            // �̶�: �й����� 86�ߵ�
    nDstLength += GsmBytes2String(buf, &pDst[nDstLength], 5); // ת��5���ֽڵ�Ŀ��PDU��
    nDstLength += GsmInvertNumbers(pDa, &pDst[nDstLength], nLength-2); // ת��TP-DA��Ŀ��PDU�����绰���벻����86
	
    
    // TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��
    buf[0] = 0x00;        // Э���ʶ(TP-PID)
    buf[1] = 0x00;        // �û���Ϣ���뷽ʽ(TP-DCS)
    buf[2] = 0x00;            // ��Ч��(TP-VP)Ϊ5����
    
    // UCS2���뷽ʽ
    buf[3] = srclen*8/7;    // ת��TP-DA��Ŀ��PDU��
    nDstLength += GsmBytes2String(buf, &pDst[nDstLength], 4);
    nDstLength += GsmBytes2String((unsigned char *)pSrc, &pDst[nDstLength], srclen);
    
    // ����Ŀ���ַ�������
    return nDstLength;
}

// 7-bit����
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ����봮ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ����봮����
int gsmEncode7bit(const char* pSrc, char* pDst, int nSrcLength)
{
	int nSrc;         // Դ�ַ����ļ���ֵ
	int nDst;         // Ŀ����봮�ļ���ֵ
	int nChar;        // ��ǰ���ڴ���������ַ��ֽڵ���ţ���Χ��0-7
	unsigned char nLeft;//��һ�ֽڲ��������
	//����ֵ��ʼ��
	nSrc=0;
	nDst=0;
	//��Դ��ÿ8���ֽڷ�Ϊһ�飬ѹ����7���ֽ�
	//ѭ���ô�����̣�ֱ��Դ����������
	//������鲻��8�ֽڣ�Ҳ����ȷ����
	while(nSrc<nSrcLength)
	{
		//ȡԴ�ַ����ļ���ֵ�����3λ
		nChar = nSrc&7;
		//����Դ����ÿ���ֽ�
		if(nChar==0)
		{
			//���ڵ�һ���ֽڣ�ֻ�Ǳ�����������������һ���ֽ�ʱʹ��
			nLeft = *pSrc;
		}
		else
		{
			//���������ֽڣ������ұ߲��������������ӣ��õ�һ��Ŀ������ֽ�
			*pDst = (*pSrc<<(8-nChar))|nLeft;
			//�����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
			nLeft = *pSrc>>nChar;
			//�޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;
		}
		//�޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	}
	// ����Ŀ�괮����
	return nDst;
}

// 7-bit����
// pSrc: Դ���봮ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ���봮����
// ����: Ŀ���ַ������� 
int gsmDecode7bit(const char* pSrc, char* pDst, int nSrcLength)
{
	int nSrc;         // Դ�ַ����ļ���ֵ
	int nDst;         // Ŀ����봮�ļ���ֵ
	int nByte;        // ��ǰ���ڴ���������ֽڵ���ţ���Χ��0-6
	unsigned char nLeft;     // ��һ�ֽڲ��������

	// ����ֵ��ʼ��
	nSrc = 0;
	nDst = 0;
	// �����ֽ���źͲ������ݳ�ʼ��
	nByte = 0;
	nLeft = 0;
	// ��Դ����ÿ7���ֽڷ�Ϊһ�飬��ѹ����8���ֽ�
	// ѭ���ô�����̣�ֱ��Դ���ݱ�������
	// ������鲻��7�ֽڣ�Ҳ����ȷ����
	while(nSrc < nSrcLength)
	{
		//1. ��Դ�ֽ��ұ߲��������������ӣ�ȥ�����λ���õ�һ��Ŀ������ֽ�
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;
		//2. �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
		nLeft = *pSrc >> (7-nByte);
		// �޸�Ŀ�괮��ָ��ͼ���ֵ
		pDst++;
		nDst++;
		// �޸��ֽڼ���ֵ
		nByte++;
		// �޸�Դ����ָ��ͼ���ֵ
		pSrc++; 
		nSrc++;
		// ����һ������һ���ֽ� 
		if(nByte == 7)
		{
			// ����õ�һ��Ŀ������ֽ�
			*pDst = nLeft;
 			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;
			// �����ֽ���źͲ������ݳ�ʼ��
			nByte = 0;
			nLeft = 0;
		}
	}
	*pDst = 0;
	

	// ����Ŀ�괮����
	return nDst;
}



