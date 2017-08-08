#include "HttpComm.h"

#include "../Paperless.h"
#include "../PaperlessDlg.h"
#include "../Curl/curl.h"
#include "../utils.h"
#include "../MyTTrace.h"
#include "../CBase64.h"

char g_sTmpFilePath[256] = {0};
int nCount = 0;
// 发送报文结构体，主要用于线程传递
SEND_DATA g_mSend_data;

// 不同窗体之间信息交互
#define BUFFER_LEN 1024
char sendBuff[BUFFER_LEN] = {0};


/**
 * 功能:通过curl，post发送报文的预处理函数
 * 参数:nFlag：发送的报文类型，0-二维码截图，1-流水号
 *		pStrData：携带的数据，二维码截图时携带截图本地路径，流水号时携带具体流水号
 * 返回值：0 - 成功，-1 - 入参检查失败，其他 - 失败
 **/ 
int SendDataPrepare(int nFlag, const char *pStrData)
{
	GtWriteTrace(30, "%s:%d: 进入发送报文预处理函数......", __FUNCTION__, __LINE__);
	if (pStrData == NULL)
	{
		return -1;
	}
	int nRet = 0;
	// 存储待发送报文
	string strSendData;
	// 初始化http报文数据结构体，此参数在线程中使用
	memset(&g_mSend_data, 0, sizeof(SEND_DATA));
	// 判断发送的报文类型，组织发送的报文
	if (nFlag == 0)
	{
		g_mSend_data.nFlag = 0;
		GtWriteTrace(30, "%s:%d: 组织发送截图报文......", __FUNCTION__, __LINE__);
		// 发送二维码截图，读取截图，组织发送的报文
		// 读取 文件内容
		FILE * pFile= NULL;
		// 存储截图文件临时字符串
		char *fileBuffer = NULL;
		// 文件长度
		long lSize = 0;
		// base64编码对象
		ZBase64 zBase;
		// base64编码后的字符串
		string encodeBase64;
		do
		{
			size_t result = 0;
			fopen_s (&pFile, pStrData, "rb");
			if (pFile == NULL)
			{
				char errMsg[256] = {0};
				strerror_s(errMsg, sizeof(errMsg)-1, errno);
				GtWriteTrace(30, "%s:%d: 打开截图文件[%s]失败, error=[%d][%s]!", __FUNCTION__, __LINE__, pStrData, errno, errMsg);
				nRet = -3;
				break;
			}
			fseek (pFile, 0, SEEK_END);
			lSize = ftell(pFile);
			rewind (pFile);
			// 判断文件是否大于10M
			if (lSize > 1024 * 1024 * 10)
			{
				GtWriteTrace(30, "%s:%d: 截图文件过大≈%.2lfM(大于10M)!", __FUNCTION__, __LINE__, (((double)lSize) / 1024 / 1024));
				nRet = -4;
				break;
			}
			// 分配内存存储整个文件
			fileBuffer = (char*) malloc(sizeof(char) * lSize);
			if (fileBuffer == NULL)
			{
				char errMsg[256] = {0};
				strerror_s(errMsg, sizeof(errMsg)-1, errno);
				GtWriteTrace(30, "%s:%d: 分配内存存储整个文件时失败, error=[%d][%s]!", __FUNCTION__, __LINE__, errno, errMsg);
				nRet = -5;
				break;
			}
			// 将文件拷贝到fileBuffer中
			result = fread(fileBuffer, 1, lSize, pFile);
			if (result != lSize)
			{
				char errMsg[256] = {0};
				strerror_s(errMsg, sizeof(errMsg)-1, errno);
				GtWriteTrace(30, "%s:%d: 读取整个文件失败, error=[%d][%s]!", __FUNCTION__, __LINE__, errno, errMsg);
				nRet = -6;
				break;
			}
			// 读取截图文件成功，进行base64编码
			encodeBase64 = zBase.Encode((const unsigned char*)fileBuffer, (int)lSize);
			// 文件读取完毕，关闭文件
			if (pFile != NULL)
			{
				fclose (pFile);
				pFile = NULL;
			}
			// 清理内存
			if (fileBuffer != NULL)
			{
				free(fileBuffer);
				fileBuffer = NULL;
			}
			// 组待发送的json报文
			Json::Value msgStr_json;//表示一个json格式的对象
			msgStr_json["type"] = "0";
			msgStr_json["num"] = "";
			msgStr_json["picSource"] = encodeBase64.c_str();
			// 转string
			strSendData = msgStr_json.toStyledString();
			// 组织报文结束
			nRet = 0;
		}while(0);
		// 清理内存，关闭文件
		if (fileBuffer != NULL)
		{
			free(fileBuffer);
			fileBuffer = NULL;
		}
		if (pFile != NULL)
		{
			fclose (pFile);
			pFile = NULL;
		}
		if (nRet != 0)
		{
			return nRet;
		}
	}
	else if (nFlag == 1)
	{
		g_mSend_data.nFlag = 1;
		Json::Value msgStr_json;//表示一个json格式的对象
		msgStr_json["type"] = "1";
		msgStr_json["num"] = pStrData;
		msgStr_json["picSource"] = "";
		// 转string
		strSendData = msgStr_json.toStyledString();
		GtWriteTrace(30, "%s:%d: 组织发送二维码报文......", __FUNCTION__, __LINE__);
	}
	else
	{
		GtWriteTrace(30, "%s:%d: 未定义报文类型，无法组织！", __FUNCTION__, __LINE__);
		return -2;
	}

	GtWriteTrace(30, "%s:%d: http报文组织完毕，报文长度len=[%d]", __FUNCTION__, __LINE__, strSendData.length());
	if (g_mSend_data.nFlag == 1)
		GtWriteTrace(30, "%s:%d: http报文组织完毕，报文data=[%s]", __FUNCTION__, __LINE__, strSendData.c_str());
	GtWriteTrace(30, "%s:%d: 报文数据赋值，准备创建线程发送http报文。", __FUNCTION__, __LINE__);
	// 长度赋值
	g_mSend_data.nSendLen = strSendData.length();
	// 申请内存存储发送数据
	g_mSend_data.pSendBuff = (char *)malloc(strSendData.length());
	memcpy(g_mSend_data.pSendBuff, strSendData.c_str(), strSendData.length());
	// 报文组织完成，开启线程，发送报文

	// 判断线程是否在运行，在运行则进行提示
	if (theApp.bIsHttpThreadRun)
	{
		// 上个线程正在运行，提示线程正在发送报文
		GtWriteTrace(30, "%s:%d: 线程正在运行！", __FUNCTION__, __LINE__);
		::MessageBoxA(NULL, "正在发送请求，请等待...", "提示", MB_OK);
	}
	else
	{
		// 上个线程运行结束，重新创建线程发送报文
		GtWriteTrace(30, "%s:%d: 重新创建线程，发送报文！", __FUNCTION__, __LINE__);
		theApp.m_pHttpThread = AfxBeginThread(ThreadSendDataFunc, NULL);
	}
	return 0;
}


