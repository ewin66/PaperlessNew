#include "../stdafx.h"
#include "SocketComm.h"
#include "../MyTTrace.h"
#include "../CBase64.h"
#include "../utils.h"
#include "../PaperlessDlg.h"

HANDLE g_hIoRes = NULL;
CString msgStr = "";
CString sendMsg="";
std::string ZJLX="10,11,21,22,23,31,32,40,50,51,60,61,99";
CString wname;
int long_time_interval, short_time_interval;
HWND hWnd;
HWND hCurFocus;
#include <Psapi.h>
#pragma comment(lib,"Psapi.Lib")  //MiniDumpWriteDump����ʱ�õ�


int InitIocpService(LPVOID lpVoid)
{
	//��ʼ����������
	g_hIoRes = CreateIoResource(0, 100, 80, 0);
	SetGIoRes(g_hIoRes);
	HANDLE hObject = CreateTcpListener(
		g_hIoRes,
		NULL,
		9190,
		5,
		AcceptEvt,
		CloseEvt,
		ReadEvt,
		WriteEvt,
		ExceptEvt,
		lpVoid
		);
	if (hObject)
	{
		StartListenerAccept(g_hIoRes, hObject, 5);
	}
	return 0;
}

void __stdcall AcceptEvt(LPVOID lpParam, HANDLE hAcceptor, PCHAR buf, DWORD len)
{
	SetLimitReadLen(g_hIoRes, hAcceptor, FALSE);
	GtWriteTrace(EM_TraceDebug, "%s:%d: iocp�ص�������AcceptEvt()!", __FUNCTION__, __LINE__);
}

void __stdcall WriteEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len)
{
	GtWriteTrace(EM_TraceDebug, "%s:%d: iocp�ص�������WriteEvt()!", __FUNCTION__, __LINE__);
}

void __stdcall ExceptEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len)
{
	GtWriteTrace(EM_TraceDebug, "%s:%d: iocp�ص�������ExceptEvt()!", __FUNCTION__, __LINE__);
}

void __stdcall CloseEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len)
{
	GtWriteTrace(EM_TraceDebug, "%s:%d: iocp�ص�������CloseEvt()!", __FUNCTION__, __LINE__);
}

/**************************************************************************
*��������void __stdcall ReadEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len)
*����˵����socket�Ķ����ݻص�����
*����˵����
*buf:���յ�������
*len�����յ����ݳ���
*��������ֵ��DWORD
***************************************************************************/
void __stdcall ReadEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len)
{
	GtWriteTrace(EM_TraceDebug, "%s:%d: ------------------->�յ�������...", __FUNCTION__, __LINE__);
	PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
	buf[len] = '\0';
	GtWriteTrace(EM_TraceDebug, "%s:%d: �յ�����[%s]", __FUNCTION__, __LINE__, buf);

	// base64����
	ZBase64 zBase;
	int msg_base64_after_len = 0;
	string strValue = zBase.Decode((const char*)buf, len, msg_base64_after_len);
	GtWriteTrace(EM_TraceDebug, "%s:%d: base64�����[%s]", __FUNCTION__, __LINE__, strValue.c_str());

	Json::Reader reader;//json����
	Json::Value value;//��ʾһ��json��ʽ�Ķ���
	std::string msgStr_rtn;
	std::string out;
	std::string tran_type;

	CString reStr="";
	Json::Reader ret_reader;//json����
	// ��ʾһ��json��ʽ�Ķ��󣬷��صı��Ķ�������������У���󷵻ظ�web��
	Json::Value msgStr_json_rtn;
	std::string ret_out;

// 	//��ȡ�����ļ�
// 	GetPrivateProfileString("Information","Wname","test.txt - ���±�",wname.GetBuffer(100),100,GetAppPath()+"\\win.ini");
// 	long_time_interval=GetPrivateProfileInt("Information","DefaultInterval",10,GetAppPath()+"\\win.ini");
// 	short_time_interval=GetPrivateProfileInt("Information","EnterInputInterval",10,GetAppPath()+"\\win.ini");
// 	
// 	sendMsg=strMsg.c_str();
// 	::MessageBoxA(NULL, sendMsg, "Ԥ�����", MB_OK);
// 			
// 	SendToWindows();
// 	reStr="000";
// 	sockaddr_in addr;
// 	addr.sin_addr.s_addr = pOpKey->remote_addr;
// 	int resCount = WriteDataEx(g_hIoRes,pOpKey,NULL,0,reStr.GetBuffer(),reStr.GetLength());
// 	//д��־


	// ������json�ŵ�value����
	if(reader.parse(strValue, value))
	{
		// �жϽ�������json�����Ƿ�δ��
		if (value.size() != 0)
		{
			tran_type = value["BWKZLX"].asString();
			if (0 == tran_type.compare("0"))
			{
				// �Զ������
				//��ȡ�����ļ�
				GetPrivateProfileString("Information","Wname","test.txt - ���±�",wname.GetBuffer(100),100,GetAppPath()+"\\win.ini");
				long_time_interval=GetPrivateProfileInt("Information","DefaultInterval",10,GetAppPath()+"\\win.ini");
				short_time_interval=GetPrivateProfileInt("Information","EnterInputInterval",10,GetAppPath()+"\\win.ini");
				reStr=JsonToSendMsg(strValue);
				if(ret_reader.parse(reStr.GetBuffer(), msgStr_json_rtn))//������json�ŵ�json����
				{
					ret_out=msgStr_json_rtn["XYM"].asString();
					GtWriteTrace(EM_TraceDebug, "%s:%d: ��Ӧ�룡[%s]",  __FUNCTION__, __LINE__,ret_out.c_str());
					if(0 == strcmp(ret_out.c_str(), "000"))
					{
						SendToWindows();
					}
					else
					{
						GtWriteTrace(EM_TraceDebug, "%s:%d: �Զ��Ԥ������ʧ�ܣ���Ӧ��[%s]",  __FUNCTION__, __LINE__,ret_out.c_str());
					}
				}
				else
				{
					msgStr_json_rtn["XYM"] = "999";
					msgStr_json_rtn["XYSM"] = "������Ӧ����ʧ��";
					GtWriteTrace(EM_TraceDebug, "%s:%d: ������Ӧ����ʧ�ܣ���Ӧ��[%s]",  __FUNCTION__, __LINE__,ret_out.c_str());
				}
			}
			else
			{
				int nRet = 0; 
				CString str = GetAppPath();
				CPaperlessDlg* pPaperlessDlg = (CPaperlessDlg*)AfxGetApp()->m_pMainWnd;
				if (0 == tran_type.compare("1"))
				{
					// ��ȡ���֤оƬ��Ϣ
					MYPERSONINFO pMyPerson;
					memset(&pMyPerson, 0, sizeof(MYPERSONINFO));
					str.Append("\\IDPicture\\HeadPictureTmp.jpg");
//#define YCP_LOCAl_TEST
#ifdef YCP_LOCAl_TEST
					nRet = 0;
					memcpy(pMyPerson.address, "����ʡ�������Ͼ���������13-1��", sizeof(pMyPerson.address));
					memcpy(pMyPerson.appendMsg, "", sizeof(pMyPerson.appendMsg));
					memcpy(pMyPerson.birthday, "19941022", sizeof(pMyPerson.birthday));
					memcpy(pMyPerson.cardId, "350425199410220517", sizeof(pMyPerson.cardId));
					memcpy(pMyPerson.cardType, "", sizeof(pMyPerson.cardType));
					memcpy(pMyPerson.EngName, "", sizeof(pMyPerson.EngName));
					memcpy(pMyPerson.govCode, "", sizeof(pMyPerson.govCode));
					pMyPerson.iFlag = 0;
					memcpy(pMyPerson.name, "Ҷ����", sizeof(pMyPerson.name));
					memcpy(pMyPerson.nation, "��", sizeof(pMyPerson.nation));
					memcpy(pMyPerson.nationCode, "", sizeof(pMyPerson.nationCode));
					memcpy(pMyPerson.otherData, "", sizeof(pMyPerson.otherData));
					memcpy(pMyPerson.police, "�����ع�����", sizeof(pMyPerson.police));
					memcpy(pMyPerson.sex, "��", sizeof(pMyPerson.sex));
					memcpy(pMyPerson.sexCode, "", sizeof(pMyPerson.sexCode));
					memcpy(pMyPerson.validEnd, "20201221", sizeof(pMyPerson.validEnd));
					memcpy(pMyPerson.validStart, "20101221", sizeof(pMyPerson.validStart));
					memcpy(pMyPerson.version, "", sizeof(pMyPerson.version));
#else
					nRet = pPaperlessDlg->pBaseReadIDCardInfo->MyReadIDCardInfo(str.GetBuffer(), &pMyPerson);
#endif
					// ͨ��������Ϣͷ��·���ͷ���ֵƴjson����
					GtWriteTrace(EM_TraceDebug, "%s:%d: ��ȡ���֤оƬ��Ϣ����ֵ return = [%d] оƬ��[%s]",  __FUNCTION__, __LINE__, nRet, str.GetBuffer());
					getIDCardInfoJson(msgStr_json_rtn, str, &pMyPerson, nRet);
				}
				else if (0 == tran_type.compare("2"))
				{
					// ��ȡ���֤������Ϣ
					str.Append("\\IDPicture\\FrontPictureTmp.jpg");
#ifdef YCP_LOCAl_TEST
					nRet = 0;
#else
					nRet = pPaperlessDlg->pBaseSaveCameraPic->MySaveDeskIDPic(str.GetBuffer());
#endif
					GtWriteTrace(EM_TraceDebug, "%s:%d: ��ȡ���֤�����շ���ֵ return = [%d] ��Ƭ[%s]",  __FUNCTION__, __LINE__, nRet, str.GetBuffer());
					// ͨ�����֤������Ϣ����ֵƴjson����
					getIDPicJson(msgStr_json_rtn, 0, str, nRet);
				}
				else if (0 == tran_type.compare("3"))
				{
					// ��ȡ���֤������Ϣ
					str.Append("\\IDPicture\\BackPictureTmp.jpg");
#ifdef YCP_LOCAl_TEST
					nRet = 0;
#else
					nRet = pPaperlessDlg->pBaseSaveCameraPic->MySaveDeskIDPic(str.GetBuffer());
#endif
					GtWriteTrace(EM_TraceDebug, "%s:%d: ��ȡ���֤�����շ���ֵ return = [%d] ��Ƭ[%s]",  __FUNCTION__, __LINE__, nRet, str.GetBuffer());
					// ͨ�����֤������Ϣ����ֵƴjson����
					getIDPicJson(msgStr_json_rtn, 1, str, nRet);
				}
				else if (0 == tran_type.compare("4"))
				{
					// ��ȡ ��������ͷ������
// 					str.Append("\\IDPicture\\EnvPictureTmp.png");
// 					nRet = pPaperlessDlg->pBaseSaveCameraPic->MySaveEnvPic(str.GetBuffer());
// 					//nRet = 0;
// 					GtWriteTrace(EM_TraceDebug, "%s:%d: ��ȡ�����շ���ֵ return = [%d] ������[%s]",  __FUNCTION__, __LINE__, nRet, str.GetBuffer());
// 					// ͨ�� ��������ͷ��Ϣ����ֵƴjson����
// 					getIDPicJson(msgStr_json_rtn, 2, str, nRet);

					// ��ȡ ��������ͷ�����գ����ر��������վ���·��
					char sFilename[256] = {0};
					strncpy(sFilename, "EnvPictureTmp.jpg", sizeof(sFilename)-1);
					str.Append("\\IDPicture\\");
					str.Append(sFilename);
#ifdef YCP_LOCAl_TEST
					nRet = 0;
#else
					nRet = pPaperlessDlg->pBaseSaveCameraPic->MySaveEnvPic(str.GetBuffer());
#endif
 					GtWriteTrace(EM_TraceDebug, "%s:%d: ��ȡ�����շ���ֵ return = [%d] ������[%s]",  __FUNCTION__, __LINE__, nRet, str.GetBuffer());
					// ͨ�� ��������ͷ����·���ͷ���ֵƴjson����
					getJsonFromPersonPic(msgStr_json_rtn, str, sFilename, nRet);
				}
				else if (0 == tran_type.compare("5"))
				{
					// ��ȡ ͨ��web������������ƣ�����������
					std::string sFileName = value["FILE_NAME"].asString();
					str.Append("\\IDPicture\\");
					str.Append(sFileName.c_str());
					GtWriteTrace(EM_TraceDebug, "%s:%d: ��Ҫ����������[%s]",  __FUNCTION__, __LINE__, str.GetBuffer());
					// ͨ�� ��������ͷ��Ƭ��ƴjson����
					getJsonFromPic(msgStr_json_rtn, str);
				}
				else if (0 == tran_type.compare("6"))
				{
					msgStr_json_rtn["XYM"] = "000";
					msgStr_json_rtn["XYSM"] = "�ɹ�";
					msgStr_json_rtn["FILE_DIR"] = "C:\\Users\\mrxu\\Desktop\\img\\test.jpg";
					msgStr_json_rtn["OTH_MSG1"] = "";
				}
				else
				{
					// δ֪�ı��Ŀ�������
					GtWriteTrace(EM_TraceDebug, "%s:%d: δ֪���Ŀ�������! tran_type = [%s]",  __FUNCTION__, __LINE__, tran_type.c_str());
					msgStr_json_rtn["XYM"]="998";
					msgStr_json_rtn["XYSM"]="δ֪�ı��Ŀ�������";
				}
			}
		}
		else
		{
			GtWriteTrace(EM_TraceDebug, "%s:%d: ����json���������ݣ�value.size() == 0",  __FUNCTION__, __LINE__);
			// ����ʧ��
			msgStr_json_rtn["XYM"]="999";
			msgStr_json_rtn["XYSM"]="����json���������ݣ�value.size() == 0";
		}
		
	}
	else
	{
		GtWriteTrace(EM_TraceDebug, "%s:%d: ����json����ʧ��! error = [%s]",  __FUNCTION__, __LINE__, (reader.getFormatedErrorMessages()).c_str());
		// ����ʧ��
		msgStr_json_rtn["XYM"]="999";
		msgStr_json_rtn["XYSM"]="json���Ľ���ʧ��";
	}
	// ��json���ķ���
	SendJsonMsg(msgStr_json_rtn, pOpKey);
	CPaperlessDlg* pPaperlessDlg1 = (CPaperlessDlg*)AfxGetApp()->m_pMainWnd;
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(pPaperlessDlg1->handle_memery, &pmc, sizeof(pmc));
	GtWriteTrace(EM_TraceInfo, "%s:%d: �ڴ�ʹ��:\n\t �ڴ�ʹ�ã�%dK����ֵ�ڴ�ʹ�ã�%dK�������ڴ�ʹ��%dK����ֵ�����ڴ�ʹ��%dK\n",
		__FUNCTION__, __LINE__, pmc.WorkingSetSize/1024, pmc.PeakWorkingSetSize/1024, pmc.PagefileUsage/1024, pmc.PeakPagefileUsage/1024);
	GtWriteTrace(EM_TraceDebug, "%s:%d: <----------------------�����������\n",  __FUNCTION__, __LINE__);
}


