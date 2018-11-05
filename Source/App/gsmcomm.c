/************************************************************************* 
* 版权所有： 广东阿尔派智能电网有限公司
* 文件版本： V1.00
* 文件名称： gsmcomm.c
* 生成日期： 2014.04.29
* 作	者： hjy
* 功	能： GSM通信通用函数
* 更新信息：
*	  更新日志1：
*		  日期：	  
*		  修改者：	  
*		  修改内容：  
*		  修改原因：  
*	  更新日志2：
**************************************************************************/
#include "gsmcomm.h"

#include <string.h>

/**************************************************************************
函数名称：GsmString2Bytes
函数版本：1.00 
作者：   hjy
创建日期：20140429
函数功能说明：可打印字符串转换为字节数据
输入参数：pSrc:源数据; pDst:目标数据; nSrcLength:长度
其他输入:
输出参数：
返回值：
***************************************************************************/
int GsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	int i;
    for(i=0; i<nSrcLength; i += 2)
    {
        // 输出高4位
        if(*pSrc>='0' && *pSrc<='9')
        {
            *pDst = (*pSrc - '0') << 4;
        }
        else
        {
            *pDst = (*pSrc - 'A' + 10) << 4;
        }
        
        pSrc++;
        
        // 输出低4位
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
    
    // 返回目标数据长度
    return nSrcLength / 2;
}


/**************************************************************************
函数名称：GsmBytes2String
函数版本：1.00 
作者：   hjy
创建日期：20140429
函数功能说明：字节数据转换为可打印字符串
如：{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01"
输入参数：pSrc:源数据; pDst:目标数据; nSrcLength:源数据长度
其他输入:
输出参数：
返回值：目标字符串长度
***************************************************************************/
int GsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    const char tab[]="0123456789ABCDEF";    // 0x0-0xf的字符查找表
    int i;
    for( i=0; i<nSrcLength; i++)
    {
        // 输出低4位
        *pDst++ = tab[*pSrc >> 4];
        
        // 输出高4位
        *pDst++ = tab[*pSrc & 0x0f];
        
        pSrc++;
    }
    
    // 输出字符串加个结束符
    *pDst = '\0';
    
    // 返回目标字符串长度
    return nSrcLength * 2;
}


/**************************************************************************
函数名称：GsmSerializeNumbers
函数版本：1.00 
作者：   hjy
创建日期：20140429
函数功能说明： 两两颠倒的字符串转换为正常顺序的字符串
如："683158812764F8" --> "8613851872468"
输入参数：pSrc:源数据; pDst:目标数据; nSrcLength:长度
其他输入:
输出参数：
返回值：
***************************************************************************/
int GsmSerializeNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength;   // 目标字符串长度
    char ch;          // 用于保存一个字符
		int i;
    
    // 复制串长度
    nDstLength = nSrcLength;
    
    if (nSrcLength & 01)
	{
        nSrcLength = nSrcLength >> 1 + 1;
    }
	else
	{
        nSrcLength = nSrcLength >> 1;
    }
    
    // 两两颠倒
    for(i=0; i<nSrcLength; i++)
    {
        ch = *pSrc++;        // 保存先出现的字符
        *pDst++ = *pSrc++;   // 复制后出现的字符
        *pDst++ = ch;        // 复制先出现的字符
    }
    
    // 最后的字符是'F'吗？
    if(*(pDst-1) == 'F')
    {
        pDst--;
        nDstLength--;        // 目标字符串长度减1
    }
    
    // 输出字符串加个结束符
    *pDst = '\0';
    
    // 返回目标字符串长度
    return nDstLength;
}