// 发送HTTP报文线程函数
UINT ThreadSendDataFunc(LPVOID pParm)
{
	theApp.bIsHttpThreadRun = TRUE;
	int nRet = 0;
	char keyStr[32] = {0};
	char sUrlAddr[256] = {0};
	char sSendRet[128] = {0};
	// 判断报文数据类型，以获取不同的url地址
	memcpy(keyStr, g_mSend_data.nFlag == 0 ? "POST_URL_PIC" : "POST_URL_NO", sizeof(keyStr)-1);
	// 发送截图，配置文件获取发送截图url地址
	GetPrivateProfileString("Information", keyStr, "no data", sUrlAddr, sizeof(sUrlAddr)-1,
		GetAppPath()+"\\win.ini");

	if (strcmp(sUrlAddr, "no data") == 0)
	{
		// 如果未配置地址，提示
		::MessageBoxA(NULL, "服务器地址未配置，请配置！", "错误", MB_OK);
		theApp.bIsHttpThreadRun = FALSE;
		return -1;
	}

	// 报文存放临时文件名
	sprintf_s(g_sTmpFilePath, 256, "%s\\HttpRecvData.loadtmp", GetFilePath().GetBuffer());
	// 删除原有文件
	DeleteFile(g_sTmpFilePath);
	// 发送报文，并将返回报文存入文件 g_sTmpFilePath 中
	nRet = SendData(sUrlAddr, g_mSend_data.nSendLen, g_mSend_data.pSendBuff, sizeof(sSendRet), sSendRet);
	if (nRet != 0)
	{
		// 报文发送失败
		char sTip[128] = {0};
		sprintf_s(sTip, sizeof(sTip)-1, "连接服务器失败：%s", sSendRet);
		::MessageBoxA(NULL, sTip, "警告", MB_OK);
	}
	else
	{
		// 数据接收完成，并且已保存到本地临时文件
		// 读取文件，解析报文并处理
		AnalyzeData();
	}
	// 释放发送报文的字符串
	free(g_mSend_data.pSendBuff);
	g_mSend_data.pSendBuff = NULL;
	theApp.bIsHttpThreadRun = FALSE;
	return 0;
}


