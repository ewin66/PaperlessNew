#ifndef _CENT_IDCARD_H_
#define _CENT_IDCARD_H_

#include "GlobalDef.h"
#include "IDCardDef.h"

#ifndef _STDOUT
#define _STDOUT _stdcall 
#endif

#ifdef __cplusplus
extern "C" {
#endif


/************************************************************************************
* 函数名：CT_ReadIDCard
*  
* 功能：  读二代证
*  
*参数：   pszDevice[in]-----通讯设备,SDT,HID,PCSC,Window下串口为COM1...
*         BpNo[in]------扩展口，NULL或""—无扩展口，其他—扩展口
*         nBaud[in]-----波特率
*         nTimeOut[in]--超时，单位：s
*         szHeadDir[in]--头像保存目录，取值为NULL或“”时取消保存图片
*         pInfo[out]----二代证文字信息
*  
*返回值： ERR_SUCCES---成功
*         ERR_DEVICE---设备错误
*         ERR_SEND-----发送错误
*         ERR_RECV-----接收错误
*         ERR_TIMEOUT--超时
*         ERR_CHECK----校验错误
*         ERR_NOCARD---无二代证
************************************************************************************/
long _STDOUT CT_ReadIDCard( const char *pszDevice, 
							const char *szBpNo,
							const long  nBaud,
							const long  nTimeOut, 
							char* szHeadDir, 
							PERSONINFO* pInfo );


long _STDOUT CT_ReadIDCardForeigner( const char *pszDevice, 
									const char* szBpNo,
									const long  nBaud,
									const long  nTimeOut, 
									char* szHeadDir, 
									PERSONINFO* pInfo );

//不建议使用
long _STDOUT CT_SaveFrontImg( const PERSONINFO* pInfo, char* szFrontImg );

long _STDOUT CT_SaveFrontImgEx( const PERSONINFO* pInfo, char* szFrontImg, char *pLogoMsg, int  nMakeLogo );

//不建议使用
long _STDOUT CT_SaveBackImg( const PERSONINFO* pInfo, char* szBackImg );


//不建议使用
long _STDOUT CT_SaveCardImg( const PERSONINFO* pInfo, char* szCardImg, BOOL bMakeLogo );


//不建议使用
long _STDOUT CT_SaveHeadImg( const PERSONINFO* pInfo, char* szHeadImg );


 //有水印可选
long _STDOUT CT_SaveImg( int nImgType, BOOL nMaker, const PERSONINFO* pInfo, char* szImg );


long _STDOUT CT_GetImgBase64( int nImgType, const PERSONINFO* pInfo, char* szBase64, int nBase64 );


long _STDOUT CT_CancelRead( );


#ifdef __cplusplus
}
#endif


#endif //_CENT_IDCARD_H_