/**************************************************************************
函数名称：GsmSerializeNumbers
函数版本：1.00 
作者：   hjy
创建日期：20140429
函数功能说明：正常顺序的字符串转换为两两颠倒的字符串，若长度为奇数，补'F'凑成偶数
如："8613851872468" --> "683158812764F8"
输入参数：pSrc:源数据; pDst:目标数据; nSrcLength:长度
其他输入:
输出参数：
返回值：目标字符串长度
***************************************************************************/
int GsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
    int nDstLength,i;   // 目标字符串长度
    char ch;          // 用于保存一个字符
    
    // 复制串长度
    nDstLength = nSrcLength;
    
    // 两两颠倒
    for(i=0; i < nSrcLength; i+=2)
    {
        ch = *pSrc++;        // 保存先出现的字符
        *pDst++ = *pSrc++;   // 复制后出现的字符
        *pDst++ = ch;        // 复制先出现的字符
    }
    
    // 源串长度是奇数吗？
    if(nSrcLength & 1)
    {
        *(pDst-2) = 'F';     // 补'F'
        nDstLength++;        // 目标串长度加1
    }
    
    // 输出字符串加个结束符
    *pDst = '\0';
    
    // 返回目标字符串长度
    return nDstLength;
}



/**************************************************************************
函数名称：GsmDecodePdu
函数版本：1.00 
作者：   hjy
创建日期：20140429
函数功能说明：GSM解码PDU包
输入参数：rxlen:现接收长度
其他输入:pPdu:PDU报文
输出参数：pOa:发送方号码;  pUdata:数据包
返回值：信息长度字节 收到"启动"
//08 91 683108705505F0 24 0D 91 683115908076F2 0008 71 11 01 41 10 30 23 04 542F 52A8 \r\n\r\n 'O' 'K' \r\n
//91为国际，后面号码要带86
//08 91 683108705505F0 --SMSC地址信息 08总长度，字节长度
//24 
//0D 91 683115908076F2 
//00 
//08 --编码
//71 11 01 41 10 30 23 
//04 542F 52A8 \r\n\r\n 'O' 'K' \r\n
***************************************************************************/
int GsmDecodePdu(char* pPdu, char *pOa, char *pScts, char* pUdata)
{
    int nDstLength;
    unsigned char tmp;
	char* p1=pPdu;

    char scts[12];
    
    // SMSC地址信息段08
    GsmString2Bytes(pPdu, &tmp, 2);    		/* 取SCA长度,字节长度*/
    tmp = tmp * 2;
    pPdu += (2+tmp+2);						/*跳过SCA,PUD(24)偏移至OA*/
    
	//TPDU
    //OA地址信息段0D 91 68 13510908672F
	//00 -- 普通GSM 类型，点到点方式
    GsmString2Bytes(pPdu, &tmp, 2);			/*取OA长度，号码的字符长度,不包括号码类型91*/
    if(tmp & 1) tmp += 1;    				/* 调整奇偶性*/
    pPdu += 6;
    GsmSerializeNumbers(pPdu, pOa, tmp-2);	/*取OA*/
    pPdu += tmp;							/*跳过OA,PID,偏移至DCS*/

	//取编码08
    GsmString2Bytes(pPdu, &tmp, 2);			/*取两个字符*/
	*p1 = tmp;								//保存数据编码方案到源的第一个字节08--UCS2,04-8bit,00-7bit
    pPdu += 2;								/*偏移至SCTS*/
    
    //SCTS信息段71 11 01 41 10 30
	//23 时区
    GsmSerializeNumbers(pPdu, scts, 12);	/*取SCTS,时间*/
    GsmString2Bytes(scts, (unsigned char *)pScts, 12);		/*转换格式*/
    pPdu += 7*2;							/*偏移至UDL*/
    
    //UDL,UD信息段04 542F 52A8
    GsmString2Bytes(pPdu, &tmp, 2);			/*取UDL 信息长度，字节*/
    nDstLength = tmp;
    pPdu += 2;
    GsmString2Bytes(pPdu, (unsigned char *)pUdata, nDstLength*2);	/*取UD*/
	*(pUdata+nDstLength) = '\0';
	*(pUdata+nDstLength+1) = '\0';
	
    
    return nDstLength;
}