/**
 * 功能:通过curl，post发送报文
 * 参数:pStrUrl：发送的url地址
 *		pStrData：发送的数据
 *		nDataSize：发送数据长度
 *		nSendRet:失败原因字符串可存放大小
 *		pSendRet：失败原因
 * 返回值：0-成功，其他失败
 **/ 
int SendData(const char *pStrUrl, int nDataSize, const char *pStrData, int nSendRet, char *pSendRet)
{
	nCount = 0;
	if (pStrUrl == NULL || pStrData == NULL || nDataSize < 0 || pSendRet == NULL || nSendRet <= 0)
	{
		return -1000;
	}
	BOOL ret = true;
	// url 打印到日志中
	GtWriteTrace(30, "[%s][%d]: 发送地址：[%s]", __FUNCTION__, __LINE__, pStrUrl);

	// post方式发送
	curl_global_init(CURL_GLOBAL_ALL);
	CURL *conn = curl_easy_init();
	curl_slist *plist = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
	// 超时时间
	curl_easy_setopt(conn, CURLOPT_TIMEOUT, 30);
	// url地址
	curl_easy_setopt(conn, CURLOPT_URL, pStrUrl);
	// 服务端返回报文的回调函数
	curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, &RecvData);
	// 设置回调函数中的用户参数
	//curl_easy_setopt(conn, CURLOPT_WRITEDATA, g_pUserStr);
	// 设置http请求头
	curl_easy_setopt(conn, CURLOPT_HTTPHEADER, plist);
	// 发送的数据
	curl_easy_setopt(conn, CURLOPT_POSTFIELDS, pStrData);
	// 发送数据的长度
	curl_easy_setopt(conn, CURLOPT_POSTFIELDSIZE, nDataSize);
	// 设置post发送方式
	curl_easy_setopt(conn, CURLOPT_POST, true);

	CURLcode code = curl_easy_perform(conn);
	if(code != CURLE_OK)
	{ 
		GtWriteTrace(30, "[%s][%d]: 发送失败：code=[%d] msg=[%s]！", __FUNCTION__, __LINE__, code, curl_easy_strerror(code));
		memcpy(pSendRet, curl_easy_strerror(code), nSendRet - 1);
	}
	else
	{
		GtWriteTrace(30, "[%s][%d]: 发送成功！", __FUNCTION__, __LINE__);
	}
	// 清理资源
	curl_easy_cleanup(conn);

	return code;
}

/**
 * 功能：通过curl，post发送报文的回调函数，接收服务端的返回，存到临时文件g_sTmpFilePath中
 **/ 
size_t RecvData(void *pBuff, size_t nSize, size_t nmemb, void *pUserp)
{
	std::string* str = dynamic_cast<std::string*>((std::string *)pUserp);
	if( NULL == str || NULL == pBuff )
	{
		GtWriteTrace(30, "[%s][%d]: 指针为空！", __FUNCTION__, __LINE__);
		return -1;
	}
	nCount += nSize * nmemb;
	// 申请内存存放接收到的数据
	char *pStrTmp = (char *) malloc(nSize * nmemb + 1);
	if (pStrTmp == NULL)
	{
		char errMsg[256] = {0};
		strerror_s(errMsg, sizeof(errMsg)-1, errno);
		GtWriteTrace(30, "[%s][%d]: 申请内存失败！err[%d][%s]", __FUNCTION__, __LINE__, errno, errMsg);
		return -2;
	}
	memset(pStrTmp, 0, nSize * nmemb + 1);
	// 拷贝返回数据到临时字符串中
	memcpy(pStrTmp, pBuff, nSize * nmemb);

	GtWriteTrace(30, "[%s][%d]: 接到数据（UTF-8）size=[%d]！", __FUNCTION__, __LINE__, nSize * nmemb);
	// utf-8转gbk
	string strUTF8 = pStrTmp;
	string strGBK = MyUtf8ToGBK(strUTF8);
	if (pStrTmp != NULL)
	{
		free(pStrTmp);
		pStrTmp = NULL;
	}
	GtWriteTrace(30, "[%s][%d]: 接到到数据（gbk）size=[%d][%s]！", __FUNCTION__, __LINE__, strGBK.length(), strGBK.c_str());
	// 接收数据写入文件中
	FILE *fp = NULL;
	fopen_s(&fp, g_sTmpFilePath, "ab");
	if (fp == NULL)
	{
		char errMsg[256] = {0};
		strerror_s(errMsg, sizeof(errMsg)-1, errno);
		GtWriteTrace(30, "[%s][%d]: 接收报文时打开文件失败 error=[%d][%s]！", __FUNCTION__, __LINE__, errno, errMsg);
	}
	else
	{
		int nRet = fwrite(strGBK.c_str(), 1, strGBK.length(), fp);
		if (nRet != strGBK.length())
		{
			GtWriteTrace(30, "[%s][%d]: 报文写入文件失败 fwrite nRet=[%d]！", __FUNCTION__, __LINE__, nRet);
		}
		fclose(fp);
	}

	return nSize * nmemb;
}


