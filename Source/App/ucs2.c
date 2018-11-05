/************************************************************************* 
* 版权所有： 广东阿尔派智能电网有限公司
* 文件版本： V1.00
* 功	能： UCS2字库
* 更新信息：
*	  更新日志1：
*		  日期：	  
*		  修改者：	  
*		  修改内容：  
*		  修改原因：  
*	  更新日志2：
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
    /*柜*/	{ 0xB9F1, 0x67DC },
	/*相*/	{ 0xCFE0, 0x76F8 },
    /*回*/	{ 0xBBD8, 0x56DE },
    /*路*/	{ 0xC2B7, 0x8DEF },
    /*应*/	{ 0xD3A6, 0x5E94 },
    /*答*/	{ 0xB4F0, 0x7B54 },
    /*超*/	{ 0xB3AC, 0x8D85 },
    /*时*/	{ 0xCAB1, 0x65F6 },
    /*设*/	{ 0xC9E8, 0x8BBE },
    /*置*/	{ 0xD6C3, 0x7F6E },
    /*成*/	{ 0xB3C9, 0x6210 },
    /*功*/	{ 0xB9A6, 0x529F },
    /*失*/	{ 0xCAA7, 0x5931 },
    /*败*/	{ 0xB0DC, 0x8D25 },
    /*线*/	{ 0xCFDF, 0x7EBF },
    /*芯*/	{ 0xD0BE, 0x82AF },
    /*插*/	{ 0xB2E5, 0x63D2 },
    /*头*/	{ 0xCDB7, 0x5934 },
    /*级*/	{ 0xBCB6, 0x7EA7 },
    /*温*/	{ 0xCEC2, 0x6E29 },
    /*度*/	{ 0xB6C8, 0x5EA6 },
    /*告*/	{ 0xB8E6, 0x544A },
    /*警*/	{ 0xBEAF, 0x8B66 },
    /*参*/	{ 0xB2CE, 0x53C2 },
    /*数*/	{ 0xCAFD, 0x6570 },
    /*有*/	{ 0xD3D0, 0x6709 },
    /*误*/	{ 0xCEF3, 0x8BEF },
    /*正*/	{ 0xD5FD, 0x6B63 },
    /*常*/	{ 0xB3A3, 0x5E38 },
    /*异*/	{ 0xD2EC, 0x5F02 },
    /*次*/	{ 0xB4CE, 0x6B21 },
    /*环*/	{ 0xBBB7, 0x73AF },
    /*境*/	{ 0xBEB3, 0x5883 },
    /*遥*/	{ 0xD2A3, 0x9065 },
    /*信*/	{ 0xD0C5, 0x4FE1 },
    /*测*/	{ 0xB2E2, 0x6D4B },
    /*量*/	{ 0xC1BF, 0x91CF },

	/*复*/	{ 0xB8B4, 0x590D },
	/*归*/	{ 0xB9E9, 0x5F52 },

	/*地*/	{ 0xB5D8, 0x5730 },
	/*址*/	{ 0xD6B7, 0x5740 },
	/*息*/	{ 0xCFA2, 0x606F },
	/*厂*/	{ 0xB3A7, 0x5382 },
	/*家*/	{ 0xBCD2, 0x5BB6 },
	/*维*/	{ 0xCEAC, 0x7EF4 },
	/*护*/	{ 0xBBA4, 0x62A4 },
	/*号*/	{ 0xBAC5, 0x53F7 },
	/*码*/	{ 0xC2EB, 0x7801 },
	/*用*/	{ 0xD3C3, 0x7528 },
	/*户*/	{ 0xBBA7, 0x6237 },
	/*列*/	{ 0xC1D0, 0x5217 },
	/*表*/	{ 0xB1ED, 0x8868 },
	/*短*/	{ 0xB6CC, 0x77ED },
	/*中*/	{ 0xD6D0, 0x4E2D },
	/*心*/	{ 0xD0C4, 0x5FC3 },
	/*子*/	{ 0xD7D3, 0x5B50 },
	/*站*/	{ 0xD5BE, 0x7AD9 },
	/*总*/	{ 0xD7DC, 0x603B },
	/*召*/	{ 0xD5D9, 0x53EC },
	/*间*/	{ 0xBCE4, 0x95F4 },
	/*隔*/	{ 0xB8F4, 0x9694 },
	/*重*/	{ 0xD6D8, 0x91CD },
	/*发*/	{ 0xB7A2, 0x53D1 },
	/*波*/	{ 0xB2A8, 0x6CE2 },
	/*特*/	{ 0xCCD8, 0x7279 },
	/*率*/	{ 0xC2CA, 0x7387 },
	/*通*/	{ 0xCDA8, 0x901A },
	/*终*/	{ 0xD6D5, 0x7EC8 },
	/*端*/	{ 0xB6CB, 0x7AEF },
	/*状*/	{ 0xD7B4, 0x72B6 },
	/*态*/	{ 0xCCAC, 0x6001 },
    /*℃*/	{ 0xA1E6, 0x2103 },

	/*系*/	{ 0xCFB5, 0x7CFB },
	/*统*/	{ 0xCDB3, 0x7EDF },
	/*钟*/	{ 0xD6D3, 0x949F },
	/*年*/	{ 0xC4EA, 0x5E74 },
	/*月*/	{ 0xD4C2, 0x6708 },
	/*日*/	{ 0xC8D5, 0x65E5 },
	/*分*/	{ 0xB7D6, 0x5206 },
	/*秒*/	{ 0xC3EB, 0x79D2 },

	/*启*/	{ 0xC6F4, 0x542F },
	/*动*/	{ 0xB6AF, 0x52A8 },

	/*差*/	{ 0xB2EE, 0x5DEE },
