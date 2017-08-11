#include "HttpComm.h"

#include "../Paperless.h"
#include "../PaperlessDlg.h"
#include "../Curl/curl.h"
#include "../utils.h"
#include "../MyTTrace.h"
#include "../CBase64.h"

char g_sTmpFilePath[256] = {0};
int nCount = 0;
// ���ͱ��Ľṹ�壬��Ҫ�����̴߳���
SEND_DATA g_mSend_data;

// ��ͬ����֮����Ϣ����
#define BUFFER_LEN 1024
char sendBuff[BUFFER_LEN] = {0};


/**
 * ����:ͨ��curl��post���ͱ��ĵ�Ԥ������
 * ����:nFlag�����͵ı������ͣ�0-��ά���ͼ��1-��ˮ��
 *		pStrData��Я�������ݣ���ά���ͼʱЯ����ͼ����·������ˮ��ʱЯ��������ˮ��
 * ����ֵ��0 - �ɹ���-1 - ��μ��ʧ�ܣ����� - ʧ��
 **/ 
int SendDataPrepare(int nFlag, const char *pStrData)
{
	GtWriteTrace(30, "%s:%d: ���뷢�ͱ���Ԥ������......", __FUNCTION__, __LINE__);
	if (pStrData == NULL)
	{
		return -1;
	}
	int nRet = 0;
	// �洢�����ͱ���
	string strSendData;
	// ��ʼ��http�������ݽṹ�壬�˲������߳���ʹ��
	memset(&g_mSend_data, 0, sizeof(SEND_DATA));
	// �жϷ��͵ı������ͣ���֯���͵ı���
	if (nFlag == 0)
	{
		g_mSend_data.nFlag = 0;
		GtWriteTrace(30, "%s:%d: ��֯���ͽ�ͼ����......", __FUNCTION__, __LINE__);
		// ���Ͷ�ά���ͼ����ȡ��ͼ����֯���͵ı���
		// ��ȡ �ļ�����
		FILE * pFile= NULL;
		// �洢��ͼ�ļ���ʱ�ַ���
		char *fileBuffer = NULL;
		// �ļ�����
		long lSize = 0;
		// base64�������
		ZBase64 zBase;
		// base64�������ַ���
		string encodeBase64;
		do
		{
			size_t result = 0;
			fopen_s (&pFile, pStrData, "rb");
			if (pFile == NULL)
			{
				char errMsg[256] = {0};
				strerror_s(errMsg, sizeof(errMsg)-1, errno);
				GtWriteTrace(30, "%s:%d: �򿪽�ͼ�ļ�[%s]ʧ��, error=[%d][%s]!", __FUNCTION__, __LINE__, pStrData, errno, errMsg);
				nRet = -3;
				break;
			}
			fseek (pFile, 0, SEEK_END);
			lSize = ftell(pFile);
			rewind (pFile);
			// �ж��ļ��Ƿ����10M
			if (lSize > 1024 * 1024 * 10)
			{
				GtWriteTrace(30, "%s:%d: ��ͼ�ļ������%.2lfM(����10M)!", __FUNCTION__, __LINE__, (((double)lSize) / 1024 / 1024));
				nRet = -4;
				break;
			}
			// �����ڴ�洢�����ļ�
			fileBuffer = (char*) malloc(sizeof(char) * lSize);
			if (fileBuffer == NULL)
			{
				char errMsg[256] = {0};
				strerror_s(errMsg, sizeof(errMsg)-1, errno);
				GtWriteTrace(30, "%s:%d: �����ڴ�洢�����ļ�ʱʧ��, error=[%d][%s]!", __FUNCTION__, __LINE__, errno, errMsg);
				nRet = -5;
				break;
			}
			// ���ļ�������fileBuffer��
			result = fread(fileBuffer, 1, lSize, pFile);
			if (result != lSize)
			{
				char errMsg[256] = {0};
				strerror_s(errMsg, sizeof(errMsg)-1, errno);
				GtWriteTrace(30, "%s:%d: ��ȡ�����ļ�ʧ��, error=[%d][%s]!", __FUNCTION__, __LINE__, errno, errMsg);
				nRet = -6;
				break;
			}
			// ��ȡ��ͼ�ļ��ɹ�������base64����
			encodeBase64 = zBase.Encode((const unsigned char*)fileBuffer, (int)lSize);
			// �ļ���ȡ��ϣ��ر��ļ�
			if (pFile != NULL)
			{
				fclose (pFile);
				pFile = NULL;
			}
			// �����ڴ�
			if (fileBuffer != NULL)
			{
				free(fileBuffer);
				fileBuffer = NULL;
			}
			// ������͵�json����
			Json::Value msgStr_json;//��ʾһ��json��ʽ�Ķ���
			msgStr_json["type"] = "0";
			msgStr_json["num"] = "";
			msgStr_json["picSource"] = encodeBase64.c_str();
			// תstring
			strSendData = msgStr_json.toStyledString();
			// ��֯���Ľ���
			nRet = 0;
		}while(0);
		// �����ڴ棬�ر��ļ�
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
		Json::Value msgStr_json;//��ʾһ��json��ʽ�Ķ���
		msgStr_json["type"] = "1";
		msgStr_json["num"] = pStrData;
		msgStr_json["picSource"] = "";
		// תstring
		strSendData = msgStr_json.toStyledString();
		GtWriteTrace(30, "%s:%d: ��֯���Ͷ�ά�뱨��......", __FUNCTION__, __LINE__);
	}
	else
	{
		GtWriteTrace(30, "%s:%d: δ���屨�����ͣ��޷���֯��", __FUNCTION__, __LINE__);
		return -2;
	}

	GtWriteTrace(30, "%s:%d: http������֯��ϣ����ĳ���len=[%d]", __FUNCTION__, __LINE__, strSendData.length());
	if (g_mSend_data.nFlag == 1)
		GtWriteTrace(30, "%s:%d: http������֯��ϣ�����data=[%s]", __FUNCTION__, __LINE__, strSendData.c_str());
	GtWriteTrace(30, "%s:%d: �������ݸ�ֵ��׼�������̷߳���http���ġ�", __FUNCTION__, __LINE__);
	// ���ȸ�ֵ
	g_mSend_data.nSendLen = strSendData.length();
	// �����ڴ�洢��������
	g_mSend_data.pSendBuff = (char *)malloc(strSendData.length());
	memcpy(g_mSend_data.pSendBuff, strSendData.c_str(), strSendData.length());
	// ������֯��ɣ������̣߳����ͱ���

	// �ж��߳��Ƿ������У��������������ʾ
	if (theApp.bIsHttpThreadRun)
	{
		// �ϸ��߳��������У���ʾ�߳����ڷ��ͱ���
		GtWriteTrace(30, "%s:%d: �߳��������У�", __FUNCTION__, __LINE__);
		::MessageBoxA(NULL, "���ڷ���������ȴ�...", "��ʾ", MB_OK);
	}
	else
	{
		// �ϸ��߳����н��������´����̷߳��ͱ���
		GtWriteTrace(30, "%s:%d: ���´����̣߳����ͱ��ģ�", __FUNCTION__, __LINE__);
		theApp.m_pHttpThread = AfxBeginThread(ThreadSendDataFunc, NULL);
	}
	return 0;
}