/**************************************************************************
函数名称：GsmEncodePdu
函数版本：1.00 
作者：   hjy
创建日期：20140429
函数功能说明：GSM编码PDU包
输入参数：rxlen:现接收长度
其他输入:pPdu:PDU报文
输出参数：pSca:短信服务中心号码; pDa:目标代码; pSrc:源数据(UCS16);
pDst:目标数据; srclen:源数据长度;
返回值：
发送方PDU格式：
SCA	--短信中心：有两种方式，一种为00，一种13 08 91 68 31 08 70 55 05 F0 13
PDU-Type 协议数据单元类型：11
MR	     短信发送参考数目：00
DA		接收方地址（手机号码）：0D 91 68 31 15 90 80 76 F2
PID		参数显示消息中心以何种方式处理消息内容:00
DCS		参数显示用户数据编码方案:08
VP		参数显示消息有效期:00
UDL		用户数据长度:
UD		用户数据:

***************************************************************************/
int GsmEncodePdu_UCS2(const char *pSca, const char *pDa, const char* pSrc, char* pDst, int srclen)
{
    int nLength;             // 内部用的串长度
    int nDstLength;          // 目标PDU串长度
    unsigned char buf[32]; // 内部用的缓冲区
    
    // SMSC地址信息段
    nLength = 13;//strlen(pSca);    // SMSC地址字符串的长度   
    buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;    // SMSC地址信息长度
    buf[1] = 0x91;        // 固定: 用国际格式号码
    nDstLength = GsmBytes2String(buf, pDst, 2);        // 转换2个字节到目标PDU串
    nDstLength += GsmInvertNumbers(pSca, &pDst[nDstLength], nLength);    // 转换SMSC到目标PDU串
    
    // TPDU段基本参数、目标地址等
    nLength = 13;//strlen(pDa);    // TP-DA地址字符串的长度
    buf[0] = 0x11;            // 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
    buf[1] = 0;               // TP-MR=0
    buf[2] = (char)nLength;   // 目标地址数字个数(TP-DA地址字符串真实长度)
    buf[3] = 0x91;            // 固定: 用国际格式号码
    buf[4] = 0x68;            // 固定: 中国区号 86颠倒
    nDstLength += GsmBytes2String(buf, &pDst[nDstLength], 5); // 转换5个字节到目标PDU串
    nDstLength += GsmInvertNumbers(pDa, &pDst[nDstLength], nLength-2); // 转换TP-DA到目标PDU串，电话号码不含有86

    
    // TPDU段协议标识、编码方式、用户信息等
    buf[0] = 0x00;        // 协议标识(TP-PID)
    buf[1] = 0x08;        // 用户信息编码方式(TP-DCS)
    buf[2] = 0x00;            // 有效期(TP-VP)为5分钟
    
    // UCS2编码方式
    buf[3] = srclen;    // 转换TP-DA到目标PDU串
    nDstLength += GsmBytes2String(buf, &pDst[nDstLength], 4);
    nDstLength += GsmBytes2String((unsigned char *)pSrc, &pDst[nDstLength], srclen);
    
    // 返回目标字符串长度
    return nDstLength;
}

int GsmEncodePdu_7bit(const char *pSca, const char *pDa, const char* pSrc, char* pDst, int srclen)
{
	  int nLength;             // 内部用的串长度
    int nDstLength;          // 目标PDU串长度
    unsigned char buf[32]; // 内部用的缓冲区
    
    // SMSC地址信息段
    nLength = 13;//strlen(pSca);    // SMSC地址字符串的长度   
    buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;    // SMSC地址信息长度
    buf[1] = 0x91;        // 固定: 用国际格式号码
    nDstLength = GsmBytes2String(buf, pDst, 2);        // 转换2个字节到目标PDU串
    nDstLength += GsmInvertNumbers(pSca, &pDst[nDstLength], nLength);    // 转换SMSC到目标PDU串
    
    // TPDU段基本参数、目标地址等
    nLength = 13;//strlen(pDa);    // TP-DA地址字符串的长度
    buf[0] = 0x11;            // 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
    buf[1] = 0;               // TP-MR=0
    buf[2] = (char)nLength;   // 目标地址数字个数(TP-DA地址字符串真实长度)
    buf[3] = 0x91;            // 固定: 用国际格式号码
    buf[4] = 0x68;            // 固定: 中国区号 86颠倒
    nDstLength += GsmBytes2String(buf, &pDst[nDstLength], 5); // 转换5个字节到目标PDU串
    nDstLength += GsmInvertNumbers(pDa, &pDst[nDstLength], nLength-2); // 转换TP-DA到目标PDU串，电话号码不含有86
	
    
    // TPDU段协议标识、编码方式、用户信息等
    buf[0] = 0x00;        // 协议标识(TP-PID)
    buf[1] = 0x00;        // 用户信息编码方式(TP-DCS)
    buf[2] = 0x00;            // 有效期(TP-VP)为5分钟
    
    // UCS2编码方式
    buf[3] = srclen*8/7;    // 转换TP-DA到目标PDU串
    nDstLength += GsmBytes2String(buf, &pDst[nDstLength], 4);
    nDstLength += GsmBytes2String((unsigned char *)pSrc, &pDst[nDstLength], srclen);
    
    // 返回目标字符串长度
    return nDstLength;
}