//add by hyk
	/*当*/	{ 0xB5B1, 0x5F53 },
	/*前*/	{ 0xC7B0, 0x524D },
	/*合*/	{ 0xBACF, 0x5408 },
	/*闸*/	{ 0xD5A2, 0x95F8 },
	/*出*/	{ 0xB3F6, 0x51FA },
	/*口*/	{ 0xBFDA, 0x53E3 },
	/*停*/	{ 0xCDA3, 0x505C },
	/*止*/	{ 0xD6B9, 0x6B62 },
	/*试*/	{ 0xCAD4, 0x8BD5 },
	/*位*/	{ 0xCEBB, 0x4F4D },
	/*负*/	{ 0xB8BA, 0x8D1F },
	/*荷*/	{ 0xBAC9, 0x8377 },
	/*开*/	{ 0xBFAA, 0x5F00 },
	/*关*/	{ 0xB9D8, 0x5173 },
	/*后*/	{ 0xBAF3, 0x540E },
	/*不*/	{ 0xB2BB, 0x4E0D },
	/*确*/	{ 0xC8B7, 0x786E },
	/*圈*/	{ 0xC8A6, 0x5708 },
	/*过*/	{ 0xB9FD, 0x8FC7 },
	/*流*/	{ 0xC1F7, 0x6D41 },
	/*未*/	{ 0xCEB4, 0x672A },
	/*已*/	{ 0xD2D1, 0x5DF2 },
	/*小*/	{ 0xD0A1, 0x5C0F },
	/*对*/	{ 0xB6D4, 0x5BF9 },
	/*备*/	{ 0xB1B8, 0x5907 },
	/*无*/	{ 0xCEDE, 0x65E0 },
	/*、*/	{ 0xA1A2, 0x3001 },
	/*。*/	{ 0xA1A3, 0x3002},

	/*上*/	{ 0xC9CF,	0x4E0A},
	/*自*/	{ 0xD7D4,	0x81EA },
	/*机*/	{ 0xBBFA,	0x673A },
	/*空*/	{ 0xBFD5,	0x7A7A },
	/*闲*/	{ 0xCFD0,	0x95F2 },
	/*载*/	{ 0xD4D8,	0x8F7D },
	/*手*/	{ 0xCAD6,	0x624B },
	/*其*/	{ 0xC6E4,	0x5176 },
	/*他*/	{ 0xCBFB,	0x4ED6 },
	/*暂*/	{ 0xD4DD,	0x6682 },
	/*记*/	{ 0xBCC7,	0x8BB0 },
	/*录*/	{ 0xC2BC,	0x5F55 },
	/*第*/	{ 0xB5DA,	0x7B2C },
	/*条*/	{ 0xCCF5,	0x6761 },
	/*始*/	{ 0xCABC,	0x59CB },
	/*按*/	{ 0xB0B4,	0x6309 },
	/*为*/	{ 0xCEAA,	0x4E3A },
	/*认*/	{ 0xC8CF,	0x8BA4 },
	/*查*/	{ 0xB2E9,	0x67E5 },
	/*询*/	{ 0xD1AF,	0x8BE2 },
	/*编*/	{ 0xB1E0,	0x7F16 },
	/*错*/	{ 0xB4ED,	0x9519 },
	/*需*/	{ 0xD0E8,	0x9700 },
	/*于*/	{ 0xD3DA,	0x4E8E },
	/*命*/	{ 0xC3FC,	0x547D },
	/*令*/	{ 0xC1EE,	0x4EE4 },
	/*请*/	{ 0xC7EB,	0x8BF7 },
	/*送*/	{ 0xCBCD,	0x9001 },
	/*先*/	{ 0xCFC8,	0x5148 },
	/*进*/	{ 0xBDF8,	0x8FDB },
	/*入*/	{ 0xC8EB,	0x5165 },
	/*再*/	{ 0xD4D9,	0x518D },
	/*你*/	{ 0xC4E3,	0x4F60 },
	/*好*/	{ 0xBAC3,	0x597D },
	/*也*/	{ 0xD2B2,	0x4E5F },
	/*修*/	{ 0xD0DE,	0x4FEE },
	/*改*/	{ 0xB8C4,	0x6539 },
	/*权*/	{ 0xC8A8,	0x6743 },
	/*限*/	{ 0xCFDE,	0x9650 },
	/*删*/	{ 0xC9BE,	0x5220 },
	/*除*/	{ 0xB3FD,	0x9664 },
	/*操*/	{ 0xB2D9,	0x64CD },
	/*作*/	{ 0xD7F7,	0x4F5C },
	/*足*/	{ 0xD7E3,	0x8DB3 },
	/*新*/	{ 0xD0C2,	0x65B0 },
	/*讯*/	{ 0xD1B6,	0x8BAF },
	/*故*/	{ 0xB9CA,	0x6545 },
	/*障*/	{ 0xD5CF,	0x969C },
	/*恢*/	{ 0xBBD6,	0x6062 },
	/*传*/	{ 0xB4AB,	0x4F20 },
	/*感*/	{ 0xB8D0,	0x611F },
	/*器*/	{ 0xC6F7,	0x5668 },
	/*严*/	{ 0xD1CF,	0x4E25 },
	/*报*/	{ 0xB1A8,	0x62A5 },
	/*退*/	{ 0xCDCB,	0x9000 },
	/*一*/	{ 0xD2BB,	0x4E00 },
	/*般*/	{ 0xB0E3,	0x822C },
	/*没*/	{ 0xC3BB,	0x6CA1 },
	/*能*/	{ 0xC4DC,	0x80FD },
	/*项*/	{ 0xCFEE,	0x9879 },
	/*目*/	{ 0xC4BF,	0x76EE },
	/*名*/	{ 0xC3FB,	0x540D },
	/*称*/	{ 0xB3C6,	0x79F0 },
	/*长*/	{ 0xB3A4,	0x957F },
	/*最*/	{ 0xD7EE,	0x6700 },
	/*多*/	{ 0xB6E0,	0x591A },
	/*个*/	{ 0xB8F6,	0x4E2A },
	/*字*/	{ 0xD7D6,	0x5B57 },
	/*五*/	{ 0xCEE5,	0x4E94 },
	/*升*/	{ 0xC9FD,	0x5347 },
	/*效*/	{ 0xD0A7,	0x6548 },

};

/**************************************************************************
函数名称：HZ2UCS2
函数版本：1.00 
作者：   
创建日期：2014.03.13
函数功能说明：汉字内码转换为UCS2
输入参数：
其他输入:
输出参数：
返回值：
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
函数名称：Str2UCS2
函数版本：1.00 
作者：   hjy
创建日期：2014.05.20
函数功能说明：字符串转换为UCS2串
输入参数：
其他输入:
输出参数：
返回值：UCS2串长度
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