// ����HTTP�����̺߳���
UINT ThreadSendDataFunc(LPVOID pParm)
{
	theApp.bIsHttpThreadRun = TRUE;
	int nRet = 0;
	char keyStr[32] = {0};
	char sUrlAddr[256] = {0};
	char sSendRet[128] = {0};
	// �жϱ����������ͣ��Ի�ȡ��ͬ��url��ַ
	memcpy(keyStr, g_mSend_data.nFlag == 0 ? "POST_URL_PIC" : "POST_URL_NO", sizeof(keyStr)-1);
	// ���ͽ�ͼ�������ļ���ȡ���ͽ�ͼurl��ַ
	GetPrivateProfileString("Information", keyStr, "no data", sUrlAddr, sizeof(sUrlAddr)-1,
		GetAppPath()+"\\win.ini");

	if (strcmp(sUrlAddr, "no data") == 0)
	{
		// ���δ���õ�ַ����ʾ
		::MessageBoxA(NULL, "��������ַδ���ã������ã�", "����", MB_OK);
		theApp.bIsHttpThreadRun = FALSE;
		return -1;
	}

	// ���Ĵ����ʱ�ļ���
	sprintf_s(g_sTmpFilePath, 256, "%s\\HttpRecvData.loadtmp", GetFilePath().GetBuffer());
	// ɾ��ԭ���ļ�
	DeleteFile(g_sTmpFilePath);
	// ���ͱ��ģ��������ر��Ĵ����ļ� g_sTmpFilePath ��
	nRet = SendData(sUrlAddr, g_mSend_data.nSendLen, g_mSend_data.pSendBuff, sizeof(sSendRet), sSendRet);
	if (nRet != 0)
	{
		// ���ķ���ʧ��
		char sTip[128] = {0};
		sprintf_s(sTip, sizeof(sTip)-1, "���ӷ�����ʧ�ܣ�%s", sSendRet);
		::MessageBoxA(NULL, sTip, "����", MB_OK);
	}
	else
	{
		// ���ݽ�����ɣ������ѱ��浽������ʱ�ļ�
		// ��ȡ�ļ����������Ĳ�����
		AnalyzeData();
	}
	// �ͷŷ��ͱ��ĵ��ַ���
	free(g_mSend_data.pSendBuff);
	g_mSend_data.pSendBuff = NULL;
	theApp.bIsHttpThreadRun = FALSE;
	return 0;
}