// ���֤ʶ���ǻ�ȡ���֤��Ϣ��Ӧ����ת��
void ReadIDCardCodeTrans(int nRet, char *sRetCode, char *sRetMsg)
{
	char sCodeTmp[4+1] = {0};
	sprintf(sCodeTmp, "%03d", nRet);
	for (int i = 0; i < N_ID_CARD_CODE_COUNT; i++)
	{
		if (0 == strcmp(sCodeTmp, S_ID_CARD_CODE[i]))
		{
			strncpy(sRetCode, sCodeTmp, 5-1);
			strncpy(sRetMsg, S_ID_CARD_MSG[i], 128-1);
			return;
		}
	}
	strncpy(sRetCode, "299", 5-1);
	strncpy(sRetMsg, "���֤ʶ���ǻ�ȡ���֤��Ϣʧ�ܣ���������", 128-1);
}


// ����ͷ��ȡ��Ƭ��Ӧ����ת��
void ReadPicCodeTrans(int nRet, char *sRetCode, char *sRetMsg)
{
	char sCodeTmp[4+1] = {0};
	sprintf(sCodeTmp, "%03d", nRet);
	for (int i = 0; i < N_READ_PIC_CODE_COUNT; i++)
	{
		if (0 == strcmp(sCodeTmp, S_READ_PIC_CODE[i]))
		{
			strncpy(sRetCode, sCodeTmp, 5-1);
			strncpy(sRetMsg, S_READ_PIC_MSG[i], 128-1);
			return ;
		}
	}
	strncpy(sRetCode, "199", 5-1);
	strncpy(sRetMsg, "����ͷ��ȡ��Ƭʧ�ܣ���������", 128-1);
}

