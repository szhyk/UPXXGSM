/************************************************************************* 
* ��Ȩ���У� �㶫���������ܵ������޹�˾
* �ļ��汾�� V1.00
* ��	�ܣ� UCS2�ֿ�
* ������Ϣ��
*	  ������־1��
*		  ���ڣ�	  
*		  �޸��ߣ�	  
*		  �޸����ݣ�  
*		  �޸�ԭ��  
*	  ������־2��
**************************************************************************/
#include <math.h>
#include <string.h>
#include "gprs.h"
#include "gsm.h"
#include "comm.h"
#include "uart.h"
#include "ucs2.h"

typedef struct ucs2_struct {
    uint16	internalcode;
    uint16 	ucs;
}ucs2_struct;


const ucs2_struct UCS2[] = {
    /*��*/	{ 0xB9F1, 0x67DC },
	/*��*/	{ 0xCFE0, 0x76F8 },
    /*��*/	{ 0xBBD8, 0x56DE },
    /*·*/	{ 0xC2B7, 0x8DEF },
    /*Ӧ*/	{ 0xD3A6, 0x5E94 },
    /*��*/	{ 0xB4F0, 0x7B54 },
    /*��*/	{ 0xB3AC, 0x8D85 },
    /*ʱ*/	{ 0xCAB1, 0x65F6 },
    /*��*/	{ 0xC9E8, 0x8BBE },
    /*��*/	{ 0xD6C3, 0x7F6E },
    /*��*/	{ 0xB3C9, 0x6210 },
    /*��*/	{ 0xB9A6, 0x529F },
    /*ʧ*/	{ 0xCAA7, 0x5931 },
    /*��*/	{ 0xB0DC, 0x8D25 },
    /*��*/	{ 0xCFDF, 0x7EBF },
    /*о*/	{ 0xD0BE, 0x82AF },
    /*��*/	{ 0xB2E5, 0x63D2 },
    /*ͷ*/	{ 0xCDB7, 0x5934 },
    /*��*/	{ 0xBCB6, 0x7EA7 },
    /*��*/	{ 0xCEC2, 0x6E29 },
    /*��*/	{ 0xB6C8, 0x5EA6 },
    /*��*/	{ 0xB8E6, 0x544A },
    /*��*/	{ 0xBEAF, 0x8B66 },
    /*��*/	{ 0xB2CE, 0x53C2 },
    /*��*/	{ 0xCAFD, 0x6570 },
    /*��*/	{ 0xD3D0, 0x6709 },
    /*��*/	{ 0xCEF3, 0x8BEF },
    /*��*/	{ 0xD5FD, 0x6B63 },
    /*��*/	{ 0xB3A3, 0x5E38 },
    /*��*/	{ 0xD2EC, 0x5F02 },
    /*��*/	{ 0xB4CE, 0x6B21 },
    /*��*/	{ 0xBBB7, 0x73AF },
    /*��*/	{ 0xBEB3, 0x5883 },
    /*ң*/	{ 0xD2A3, 0x9065 },
    /*��*/	{ 0xD0C5, 0x4FE1 },
    /*��*/	{ 0xB2E2, 0x6D4B },
    /*��*/	{ 0xC1BF, 0x91CF },

	/*��*/	{ 0xB8B4, 0x590D },
	/*��*/	{ 0xB9E9, 0x5F52 },

	/*��*/	{ 0xB5D8, 0x5730 },
	/*ַ*/	{ 0xD6B7, 0x5740 },
	/*Ϣ*/	{ 0xCFA2, 0x606F },
	/*��*/	{ 0xB3A7, 0x5382 },
	/*��*/	{ 0xBCD2, 0x5BB6 },
	/*ά*/	{ 0xCEAC, 0x7EF4 },
	/*��*/	{ 0xBBA4, 0x62A4 },
	/*��*/	{ 0xBAC5, 0x53F7 },
	/*��*/	{ 0xC2EB, 0x7801 },
	/*��*/	{ 0xD3C3, 0x7528 },
	/*��*/	{ 0xBBA7, 0x6237 },
	/*��*/	{ 0xC1D0, 0x5217 },
	/*��*/	{ 0xB1ED, 0x8868 },
	/*��*/	{ 0xB6CC, 0x77ED },
	/*��*/	{ 0xD6D0, 0x4E2D },
	/*��*/	{ 0xD0C4, 0x5FC3 },
	/*��*/	{ 0xD7D3, 0x5B50 },
	/*վ*/	{ 0xD5BE, 0x7AD9 },
	/*��*/	{ 0xD7DC, 0x603B },
	/*��*/	{ 0xD5D9, 0x53EC },
	/*��*/	{ 0xBCE4, 0x95F4 },
	/*��*/	{ 0xB8F4, 0x9694 },
	/*��*/	{ 0xD6D8, 0x91CD },
	/*��*/	{ 0xB7A2, 0x53D1 },
	/*��*/	{ 0xB2A8, 0x6CE2 },
	/*��*/	{ 0xCCD8, 0x7279 },
	/*��*/	{ 0xC2CA, 0x7387 },
	/*ͨ*/	{ 0xCDA8, 0x901A },
	/*��*/	{ 0xD6D5, 0x7EC8 },
	/*��*/	{ 0xB6CB, 0x7AEF },
	/*״*/	{ 0xD7B4, 0x72B6 },
	/*̬*/	{ 0xCCAC, 0x6001 },
    /*��*/	{ 0xA1E6, 0x2103 },

	/*ϵ*/	{ 0xCFB5, 0x7CFB },
	/*ͳ*/	{ 0xCDB3, 0x7EDF },
	/*��*/	{ 0xD6D3, 0x949F },
	/*��*/	{ 0xC4EA, 0x5E74 },
	/*��*/	{ 0xD4C2, 0x6708 },
	/*��*/	{ 0xC8D5, 0x65E5 },
	/*��*/	{ 0xB7D6, 0x5206 },
	/*��*/	{ 0xC3EB, 0x79D2 },

	/*��*/	{ 0xC6F4, 0x542F },
	/*��*/	{ 0xB6AF, 0x52A8 },

	/*��*/	{ 0xB2EE, 0x5DEE },
//add by hyk
	/*��*/	{ 0xB5B1, 0x5F53 },
	/*ǰ*/	{ 0xC7B0, 0x524D },
	/*��*/	{ 0xBACF, 0x5408 },
	/*բ*/	{ 0xD5A2, 0x95F8 },
	/*��*/	{ 0xB3F6, 0x51FA },
	/*��*/	{ 0xBFDA, 0x53E3 },
	/*ͣ*/	{ 0xCDA3, 0x505C },
	/*ֹ*/	{ 0xD6B9, 0x6B62 },
	/*��*/	{ 0xCAD4, 0x8BD5 },
	/*λ*/	{ 0xCEBB, 0x4F4D },
	/*��*/	{ 0xB8BA, 0x8D1F },
	/*��*/	{ 0xBAC9, 0x8377 },
	/*��*/	{ 0xBFAA, 0x5F00 },
	/*��*/	{ 0xB9D8, 0x5173 },
	/*��*/	{ 0xBAF3, 0x540E },
	/*��*/	{ 0xB2BB, 0x4E0D },
	/*ȷ*/	{ 0xC8B7, 0x786E },
	/*Ȧ*/	{ 0xC8A6, 0x5708 },
	/*��*/	{ 0xB9FD, 0x8FC7 },
	/*��*/	{ 0xC1F7, 0x6D41 },
	/*δ*/	{ 0xCEB4, 0x672A },
	/*��*/	{ 0xD2D1, 0x5DF2 },
	/*С*/	{ 0xD0A1, 0x5C0F },
	/*��*/	{ 0xB6D4, 0x5BF9 },
	/*��*/	{ 0xB1B8, 0x5907 },
	/*��*/	{ 0xCEDE, 0x65E0 },
	/*��*/	{ 0xA1A2, 0x3001 },
	/*��*/	{ 0xA1A3, 0x3002},

	/*��*/	{ 0xC9CF,	0x4E0A},
	/*��*/	{ 0xD7D4,	0x81EA },
	/*��*/	{ 0xBBFA,	0x673A },
	/*��*/	{ 0xBFD5,	0x7A7A },
	/*��*/	{ 0xCFD0,	0x95F2 },
	/*��*/	{ 0xD4D8,	0x8F7D },
	/*��*/	{ 0xCAD6,	0x624B },
	/*��*/	{ 0xC6E4,	0x5176 },
	/*��*/	{ 0xCBFB,	0x4ED6 },
	/*��*/	{ 0xD4DD,	0x6682 },
	/*��*/	{ 0xBCC7,	0x8BB0 },
	/*¼*/	{ 0xC2BC,	0x5F55 },
	/*��*/	{ 0xB5DA,	0x7B2C },
	/*��*/	{ 0xCCF5,	0x6761 },
	/*ʼ*/	{ 0xCABC,	0x59CB },
	/*��*/	{ 0xB0B4,	0x6309 },
	/*Ϊ*/	{ 0xCEAA,	0x4E3A },
	/*��*/	{ 0xC8CF,	0x8BA4 },
	/*��*/	{ 0xB2E9,	0x67E5 },
	/*ѯ*/	{ 0xD1AF,	0x8BE2 },
	/*��*/	{ 0xB1E0,	0x7F16 },
	/*��*/	{ 0xB4ED,	0x9519 },
	/*��*/	{ 0xD0E8,	0x9700 },
	/*��*/	{ 0xD3DA,	0x4E8E },
	/*��*/	{ 0xC3FC,	0x547D },
	/*��*/	{ 0xC1EE,	0x4EE4 },
	/*��*/	{ 0xC7EB,	0x8BF7 },
	/*��*/	{ 0xCBCD,	0x9001 },
	/*��*/	{ 0xCFC8,	0x5148 },
	/*��*/	{ 0xBDF8,	0x8FDB },
	/*��*/	{ 0xC8EB,	0x5165 },
	/*��*/	{ 0xD4D9,	0x518D },
	/*��*/	{ 0xC4E3,	0x4F60 },
	/*��*/	{ 0xBAC3,	0x597D },
	/*Ҳ*/	{ 0xD2B2,	0x4E5F },
	/*��*/	{ 0xD0DE,	0x4FEE },
	/*��*/	{ 0xB8C4,	0x6539 },
	/*Ȩ*/	{ 0xC8A8,	0x6743 },
	/*��*/	{ 0xCFDE,	0x9650 },
	/*ɾ*/	{ 0xC9BE,	0x5220 },
	/*��*/	{ 0xB3FD,	0x9664 },
	/*��*/	{ 0xB2D9,	0x64CD },
	/*��*/	{ 0xD7F7,	0x4F5C },
	/*��*/	{ 0xD7E3,	0x8DB3 },
	/*��*/	{ 0xD0C2,	0x65B0 },
	/*Ѷ*/	{ 0xD1B6,	0x8BAF },
	/*��*/	{ 0xB9CA,	0x6545 },
	/*��*/	{ 0xD5CF,	0x969C },
	/*��*/	{ 0xBBD6,	0x6062 },
	/*��*/	{ 0xB4AB,	0x4F20 },
	/*��*/	{ 0xB8D0,	0x611F },
	/*��*/	{ 0xC6F7,	0x5668 },
	/*��*/	{ 0xD1CF,	0x4E25 },
	/*��*/	{ 0xB1A8,	0x62A5 },
	/*��*/	{ 0xCDCB,	0x9000 },
	/*һ*/	{ 0xD2BB,	0x4E00 },
	/*��*/	{ 0xB0E3,	0x822C },
	/*û*/	{ 0xC3BB,	0x6CA1 },
	/*��*/	{ 0xC4DC,	0x80FD },
	/*��*/	{ 0xCFEE,	0x9879 },
	/*Ŀ*/	{ 0xC4BF,	0x76EE },
	/*��*/	{ 0xC3FB,	0x540D },
	/*��*/	{ 0xB3C6,	0x79F0 },
	/*��*/	{ 0xB3A4,	0x957F },
	/*��*/	{ 0xD7EE,	0x6700 },
	/*��*/	{ 0xB6E0,	0x591A },
	/*��*/	{ 0xB8F6,	0x4E2A },
	/*��*/	{ 0xD7D6,	0x5B57 },
	/*��*/	{ 0xCEE5,	0x4E94 },
	/*��*/	{ 0xC9FD,	0x5347 },
	/*Ч*/	{ 0xD0A7,	0x6548 },

};