/**
 * ����:ͨ��curl��post���ͱ���
 * ����:pStrUrl�����͵�url��ַ
 *		pStrData�����͵�����
 *		nDataSize���������ݳ���
 *		nSendRet:ʧ��ԭ���ַ����ɴ�Ŵ�С
 *		pSendRet��ʧ��ԭ��
 * ����ֵ��0-�ɹ�������ʧ��
 **/ 
int SendData(const char *pStrUrl, int nDataSize, const char *pStrData, int nSendRet, char *pSendRet)
{
	nCount = 0;
	if (pStrUrl == NULL || pStrData == NULL || nDataSize < 0 || pSendRet == NULL || nSendRet <= 0)
	{
		return -1000;
	}
	BOOL ret = true;
	// url ��ӡ����־��
	GtWriteTrace(30, "[%s][%d]: ���͵�ַ��[%s]", __FUNCTION__, __LINE__, pStrUrl);

	// post��ʽ����
	curl_global_init(CURL_GLOBAL_ALL);
	CURL *conn = curl_easy_init();
	curl_slist *plist = curl_slist_append(NULL, "Content-Type:application/json;charset=UTF-8");
	// ��ʱʱ��
	curl_easy_setopt(conn, CURLOPT_TIMEOUT, 30);
	// url��ַ
	curl_easy_setopt(conn, CURLOPT_URL, pStrUrl);
	// ����˷��ر��ĵĻص�����
	curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, &RecvData);
	// ���ûص������е��û�����
	//curl_easy_setopt(conn, CURLOPT_WRITEDATA, g_pUserStr);
	// ����http����ͷ
	curl_easy_setopt(conn, CURLOPT_HTTPHEADER, plist);
	// ���͵�����
	curl_easy_setopt(conn, CURLOPT_POSTFIELDS, pStrData);
	// �������ݵĳ���
	curl_easy_setopt(conn, CURLOPT_POSTFIELDSIZE, nDataSize);
	// ����post���ͷ�ʽ
	curl_easy_setopt(conn, CURLOPT_POST, true);

	CURLcode code = curl_easy_perform(conn);
	if(code != CURLE_OK)
	{ 
		GtWriteTrace(30, "[%s][%d]: ����ʧ�ܣ�code=[%d] msg=[%s]��", __FUNCTION__, __LINE__, code, curl_easy_strerror(code));
		memcpy(pSendRet, curl_easy_strerror(code), nSendRet - 1);
	}
	else
	{
		GtWriteTrace(30, "[%s][%d]: ���ͳɹ���", __FUNCTION__, __LINE__);
	}
	// ������Դ
	curl_easy_cleanup(conn);

	return code;
}

/**
 * ���ܣ�ͨ��curl��post���ͱ��ĵĻص����������շ���˵ķ��أ��浽��ʱ�ļ�g_sTmpFilePath��
 **/ 
size_t RecvData(void *pBuff, size_t nSize, size_t nmemb, void *pUserp)
{
	std::string* str = dynamic_cast<std::string*>((std::string *)pUserp);
	if( NULL == str || NULL == pBuff )
	{
		GtWriteTrace(30, "[%s][%d]: ָ��Ϊ�գ�", __FUNCTION__, __LINE__);
		return -1;
	}
	nCount += nSize * nmemb;
	// �����ڴ��Ž��յ�������
	char *pStrTmp = (char *) malloc(nSize * nmemb + 1);
	if (pStrTmp == NULL)
	{
		char errMsg[256] = {0};
		strerror_s(errMsg, sizeof(errMsg)-1, errno);
		GtWriteTrace(30, "[%s][%d]: �����ڴ�ʧ�ܣ�err[%d][%s]", __FUNCTION__, __LINE__, errno, errMsg);
		return -2;
	}
	memset(pStrTmp, 0, nSize * nmemb + 1);
	// �����������ݵ���ʱ�ַ�����
	memcpy(pStrTmp, pBuff, nSize * nmemb);

	GtWriteTrace(30, "[%s][%d]: �ӵ����ݣ�UTF-8��size=[%d]��", __FUNCTION__, __LINE__, nSize * nmemb);
	// utf-8תgbk
	string strUTF8 = pStrTmp;
	string strGBK = MyUtf8ToGBK(strUTF8);
	if (pStrTmp != NULL)
	{
		free(pStrTmp);
		pStrTmp = NULL;
	}
	GtWriteTrace(30, "[%s][%d]: �ӵ������ݣ�gbk��size=[%d][%s]��", __FUNCTION__, __LINE__, strGBK.length(), strGBK.c_str());
	// ��������д���ļ���
	FILE *fp = NULL;
	fopen_s(&fp, g_sTmpFilePath, "ab");
	if (fp == NULL)
	{
		char errMsg[256] = {0};
		strerror_s(errMsg, sizeof(errMsg)-1, errno);
		GtWriteTrace(30, "[%s][%d]: ���ձ���ʱ���ļ�ʧ�� error=[%d][%s]��", __FUNCTION__, __LINE__, errno, errMsg);
	}
	else
	{
		int nRet = fwrite(strGBK.c_str(), 1, strGBK.length(), fp);
		if (nRet != strGBK.length())
		{
			GtWriteTrace(30, "[%s][%d]: ����д���ļ�ʧ�� fwrite nRet=[%d]��", __FUNCTION__, __LINE__, nRet);
		}
		fclose(fp);
	}

	return nSize * nmemb;
}