// ͨ�� PERSONINFO �ṹ����json����
void getIDCardInfoJson(Json::Value &jsonBuff, CString strDir, MYPERSONINFO *pPerson, int nRet)
{
	char pinyin[512] = {0};
	// ��Ч����
	char effDate[128] = {0};
	char strFlag[8] = {0};
	// ������Ϣ
	char sRetCode[4+1] = {0};
	char sRetMsg[128+1] = {0};

	if (nRet == 0)
	{
		FILE * pFile= NULL;
		char *fileBuffer = NULL;
		long lSize = 0;
		ZBase64 zBase;
		string encodeBase64_pic;
		size_t result = 0;
		char PIC_FLAG[16] = {0};
		// ��ȡͷ����
		do 
		{
			pFile = fopen (strDir.GetBuffer(), "rb");
			strDir.ReleaseBuffer();
			if (pFile == NULL)
			{
				GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDCardInfoJson() open [%s] failed! err=[%d][%s]", strDir.GetBuffer(), errno, strerror(errno));
				nRet = 200;
				break;
			}
			fseek (pFile, 0, SEEK_END);
			lSize = ftell(pFile);
			rewind (pFile);
			// �����ڴ�洢�����ļ�
			fileBuffer = (char*) malloc(sizeof(char) * lSize);
			if (fileBuffer == NULL)
			{
				GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDCardInfoJson() malloc failed! err=[%d][%s]", errno, strerror(errno));
				nRet = 202;
				// �ر��ļ�
				fclose(pFile);
				pFile = NULL;
				break;
			}
			// ���ļ�������fileBuffer��
			result = fread(fileBuffer, 1, lSize, pFile);
			if (result != lSize)
			{
				GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDCardInfoJson() read [%s] failed! err=[%d][%s]", strDir.GetBuffer(), errno, strerror(errno));
				nRet = 201;
				// �ر��ļ����ͷ��ڴ�
				fclose(pFile);
				pFile = NULL;
				free(fileBuffer);
				fileBuffer = NULL;
				break;
			}
			// ��ȡ���֤�������ļ��ɹ�������base64����
			encodeBase64_pic = zBase.Encode((const unsigned char*)fileBuffer, (int)lSize);
			// �ͷ��ڴ�
			free(fileBuffer);
			fileBuffer = NULL;
			fclose(pFile);
			pFile = NULL;
			nRet = 0;
		}while (0);

		// ������ת��
		ReadIDCardCodeTrans(nRet, sRetCode, sRetMsg);
		// ��ȡ����ƴ��
		MyGetPinYin(pPerson->name, pinyin);
		// ƴ���֤��Ч����
		sprintf(effDate, "%4.4s.%2.2s.%2.2s-%4.4s.%2.2s.%2.2s", pPerson->validStart, pPerson->validStart+4, pPerson->validStart+4+2,
			pPerson->validEnd, pPerson->validEnd+4, pPerson->validEnd+4+2);
		sprintf(strFlag, "%4d", pPerson->iFlag);
		jsonBuff["XYM"] = sRetCode;
		jsonBuff["XYSM"] = sRetMsg;
		jsonBuff["NAME"] = pPerson->name;
		jsonBuff["NAME_PINYIN"] = pinyin;
		jsonBuff["VERSION"] = pPerson->version;
		jsonBuff["GOV_CODE"] = pPerson->govCode;
		jsonBuff["CARD_TYPE"] = pPerson->cardType;
		jsonBuff["SEX"] = pPerson->sex;
		jsonBuff["NATION"] = pPerson->nation;
		jsonBuff["BIRTH_DAY"] = pPerson->birthday;
		jsonBuff["PAPER_ADDR"] = pPerson->address;
		jsonBuff["ID_NO"] = pPerson->cardId;
		jsonBuff["DFFECTIVE_DATE"] = effDate;
		jsonBuff["ISSUING_AUTHORITY"] = pPerson->police;
		jsonBuff["APPEND_MSG"] = pPerson->appendMsg;
		jsonBuff["FLAG"] = strFlag;
		jsonBuff["HRAD_PIC"] = encodeBase64_pic.c_str();
		jsonBuff["OTH_MSG1"] = "";
	}
	else
	{
		// ������ת��
		ReadIDCardCodeTrans(nRet, sRetCode, sRetMsg);
		jsonBuff["XYM"] = sRetCode;
		jsonBuff["XYSM"] = sRetMsg;
		jsonBuff["NAME"] = "";
		jsonBuff["NAME_PINYIN"] = "";
		jsonBuff["ENGLISH_NAME"] = "";
		jsonBuff["VERSION"] = "";
		jsonBuff["GOV_CODE"] = "";
		jsonBuff["CARD_TYPE"] = "";
		jsonBuff["SEX"] = "";
		jsonBuff["NATION"] = "";
		jsonBuff["BIRTH_DAY"] = "";
		jsonBuff["PAPER_ADDR"] = "";
		jsonBuff["ID_NO"] = "";
		jsonBuff["DFFECTIVE_DATE"] = "";
		jsonBuff["ISSUING_AUTHORITY"] = "";
		jsonBuff["APPEND_MSG"] = "";
		jsonBuff["FLAG"] = "";
		jsonBuff["HRAD_PIC"] = "";
		jsonBuff["OTH_MSG1"] = "";
	}
	return ;
}


/* ���ܣ�ͨ�����֤���������鷵�ر���
 * ��Σ�flag 0-���� 1-����
 *		strDir ��Ƭ����·��
 *		nRet ��������֮ǰ���������ͼƬ�Ƿ�ɹ�
 * ���Σ�jsonBuff�����ر��� 
*/ 
void getIDPicJson(Json::Value &jsonBuff, int flag, CString strDir, int nRet)
{
	// ��ȡ���֤������
	FILE * pFile= NULL;
	char *fileBuffer = NULL;
	long lSize = 0;
	ZBase64 zBase;
	string encodeBase64_pic;
	size_t result = 0;
	char sRetCode[4+1] = {0};
	char sRetMsg[128+1] = {0};
	char PIC_FLAG[16] = {0};
	switch (flag) 
	{
	case 0:
		strncpy(PIC_FLAG, "FRONT_PIC", sizeof(PIC_FLAG)-1);
		break;
	case 1:
		strncpy(PIC_FLAG, "BACK_PIC", sizeof(PIC_FLAG)-1);
		break;
	case 2:
		strncpy(PIC_FLAG, "LIVE_PIC", sizeof(PIC_FLAG)-1);
		break;
	}
	// �ж� �����ǻ�ȡͼƬ�Ƿ�ɹ������ɹ�����ʧ��
	if (nRet == 0)
	{
		do 
		{
			pFile = fopen (strDir.GetBuffer(strDir.GetLength()), "rb");
			strDir.ReleaseBuffer();
			if (pFile == NULL)
			{
				GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDPicJson() open [%s] failed! err=[%d][%s]", strDir.GetBuffer(), errno, strerror(errno));
				nRet = 101;
				jsonBuff[PIC_FLAG] = "";
				break;
			}
			fseek (pFile, 0, SEEK_END);
			lSize = ftell(pFile);
			rewind (pFile);
			// �����ڴ�洢�����ļ�
			fileBuffer = (char*) malloc(sizeof(char) * lSize);
			if (fileBuffer == NULL)
			{
				GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDPicJson() malloc failed! err=[%d][%s]", errno, strerror(errno));
				nRet = 103;
				jsonBuff[PIC_FLAG] = "";
				// �ر��ļ�
				fclose(pFile);
				pFile = NULL;
				break;
			}
			// ���ļ�������fileBuffer��
			result = fread(fileBuffer, 1, lSize, pFile);
			if (result != lSize)
			{
				GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDPicJson() read [%s] failed! err=[%d][%s]", strDir.GetBuffer(), errno, strerror(errno));
				nRet = 102;
				jsonBuff[PIC_FLAG] = "";
				// �ر��ļ����ͷ��ڴ�
				fclose(pFile);
				pFile = NULL;
				free(fileBuffer);
				fileBuffer = NULL;
				break;
			}
			// ��ȡ���֤�������ļ��ɹ�������base64����
			encodeBase64_pic = zBase.Encode((const unsigned char*)fileBuffer, (int)lSize);
			// �ͷ��ڴ�
			free(fileBuffer);
			fileBuffer = NULL;
			fclose(pFile);
			pFile = NULL;
			jsonBuff[PIC_FLAG] = encodeBase64_pic.c_str();
			//GtWriteTrace(EM_TraceDebug, "[MainFrm]file buff=[%s]", encodeBase64_pic.c_str());
		} while (0);
		// ������ת��
		ReadPicCodeTrans(nRet, sRetCode, sRetMsg);
	}
	else
	{
		// ������ת��
		ReadPicCodeTrans(nRet, sRetCode, sRetMsg);
		jsonBuff[PIC_FLAG] = "";
	}
	jsonBuff["XYM"] = sRetCode;
	jsonBuff["XYSM"] = sRetMsg;
	jsonBuff["OTH_MSG1"] = "";
	return ;
}


/* ���ܣ�ͨ��������Ƭ�鷵�ر���
 * ��Σ�strDir ��Ƭ����·��
 *		pFilename ��Ƭ����
 *		nRet ��������֮ǰ�������������Ƿ�ɹ�
 *		jsonBuff�����ر��� 
*/ 
void getJsonFromPersonPic(Json::Value &jsonBuff, CString strDir, char *pFilename, int nRet)
{
	// ��ȡ���֤������
	FILE * pFile= NULL;
	char *fileBuffer = NULL;
	long lSize = 0;
	ZBase64 zBase;
	string encodeBase64_pic;
	size_t result = 0;
	char sRetCode[4+1] = {0};
	char sRetMsg[128+1] = {0};
	char PIC_FLAG[16] = {0};
	// �ж� �����ǻ�ȡͼƬ�Ƿ�ɹ������ɹ�����ʧ��
	if (nRet == 0)
	{
		jsonBuff["FILE_DIR"] = strDir.GetBuffer();
	}
	else
	{
		jsonBuff["FILE_DIR"] = "";
	}
	// ��Ӧ��ת��
	ReadPicCodeTrans(nRet, sRetCode, sRetMsg);
	jsonBuff["XYM"] = sRetCode;
	jsonBuff["XYSM"] = sRetMsg;
	jsonBuff["FILE_NAME"] = pFilename;
	jsonBuff["OTH_MSG1"] = "";
	return ;
}