/**************************************************************************
�������ƣ�HZ2UCS2
�����汾��1.00 
���ߣ�   
�������ڣ�2014.03.13
��������˵������������ת��ΪUCS2
���������
��������:
���������
����ֵ��
***************************************************************************/
void HZ2UCS2(uint16 hzcode, char *pucs2)
{
    uint16 i;
    for(i = 0; i < sizeof(UCS2)/sizeof(UCS2[0]); i++)
    {
        if(hzcode == UCS2[i].internalcode)
        {
			*(pucs2) = (UCS2[i].ucs>>8)&0xFF;
            *(pucs2+1) = UCS2[i].ucs&0xFF;
//            *(pucs2) = UCS2[i].ucs[0];
//             *(pucs2+1) = UCS2[i].ucs[1];
            break;
        }
    }
}

uint16 UCS2ToHZ(const char *pucs2)
{
    uint16 i,dat;
    for(i = 0; i < sizeof(UCS2)/sizeof(UCS2[0]); i++)
    {
//    if( (*pucs2 == UCS2[i].ucs[0]) && (*(pucs2+1) == UCS2[i].ucs[1]) )
			dat = *pucs2;
			dat <<= 8;
			dat |= *(pucs2+1);
			if( dat == UCS2[i].ucs )
				return UCS2[i].internalcode;
    }
		return 0;
}
/**************************************************************************
�������ƣ�Str2UCS2
�����汾��1.00 
���ߣ�   hjy
�������ڣ�2014.05.20
��������˵�����ַ���ת��ΪUCS2��
���������
��������:
���������
����ֵ��UCS2������
***************************************************************************/
int Str2UCS2(const char *pStr, char *pUCS2)
{
    int len;
    uint16 hzcode;
    
    len = 0;
    while (*pStr != '\0')
	{
        if (*pStr > 0x80)
		{
            hzcode = (*pStr << 8) + *(pStr+1);
            HZ2UCS2(hzcode, pUCS2);
            pStr += 2;
        }
		else
		{
            *pUCS2 = 0x00;
            *(pUCS2+1) = *pStr;
            pStr += 1;
        }
        pUCS2 += 2;
        len += 2;
    }
    
    return len;
}

int UCS2ToStr(const char *pUCS2, char *pStr, int iLen)
{
	int len,k;
	uint16 hzcode;
	
	len = 0;
	k = 0;
	while (k < iLen)
	{
		if (*pUCS2 == 0x0)
		{
			*pStr = *(pUCS2+1);
			pStr += 1;
			len += 1;
		}
		else
		{
			hzcode = UCS2ToHZ(pUCS2);
			*pStr = hzcode>>8;
			*(pStr+1) = hzcode&0xFF;
			pStr += 2;
			len += 2;
		}
		pUCS2 += 2;
		k += 2;
	}
	
	return len;
}





