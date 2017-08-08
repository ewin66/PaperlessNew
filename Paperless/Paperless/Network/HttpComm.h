// http通讯
#pragma once

#include "../stdafx.h"

#define CURL_STATICLIB 
#if defined(_DEBUG) 
#pragma comment(lib, "libcurld.lib") 
#else
#pragma comment(lib, "libcurl.lib") 
#endif
#pragma comment ( lib, "winmm.lib" ) 
#pragma comment ( lib, "ws2_32.lib" ) 
#pragma comment ( lib, "wldap32.lib" ) 

#include <string>
#include <string.h>
using std::string;

#include "../Json/json.h"
extern char g_sTmpFilePath[256];

// http发送的数据结构体
typedef struct S_SEND_DATA
{
	// 发送报文类型
	int nFlag;
	int nSendLen;
	char *pSendBuff;
}SEND_DATA;

/**
 * 功能:通过curl，post发送报文的预处理函数
 * 参数:nFlag：发送的报文类型，0-二维码截图，1-流水号
 *		pStrData：携带的数据，二维码截图时携带截图本地路径，流水号时携带具体流水号
 * 返回值：0 - 成功，-1 - 入参检查失败，其他 - 失败
 **/ 
int SendDataPrepare(int nFlag, const char *pStrData);


// 发送HTTP报文线程函数
UINT ThreadSendDataFunc(LPVOID pParm);

/**
 * 功能:通过curl，post发送报文
 * 参数:pStrUrl：发送的url地址
 *		pStrData：发送的数据
 *		nDataSize：发送数据长度
 *		nSendRet:失败原因字符串可存放大小
 *		pSendRet：失败原因
 * 返回值：0-成功，其他失败
 **/ 
int SendData(const char *pStrUrl, int nDataSize, const char *pStrData, int nSendRet, char *pSendRet);

/**
 * 功能：通过curl，post发送报文的回调函数，接收服务端的返回，存到临时文件g_sTmpFilePath中
 **/ 
size_t RecvData(void *pBuff, size_t nSize, size_t nMemb, void *pUserp);

/**
 * 功能：从文件读取返回报文并解析
 **/ 
int AnalyzeData();