/**
 * ���ܣ����ļ���ȡ���ر��Ĳ�����
 **/ 
int AnalyzeData()
{
	GtWriteTrace(30, "[%s][%d]: --------------�յ������ܴ�С[%d]", __FUNCTION__, __LINE__, nCount);
	// ���ļ��ж�ȡ����
	FILE *fp = NULL;
	// �ļ���С
	long lSize = 0;
	// ����ļ����ݵ��ַ���ָ��
	char *fileBuffer = NULL;
	// �������������ݴ�С
	size_t result = 0;

	fopen_s(&fp, g_sTmpFilePath, "rb");
	if (fp == NULL)
	{
		// �ļ���ʧ��
		char errMsg[256] = {0};
		strerror_s(errMsg, sizeof(errMsg)-1, errno);
		GtWriteTrace(30, "[%s][%d]: ��ȡ����ʱ�ļ���ʧ�ܣ� err=[%d][%s]", __FUNCTION__, __LINE__, errno, errMsg);
		::MessageBoxA(NULL, "��ȡ����ʱ�ļ���ʧ�ܣ�", "����", MB_OK);
		return -1;
	}
	fseek (fp, 0, SEEK_END);
	lSize = ftell(fp);
	rewind (fp);
	// �����ڴ�洢�����ļ�
	fileBuffer= (char*) malloc(sizeof(char) * lSize + 1);
	if (fileBuffer == NULL)
	{
		// �ر��ļ�
		fclose(fp);
		fp = NULL;
		char errMsg[256] = {0};
		strerror_s(errMsg, sizeof(errMsg)-1, errno);
		GtWriteTrace(30, "[%s][%d]: ��ȡ�����ļ�ʱ�����ڴ�ʧ�ܣ� err=[%d][%s]", __FUNCTION__, __LINE__, errno, errMsg);
		::MessageBoxA(NULL, "��ȡ�����ļ�ʱ�����ڴ�ʧ�ܣ�", "����", MB_OK);
		return -2;
	}
	memset(fileBuffer, 0, sizeof(char) * lSize + 1);
	// ���ļ�������fileBuffer��
	result = fread(fileBuffer, 1, lSize, fp);
	if (result != lSize)
	{
		// �ر��ļ����ͷ��ڴ�
		fclose(fp);
		fp = NULL;
		free(fileBuffer);
		fileBuffer = NULL;
		char errMsg[256] = {0};
		strerror_s(errMsg, sizeof(errMsg)-1, errno);
		GtWriteTrace(30, "[%s][%d]: ��ȡ�����ļ�[%s]ʧ�ܣ� err=[%d][%s]", __FUNCTION__, __LINE__, g_sTmpFilePath, errno, errMsg);
		::MessageBoxA(NULL, "��ȡ�����ļ�ʧ�ܣ�", "����", MB_OK);
		return -3;
	}
	// ������ɣ��ر��ļ�
	fclose(fp);
	fp = NULL;

	GtWriteTrace(30, "[%s][%d]: fileBuffer [%s]", __FUNCTION__, __LINE__, fileBuffer);
// 	// ��ʱ�ַ���
// 	char strTmp[256] = {0};
// 	CString iniDir = GetAppPath()+"\\win.ini";
// 	// ��ȡ�����ļ��еĴ�����ʾ��λ��
// 	GetPrivateProfileString("Information", "urltest", "-1", strTmp, sizeof(strTmp)-1, iniDir);
// 	char buffer[128] = {0};
// 	sprintf((char *)buffer, "{\"code\":\"0\", \"msg\":\"�ɹ�\", \"url\":\"%s\"}", strTmp);//�����ַ���
// 	//sprintf((char *)buffer, "{\"code\":\"1\", \"msg\":\"ʧ��\", \"url\":\"\"}");//�����ַ���
// 	//sprintf((char *)buffer, "{\"code\":\"2\", \"msg\":\"ʧ��\", \"url\":\"\"}");//�����ַ���
// 	string recvBuff = (char *)buffer;
	string recvBuff = (char *)fileBuffer;
	// ��������˷��ص�json�������ݣ���ȡ��������
	//json����
	Json::Reader reader;
	//��ʾһ��json��ʽ�Ķ���
	Json::Value value;
	// ��ȡ������Ϣ
	string code;
	string msg;
	string url;
	//����json���ģ��浽value��
	GtWriteTrace(30, "[%s][%d]: recvBuff=[%s]", __FUNCTION__, __LINE__, recvBuff.c_str());
	if(reader.parse(recvBuff, value))
	{
		// ��ȡ��������
		if (value.size() != 0)
		{
			// ��ȡ������
			code = value["code"].asString();
			if (code != "")
			{
				// ���Ի���
				CPaperlessDlg* pPaperlessDlg = (CPaperlessDlg*)AfxGetApp()->m_pMainWnd;
				// ��ȡ������Ϣ
				msg = value["msg"].asString();
				// ��ȡurl����ʧ����Ϣ
				url = value["url"].asString();
				//if (code == "0")
				if (code.compare("0") == 0)
				{
					// �����ɹ�����ȡ��һ��"|"ǰ����ַ
					int ret;
					char strUrl[512] = {0};
					ret = splitString(strUrl, url.c_str(), 0);
					if (ret != 0 || strlen(strUrl) == 0)
					{
						// ��ȡurl��ַʧ��
						GtWriteTrace(30, "[%s][%d]�������ݲ�ѯ����url��ַ", __FUNCTION__, __LINE__);
						::MessageBoxA(NULL, "�����δ����url��ַ��", "����", MB_OK);
					}
					else 
					{
						GtWriteTrace(30, "[%s][%d]����˷���url��ַ=[%s]\n", __FUNCTION__, __LINE__, strUrl);
						memset(sendBuff, 0, sizeof(sendBuff));
						// ��ֵ��ַ��ȫ�ֱ�����
						memcpy(sendBuff, strUrl, strlen(strUrl));
						// ���������ˮ�Ŵ��ڴ�����Ϊ��ʾ״̬����Ҫ�������ش���
						if (pPaperlessDlg->pInputDlg != NULL && pPaperlessDlg->pInputDlg->IsWindowVisible())
						{
							pPaperlessDlg->pInputDlg->ShowWindow(SW_HIDE);
						}
						ShowUrlInIE(strUrl);
// 						// ��ʾ��ҳ(����Ϣ��ָ������)
// 						::PostMessageA(pPaperlessDlg->pMyHtmlView->GetSafeHwnd(), WM_HTML_SHOW, (WPARAM)sendBuff, NULL);
					}
				}
				else if(code.compare("1") == 0)
				{
					// ��ά��ͼƬʶ��ʧ��
					GtWriteTrace(30, "[%s][%d]��ά���ͼʶ��ʧ��", __FUNCTION__, __LINE__);
					// ����Ϣ�������ڴ���
					::PostMessageA(pPaperlessDlg->GetSafeHwnd(), WM_SCREENDLG_MSG, (WPARAM)RECOGNIZE_PICTURE_FAILED, NULL);
				}else if(code.compare("2") == 0)
				{
					// ��ˮ�Ų�����
					GtWriteTrace(30, "[%s][%d]��ˮ�Ų�����", __FUNCTION__, __LINE__);
					// ����Ϣ�������ڴ���
					::PostMessageA(pPaperlessDlg->GetSafeHwnd(), WM_SCREENDLG_MSG, (WPARAM)QR_CODE_NOT_EXIST, NULL);
				}
				else
				{
					// ������������
					GtWriteTrace(30, "[%s][%d]����˷���δ���彻������=[%d]", __FUNCTION__, __LINE__, code.c_str());
					::MessageBoxA(NULL, "����˷���δ���彻�����ͣ�", "����", MB_OK);
				}
			}
			else
			{
				// δ���ؽ�������
				GtWriteTrace(30, "[%s][%d]�����δ���ؽ������ͣ�", __FUNCTION__, __LINE__);
				::MessageBoxA(NULL, "�����δ���ؽ������ͣ�", "����", MB_OK);
			}
		}
		else
		{
			// json����ʧ��
			GtWriteTrace(30, "[%s][%d]���������ط�json��ʽ����1,��������json�ṹ��size=0!", __FUNCTION__, __LINE__);
			::MessageBoxA(NULL, "���������ط�json��ʽ����1��", "����", MB_OK);
		}

	}
	else
	{
		// json����ʧ��
		GtWriteTrace(30, "[%s][%d]���������ط�json��ʽ����2!", __FUNCTION__, __LINE__);
		::MessageBoxA(NULL, "���������ط�json��ʽ����2��", "����", MB_OK);
	}
	// �������������֮ǰδ���������
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


typedef struct S_GET_WND_PARAM
{
	// ��Ҫ�Ƚϵ�URL��ַ��ͨ���Ƚϴ�IE�ĵ�ַ��url��ַ�ж��Ƿ�֮ǰ������ʾ�˲���Ϣ��ַ��
	char sUrlCmp[256];
	// �ش��Ĵ��ھ�����ҵ���ʾ�˲���Ϣ��ַ��IE���ڣ�û�ҵ�����NULL
	HWND hWnd;
}GET_WND_PARAM, *LP_GET_WND_PARAM;


int GetIEUrl(HWND hWnd, char *pUrl, int nLen)
{
	if (pUrl == NULL || hWnd == NULL || nLen <= 0)
	{
		return -1;
	}
	// 	HWND hwnd;
	// 	hwnd = ::FindWindowEx(0, 0, TEXT("IEFrame"), NULL);
	// 	if (hwnd == NULL) return -1;
	HWND hwnd = hWnd;

	hwnd = ::FindWindowEx(hwnd, 0, TEXT("WorkerW"), NULL);
	if (hwnd == NULL) return -2;

	hwnd = ::FindWindowEx(hwnd, 0, TEXT("RebarWindow32"), NULL);
	if (hwnd == NULL) return -4;

	hwnd = ::FindWindowEx(hwnd, 0, TEXT("ComboBoxEx32"), NULL);
	if (hwnd == NULL) return -5;

	hwnd = ::FindWindowEx(hwnd, 0, TEXT("ComboBox"), NULL);
	if (hwnd == NULL) return -6;

	hwnd = ::FindWindowEx(hwnd, 0, TEXT("Edit"), NULL);  
	if (hwnd == NULL) return -7;

	::SendMessage( hwnd, WM_GETTEXT, nLen, (LPARAM)pUrl);
	//GtWriteTrace(30, "%s:%d: ��ȡ����Url��ַ=[%s]!", __FUNCTION__, __LINE__, pUrl);
	return 0;
}


int SetIEUrl(HWND hWnd, const char *pUrl, int nLen)
{
	if (pUrl == NULL || hWnd == NULL || nLen <= 0)
	{
		return -1;
	}
	HWND hwnd = hWnd;

	hwnd = ::FindWindowEx(hwnd, 0, TEXT("WorkerW"), NULL);
	if (hwnd == NULL) return -2;

	hwnd = ::FindWindowEx(hwnd, 0, TEXT("RebarWindow32"), NULL);
	if (hwnd == NULL) return -4;

	hwnd = ::FindWindowEx(hwnd, 0, TEXT("ComboBoxEx32"), NULL);
	if (hwnd == NULL) return -5;

	hwnd = ::FindWindowEx(hwnd, 0, TEXT("ComboBox"), NULL);
	if (hwnd == NULL) return -6;

	hwnd = ::FindWindowEx(hwnd, 0, TEXT("Edit"), NULL);  
	if (hwnd == NULL) return -7;

	::SendMessage( hwnd, WM_SETTEXT, nLen, (LPARAM)pUrl);
	::SendMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0);
	Sleep(10);
	::SendMessage(hwnd, WM_KEYUP, VK_RETURN, 0); 
	//GtWriteTrace(30, "%s:%d: ��ȡ����Url��ַ=[%s]!", __FUNCTION__, __LINE__, pUrl);
	return 0;
}