// 7-bit编码
// pSrc: 源字符串指针
// pDst: 目标编码串指针
// nSrcLength: 源字符串长度
// 返回: 目标编码串长度
int gsmEncode7bit(const char* pSrc, char* pDst, int nSrcLength)
{
	int nSrc;         // 源字符串的计数值
	int nDst;         // 目标编码串的计数值
	int nChar;        // 当前正在处理的组内字符字节的序号，范围是0-7
	unsigned char nLeft;//上一字节残余的数据
	//计数值初始化
	nSrc=0;
	nDst=0;
	//将源串每8个字节分为一组，压缩成7个字节
	//循环该处理过程，直至源串被处理完
	//如果分组不到8字节，也能正确处理
	while(nSrc<nSrcLength)
	{
		//取源字符串的计数值的最低3位
		nChar = nSrc&7;
		//处理源串的每个字节
		if(nChar==0)
		{
			//组内第一个字节，只是保存起来，待处理下一个字节时使用
			nLeft = *pSrc;
		}
		else
		{
			//组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节
			*pDst = (*pSrc<<(8-nChar))|nLeft;
			//将该字节剩下的左边部分，作为残余数据保存起来
			nLeft = *pSrc>>nChar;
			//修改目标串的指针和计数值
			pDst++;
			nDst++;
		}
		//修改源串的指针和计数值
		pSrc++;
		nSrc++;
	}
	// 返回目标串长度
	return nDst;
}

// 7-bit解码
// pSrc: 源编码串指针
// pDst: 目标字符串指针
// nSrcLength: 源编码串长度
// 返回: 目标字符串长度 
int gsmDecode7bit(const char* pSrc, char* pDst, int nSrcLength)
{
	int nSrc;         // 源字符串的计数值
	int nDst;         // 目标解码串的计数值
	int nByte;        // 当前正在处理的组内字节的序号，范围是0-6
	unsigned char nLeft;     // 上一字节残余的数据

	// 计数值初始化
	nSrc = 0;
	nDst = 0;
	// 组内字节序号和残余数据初始化
	nByte = 0;
	nLeft = 0;
	// 将源数据每7个字节分为一组，解压缩成8个字节
	// 循环该处理过程，直至源数据被处理完
	// 如果分组不到7字节，也能正确处理
	while(nSrc < nSrcLength)
	{
		//1. 将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
		*pDst = ((*pSrc << nByte) | nLeft) & 0x7f;
		//2. 将该字节剩下的左边部分，作为残余数据保存起来
		nLeft = *pSrc >> (7-nByte);
		// 修改目标串的指针和计数值
		pDst++;
		nDst++;
		// 修改字节计数值
		nByte++;
		// 修改源串的指针和计数值
		pSrc++; 
		nSrc++;
		// 到了一组的最后一个字节 
		if(nByte == 7)
		{
			// 额外得到一个目标解码字节
			*pDst = nLeft;
 			// 修改目标串的指针和计数值
			pDst++;
			nDst++;
			// 组内字节序号和残余数据初始化
			nByte = 0;
			nLeft = 0;
		}
	}
	*pDst = 0;
	

	// 返回目标串长度
	return nDst;
}