/* ���ܣ�ͨ��������Ƭ�鷵�ر���
 * ��Σ�strDir ��Ƭ���ؾ���·��
 *		jsonBuff�����ر��� 
*/ 
void getJsonFromPic(Json::Value &jsonBuff, CString strDir)
{
	// ��ȡ���֤������
	FILE * pFile= NULL;
	char *fileBuffer = NULL;
	long lSize = 0;
	ZBase64 zBase;
	string encodeBase64_pic;
	size_t result = 0;
	char sRetCode[4+1] = {0};
	char sRetMsg[128+1] = {0};
	int nRet = 0;
	do 
	{
		pFile = fopen (strDir.GetBuffer(strDir.GetLength()), "rb");
		strDir.ReleaseBuffer();
		if (pFile == NULL)
		{
			GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDPicJson() open [%s] failed! err=[%d][%s]", strDir.GetBuffer(), errno, strerror(errno));
			nRet = 101;
			jsonBuff["LIVE_PIC"] = "";
			break;
		}
		fseek (pFile, 0, SEEK_END);
		lSize = ftell(pFile);
		rewind (pFile);
		// �����ڴ�洢�����ļ�
		fileBuffer = (char*) malloc(sizeof(char) * lSize);
		if (fileBuffer == NULL)
		{
			GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDPicJson() malloc failed! err=[%d][%s]", errno, strerror(errno));
			nRet = 103;
			jsonBuff["LIVE_PIC"] = "";
			// �ر��ļ�
			fclose(pFile);
			pFile = NULL;
			break;
		}
		// ���ļ�������fileBuffer��
		result = fread(fileBuffer, 1, lSize, pFile);
		if (result != lSize)
		{
			GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDPicJson() read [%s] failed! err=[%d][%s]", strDir.GetBuffer(), errno, strerror(errno));
			nRet = 102;
			jsonBuff["LIVE_PIC"] = "";
			// �ر��ļ����ͷ��ڴ�
			fclose(pFile);
			pFile = NULL;
			free(fileBuffer);
			fileBuffer = NULL;
			break;
		}
		// ��ȡ���֤�������ļ��ɹ�������base64����
		encodeBase64_pic = zBase.Encode((const unsigned char*)fileBuffer, (int)lSize);
		// �ͷ��ڴ�
		free(fileBuffer);
		fileBuffer = NULL;
		fclose(pFile);
		pFile = NULL;
		jsonBuff["LIVE_PIC"] = encodeBase64_pic.c_str();
		//GtWriteTrace(EM_TraceDebug, "[MainFrm]file buff=[%s]", encodeBase64_pic.c_str());
	} while (0);
	// ������ת��
	ReadPicCodeTrans(nRet, sRetCode, sRetMsg);
	jsonBuff["XYM"] = sRetCode;
	jsonBuff["XYSM"] = sRetMsg;
	jsonBuff["OTH_MSG1"] = "";
	return ;
}


// ����json���ݱ���
void SendJsonMsg(Json::Value &jsonBuff, PIO_OP_KEY pOpKey)
{
	sockaddr_in addr;
	addr.sin_addr.s_addr = pOpKey->remote_addr;

	// json����ת��string��ʽ
	string msgStr_rtn_gbk = jsonBuff.toStyledString();

	// ת�� GBKToUtf8
	GtWriteTrace(EM_TraceDebug, "%s:%d: ת��ǰ����(GBK)=[%d]!", __FUNCTION__, __LINE__, msgStr_rtn_gbk.length());
	//GtWriteTrace(EM_TraceDebug, "%s:%d: ת��ǰ����(GBK)=[%s]!", __FUNCTION__, __LINE__, msgStr_rtn_gbk.c_str());
	string msgStr_rtn = MyGBKToUtf8(msgStr_rtn_gbk);
	//string msgStr_rtn = msgStr_rtn_gbk;
	GtWriteTrace(EM_TraceDebug, "%s:%d: ת��󳤶�(UTF-8)=[%d]!", __FUNCTION__, __LINE__, msgStr_rtn.length());

	// ���ͱ��ģ���������=10�ֽڱ��ĳ���+�����壻����������ƣ��ʳ�������Ҫѭ�����÷��ͺ���
	// ����ͷ���ȣ�10�ֽ�
	char sPreBuff[10+1] = {0};
	// �������ֽڳ���
	long countLen = msgStr_rtn.length();

	// ���η���������ֽڳ��ȣ�64 * 1024 socket�����(iocp.h)�����Ϊ64 * 1024�ֽ�
	const int SEND_MAX_LEN = 32 * 1024;
	// ��Ҫ���͵Ĵ���
	int count = (countLen - 1) / SEND_MAX_LEN + 1;
	// ���η��͵��ֽ���
	int sendLen = 0;
	// �����͵ı���ָ��
	char *sendBuff = (char *)msgStr_rtn.c_str();
	// ���ͺ�������ֵ
	int resCount = 0;

	sprintf(sPreBuff, "%010ld", countLen);
	// ���ͱ���ͷ
	GtWriteTrace(EM_TraceDebug, "%s:%d: ���ͱ���ͷ=[%s]!", __FUNCTION__, __LINE__, sPreBuff);
	WriteDataEx(g_hIoRes, pOpKey, NULL, 0, sPreBuff, sizeof(sPreBuff)-1);
	GtWriteTrace(EM_TraceDebug, "%s:%d: �����ͱ��ĳ���=[%d], ��%d�η���!", __FUNCTION__, __LINE__, countLen, count);

	// ѭ�����ͱ��ģ�ÿ�η���SEND_MAX_LEN�ֽ�
	for (int i = 0; i < count; i++)
	{
		// ��ȡ���η����ֽ��������һ�η��� countLen % SEND_MAX_LEN �ֽ�
		sendLen = (i == count - 1) ? countLen % SEND_MAX_LEN : SEND_MAX_LEN;
		resCount = WriteDataEx(g_hIoRes, pOpKey, NULL, 0, sendBuff + i * SEND_MAX_LEN, sendLen);
		if (resCount < 0)
		{
			GtWriteTrace(EM_TraceDebug, "%s:%d: ��%d�η��ͱ���ʧ�ܣ�����WriteDataEx()����ֵ[%d] < 0��ֹͣ���ͱ��ģ�", __FUNCTION__, __LINE__, i + 1, resCount);
			break;
		}
		if (resCount == 0)
		{
			Sleep(10);
		}
		GtWriteTrace(EM_TraceDebug, "%s:%d: ��%d�η��ͱ�����ɣ����η���[%d]�ֽ�!", __FUNCTION__, __LINE__, i + 1, sendLen);
		if (i == count - 1)
		{
			GtWriteTrace(EM_TraceDebug, "%s:%d: ����ȫ��������ɣ���[%d]�ֽ�!", __FUNCTION__, __LINE__, countLen);
		}
	}
	return ;
}