BOOL CALLBACK MyEnumProc(HWND hwnd, LPARAM lParam)
{
	int ret = 0;
	char sUrl[255] = {0};
	LP_GET_WND_PARAM p_GetWndParam = (LP_GET_WND_PARAM)lParam;
	ret = GetIEUrl(hwnd, sUrl, sizeof(sUrl)-1);
	//GtWriteTrace(30, "%s:%d: ret=[%d], url=[%s]", __FUNCTION__, __LINE__, ret, sUrl);
	if (ret == 0)
	{
		GtWriteTrace(30, "%s:%d: �ҵ�IE���� Url��ַ=[%s], ���Ƚϵ�Url��ַ(strncmp)=[%s]", __FUNCTION__, __LINE__, sUrl, p_GetWndParam->sUrlCmp);
		//GtWriteTrace(30, "%s:%d: +++++++++++++++ �ɹ���ȡ��URL��ַ +++++++++++++++", __FUNCTION__, __LINE__);
		if (strncmp(sUrl, p_GetWndParam->sUrlCmp, strlen(p_GetWndParam->sUrlCmp)) == 0)
		{
			p_GetWndParam->hWnd = hwnd;
			GtWriteTrace(30, "%s:%d: �ҵ�ԭ����ʾ��ҵ���IE����", __FUNCTION__, __LINE__);
			return false;
		}
	}
	return true;
}


