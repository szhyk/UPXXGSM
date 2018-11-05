
#ifndef _GSMCOMM_H
#define _GSMCOMM_H

int GsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength);
int GsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength);


int GsmDecodePdu(char* pPdu, char *pOa, char *pScts, char* pUdata);
int GsmEncodePdu_UCS2(const char *pSca, const char *pDa, const char* pSrc, char* pDst, int srclen);
int GsmEncodePdu_7bit(const char *pSca, const char *pDa, const char* pSrc, char* pDst, int srclen);

int gsmDecode7bit(const char* pSrc, char* pDst, int nSrcLength);
int gsmEncode7bit(const char* pSrc, char* pDst, int nSrcLength);

#endif