/**
 * 功能：从文件读取返回报文并解析
 **/ 
int AnalyzeData()
{
	GtWriteTrace(30, "[%s][%d]: --------------收到报文总大小[%d]", __FUNCTION__, __LINE__, nCount);
	// 从文件中读取报文
	FILE *fp = NULL;
	// 文件大小
	long lSize = 0;
	// 存放文件内容的字符串指针
	char *fileBuffer = NULL;
	// 真正读到的内容大小
	size_t result = 0;

	fopen_s(&fp, g_sTmpFilePath, "rb");
	if (fp == NULL)
	{
		// 文件打开失败
		char errMsg[256] = {0};
		strerror_s(errMsg, sizeof(errMsg)-1, errno);
		GtWriteTrace(30, "[%s][%d]: 读取报文时文件打开失败！ err=[%d][%s]", __FUNCTION__, __LINE__, errno, errMsg);
		::MessageBoxA(NULL, "读取报文时文件打开失败！", "错误", MB_OK);
		return -1;
	}
	fseek (fp, 0, SEEK_END);
	lSize = ftell(fp);
	rewind (fp);
	// 分配内存存储整个文件
	fileBuffer= (char*) malloc(sizeof(char) * lSize + 1);
	if (fileBuffer == NULL)
	{
		// 关闭文件
		fclose(fp);
		fp = NULL;
		char errMsg[256] = {0};
		strerror_s(errMsg, sizeof(errMsg)-1, errno);
		GtWriteTrace(30, "[%s][%d]: 读取报文文件时分配内存失败！ err=[%d][%s]", __FUNCTION__, __LINE__, errno, errMsg);
		::MessageBoxA(NULL, "读取报文文件时分配内存失败！", "错误", MB_OK);
		return -2;
	}
	memset(fileBuffer, 0, sizeof(char) * lSize + 1);
	// 将文件拷贝到fileBuffer中
	result = fread(fileBuffer, 1, lSize, fp);
	if (result != lSize)
	{
		// 关闭文件，释放内存
		fclose(fp);
		fp = NULL;
		free(fileBuffer);
		fileBuffer = NULL;
		char errMsg[256] = {0};
		strerror_s(errMsg, sizeof(errMsg)-1, errno);
		GtWriteTrace(30, "[%s][%d]: 读取报文文件[%s]失败！ err=[%d][%s]", __FUNCTION__, __LINE__, g_sTmpFilePath, errno, errMsg);
		::MessageBoxA(NULL, "读取报文文件失败！", "错误", MB_OK);
		return -3;
	}
	// 拷贝完成，关闭文件
	fclose(fp);
	fp = NULL;

	GtWriteTrace(30, "[%s][%d]: fileBuffer [%s]", __FUNCTION__, __LINE__, fileBuffer);
	char buffer[128] = {0};
	//sprintf((char *)buffer, "{\"code\":\"0\", \"msg\":\"成功\", \"url\":\"http://www.baidu.com\"}");//测试字符串
	//sprintf((char *)buffer, "{\"code\":\"1\", \"msg\":\"失败\", \"url\":\"\"}");//测试字符串
	//sprintf((char *)buffer, "{\"code\":\"2\", \"msg\":\"失败\", \"url\":\"\"}");//测试字符串
	string recvBuff = (char *)fileBuffer;
	//string recvBuff = (char *)buffer;
	// 解析服务端返回的json类型数据，获取交易类型
	//json解析
	Json::Reader reader;
	//表示一个json格式的对象
	Json::Value value;
	// 获取返回信息
	string code;
	string msg;
	string url;
	//解析json报文，存到value中
	GtWriteTrace(30, "[%s][%d]: recvBuff=[%s]", __FUNCTION__, __LINE__, recvBuff.c_str());
	if(reader.parse(recvBuff, value))
	{
		// 获取交易类型
		if (value.size() != 0)
		{
			// 获取返回码
			code = value["code"].asString();
			if (code != "")
			{
				// 主对话框
				CPaperlessDlg* pPaperlessDlg = (CPaperlessDlg*)AfxGetApp()->m_pMainWnd;
				// 获取返回信息
				msg = value["msg"].asString();
				// 获取url或者失败信息
				url = value["url"].asString();
				//if (code == "0")
				if (code.compare("0") == 0)
				{
					// 解析成功，截取第一个"|"前的网址
					int ret;
					char strUrl[512] = {0};
					ret = splitString(strUrl, url.c_str(), 0);
					if (ret != 0 || strlen(strUrl) == 0)
					{
						// 获取url地址失败
						GtWriteTrace(30, "[%s][%d]返回数据查询不到url地址", __FUNCTION__, __LINE__);
						::MessageBoxA(NULL, "服务端未返回url地址！", "错误", MB_OK);
					}
					else 
					{
						GtWriteTrace(30, "[%s][%d]服务端返回url地址=[%s]", __FUNCTION__, __LINE__, strUrl);
						memset(sendBuff, 0, sizeof(sendBuff));
						// 赋值网址到全局变量中
						memcpy(sendBuff, strUrl, strlen(strUrl));
						// 如果输入流水号窗口存在且为显示状态，需要进行隐藏处理
						if (pPaperlessDlg->pInputDlg != NULL && pPaperlessDlg->pInputDlg->IsWindowVisible())
						{
							pPaperlessDlg->pInputDlg->ShowWindow(SW_HIDE);
						}
						ShellExecute(NULL, _T("open"), _T("IExplore.exe"), _T(strUrl), NULL, SW_SHOWNORMAL);
// 						// 显示网页(发消息给指定窗口)
// 						::PostMessageA(pPaperlessDlg->pMyHtmlView->GetSafeHwnd(), WM_HTML_SHOW, (WPARAM)sendBuff, NULL);
					}
				}
				else if(code.compare("1") == 0)
				{
					// 二维码图片识别失败
					GtWriteTrace(30, "[%s][%d]二维码截图识别失败", __FUNCTION__, __LINE__);
					// 发消息到主窗口处理
					::PostMessageA(pPaperlessDlg->GetSafeHwnd(), WM_SCREENDLG_MSG, (WPARAM)RECOGNIZE_PICTURE_FAILED, NULL);
				}else if(code.compare("2") == 0)
				{
					// 流水号不存在
					GtWriteTrace(30, "[%s][%d]流水号不存在", __FUNCTION__, __LINE__);
					// 发消息到主窗口处理
					::PostMessageA(pPaperlessDlg->GetSafeHwnd(), WM_SCREENDLG_MSG, (WPARAM)QR_CODE_NOT_EXIST, NULL);
				}
				else
				{
					// 其他交易类型
					GtWriteTrace(30, "[%s][%d]服务端返回未定义交易类型=[%d]", __FUNCTION__, __LINE__, code.c_str());
					::MessageBoxA(NULL, "服务端返回未定义交易类型！", "错误", MB_OK);
				}
			}
			else
			{
				// 未返回交易类型
				GtWriteTrace(30, "[%s][%d]服务端未返回交易类型！", __FUNCTION__, __LINE__);
				::MessageBoxA(NULL, "服务端未返回交易类型！", "错误", MB_OK);
			}
		}
		else
		{
			// json解析失败
			GtWriteTrace(30, "[%s][%d]服务器返回非json格式报文1,解析出的json结构体size=0!", __FUNCTION__, __LINE__);
			::MessageBoxA(NULL, "服务器返回非json格式报文1！", "错误", MB_OK);
		}

	}
	else
	{
		// json解析失败
		GtWriteTrace(30, "[%s][%d]服务器返回非json格式报文2!", __FUNCTION__, __LINE__);
		::MessageBoxA(NULL, "服务器返回非json格式报文2！", "错误", MB_OK);
	}
	// 处理结束，清理之前未清理的数据
	if (fp != NULL)
	{
		fclose(fp);
	}
	if (fileBuffer != NULL)
	{
		free(fileBuffer);
		fileBuffer = NULL;
	}
	return 0;
}