// �洢�Ѵ򿪵�IE���ھ��
HWND g_hWnd_Saved = NULL;
// ��ʾURL��ַ
int ShowUrlInIE(const char *pShowUrl)
{
	GtWriteTrace(30, "%s:%d: ������ʾIE���ں���......", __FUNCTION__, __LINE__);
	if (pShowUrl == NULL || strlen(pShowUrl) > 255)
	{
		return -1;
	}
	// url�����һ��'/'����ǰ���ַ���
	char sUrlTmp[256] = {0};
	int nUrlLen = strlen(pShowUrl);
	// ��ȡurl�����һ��'/'����ǰ���ַ���
	for (int i = nUrlLen - 1; i >= 0; i--)
	{
		if (pShowUrl[i] == '/')
		{
			strncpy(sUrlTmp, pShowUrl, i);
			break;
		}
	}
	if (strcmp(sUrlTmp, "") == 0)
	{
		strncpy(sUrlTmp, pShowUrl, sizeof(sUrlTmp)-1);
	}

	/******************************** �����ļ���ȡ����λ�� *****************************/
	// ���Ͻ�x����
	int nWinXPos = 0;
	// ���Ͻ�y����
	int nWinYPos = 0;
	// ���ڿ�
	int nWinWidth = 0;
	// ���ڸ�
	int nWinHigh = 0;
	// ��ʱ�ַ���
	char strTmp[32] = {0};
	CString iniDir = GetAppPath()+"\\win.ini";
	// ��ȡ�����ļ��еĴ�����ʾ��λ��
	GetPrivateProfileString("Information", "WinXPos", "-1", strTmp, sizeof(strTmp)-1, iniDir);
	nWinXPos = atoi(strTmp);
	GetPrivateProfileString("Information", "WinYPos", "-1", strTmp, sizeof(strTmp)-1, iniDir);
	nWinYPos = atoi(strTmp);
	GetPrivateProfileString("Information", "WinWidth", "-1", strTmp, sizeof(strTmp)-1, iniDir);
	nWinWidth = atoi(strTmp);
	GetPrivateProfileString("Information", "WinHigh", "-1", strTmp, sizeof(strTmp)-1, iniDir);
	nWinHigh = atoi(strTmp);

	/******************************** �ж��Ƿ����IE���ڵľ�� *****************************/
//	if (g_hWnd_Saved == NULL)
	{
		/******************************** Ӧ�ó���δ�洰�ھ��������windows���ھ������windows�Ƿ����Ѵ���ַ��IE *****************************/
		// �����ص������Ĳ���
		S_GET_WND_PARAM m_GetWndParam;
		memset(&m_GetWndParam, 0, sizeof(S_GET_WND_PARAM));
		// ��ַ������ֵ����ȡ���һ��'/'ǰ��url��ַ
		GtWriteTrace(30, "%s:%d: ++++++����URL=[%s], ���Ƚ�URL(strncmp)=[%s]", __FUNCTION__, __LINE__, pShowUrl, sUrlTmp);
		strncpy(m_GetWndParam.sUrlCmp, sUrlTmp, sizeof(m_GetWndParam.sUrlCmp)-1);
		// �ص���������windows�Ƿ����Ѵ���ַ��IE���ҵ�ַ����ͬ�ģ�
		GtWriteTrace(30, "%s:%d: ��ʼ����windows����IE����......", __FUNCTION__, __LINE__);
		EnumWindows(MyEnumProc, (LPARAM)&m_GetWndParam);
		GtWriteTrace(30, "%s:%d: ����windows����IE���ڽ���", __FUNCTION__, __LINE__);
		if (m_GetWndParam.hWnd == NULL)
		{
			GtWriteTrace(30, "%s:%d: �Ҳ����Ѵ��ڱ�ҵ��Ĵ��ڣ��Լ�����......", __FUNCTION__, __LINE__);
			/******************************** windowsû���Ѵ���ַ��IE���Լ�����һ��IE���� *****************************/
			STARTUPINFO si = { sizeof(si) }; 
			PROCESS_INFORMATION pi; 
			char sCmdLine[256] = {0};

			si.dwFlags = STARTF_USESHOWWINDOW; 
			//TRUE��ʾ��ʾ�����Ľ��̵Ĵ���
			si.wShowWindow = TRUE; 
			//��Unicode�汾�д˲�������Ϊ�����ַ�������Ϊ�˲����ᱻ�޸�;
			sprintf_s(sCmdLine, sizeof(sCmdLine), "C:\\Program Files\\Internet Explorer\\iexplore.exe %s", pShowUrl);

			BOOL bRet = ::CreateProcess ( NULL, sCmdLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi); 
			int error = GetLastError();
			if(bRet) 
			{
				// �����ɹ����ȴ��½���������ĳ�ʼ�����ȴ��û����룬��һ������Ϊ��ʱʱ��
				GtWriteTrace(30, "%s:%d: ����IE���ڳɹ�!", __FUNCTION__, __LINE__);
				WaitForInputIdle(pi.hProcess, 5000);

				// ���²��Ҵ��ڣ��ҵ����ƶ���λ��
				// �����ص������Ĳ���
				S_GET_WND_PARAM m_GetWndParam_child;
				memset(&m_GetWndParam_child, 0, sizeof(S_GET_WND_PARAM));
				// ��ַ������ֵ
				strncmp(m_GetWndParam_child.sUrlCmp, pShowUrl, sizeof(m_GetWndParam_child.sUrlCmp)-1);
				// �ص�����������Ѱ�Ҵ��ڣ���Ҫ�ı����С
				EnumWindows(MyEnumProc, (LPARAM)&m_GetWndParam_child);
				if (m_GetWndParam_child.hWnd == NULL)
				{
					GtWriteTrace(30, "%s:%d: ��ȡIE����ʧ�ܣ��޷��ƶ���λ�ã�", __FUNCTION__, __LINE__);
					::MessageBoxA(NULL, "��ȡIE����ʧ�ܣ��޷��ƶ���λ�ã�", "����", MB_OK);
				}
				else
				{
					GtWriteTrace(30, "%s:%d: �½����:0x%08X!", __FUNCTION__, __LINE__, m_GetWndParam_child.hWnd);
					//g_hWnd_Saved = m_GetWndParam_child.hWnd;
					::MoveWindow(m_GetWndParam_child.hWnd, nWinXPos, nWinYPos, nWinWidth, nWinHigh, TRUE);
				}

				::CloseHandle (pi.hThread); 
				::CloseHandle (pi.hProcess); 
			} 
			else
			{
				// ����ʧ��
				GtWriteTrace(30, "%s:%d: ����IE����ʧ�� error=[%d]!", __FUNCTION__, __LINE__, error);
				::MessageBoxA(NULL, "����IE����ʧ��", "����", MB_OK);
			}
		}
		else
		{
			// �ҵ���windows�Ѵ��ڵ�IE���ڣ��ҵ�ַ����ͬ�ģ�
			GtWriteTrace(30, "%s:%d: windowsԭ�ȴ��ھ��:0x%08X!", __FUNCTION__, __LINE__, m_GetWndParam.hWnd);
			// ��IEҳ����ʾ�µ�url��ַ
			SetIEUrl(m_GetWndParam.hWnd, pShowUrl, strlen(pShowUrl));
			// ��ʾIE����
			//::ShowWindow(m_GetWndParam.hWnd, SW_SHOWNA);
			::SetWindowPos(m_GetWndParam.hWnd, HWND_TOPMOST, 0, 0, 100, 100, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
			::SetWindowPos(m_GetWndParam.hWnd, HWND_NOTOPMOST, 0, 0, 100, 100, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
			::SetWindowPos(m_GetWndParam.hWnd, HWND_TOP, 0, 0, 100, 100, SWP_NOSIZE | SWP_NOMOVE | SWP_SHOWWINDOW);
			// ����λ��д�������ļ�
			// �����������Ϣ
			// �����ֵ
			//g_hWnd_Saved = m_GetWndParam.hWnd;
		}
	}
// 	else
// 	{
// 		// ԭ�ȴ��ھ�����ڣ��жϾ���Ƿ���Ч
// 		// �ж�ԭ�ȴ�����ַ�Ƿ��Ǵ���ʾ����ַ��ͬ����������ͬ�򴴽�
// 		// ԭ�ȴ��ھ�����ڣ���ԭ�ȴ�����ʾ������Ҫ�ƶ�����λ��
// 		GtWriteTrace(30, "%s:%d: ���������о�� ���:0x%08X!", __FUNCTION__, __LINE__, g_hWnd_Saved);
// 		// ��IEҳ����ʾ�µ�url��ַ
// 		SetIEUrl(g_hWnd_Saved, pShowUrl, strlen(pShowUrl));
// 		// ��ʾIE����
// 		::ShowWindow(g_hWnd_Saved, SW_SHOW);
// 	}
	GtWriteTrace(30, "%s:%d: �����˳���ʾIE���ں�����\n", __FUNCTION__, __LINE__);
	return 0;
}
