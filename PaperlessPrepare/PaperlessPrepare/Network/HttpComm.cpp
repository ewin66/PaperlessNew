#include "HttpComm.h"

#include "../Curl/curl.h"
#include "../utils.h"
#include "../MyTTrace.h"
#include "../resource.h"
#include "../PaperlessPrepareDlg.h"
#include "../CBase64.h"

char g_sTmpFilePath[256] = {0};
int nCount = 0;

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
	GtWriteTrace(30, "[%s][%d]: 接到到数据（gbk）size=[%d]！", __FUNCTION__, __LINE__, strGBK.length());
	//GtWriteTrace(30, "[%s][%d]:	 buff=[%s]！", __FUNCTION__, __LINE__, strGBK.c_str());
	//sprintf((char *)pBuff, "{\"type\":\"2\", \"code\":\"2\", \"msg\":\"成功\", \"filename\":\"XXbgService.exe\", \"version\":\"1.1\", \"application\":\"eyJCV0taTFgiOiI0IiwgIkpZRE0iOiIxMDEwMTAifQ==\"}");//测试字符串
	//sprintf((char *)pBuff, "{\"type\":\"2\", \"code\":\"0\", \"msg\":\"succeed\", \"filename\":\"XXbgService.exe\", \"version\":\"1.1\", \"application\":\"eyJCV0taTFgiOiI0IiwgIkpZRE0iOiIxMDEwMTAifQ==\"}");//测试字符串
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
		((CPaperlessPrepareDlg *)(AfxGetApp()->m_pMainWnd))->MyRetryWin("报文文件打开失败！");
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
		((CPaperlessPrepareDlg *)(AfxGetApp()->m_pMainWnd))->MyRetryWin("本地报文读取请求失败！");
		return -2;
	}
	memset(fileBuffer, 0, sizeof(fileBuffer));
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
		((CPaperlessPrepareDlg *)(AfxGetApp()->m_pMainWnd))->MyRetryWin("本地报文读取失败！");
		return -3;
	}
	// 拷贝完成，关闭文件
	fclose(fp);
	fp = NULL;

	GtWriteTrace(30, "[%s][%d]: fileBuffer [%s]", __FUNCTION__, __LINE__, fileBuffer);
	string recvBuff = (char *)fileBuffer;
	// 解析服务端返回的json类型数据，获取交易类型
	//json解析
	Json::Reader reader;
	//表示一个json格式的对象
	Json::Value value;
	// 获取返回信息
	string type;
	//解析json报文，存到value中
	GtWriteTrace(30, "[%s][%d]: recvBuff=[%s]", __FUNCTION__, __LINE__, recvBuff.c_str());
	if(reader.parse(recvBuff, value))
	{
		// 获取交易类型
		if (value.size() != 0)
		{
			type = value["type"].asString();
			if (type == "2")
			{
				// 应用程序更新的返回报文处理
				int nRet = DealUpdateRet(value);
				if (nRet != 0)
				{
					return -4;
				}
			}
			else
			{
				GtWriteTrace(30, "[%s][%d]服务器返回未定义的交易类型，type=[%s]!", __FUNCTION__, __LINE__, type);
				((CPaperlessPrepareDlg *)(AfxGetApp()->m_pMainWnd))->MyRetryWin("服务器处理错误！");
			}
		}
		else
		{
			// json解析失败
			GtWriteTrace(30, "[%s][%d]服务器返回非json格式报文,解析出的json结构体size=0!", __FUNCTION__, __LINE__);
			((CPaperlessPrepareDlg *)(AfxGetApp()->m_pMainWnd))->MyRetryWin("服务器处理异常！");
		}

	}
	else
	{
		// json解析失败
		GtWriteTrace(30, "[%s][%d]服务器返回非json格式报文!", __FUNCTION__, __LINE__);
		((CPaperlessPrepareDlg *)(AfxGetApp()->m_pMainWnd))->MyRetryWin("服务器处理异常2！");
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


// 处理应用程序更新的返回
int DealUpdateRet(Json::Value &sDataValue)
{
	string code;
	string msg;
	string filename;
	string version;
	string application;

	// 获取返回码
	code = sDataValue["code"].asString();
	if ("0" == code)
	{
		// 服务器返回新版本
		GtWriteTrace(30, "[%s][%d]服务器返回更新版本!", __FUNCTION__, __LINE__);
		filename = sDataValue["filename"].asString();
		version = sDataValue["version"].asString();
		application = sDataValue["application"].asString();

		char sSrcFile[256] = {0};
		char sDestFile[256] = {0};
		sprintf_s(sSrcFile, sizeof(sSrcFile)-1, "%s\\%s", GetFilePath().GetBuffer(), filename.c_str());
		sprintf_s(sDestFile, sizeof(sDestFile)-1, "%s\\%s.loadtmp", GetFilePath().GetBuffer(), filename.c_str());
		GtWriteTrace(30, "[%s][%d]源文件=[%s]，目标文件=[%s]!", __FUNCTION__, __LINE__, sSrcFile, sDestFile);

		// 源文件备份，覆盖剪切
		MoveFileEx(sSrcFile, sDestFile, MOVEFILE_REPLACE_EXISTING);

		// 将文件进行base64解码，并写入到文件中
		int nRet = 0;
		ZBase64 zBase;
		int nBase64AfterLens = 0;
		string sBase64After = zBase.Decode(application.c_str(), application.length(), nBase64AfterLens);

		GtWriteTrace(30, "[%s][%d]待写入 len=[%d]!", __FUNCTION__, __LINE__, nBase64AfterLens);

		FILE *fp = NULL;
		fopen_s(&fp, sSrcFile, "wb");
		if (fp == NULL)
		{
			// 文件打开失败
			char errMsg[256] = {0};
			strerror_s(errMsg, sizeof(errMsg)-1, errno);
			GtWriteTrace(30, "[%s][%d]: 应用程序写入时打开文件失败！ err=[%d][%s]", __FUNCTION__, __LINE__, errno, errMsg);
			((CPaperlessPrepareDlg *)(AfxGetApp()->m_pMainWnd))->MyRetryWin("保存最新版本程序失败！");
			return -2;
		}
		else
		{
			GtWriteTrace(30, "[%s][%d]更新版本保存到本地!", __FUNCTION__, __LINE__);
			nRet = fwrite(sBase64After.c_str(), 1, nBase64AfterLens, fp);
			fclose(fp);
			if (nRet != nBase64AfterLens)
			{
				// 写文件失败，源文件重新拷贝回来
				MoveFileEx(sDestFile, sSrcFile, MOVEFILE_REPLACE_EXISTING);
				((CPaperlessPrepareDlg *)(AfxGetApp()->m_pMainWnd))->MyRetryWin("保存最新版本程序失败！");
				return -1;
			}
			// 应用程序更新成功，版本号写入配置文件中
			WritePrivateProfileString("Information", "Version", version.c_str(), GetFilePath()+"\\win.ini");
		}
		// 启动主程序，成功启动后将关闭本程序
		StartPaperless();
	}
	else if ("1" == code)
	{
		// 当前文件和最新版本一致
		GtWriteTrace(30, "[%s][%d]最新版本和当前版本一致!", __FUNCTION__, __LINE__);
		// 启动主程序，成功启动后将关闭本程序
		StartPaperless();
	}
	else
	{
		GtWriteTrace(30, "[%s][%d]服务器返回失败:%s", __FUNCTION__, __LINE__, msg.c_str());
		// 服务器返回失败
		msg = sDataValue["msg"].asString();
		char tmp[128] = {0};
		sprintf_s(tmp, sizeof(tmp)-1, "服务器返回失败:%s", msg.c_str());
		((CPaperlessPrepareDlg *)(AfxGetApp()->m_pMainWnd))->MyRetryWin(tmp);
	}
	return 0;
}



// 启动主程序，成功启动后将关闭本程序
int StartPaperless()
{
	char sAppDir[256] = {0};
	char sRetMsg[128] = {0};

	// 主应用程序路径
	sprintf_s(sAppDir, sizeof(sAppDir)-1, "%s\\XXbgService.exe", GetAppPath().GetBuffer());
	//GetPrivateProfileString("Information", "AppDir", "../Paperless.exe", sAppDir, sizeof(sAppDir)-1, GetFilePath()+"\\win.ini");
	GtWriteTrace(30, "[%s][%d]启动程序=[%s]!", __FUNCTION__, __LINE__, sAppDir);
	// 启动主程序
	LPCSTR lpcsDir = _T(sAppDir);
	ShellExecute(NULL, "open", sAppDir, NULL, NULL, SW_SHOWNORMAL);
	// 关闭本程序
	((CPaperlessPrepareDlg *)(AfxGetApp()->m_pMainWnd))->OnBnClickedCancel();
	return 0;
}