CString RetMsg(string xym,string xynr)
{
	CString ret_str="";
	Json::Value msgStr_json_rtn;//��ʾһ��json��ʽ�Ķ��� 
	std::string msgStr_rtn;
	msgStr_json_rtn["XYM"]=xym.c_str();
	msgStr_json_rtn["XYSM"]=xynr.c_str();
	msgStr_rtn=msgStr_json_rtn.toStyledString();
	GtWriteTrace(EM_TraceDebug,"%s",msgStr_rtn.c_str());
	ret_str = msgStr_rtn.c_str();
	return ret_str;
}
CString Json_060104_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json����
	std::string out="";
	CString reStr="";
	out=value["XM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//����Ϊ��
		reStr=RetMsg("001","�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		//���ܰ�������
		bool bIsDigit=FALSE;; 
		for(int i=0;i<out.length();i++)
		{
			if(isdigit(out.at(i)))
			{
				GtWriteTrace(EM_TraceDebug,"%s",out.c_str());
				bIsDigit=TRUE;
				break;
			}
		}
		if(bIsDigit==TRUE)
		{
			reStr=RetMsg("018","����(Ӣ��/ƴ��):���ܰ�������");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZJSFCQYX"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//֤���Ƿ�����ЧΪ��
		reStr=RetMsg("002","֤���Ƿ�����Ч�Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(out!="1" && out!="2")
		{
			//֤��������Ϊ��
			reStr=RetMsg("003","֤���Ƿ�����Ч:�������ݲ�����");
			//д��־
			return reStr;
		}
		sendMsg+=out.c_str();
		
		if(out=="2")
		{
			out=value["ZJDQR"].asString();
			if(out.empty()!=0) //true 1 false 0
			{
				//֤��������Ϊ��
				reStr=RetMsg("003","֤���������Ǳ��������Ϊ��");
				//д��־
				return reStr;
			}
			else
			{
				sendMsg+=out.c_str();
				sendMsg+="\r\n";
			}
		}
	}
			
	out=value["FZJGDQDM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//��֤���ص�������Ϊ��
		reStr=RetMsg("004","��֤���ص��������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["XB"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�Ա�Ϊ��
		reStr=RetMsg("005","�Ա��Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(out!="1" && out!="2")
		{
			reStr=RetMsg("004","�Ա��������ݲ�����");
			return reStr;
		}
		sendMsg+=out.c_str();
	}
	out=value["GJ"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//����Ϊ��
		reStr=RetMsg("006","�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
	}
	out=value["GDDH"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		out=value["YDDH"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//�ƶ��绰Ϊ��
			reStr=RetMsg("007","�̶��绰���ƶ��绰��������һ��");
			//д��־
			return reStr;
		}
		else
		{
			if(out.length()!=11)
			{
				reStr=RetMsg("007","�ƶ��绰:������11λ����");
				//д��־
				return reStr;
			}
			sendMsg+="\r\n";
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		out=value["YDDH"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//�̶��绰��Ϊ�գ��ƶ��绰Ϊ��
			sendMsg+="\r\n";
		}
		else
		{
			if(out.length()!=11)
			{
				reStr=RetMsg("007","�ƶ��绰:������11λ����");
				//д��־
				return reStr;
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
	}
	
	out=value["TXDZ"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//ͨѶ��ַΪ��
		reStr=RetMsg("008","ͨѶ��ַ�Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(out.length()<4)
		{
			reStr=RetMsg("012","ͨѶ��ַ��������Ϊ4���ַ�(1������Ϊ2���ַ�)");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["YZBM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�������벻�Ǳ�����
		sendMsg+="\r\n";
	}
	else
	{
		if(out.length()!=6)
		{
			reStr=RetMsg("012","��������Ϊ6λ����");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZY"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//ְҵΪ��
		reStr=RetMsg("009","ְҵ�Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(atoi(out.c_str())<1 || atoi(out.c_str())>8)
		{
			reStr=RetMsg("009","ְҵ�������ݲ�����");
			//д��־
			return reStr;
		}
		sendMsg+=out.c_str();
	}
	reStr=RetMsg("000","���ͳɹ�");
	return reStr;
}

CString Json_010101_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json����
	std::string out="";
	std::string zjlx="";
	sendMsg+=out.c_str();
	CString reStr="";
	out=value["HM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//����Ϊ��
		reStr=RetMsg("002","�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		GtWriteTrace(EM_TraceDebug,"%s",sendMsg);
		if(out.length()<2)
		{
			reStr=RetMsg("012","������������Ϊ2���ַ�(1������Ϊ2���ַ�)");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		GtWriteTrace(EM_TraceDebug,"%s",sendMsg);
	}
	zjlx=value["ZJLX"].asString();
	if(zjlx.empty()!=0) //true 1 false 0
	{
		//֤������Ϊ��
		reStr=RetMsg("013","֤�������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(ZJLX.find(zjlx)==-1)
		{
			reStr=RetMsg("014","֤�����Ͳ�����");
			return reStr;
		}
		sendMsg+=zjlx.c_str();
	}
	out=value["ZJHM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//֤������Ϊ��
		reStr=RetMsg("015","֤�������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(zjlx=="10")
		{
			if(out.length()!=18)
			{
				reStr=RetMsg("016","���ֻ֤��18λ");
				return reStr;
			}
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+="\r\n";
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	Json::Reader tmp_reader;//json����
	Json::Value tmp_json_rtn;//��ʾһ��json��ʽ�Ķ��� 
	std::string tmp_out;
	reStr=Json_060104_SendMsg(value);
	if(tmp_reader.parse(reStr.GetBuffer(), tmp_json_rtn))//������json�ŵ�json����
	{
		tmp_out=tmp_json_rtn["XYM"].asString();
		GtWriteTrace(EM_TraceDebug, "��Ӧ��=[%s]", tmp_out.c_str());
		if(0 != strcmp(tmp_out.c_str(), "000"))
		{
			return reStr;
		}
	}
	else
	{
		reStr=RetMsg("999","����ʧ��");
		return reStr;
	}
	return reStr;
}

CString Json_101003_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json����
	std::string out="";
	std::string zjlx="";
	sendMsg+=out.c_str();
	CString reStr="";

	out=value["ZFZFF"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�ʷ�֧����Ϊ��
		reStr=RetMsg("017","�ʷ�֧�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(out!="1"&&out!="2")
		{
			reStr=RetMsg("017","�ʷ�֧�������������ݲ�����");
			//д��־
			return reStr;
		}
		sendMsg+=out.c_str();
	}
	out=value["SKRXM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�տ�������Ϊ��
		reStr=RetMsg("018","�տ��������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["SKRZKH"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�տ����˺�/����Ϊ��
		reStr=RetMsg("019","�տ����˺�/�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+="\r\n";
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["HKJE"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�����Ϊ��
		reStr=RetMsg("020","������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+="\r\n";
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZZLX"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//ת������Ϊ��
		reStr=RetMsg("021","ת�������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(atoi(out.c_str())<1||atoi(out.c_str())>3)
		{
			reStr=RetMsg("021","ת������:�������ݲ�����");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["HKRXM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//���������Ϊ��
		reStr=RetMsg("022","����������Ǳ��������Ϊ��");
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}

	out=value["HKRDZDH"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//����˵�ַ/�绰Ϊ��
		reStr=RetMsg("023","����˵�ַ/�绰�Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}

	zjlx=value["HKRZJLX"].asString();
	if(zjlx.empty()!=0) //true 1 false 0
	{
		sendMsg+="\r\n";
		//�����Ƿ����븨��֤��
		sendMsg+="\r\n";
	}
	else
	{
		if(ZJLX.find(zjlx)==-1)
		{
			reStr=RetMsg("014","�����֤�����Ͳ�����");
			return reStr;
		}
		sendMsg+=zjlx.c_str();
		out=value["HKRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 
		{
			//����Ϊ��
			reStr=RetMsg("024","�����֤�������Ǳ��������Ϊ��");
			//д��־
			return reStr;
		}
		else
		{
			if(zjlx=="10")
			{
				if(out.length()!=18)
				{
					reStr=RetMsg("016","���ֻ֤��18λ");
					return reStr;
				}
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
			sendMsg+="\r\n";
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
	}
	out=value["XLKHBZ"].asString();
	if(out.empty()!=0) //true 1 false 
	{
		//���Ͽͻ���־Ϊ��
		reStr=RetMsg("025","���Ͽͻ���־�Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(0==strcmp(out.c_str(),"0"))
		{
			Json::Reader tmp_reader;//json����
			Json::Value tmp_json_rtn;//��ʾһ��json��ʽ�Ķ��� 
			std::string tmp_out;
			//�¿ͻ�
			reStr=Json_060104_SendMsg(value);
			GtWriteTrace(EM_TraceDebug, "�ͻ�¼�뷵��json=[%s]", reStr.GetBuffer());
			if(tmp_reader.parse(reStr.GetBuffer(), tmp_json_rtn))//������json�ŵ�json����
			{
				tmp_out=tmp_json_rtn["XYM"].asString();
				GtWriteTrace(EM_TraceDebug, "��Ӧ��=[%s]", tmp_out.c_str());
				if(0 != strcmp(tmp_out.c_str(), "000"))
				{
					return reStr;
				}
			}
			else
			{
				reStr=RetMsg("999","����ʧ��");
				return reStr;
			}
		}
		else
		{
			//�Ͽͻ�
			/*if(::MessageBox(NULL,"�Ͽͻ��ͻ�¼���Ƿ񸲸�","��ʾ",MB_OK|MB_OKCANCEL)==IDOK)
			{
				reStr=Json_060104_SendMsg(value);
				return reStr;
			}*/
		}
	}
	reStr=RetMsg("000","���ͳɹ�");
	return reStr;
}

CString Json_101004_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json����
	std::string out="";
	std::string zjlx="";
	sendMsg+=out.c_str();
	CString reStr="";	

	out=value["ZFZFF"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�ʷ�֧����Ϊ��
		reStr=RetMsg("026","�ʷ�֧�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(out!="1"&&out!="2")
		{
			reStr=RetMsg("017","�ʷ�֧�������������ݲ�����");
			//д��־
			return reStr;
		}
		sendMsg+=out.c_str();
	}
	out=value["SKRXM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�տ�������Ϊ��
		reStr=RetMsg("027","�տ��������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(out.length()<4)
		{
			reStr=RetMsg("027","�տ�������:����С��4λ��������");
			//д��־
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["SKRZKH"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�տ����˺�/����Ϊ��
		reStr=RetMsg("028","�տ����˺�/�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+=out.c_str();
	}
	out=value["HKJE"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�����Ϊ��
		reStr=RetMsg("029","������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+="\r\n";
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZZLX"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//ת������Ϊ��
		reStr=RetMsg("030","ת�������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(atoi(out.c_str())<1||atoi(out.c_str())>3)
		{
			reStr=RetMsg("021","ת������:�������ݲ�����");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZHMMBZ"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�˻������־Ϊ��
		reStr=RetMsg("032","�˻������־�Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(0==strcmp(out.c_str(),"ZHMM_BZ"))
		{
			sendMsg+='&';
		}
		else
		{
			reStr=RetMsg("032","�˻������־����ֵ����");
			//д��־
			return reStr;
		}
		
	}
	zjlx=value["HKRZJLX"].asString();
	if(zjlx.empty()!=0) //true 1 false 0
	{
		//�����֤������Ϊ�գ�ֱ�������Ƿ����븨��֤��
		sendMsg+="\r\n";
	}
	else
	{
		if(ZJLX.find(zjlx)==-1)
		{
			reStr=RetMsg("014","�����֤�����Ͳ�����");
			return reStr;
		}
		sendMsg+=zjlx.c_str();
		out=value["HKRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//�����֤������Ϊ��
			reStr=RetMsg("033","�����֤�������Ǳ��������Ϊ��");
			//д��־
			return reStr;
		}
		else
		{
			if(zjlx=="10")
			{
				if(out.length()!=18 && out.length()!=15)
				{
					reStr=RetMsg("016","���ֻ֤��15λ��18λ");
					return reStr;
				}
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
	}
	//�����Ƿ����븨��֤��
	sendMsg+="\r\n";
	out=value["SFDLRDB"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�Ƿ�����˴���Ϊ��,Ĭ��Ϊ2-��
		sendMsg+="\r\n";
	}
	else
	{
		if(out!="1"&&out!="2")
		{
			reStr=RetMsg("016","�Ƿ�����˴���������������");
			return reStr;
		}
		if(out=="2")
		{
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
		else if(out=="1")
		{
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
			out=value["DLRXM"].asString();
			if(out.empty()!=0) //true 1 false 0
			{
				//����Ϊ��
				reStr=RetMsg("034","�����Ǳ��������Ϊ��");
				return reStr;
			}
			else 
			{
				if(out.length()<2)
				{
					reStr=RetMsg("034","��������������������Ϊ2���ַ�(1������Ϊ2���ַ�)");
					return reStr;
				}
				GtWriteTrace(EM_TraceDebug,"%s",out.c_str());
				//����ȫ���������
				bool bIsNotDigit=FALSE;; 
				char is_c_flag=0;
				char szchinese[3] = {0}; 
				for(int i=0;i<out.length();i++)
				{
					GtWriteTrace(EM_TraceDebug,"[%d]%c",i,out.at(i));
					if(out.at(i)&0x80)
					{
						szchinese[is_c_flag]=out.at(i);
						is_c_flag++;
						
						if(is_c_flag==2)
						{
							GtWriteTrace(EM_TraceDebug,"%s",szchinese);
							memset(szchinese,0x00,sizeof(szchinese));
							bIsNotDigit=TRUE;
							is_c_flag=0;
							break;
						}
					}
				}
				if(bIsNotDigit==FALSE)
				{
						//GtWriteTrace(EM_TraceDebug,"����������:����ȫ���������");
					reStr=RetMsg("018","����������:����ȫ���������");
					return reStr;
				}
				sendMsg+=out.c_str();
				sendMsg+="\r\n";
			}
			zjlx=value["DLRZJLX"].asString();
			if(zjlx.empty()!=0) //true 1 false 0
			{
				//֤������Ϊ��
				reStr=RetMsg("035","֤�������Ǳ��������Ϊ��");
				return reStr;
			}
			else 
			{
				if(ZJLX.find(zjlx)==-1)
				{
					reStr=RetMsg("014","�����֤�����Ͳ�����");
					return reStr;
				}
				sendMsg+=zjlx.c_str();
			}
			out=value["DLRZJHM"].asString();
			if(out.empty()!=0) //true 1 false 0
			{
				//֤������Ϊ��
				reStr=RetMsg("036","֤�������Ǳ��������Ϊ��");
				return reStr;
			}
			else 
			{
				if(zjlx=="10")
				{
					if(out.length()!=18)
					{
						reStr=RetMsg("016","���ֻ֤��18λ");
						return reStr;
					}
				}
				sendMsg+=out.c_str();
				sendMsg+="\r\n";
				sendMsg+="\r\n";
				sendMsg+=out.c_str();
				sendMsg+="\r\n";
			}
			out=value["DLRLXDH"].asString();
			if(out.empty()!=0) //true 1 false 0
			{
				//��ϵ�绰Ϊ��
				reStr=RetMsg("037","��ϵ�绰�Ǳ��������Ϊ��");
				return reStr;
			}
			else 
			{
				if(out.length()<9)
				{
					reStr=RetMsg("016","��ϵ�绰������������9λ�ַ�(������)");
					return reStr;
				}
				sendMsg+=out.c_str();
			}
		}
	}
	reStr=RetMsg("000","���ͳɹ�");
	return reStr;
}

CString Json_101005_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json����
	std::string out="";
	std::string zjlx="";
	std::string hkfs="";
	std::string zfzff="";
	sendMsg+=out.c_str();
	CString reStr="";

	hkfs=value["HKFS"].asString();
	if(hkfs.empty()!=0) //true 1 false 0
	{
		//��ʽΪ��
		reStr=RetMsg("014","��ʽ�Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(hkfs!="1"&&hkfs!="2")
		{
			reStr=RetMsg("017","��ʽ���������ݲ�����");
			//д��־
			return reStr;
		}
		sendMsg+=hkfs.c_str();
	}
	zfzff=value["ZFZFF"].asString();
	if(zfzff.empty()!=0) //true 1 false 0
	{
		//�ʷ�֧����Ϊ��
		reStr=RetMsg("014","�ʷ�֧�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(zfzff!="1"&&zfzff!="2")
		{
			reStr=RetMsg("017","�ʷ�֧�������������ݲ�����");
			//д��־
			return reStr;
		}
		sendMsg+=zfzff.c_str();
	}
	out=value["SKDWMC"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�տλ����Ϊ��
		reStr=RetMsg("015","�տλ�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["SKDWZH"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�տλ�˺�Ϊ��
		reStr=RetMsg("016","�տλ�˺��Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["HKJE"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�����Ϊ��
		reStr=RetMsg("017","������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZZLX"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//ת������Ϊ��
		reStr=RetMsg("018","ת�������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(atoi(out.c_str())<1||atoi(out.c_str())>3)
		{
			reStr=RetMsg("021","ת������:�������ݲ�����");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	if(0==strcmp(hkfs.c_str(),"2"))
	{
		/*out=value["ZPRZLSH"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//֧Ʊ������ˮ��Ϊ��
			reStr=RetMsg("018","֧Ʊ������ˮ���Ǳ��������Ϊ��");
			//д��־
			return reStr;
		}
		else
		{
			if(out.length()!=17)
			{
				//֧Ʊ������ˮ�ų��ȴ���
				reStr=RetMsg("018","֧Ʊ������ˮ��:������17λ���ֻ���ĸ");
				return reStr;
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
		out=value["RZRQ"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//��������Ϊ��
			reStr=RetMsg("018","���������Ǳ��������Ϊ��");
			//д��־
			return reStr;
		}
		else
		{
			//�������ڲ��ܴ��ڽ�������
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}*/
		out=value["ZPLSHBZ"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//֧Ʊ��ˮ�ű�־Ϊ��
			reStr=RetMsg("018","֧Ʊ��ˮ�ű�־�Ǳ��������Ϊ��");
			return reStr;
		}
		else
		{
			if(out!="ZPLSH_BZ")
			{
				//֧Ʊ��ˮ�ű�־Ϊ��
				reStr=RetMsg("018","֧Ʊ��ˮ�ű�־��������");
				return reStr;
			}
			sendMsg+="&";
		}
		
	}
	
	if(zfzff=="2")
	{
		out=value["XYH"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//Э���Ϊ��
			reStr=RetMsg("018","Э����Ǳ��������Ϊ��");
			//д��־
			return reStr;
		}
		else
		{
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
	}

	out=value["FY"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		sendMsg+="\r\n";
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}

	out=value["HKRXM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//���������Ϊ��
		reStr=RetMsg("018","����������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(out.length()<2)
		{
			reStr=RetMsg("018","���������:��������Ϊ2���ַ�(1�����������ַ�)");
			//д��־
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}

	out=value["HKRDZDH"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//����˵�ַ/�绰Ϊ��
		reStr=RetMsg("04","����˵�ַ/�绰�Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}

	zjlx=value["HKRZJLX"].asString();
	if(zjlx.empty()!=0) //true 1 false 0
	{
		sendMsg+="\r\n";
		//�����Ƿ����븨��֤��
		sendMsg+="\r\n";
	}
	else
	{
		if(ZJLX.find(zjlx)==-1)
		{
			reStr=RetMsg("014","�����֤�����Ͳ�����");
			return reStr;
		}
		sendMsg+=zjlx.c_str();
		sendMsg+="\r\n";
		out=value["HKRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 
		{
			//����Ϊ��
			reStr=RetMsg("05","�����֤�������Ǳ��������Ϊ��");
			//д��־
			return reStr;
		}
		else
		{
			if(zjlx=="10")
			{
				if(out.length()!=18 && out.length()!=15)
				{
					reStr=RetMsg("016","���ֻ֤��15λ��18λ");
					return reStr;
				}
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
			sendMsg+="\r\n";
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
	}
	out=value["XLKHBZ"].asString();
	if(out.empty()!=0) //true 1 false 
	{
		//���Ͽͻ���־Ϊ��
		reStr=RetMsg("05","���Ͽͻ���־�Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(0==strcmp(out.c_str(),"0"))
		{
			Json::Reader tmp_reader;//json����
			Json::Value tmp_json_rtn;//��ʾһ��json��ʽ�Ķ��� 
			std::string tmp_out;
			//�¿ͻ�
			reStr=Json_060104_SendMsg(value);
			if(tmp_reader.parse(reStr.GetBuffer(), tmp_json_rtn))//������json�ŵ�json����
			{
				tmp_out=tmp_json_rtn["XYM"].asString();
				GtWriteTrace(EM_TraceDebug, "��Ӧ��=[%s]", tmp_out.c_str());
				if(0 != strcmp(tmp_out.c_str(), "000"))
				{
					return reStr;
				}
			}
			else
			{
				reStr=RetMsg("999","����ʧ��");
				return reStr;
			}
			sendMsg+='&';
		}
		else
		{
			//�Ͽͻ�
			/*if(::MessageBox(NULL,"�Ͽͻ��ͻ�¼���Ƿ񸲸�","��ʾ",MB_OK|MB_OKCANCEL)==IDOK)
			{
				reStr=Json_060104_SendMsg(value);
				return reStr;
			}*/
			sendMsg+='&';
		}
	}
	//�����Ƿ����븨��֤��
	sendMsg+="\r\n";

	out=value["DLRXM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�Ƿ�����˴���Ϊ��,Ĭ��Ϊ2-��
		sendMsg+="\r\n";
	}
	else
	{
		if(out.length()<2)
		{
			reStr=RetMsg("025","��������������������Ϊ2���ַ�(1�����������ַ�)");
			return reStr;
		}
		//����ȫ���������
		bool bIsNotDigit=FALSE;; 
		char is_c_flag=0;
		char szchinese[3] = {0}; 
		for(int i=0;i<out.length();i++)
		{
			GtWriteTrace(EM_TraceDebug,"[%d]%c",i,out.at(i));
			if(out.at(i)&0x80)
			{
				szchinese[is_c_flag]=out.at(i);
				is_c_flag++;
						
				if(is_c_flag==2)
				{
					GtWriteTrace(EM_TraceDebug,"%s",szchinese);
					memset(szchinese,0x00,sizeof(szchinese));
					bIsNotDigit=TRUE;
					is_c_flag=0;
					break;
				}
			}
		}
		if(bIsNotDigit==FALSE)
		{
				//GtWriteTrace(EM_TraceDebug,"����������:����ȫ���������");
			reStr=RetMsg("018","����������:����ȫ���������");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+="\r\n";
		zjlx=value["DLRZJLX"].asString();
		if(zjlx.empty()!=0) //true 1 false 0
		{
			//֤������Ϊ��
			reStr=RetMsg("025","֤�������Ǳ��������Ϊ��");
			return reStr;
		}
		else 
		{
			if(ZJLX.find(zjlx)==-1)
			{
				reStr=RetMsg("014","�����֤�����Ͳ�����");
				return reStr;
			}
			sendMsg+=zjlx.c_str();
		}
		out=value["DLRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//֤������Ϊ��
			reStr=RetMsg("026","֤�������Ǳ��������Ϊ��");
			return reStr;
		}
		else 
		{
			if(zjlx=="10")
			{
				if(out.length()!=18 && out.length()!=15)
				{
					reStr=RetMsg("016","���ֻ֤��Ϊ18λ");
					return reStr;
				}
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
			sendMsg+="\r\n";
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
		out=value["DLRLXDH"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//��ϵ�绰Ϊ��
			reStr=RetMsg("027","��ϵ�绰�Ǳ��������Ϊ��");
			return reStr;
		}
		else 
		{
			if(out.length()<9)
			{
				reStr=RetMsg("027","��ϵ�绰:����������9λ�ַ�(������)");
				return reStr;
			}
			sendMsg+=out.c_str();
		}
	}

	reStr=RetMsg("000","���ͳɹ�");
	return reStr;
}

CString Json_101006_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json����
	std::string out="";
	std::string zjlx="";
	sendMsg+=out.c_str();
	std::string zfzff="";
	CString reStr="";	

	zfzff=value["ZFZFF"].asString();
	if(zfzff.empty()!=0) //true 1 false 0
	{
		//�ʷ�֧����Ϊ��
		reStr=RetMsg("014","�ʷ�֧�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(zfzff!="1"&&zfzff!="2")
		{
			reStr=RetMsg("017","�ʷ�֧�������������ݲ�����");
			//д��־
			return reStr;
		}
		sendMsg+=zfzff.c_str();
		sendMsg+="\r\n";
	}
	out=value["HCZHKHBZ"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//����˻����ű�־Ϊ��
		reStr=RetMsg("015","����˻����ű�־�Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(out=="HCZHKH_BZ")
		{
			sendMsg+='&';
		}
		else
		{
			reStr=RetMsg("017","����˻����ű�־��������");
			//д��־
			return reStr;
		}
	}
	out=value["SKDWMC"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�տλ����Ϊ��
		reStr=RetMsg("016","�տλ�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["SKDWZH"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�տλ�˺�Ϊ��
		reStr=RetMsg("017","�տλ�˺��Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["HKJE"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�����Ϊ��
		reStr=RetMsg("017","������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+="\r\n";
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZZLX"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//ת������Ϊ��
		reStr=RetMsg("018","ת�������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		if(atoi(out.c_str())<1||atoi(out.c_str())>3)
		{
			reStr=RetMsg("021","ת������:�������ݲ�����");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	if(0==strcmp(zfzff.c_str(),"2"))
	{
		out=value["XYH"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//Э���Ϊ��
			reStr=RetMsg("018","Э����Ǳ��������Ϊ��");
			//д��־
			return reStr;
		}
		else
		{
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
	}
	out=value["FY"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		sendMsg+="\r\n";
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	zjlx=value["HKRZJLX"].asString();
	if(zjlx.empty()!=0) //true 1 false 0
	{
		//�����֤������Ϊ�գ�ֱ�������Ƿ����븨��֤��
		sendMsg+="\r\n";
		sendMsg+='&';
	}
	else
	{
		if(ZJLX.find(zjlx)==-1)
		{
			reStr=RetMsg("014","�����֤�����Ͳ�����");
			return reStr;
		}
		sendMsg+=zjlx.c_str();
		out=value["HKRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//�����֤������Ϊ��
			reStr=RetMsg("023","�����֤�������Ǳ��������Ϊ��");
			//д��־
			return reStr;
		}
		else
		{
			if(zjlx=="10")
			{
				if(out.length()!=18 && out.length()!=15)
				{
					reStr=RetMsg("016","���ֻ֤��15λ��18λ");
					return reStr;
				}
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
			sendMsg+='&';
			//GtWriteTrace(EM_TraceDebug,"%s",sendMsg);
		}
	}
	//�����Ƿ����븨��֤��
	sendMsg+="\r\n";
/*	out=value["SFSRFZZJ"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�Ƿ����븨��֤��Ϊ��,Ĭ��Ϊ2-��
		sendMsg+="\r\n";
	}
	else
	{
		if(0==strcmp(out.c_str(),"2"))
		{
			sendMsg+="\r\n";
		}
		else if(0==strcmp(out.c_str(),"1"))
		{
			//�Ƿ����븨��֤��Ϊ��,Ĭ��Ϊ2-��
			sendMsg+="\r\n";
			sendMsg+='&';
		}
	}
	*/
	out=value["DLRXM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//�Ƿ�����˴���Ϊ��,Ĭ��Ϊ2-��
		sendMsg+="\r\n";
	}
	else
	{
		if(out.length()<2)
		{
			reStr=RetMsg("025","��������������������Ϊ2���ַ�(1�����������ַ�)");
			return reStr;
		}
		bool bIsNotDigit=FALSE;; 
		char is_c_flag=0;
		char szchinese[3] = {0}; 
		for(int i=0;i<out.length();i++)
		{
			GtWriteTrace(EM_TraceDebug,"[%d]%c",i,out.at(i));
			if(out.at(i)&0x80)
			{
				szchinese[is_c_flag]=out.at(i);
				is_c_flag++;
						
				if(is_c_flag==2)
				{
					GtWriteTrace(EM_TraceDebug,"%s",szchinese);
					memset(szchinese,0x00,sizeof(szchinese));
					bIsNotDigit=TRUE;
					is_c_flag=0;
					break;
				}
			}
		}
		if(bIsNotDigit==FALSE)
		{
				//GtWriteTrace(EM_TraceDebug,"����������:����ȫ���������");
			reStr=RetMsg("018","����������:����ȫ���������");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+="\r\n";
		zjlx=value["DLRZJLX"].asString();
		if(zjlx.empty()!=0) //true 1 false 0
		{
			//֤������Ϊ��
			reStr=RetMsg("025","֤�������Ǳ��������Ϊ��");
			return reStr;
		}
		else 
		{
			if(ZJLX.find(zjlx)==-1)
			{
				reStr=RetMsg("014","�����֤�����Ͳ�����");
				return reStr;
			}
			sendMsg+=zjlx.c_str();
		}
		out=value["DLRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//֤������Ϊ��
			reStr=RetMsg("026","֤�������Ǳ��������Ϊ��");
			return reStr;
		}
		else 
		{
			if(zjlx=="10")
			{
				if(out.length()!=18 && out.length()!=15)
				{
					reStr=RetMsg("016","���ֻ֤��Ϊ18λ");
					return reStr;
				}
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
			sendMsg+="\r\n";
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
		out=value["DLRLXDH"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//��ϵ�绰Ϊ��
			reStr=RetMsg("027","��ϵ�绰�Ǳ��������Ϊ��");
			return reStr;
		}
		else 
		{
			if(out.length()<9)
			{
				reStr=RetMsg("027","��ϵ�绰:����������9λ�ַ�(������)");
				return reStr;
			}
			sendMsg+=out.c_str();
		}
	}
	reStr=RetMsg("000","���ͳɹ�");
	return reStr;
}

CString Json_970101_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json����
	std::string out="";
	sendMsg+=out.c_str();
	CString reStr="";	

	out=value["XM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//����Ϊ��
		reStr=RetMsg("014","�����Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZJLX"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//֤������Ϊ��
		reStr=RetMsg("015","֤�������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZJHM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//֤������Ϊ��
		reStr=RetMsg("016","֤�������Ǳ��������Ϊ��");
		//д��־
		return reStr;
	}
	else
	{
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+="\r\n";
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	reStr=RetMsg("000","���ͳɹ�");
	return reStr;
}
int SendToWindows()
{
	// �Զ�����׽��д���
		std::string msgStr_rtn;

		GtWriteTrace(EM_TraceDebug,"sendMsg:[%s]",sendMsg);
		BOOL bRes = FALSE;

		int iPos = sendMsg.Find('&');
		CString strTmp="";
		if(iPos!=-1)
		{
			strTmp = sendMsg.Left(iPos);
			GtWriteTrace(EM_TraceDebug,"strTmp:[%s]",strTmp);
			GtWriteTrace(EM_TraceDebug,"sendMsg:[%s]",sendMsg);
			sendMsg = sendMsg.Mid(iPos + 1);
			GtWriteTrace(EM_TraceDebug,"sendMsg:[%s]",sendMsg);
		}
		else
		{
			strTmp=sendMsg;
			sendMsg="";
			GtWriteTrace(EM_TraceDebug,"sendMsg:[%s]",sendMsg);
		}
		int strlen = strTmp.GetLength();
		CString tmp;
		tmp.Format("wname.GetBuffer()=[%s], sendMsg=[%s]", wname.GetBuffer(), sendMsg.GetBuffer());
		//::MessageBoxA(NULL, tmp, "test", MB_OK);
		//��ȡ���ھ��
		if(wname.GetBuffer() == "")
		{
			::MessageBox(NULL,"Ŀ�괰��δ����","��ʾ",MB_OK);
			return -1;
		}

		hWnd = ::FindWindow(NULL,wname.GetBuffer());
		if(NULL==hWnd){
			::MessageBox(NULL,"û���ҵ�����","��ʾ",MB_OK);
			return -1;
		}

		//Ŀ�괰���߳��뵱ǰ�̹߳���
		DWORD curtid = GetCurrentThreadId();
		DWORD tid = GetWindowThreadProcessId(hWnd, NULL);
		DWORD error = 0;
	/**���������̣߳�ʹĿ���߳��뵱ǰ�̹߳�����Ϣ���У��ⲽ�ܹؼ���
		�ɹ��������Ŀ���̴߳���Ϊ����ڣ�����ȡ����λ�õľ��.*/
		hCurFocus = NULL;
		//Ŀ�괰���ö�
		//::SetWindowPos(hWnd,HWND_TOPMOST,20,20,820,420,SWP_SHOWWINDOW);
		::ShowWindow(hWnd,SW_SHOWMAXIMIZED);
		if(AttachThreadInput(tid,curtid , true))
		{
		//	HWND tmpHWnd = ::SetActiveWindow(hWnd);	
			bRes = ::SetForegroundWindow(hWnd);
			hCurFocus = ::GetFocus();
		}
		else 
		{
			error = GetLastError();
			CString errStr = "�����߳�ʧ��" + error;
			::MessageBox(NULL,errStr,"��ʾ",MB_OK);
			return -1;
		}

	
		//Ŀ�괰���ö�
		//::SetWindowPos(hWnd,HWND_TOPMOST,20,20,820,420,SWP_SHOWWINDOW);
		//���Ϳ��߳���Ϣ
		byte temp[2] = {0};
		LRESULT res = 0;
		if(hCurFocus)
		{
			for(int i=0; i<strlen; )
			{
				temp[0] = strTmp.GetBuffer()[i];
				if(temp[0] < 128)
				{
					res = ::PostMessage(hCurFocus, WM_CHAR, temp[0], 0);
					if(temp[0]==10)
					{
						Sleep(long_time_interval);
					}
					else
					{
						Sleep(short_time_interval);
					}
				}
				else
				{
					temp[1] = strTmp.GetBuffer()[i+1];
					::PostMessage(hCurFocus, WM_CHAR, temp[0], 0);
					::PostMessage(hCurFocus, WM_CHAR, temp[1], 0);
					i++;
				}
				i++;
			}
		}

		//�����̹߳���
		if(!AttachThreadInput(tid, curtid, false))
		{
			::MessageBox(NULL,"ȡ���̹߳���ʧ�ܣ�","��ʾ",MB_OK);
			return -1;
		}
		//ȡ���ö�
		//::SetWindowPos(hWnd,HWND_NOTOPMOST,100,100,900,500,SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
	//	::SetWindowPos(hWnd,HWND_NOTOPMOST,100,100,900,500,SWP_SHOWWINDOW);
		return 0;
}

/*
	JSONת���ɷ��͸�������ϵͳ���ַ�������
	CString JsonToSendMsg(string str,CString &sendMsg)
	���������
		str:�յ��ı���
		sendMsg:���ص��ַ�������
	���������
		���ص�CString���͵�jsonӦ����
*/
CString JsonToSendMsg(string str)
{
	Json::Reader reader;//json����
	Json::Value jValue;//��ʾһ��json��ʽ�Ķ���
	std::string out="";
	CString reStr="";
	if(reader.parse(str,jValue))//������json�ŵ�json����
	{
		out=jValue["JYDM"].asString();
		GtWriteTrace(EM_TraceDebug,"%s",out.c_str());
		//���ڿ���
		if(0==strcmp(out.c_str(),"010101"))
		{
			sendMsg="";
			sendMsg+=out.c_str();
			reStr=Json_010101_SendMsg(jValue);
			return reStr;
		}
		//�����ֽ��˻����
		if(0==strcmp(out.c_str(),"101003"))
		{
			sendMsg="";
			sendMsg+=out.c_str();
			reStr=Json_101003_SendMsg(jValue);
			
			return reStr;
		}
		//�����˻����˻����
		if(0==strcmp(out.c_str(),"101004"))
		{
			sendMsg="";
			sendMsg+=out.c_str();
			reStr=Json_101004_SendMsg(jValue);
			
			return reStr;
		}
		//�����ֽ�/֧Ʊ���Թ��˻����
		if(0==strcmp(out.c_str(),"101005"))
		{
			sendMsg="";
			sendMsg+=out.c_str();
			reStr=Json_101005_SendMsg(jValue);
			
			return reStr;
		}
		//�����˻����Թ��˻����
		if(0==strcmp(out.c_str(),"101006"))
		{
			sendMsg="";
			sendMsg+=out.c_str();
			reStr=Json_101006_SendMsg(jValue);
			
			return reStr;
		}
		//��������������ͨ
		if(0==strcmp(out.c_str(),"970101"))
		{
			sendMsg="";
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
			reStr=Json_970101_SendMsg(jValue);
			
			return reStr;
		}
		else
		{
			//δ֪������
			reStr=RetMsg("001","δ֪������");
			return reStr;
		}		
	}
	else
	{
		GtWriteTrace(EM_TraceDebug,"error = [%s]", (reader.getFormatedErrorMessages()).c_str());
		//����ʧ��
		reStr=RetMsg("999","����ʧ��");
		return reStr;
	}
}