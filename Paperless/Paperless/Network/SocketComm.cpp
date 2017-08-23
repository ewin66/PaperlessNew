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
#pragma comment(lib,"Psapi.Lib")  //MiniDumpWriteDump链接时用到


int InitIocpService(LPVOID lpVoid)
{
	//初始化网络连接
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
	GtWriteTrace(EM_TraceDebug, "%s:%d: iocp回调函数：AcceptEvt()!", __FUNCTION__, __LINE__);
}

void __stdcall WriteEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len)
{
	GtWriteTrace(EM_TraceDebug, "%s:%d: iocp回调函数：WriteEvt()!", __FUNCTION__, __LINE__);
}

void __stdcall ExceptEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len)
{
	GtWriteTrace(EM_TraceDebug, "%s:%d: iocp回调函数：ExceptEvt()!", __FUNCTION__, __LINE__);
}

void __stdcall CloseEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len)
{
	GtWriteTrace(EM_TraceDebug, "%s:%d: iocp回调函数：CloseEvt()!", __FUNCTION__, __LINE__);
}

/**************************************************************************
*函数名：void __stdcall ReadEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len)
*函数说明：socket的读数据回调函数
*参数说明：
*buf:接收到的数据
*len：接收的数据长度
*函数返回值：DWORD
***************************************************************************/
void __stdcall ReadEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len)
{
	GtWriteTrace(EM_TraceDebug, "%s:%d: ------------------->收到新请求...", __FUNCTION__, __LINE__);
	PIO_OP_KEY pOpKey = (PIO_OP_KEY)hObject;
	buf[len] = '\0';
	GtWriteTrace(EM_TraceDebug, "%s:%d: 收到报文[%s]", __FUNCTION__, __LINE__, buf);

	// base64解码
	ZBase64 zBase;
	int msg_base64_after_len = 0;
	string strValue = zBase.Decode((const char*)buf, len, msg_base64_after_len);
	GtWriteTrace(EM_TraceDebug, "%s:%d: base64解码后[%s]", __FUNCTION__, __LINE__, strValue.c_str());

	Json::Reader reader;//json解析
	Json::Value value;//表示一个json格式的对象
	std::string msgStr_rtn;
	std::string out;
	std::string tran_type;

	CString reStr="";
	Json::Reader ret_reader;//json解析
	// 表示一个json格式的对象，返回的报文都填入这个对象中，最后返回给web端
	Json::Value msgStr_json_rtn;
	std::string ret_out;

// 	//读取配置文件
// 	GetPrivateProfileString("Information","Wname","test.txt - 记事本",wname.GetBuffer(100),100,GetAppPath()+"\\win.ini");
// 	long_time_interval=GetPrivateProfileInt("Information","DefaultInterval",10,GetAppPath()+"\\win.ini");
// 	short_time_interval=GetPrivateProfileInt("Information","EnterInputInterval",10,GetAppPath()+"\\win.ini");
// 	
// 	sendMsg=strMsg.c_str();
// 	::MessageBoxA(NULL, sendMsg, "预填单报文", MB_OK);
// 			
// 	SendToWindows();
// 	reStr="000";
// 	sockaddr_in addr;
// 	addr.sin_addr.s_addr = pOpKey->remote_addr;
// 	int resCount = WriteDataEx(g_hIoRes,pOpKey,NULL,0,reStr.GetBuffer(),reStr.GetLength());
// 	//写日志


	// 解析出json放到value中区
	if(reader.parse(strValue, value))
	{
		// 判断解析出的json数据是否未空
		if (value.size() != 0)
		{
			tran_type = value["BWKZLX"].asString();
			if (0 == tran_type.compare("0"))
			{
				// 自动填单交易
				//读取配置文件
				GetPrivateProfileString("Information","Wname","test.txt - 记事本",wname.GetBuffer(100),100,GetAppPath()+"\\win.ini");
				long_time_interval=GetPrivateProfileInt("Information","DefaultInterval",10,GetAppPath()+"\\win.ini");
				short_time_interval=GetPrivateProfileInt("Information","EnterInputInterval",10,GetAppPath()+"\\win.ini");
				reStr=JsonToSendMsg(strValue);
				if(ret_reader.parse(reStr.GetBuffer(), msgStr_json_rtn))//解析出json放到json中区
				{
					ret_out=msgStr_json_rtn["XYM"].asString();
					GtWriteTrace(EM_TraceDebug, "%s:%d: 响应码！[%s]",  __FUNCTION__, __LINE__,ret_out.c_str());
					if(0 == strcmp(ret_out.c_str(), "000"))
					{
						SendToWindows();
					}
					else
					{
						GtWriteTrace(EM_TraceDebug, "%s:%d: 自动填单预处理检查失败！响应码[%s]",  __FUNCTION__, __LINE__,ret_out.c_str());
					}
				}
				else
				{
					msgStr_json_rtn["XYM"] = "999";
					msgStr_json_rtn["XYSM"] = "解析响应报文失败";
					GtWriteTrace(EM_TraceDebug, "%s:%d: 解析响应报文失败！响应码[%s]",  __FUNCTION__, __LINE__,ret_out.c_str());
				}
			}
			else
			{
				int nRet = 0; 
				CString str = GetAppPath();
				CPaperlessDlg* pPaperlessDlg = (CPaperlessDlg*)AfxGetApp()->m_pMainWnd;
				if (0 == tran_type.compare("1"))
				{
					// 获取身份证芯片信息
					MYPERSONINFO pMyPerson;
					memset(&pMyPerson, 0, sizeof(MYPERSONINFO));
					str.Append("\\IDPicture\\HeadPictureTmp.jpg");
//#define YCP_LOCAl_TEST
#ifdef YCP_LOCAl_TEST
					nRet = 0;
					memcpy(pMyPerson.address, "福建省大田县上京镇三阳村13-1号", sizeof(pMyPerson.address));
					memcpy(pMyPerson.appendMsg, "", sizeof(pMyPerson.appendMsg));
					memcpy(pMyPerson.birthday, "19941022", sizeof(pMyPerson.birthday));
					memcpy(pMyPerson.cardId, "350425199410220517", sizeof(pMyPerson.cardId));
					memcpy(pMyPerson.cardType, "", sizeof(pMyPerson.cardType));
					memcpy(pMyPerson.EngName, "", sizeof(pMyPerson.EngName));
					memcpy(pMyPerson.govCode, "", sizeof(pMyPerson.govCode));
					pMyPerson.iFlag = 0;
					memcpy(pMyPerson.name, "叶长鹏", sizeof(pMyPerson.name));
					memcpy(pMyPerson.nation, "汉", sizeof(pMyPerson.nation));
					memcpy(pMyPerson.nationCode, "", sizeof(pMyPerson.nationCode));
					memcpy(pMyPerson.otherData, "", sizeof(pMyPerson.otherData));
					memcpy(pMyPerson.police, "大田县公安局", sizeof(pMyPerson.police));
					memcpy(pMyPerson.sex, "男", sizeof(pMyPerson.sex));
					memcpy(pMyPerson.sexCode, "", sizeof(pMyPerson.sexCode));
					memcpy(pMyPerson.validEnd, "20201221", sizeof(pMyPerson.validEnd));
					memcpy(pMyPerson.validStart, "20101221", sizeof(pMyPerson.validStart));
					memcpy(pMyPerson.version, "", sizeof(pMyPerson.version));
#else
					nRet = pPaperlessDlg->pBaseReadIDCardInfo->MyReadIDCardInfo(str.GetBuffer(), &pMyPerson);
#endif
					// 通过个人信息头像路径和返回值拼json报文
					GtWriteTrace(EM_TraceDebug, "%s:%d: 获取身份证芯片信息返回值 return = [%d] 芯片照[%s]",  __FUNCTION__, __LINE__, nRet, str.GetBuffer());
					getIDCardInfoJson(msgStr_json_rtn, str, &pMyPerson, nRet);
				}
				else if (0 == tran_type.compare("2"))
				{
					// 获取身份证正面信息
					str.Append("\\IDPicture\\FrontPictureTmp.jpg");
#ifdef YCP_LOCAl_TEST
					nRet = 0;
#else
					nRet = pPaperlessDlg->pBaseSaveCameraPic->MySaveDeskIDPic(str.GetBuffer());
#endif
					GtWriteTrace(EM_TraceDebug, "%s:%d: 获取身份证正面照返回值 return = [%d] 照片[%s]",  __FUNCTION__, __LINE__, nRet, str.GetBuffer());
					// 通过身份证正面信息返回值拼json报文
					getIDPicJson(msgStr_json_rtn, 0, str, nRet);
				}
				else if (0 == tran_type.compare("3"))
				{
					// 获取身份证反面信息
					str.Append("\\IDPicture\\BackPictureTmp.jpg");
#ifdef YCP_LOCAl_TEST
					nRet = 0;
#else
					nRet = pPaperlessDlg->pBaseSaveCameraPic->MySaveDeskIDPic(str.GetBuffer());
#endif
					GtWriteTrace(EM_TraceDebug, "%s:%d: 获取身份证反面照返回值 return = [%d] 照片[%s]",  __FUNCTION__, __LINE__, nRet, str.GetBuffer());
					// 通过身份证反面信息返回值拼json报文
					getIDPicJson(msgStr_json_rtn, 1, str, nRet);
				}
				else if (0 == tran_type.compare("4"))
				{
					// 获取 环境摄像头人像照
// 					str.Append("\\IDPicture\\EnvPictureTmp.png");
// 					nRet = pPaperlessDlg->pBaseSaveCameraPic->MySaveEnvPic(str.GetBuffer());
// 					//nRet = 0;
// 					GtWriteTrace(EM_TraceDebug, "%s:%d: 获取人像照返回值 return = [%d] 人像照[%s]",  __FUNCTION__, __LINE__, nRet, str.GetBuffer());
// 					// 通过 环境摄像头信息返回值拼json报文
// 					getIDPicJson(msgStr_json_rtn, 2, str, nRet);

					// 获取 环境摄像头人像照，返回本地人像照绝对路径
					char sFilename[256] = {0};
					strncpy(sFilename, "EnvPictureTmp.jpg", sizeof(sFilename)-1);
					str.Append("\\IDPicture\\");
					str.Append(sFilename);
#ifdef YCP_LOCAl_TEST
					nRet = 0;
#else
					nRet = pPaperlessDlg->pBaseSaveCameraPic->MySaveEnvPic(str.GetBuffer());
#endif
 					GtWriteTrace(EM_TraceDebug, "%s:%d: 获取人像照返回值 return = [%d] 人像照[%s]",  __FUNCTION__, __LINE__, nRet, str.GetBuffer());
					// 通过 环境摄像头本地路径和返回值拼json报文
					getJsonFromPersonPic(msgStr_json_rtn, str, sFilename, nRet);
				}
				else if (0 == tran_type.compare("5"))
				{
					// 获取 通过web请求的人像名称，返回人像照
					std::string sFileName = value["FILE_NAME"].asString();
					str.Append("\\IDPicture\\");
					str.Append(sFileName.c_str());
					GtWriteTrace(EM_TraceDebug, "%s:%d: 将要发送人像照[%s]",  __FUNCTION__, __LINE__, str.GetBuffer());
					// 通过 环境摄像头照片，拼json报文
					getJsonFromPic(msgStr_json_rtn, str);
				}
				else if (0 == tran_type.compare("6"))
				{
					msgStr_json_rtn["XYM"] = "000";
					msgStr_json_rtn["XYSM"] = "成功";
					msgStr_json_rtn["FILE_DIR"] = "C:\\Users\\mrxu\\Desktop\\img\\test.jpg";
					msgStr_json_rtn["OTH_MSG1"] = "";
				}
				else
				{
					// 未知的报文控制类型
					GtWriteTrace(EM_TraceDebug, "%s:%d: 未知报文控制类型! tran_type = [%s]",  __FUNCTION__, __LINE__, tran_type.c_str());
					msgStr_json_rtn["XYM"]="998";
					msgStr_json_rtn["XYSM"]="未知的报文控制类型";
				}
			}
		}
		else
		{
			GtWriteTrace(EM_TraceDebug, "%s:%d: 解析json报文无数据！value.size() == 0",  __FUNCTION__, __LINE__);
			// 解析失败
			msgStr_json_rtn["XYM"]="999";
			msgStr_json_rtn["XYSM"]="解析json报文无数据！value.size() == 0";
		}
		
	}
	else
	{
		GtWriteTrace(EM_TraceDebug, "%s:%d: 解析json报文失败! error = [%s]",  __FUNCTION__, __LINE__, (reader.getFormatedErrorMessages()).c_str());
		// 解析失败
		msgStr_json_rtn["XYM"]="999";
		msgStr_json_rtn["XYSM"]="json报文解析失败";
	}
	// 将json报文发送
	SendJsonMsg(msgStr_json_rtn, pOpKey);
	CPaperlessDlg* pPaperlessDlg1 = (CPaperlessDlg*)AfxGetApp()->m_pMainWnd;
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(pPaperlessDlg1->handle_memery, &pmc, sizeof(pmc));
	GtWriteTrace(EM_TraceInfo, "%s:%d: 内存使用:\n\t 内存使用：%dK，峰值内存使用：%dK，虚拟内存使用%dK，峰值虚拟内存使用%dK\n",
		__FUNCTION__, __LINE__, pmc.WorkingSetSize/1024, pmc.PeakWorkingSetSize/1024, pmc.PagefileUsage/1024, pmc.PeakPagefileUsage/1024);
	GtWriteTrace(EM_TraceDebug, "%s:%d: <----------------------请求处理结束！\n",  __FUNCTION__, __LINE__);
}


// 身份证识读仪获取身份证信息响应码内转外
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
	strncpy(sRetMsg, "身份证识读仪获取身份证信息失败，其他错误！", 128-1);
}


// 摄像头获取照片响应码内转外
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
	strncpy(sRetMsg, "摄像头获取照片失败，其他错误！", 128-1);
}

// 通过 PERSONINFO 结构体组json报文
void getIDCardInfoJson(Json::Value &jsonBuff, CString strDir, MYPERSONINFO *pPerson, int nRet)
{
	char pinyin[512] = {0};
	// 有效期限
	char effDate[128] = {0};
	char strFlag[8] = {0};
	// 返回信息
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
		// 获取头像照
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
			// 分配内存存储整个文件
			fileBuffer = (char*) malloc(sizeof(char) * lSize);
			if (fileBuffer == NULL)
			{
				GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDCardInfoJson() malloc failed! err=[%d][%s]", errno, strerror(errno));
				nRet = 202;
				// 关闭文件
				fclose(pFile);
				pFile = NULL;
				break;
			}
			// 将文件拷贝到fileBuffer中
			result = fread(fileBuffer, 1, lSize, pFile);
			if (result != lSize)
			{
				GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDCardInfoJson() read [%s] failed! err=[%d][%s]", strDir.GetBuffer(), errno, strerror(errno));
				nRet = 201;
				// 关闭文件，释放内存
				fclose(pFile);
				pFile = NULL;
				free(fileBuffer);
				fileBuffer = NULL;
				break;
			}
			// 读取身份证正面照文件成功，进行base64编码
			encodeBase64_pic = zBase.Encode((const unsigned char*)fileBuffer, (int)lSize);
			// 释放内存
			free(fileBuffer);
			fileBuffer = NULL;
			fclose(pFile);
			pFile = NULL;
			nRet = 0;
		}while (0);

		// 返回码转换
		ReadIDCardCodeTrans(nRet, sRetCode, sRetMsg);
		// 获取姓名拼音
		MyGetPinYin(pPerson->name, pinyin);
		// 拼身份证有效期限
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
		// 返回码转换
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


/* 功能：通过身份证正反面照组返回报文
 * 入参：flag 0-正面 1-反面
 *		strDir 照片本地路径
 *		nRet 调本函数之前，保存身份图片是否成功
 * 出参：jsonBuff：返回报文 
*/ 
void getIDPicJson(Json::Value &jsonBuff, int flag, CString strDir, int nRet)
{
	// 获取身份证正面照
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
	// 判断 高拍仪获取图片是否成功，不成功返回失败
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
			// 分配内存存储整个文件
			fileBuffer = (char*) malloc(sizeof(char) * lSize);
			if (fileBuffer == NULL)
			{
				GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDPicJson() malloc failed! err=[%d][%s]", errno, strerror(errno));
				nRet = 103;
				jsonBuff[PIC_FLAG] = "";
				// 关闭文件
				fclose(pFile);
				pFile = NULL;
				break;
			}
			// 将文件拷贝到fileBuffer中
			result = fread(fileBuffer, 1, lSize, pFile);
			if (result != lSize)
			{
				GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDPicJson() read [%s] failed! err=[%d][%s]", strDir.GetBuffer(), errno, strerror(errno));
				nRet = 102;
				jsonBuff[PIC_FLAG] = "";
				// 关闭文件，释放内存
				fclose(pFile);
				pFile = NULL;
				free(fileBuffer);
				fileBuffer = NULL;
				break;
			}
			// 读取身份证正面照文件成功，进行base64编码
			encodeBase64_pic = zBase.Encode((const unsigned char*)fileBuffer, (int)lSize);
			// 释放内存
			free(fileBuffer);
			fileBuffer = NULL;
			fclose(pFile);
			pFile = NULL;
			jsonBuff[PIC_FLAG] = encodeBase64_pic.c_str();
			//GtWriteTrace(EM_TraceDebug, "[MainFrm]file buff=[%s]", encodeBase64_pic.c_str());
		} while (0);
		// 返回码转换
		ReadPicCodeTrans(nRet, sRetCode, sRetMsg);
	}
	else
	{
		// 返回码转换
		ReadPicCodeTrans(nRet, sRetCode, sRetMsg);
		jsonBuff[PIC_FLAG] = "";
	}
	jsonBuff["XYM"] = sRetCode;
	jsonBuff["XYSM"] = sRetMsg;
	jsonBuff["OTH_MSG1"] = "";
	return ;
}


/* 功能：通过人像照片组返回报文
 * 入参：strDir 照片本地路径
 *		pFilename 照片名称
 *		nRet 调本函数之前，保存人像照是否成功
 *		jsonBuff：返回报文 
*/ 
void getJsonFromPersonPic(Json::Value &jsonBuff, CString strDir, char *pFilename, int nRet)
{
	// 获取身份证正面照
	FILE * pFile= NULL;
	char *fileBuffer = NULL;
	long lSize = 0;
	ZBase64 zBase;
	string encodeBase64_pic;
	size_t result = 0;
	char sRetCode[4+1] = {0};
	char sRetMsg[128+1] = {0};
	char PIC_FLAG[16] = {0};
	// 判断 高拍仪获取图片是否成功，不成功返回失败
	if (nRet == 0)
	{
		jsonBuff["FILE_DIR"] = strDir.GetBuffer();
	}
	else
	{
		jsonBuff["FILE_DIR"] = "";
	}
	// 响应码转换
	ReadPicCodeTrans(nRet, sRetCode, sRetMsg);
	jsonBuff["XYM"] = sRetCode;
	jsonBuff["XYSM"] = sRetMsg;
	jsonBuff["FILE_NAME"] = pFilename;
	jsonBuff["OTH_MSG1"] = "";
	return ;
}


/* 功能：通过人像照片组返回报文
 * 入参：strDir 照片本地绝对路径
 *		jsonBuff：返回报文 
*/ 
void getJsonFromPic(Json::Value &jsonBuff, CString strDir)
{
	// 获取身份证正面照
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
		// 分配内存存储整个文件
		fileBuffer = (char*) malloc(sizeof(char) * lSize);
		if (fileBuffer == NULL)
		{
			GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDPicJson() malloc failed! err=[%d][%s]", errno, strerror(errno));
			nRet = 103;
			jsonBuff["LIVE_PIC"] = "";
			// 关闭文件
			fclose(pFile);
			pFile = NULL;
			break;
		}
		// 将文件拷贝到fileBuffer中
		result = fread(fileBuffer, 1, lSize, pFile);
		if (result != lSize)
		{
			GtWriteTrace(EM_TraceDebug, "[MainFrm]getIDPicJson() read [%s] failed! err=[%d][%s]", strDir.GetBuffer(), errno, strerror(errno));
			nRet = 102;
			jsonBuff["LIVE_PIC"] = "";
			// 关闭文件，释放内存
			fclose(pFile);
			pFile = NULL;
			free(fileBuffer);
			fileBuffer = NULL;
			break;
		}
		// 读取身份证正面照文件成功，进行base64编码
		encodeBase64_pic = zBase.Encode((const unsigned char*)fileBuffer, (int)lSize);
		// 释放内存
		free(fileBuffer);
		fileBuffer = NULL;
		fclose(pFile);
		pFile = NULL;
		jsonBuff["LIVE_PIC"] = encodeBase64_pic.c_str();
		//GtWriteTrace(EM_TraceDebug, "[MainFrm]file buff=[%s]", encodeBase64_pic.c_str());
	} while (0);
	// 返回码转换
	ReadPicCodeTrans(nRet, sRetCode, sRetMsg);
	jsonBuff["XYM"] = sRetCode;
	jsonBuff["XYSM"] = sRetMsg;
	jsonBuff["OTH_MSG1"] = "";
	return ;
}


// 返回json数据报文
void SendJsonMsg(Json::Value &jsonBuff, PIO_OP_KEY pOpKey)
{
	sockaddr_in addr;
	addr.sin_addr.s_addr = pOpKey->remote_addr;

	// json数据转成string格式
	string msgStr_rtn_gbk = jsonBuff.toStyledString();

	// 转码 GBKToUtf8
	GtWriteTrace(EM_TraceDebug, "%s:%d: 转码前长度(GBK)=[%d]!", __FUNCTION__, __LINE__, msgStr_rtn_gbk.length());
	//GtWriteTrace(EM_TraceDebug, "%s:%d: 转码前数据(GBK)=[%s]!", __FUNCTION__, __LINE__, msgStr_rtn_gbk.c_str());
	string msgStr_rtn = MyGBKToUtf8(msgStr_rtn_gbk);
	//string msgStr_rtn = msgStr_rtn_gbk;
	GtWriteTrace(EM_TraceDebug, "%s:%d: 转码后长度(UTF-8)=[%d]!", __FUNCTION__, __LINE__, msgStr_rtn.length());

	// 发送报文，完整报文=10字节报文长度+报文体；因网络库限制，故长报文需要循环调用发送函数
	// 报文头长度，10字节
	char sPreBuff[10+1] = {0};
	// 报文总字节长度
	long countLen = msgStr_rtn.length();

	// 单次发送最大报文字节长度，64 * 1024 socket网络库(iocp.h)中最大为64 * 1024字节
	const int SEND_MAX_LEN = 32 * 1024;
	// 需要发送的次数
	int count = (countLen - 1) / SEND_MAX_LEN + 1;
	// 本次发送的字节数
	int sendLen = 0;
	// 待发送的报文指针
	char *sendBuff = (char *)msgStr_rtn.c_str();
	// 发送函数返回值
	int resCount = 0;

	sprintf(sPreBuff, "%010ld", countLen);
	// 发送报文头
	GtWriteTrace(EM_TraceDebug, "%s:%d: 发送报文头=[%s]!", __FUNCTION__, __LINE__, sPreBuff);
	WriteDataEx(g_hIoRes, pOpKey, NULL, 0, sPreBuff, sizeof(sPreBuff)-1);
	GtWriteTrace(EM_TraceDebug, "%s:%d: 待发送报文长度=[%d], 分%d次发送!", __FUNCTION__, __LINE__, countLen, count);

	// 循环发送报文，每次发送SEND_MAX_LEN字节
	for (int i = 0; i < count; i++)
	{
		// 获取本次发送字节数，最后一次发送 countLen % SEND_MAX_LEN 字节
		sendLen = (i == count - 1) ? countLen % SEND_MAX_LEN : SEND_MAX_LEN;
		resCount = WriteDataEx(g_hIoRes, pOpKey, NULL, 0, sendBuff + i * SEND_MAX_LEN, sendLen);
		if (resCount < 0)
		{
			GtWriteTrace(EM_TraceDebug, "%s:%d: 第%d次发送报文失败，函数WriteDataEx()返回值[%d] < 0，停止发送报文！", __FUNCTION__, __LINE__, i + 1, resCount);
			break;
		}
		if (resCount == 0)
		{
			Sleep(10);
		}
		GtWriteTrace(EM_TraceDebug, "%s:%d: 第%d次发送报文完成，本次发送[%d]字节!", __FUNCTION__, __LINE__, i + 1, sendLen);
		if (i == count - 1)
		{
			GtWriteTrace(EM_TraceDebug, "%s:%d: 报文全部发送完成，共[%d]字节!", __FUNCTION__, __LINE__, countLen);
		}
	}
	return ;
}


CString RetMsg(string xym,string xynr)
{
	CString ret_str="";
	Json::Value msgStr_json_rtn;//表示一个json格式的对象 
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
	Json::Reader reader;//json解析
	std::string out="";
	CString reStr="";
	out=value["XM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//姓名为空
		reStr=RetMsg("001","姓名是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		//不能包含数字
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
			reStr=RetMsg("018","姓名(英文/拼音):不能包含数字");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZJSFCQYX"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//证件是否长期有效为空
		reStr=RetMsg("002","证件是否长期有效是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(out!="1" && out!="2")
		{
			//证件到期日为空
			reStr=RetMsg("003","证件是否长期有效:输入数据不存在");
			//写日志
			return reStr;
		}
		sendMsg+=out.c_str();
		
		if(out=="2")
		{
			out=value["ZJDQR"].asString();
			if(out.empty()!=0) //true 1 false 0
			{
				//证件到期日为空
				reStr=RetMsg("003","证件到期日是必输项，不可为空");
				//写日志
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
		//发证机关地区代码为空
		reStr=RetMsg("004","发证机关地区代码是必输项，不可为空");
		//写日志
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
		//性别为空
		reStr=RetMsg("005","性别是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(out!="1" && out!="2")
		{
			reStr=RetMsg("004","性别：输入数据不存在");
			return reStr;
		}
		sendMsg+=out.c_str();
	}
	out=value["GJ"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//国籍为空
		reStr=RetMsg("006","国籍是必输项，不可为空");
		//写日志
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
			//移动电话为空
			reStr=RetMsg("007","固定电话跟移动电话必输其中一项");
			//写日志
			return reStr;
		}
		else
		{
			if(out.length()!=11)
			{
				reStr=RetMsg("007","移动电话:清输入11位长度");
				//写日志
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
			//固定电话不为空，移动电话为空
			sendMsg+="\r\n";
		}
		else
		{
			if(out.length()!=11)
			{
				reStr=RetMsg("007","移动电话:清输入11位长度");
				//写日志
				return reStr;
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
	}
	
	out=value["TXDZ"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//通讯地址为空
		reStr=RetMsg("008","通讯地址是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(out.length()<4)
		{
			reStr=RetMsg("012","通讯地址长度至少为4个字符(1个汉字为2个字符)");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["YZBM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//邮政编码不是必输项
		sendMsg+="\r\n";
	}
	else
	{
		if(out.length()!=6)
		{
			reStr=RetMsg("012","邮政编码为6位数字");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZY"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//职业为空
		reStr=RetMsg("009","职业是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(atoi(out.c_str())<1 || atoi(out.c_str())>8)
		{
			reStr=RetMsg("009","职业输入数据不存在");
			//写日志
			return reStr;
		}
		sendMsg+=out.c_str();
	}
	reStr=RetMsg("000","发送成功");
	return reStr;
}

CString Json_010101_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json解析
	std::string out="";
	std::string zjlx="";
	sendMsg+=out.c_str();
	CString reStr="";
	out=value["HM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//户名为空
		reStr=RetMsg("002","户名是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		GtWriteTrace(EM_TraceDebug,"%s",sendMsg);
		if(out.length()<2)
		{
			reStr=RetMsg("012","户名长度至少为2个字符(1个汉字为2个字符)");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		GtWriteTrace(EM_TraceDebug,"%s",sendMsg);
	}
	zjlx=value["ZJLX"].asString();
	if(zjlx.empty()!=0) //true 1 false 0
	{
		//证件类型为空
		reStr=RetMsg("013","证件类型是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(ZJLX.find(zjlx)==-1)
		{
			reStr=RetMsg("014","证件类型不存在");
			return reStr;
		}
		sendMsg+=zjlx.c_str();
	}
	out=value["ZJHM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//证件号码为空
		reStr=RetMsg("015","证件号码是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(zjlx=="10")
		{
			if(out.length()!=18)
			{
				reStr=RetMsg("016","身份证只能18位");
				return reStr;
			}
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+="\r\n";
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	Json::Reader tmp_reader;//json解析
	Json::Value tmp_json_rtn;//表示一个json格式的对象 
	std::string tmp_out;
	reStr=Json_060104_SendMsg(value);
	if(tmp_reader.parse(reStr.GetBuffer(), tmp_json_rtn))//解析出json放到json中区
	{
		tmp_out=tmp_json_rtn["XYM"].asString();
		GtWriteTrace(EM_TraceDebug, "响应码=[%s]", tmp_out.c_str());
		if(0 != strcmp(tmp_out.c_str(), "000"))
		{
			return reStr;
		}
	}
	else
	{
		reStr=RetMsg("999","解析失败");
		return reStr;
	}
	return reStr;
}

CString Json_101003_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json解析
	std::string out="";
	std::string zjlx="";
	sendMsg+=out.c_str();
	CString reStr="";

	out=value["ZFZFF"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//资费支付方为空
		reStr=RetMsg("017","资费支付方是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(out!="1"&&out!="2")
		{
			reStr=RetMsg("017","资费支付方：输入数据不存在");
			//写日志
			return reStr;
		}
		sendMsg+=out.c_str();
	}
	out=value["SKRXM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//收款人姓名为空
		reStr=RetMsg("018","收款人姓名是必输项，不可为空");
		//写日志
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
		//收款人账号/卡号为空
		reStr=RetMsg("019","收款人账号/卡号是必输项，不可为空");
		//写日志
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
		//汇款金额为空
		reStr=RetMsg("020","汇款金额是必输项，不可为空");
		//写日志
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
		//转账类型为空
		reStr=RetMsg("021","转账类型是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(atoi(out.c_str())<1||atoi(out.c_str())>3)
		{
			reStr=RetMsg("021","转账类型:输入数据不存在");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["HKRXM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//汇款人姓名为空
		reStr=RetMsg("022","汇款人姓名是必输项，不可为空");
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
		//汇款人地址/电话为空
		reStr=RetMsg("023","汇款人地址/电话是必输项，不可为空");
		//写日志
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
		//跳过是否输入辅助证件
		sendMsg+="\r\n";
	}
	else
	{
		if(ZJLX.find(zjlx)==-1)
		{
			reStr=RetMsg("014","汇款人证件类型不存在");
			return reStr;
		}
		sendMsg+=zjlx.c_str();
		out=value["HKRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 
		{
			//姓名为空
			reStr=RetMsg("024","汇款人证件号码是必输项，不可为空");
			//写日志
			return reStr;
		}
		else
		{
			if(zjlx=="10")
			{
				if(out.length()!=18)
				{
					reStr=RetMsg("016","身份证只能18位");
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
		//新老客户标志为空
		reStr=RetMsg("025","新老客户标志是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(0==strcmp(out.c_str(),"0"))
		{
			Json::Reader tmp_reader;//json解析
			Json::Value tmp_json_rtn;//表示一个json格式的对象 
			std::string tmp_out;
			//新客户
			reStr=Json_060104_SendMsg(value);
			GtWriteTrace(EM_TraceDebug, "客户录入返回json=[%s]", reStr.GetBuffer());
			if(tmp_reader.parse(reStr.GetBuffer(), tmp_json_rtn))//解析出json放到json中区
			{
				tmp_out=tmp_json_rtn["XYM"].asString();
				GtWriteTrace(EM_TraceDebug, "响应码=[%s]", tmp_out.c_str());
				if(0 != strcmp(tmp_out.c_str(), "000"))
				{
					return reStr;
				}
			}
			else
			{
				reStr=RetMsg("999","解析失败");
				return reStr;
			}
		}
		else
		{
			//老客户
			/*if(::MessageBox(NULL,"老客户客户录入是否覆盖","提示",MB_OK|MB_OKCANCEL)==IDOK)
			{
				reStr=Json_060104_SendMsg(value);
				return reStr;
			}*/
		}
	}
	reStr=RetMsg("000","发送成功");
	return reStr;
}

CString Json_101004_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json解析
	std::string out="";
	std::string zjlx="";
	sendMsg+=out.c_str();
	CString reStr="";	

	out=value["ZFZFF"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//资费支付方为空
		reStr=RetMsg("026","资费支付方是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(out!="1"&&out!="2")
		{
			reStr=RetMsg("017","资费支付方：输入数据不存在");
			//写日志
			return reStr;
		}
		sendMsg+=out.c_str();
	}
	out=value["SKRXM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//收款人姓名为空
		reStr=RetMsg("027","收款人姓名是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(out.length()<4)
		{
			reStr=RetMsg("027","收款人姓名:长度小于4位，请重输");
			//写日志
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["SKRZKH"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//收款人账号/卡号为空
		reStr=RetMsg("028","收款人账号/卡号是必输项，不可为空");
		//写日志
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
		//汇款金额为空
		reStr=RetMsg("029","汇款金额是必输项，不可为空");
		//写日志
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
		//转账类型为空
		reStr=RetMsg("030","转账类型是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(atoi(out.c_str())<1||atoi(out.c_str())>3)
		{
			reStr=RetMsg("021","转账类型:输入数据不存在");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}
	out=value["ZHMMBZ"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//账户密码标志为空
		reStr=RetMsg("032","账户密码标志是必输项，不可为空");
		//写日志
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
			reStr=RetMsg("032","账户密码标志输入值有误");
			//写日志
			return reStr;
		}
		
	}
	zjlx=value["HKRZJLX"].asString();
	if(zjlx.empty()!=0) //true 1 false 0
	{
		//汇款人证件类型为空，直接跳到是否输入辅助证件
		sendMsg+="\r\n";
	}
	else
	{
		if(ZJLX.find(zjlx)==-1)
		{
			reStr=RetMsg("014","汇款人证件类型不存在");
			return reStr;
		}
		sendMsg+=zjlx.c_str();
		out=value["HKRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//汇款人证件号码为空
			reStr=RetMsg("033","汇款人证件号码是必输项，不可为空");
			//写日志
			return reStr;
		}
		else
		{
			if(zjlx=="10")
			{
				if(out.length()!=18 && out.length()!=15)
				{
					reStr=RetMsg("016","身份证只能15位或18位");
					return reStr;
				}
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
	}
	//跳过是否输入辅助证件
	sendMsg+="\r\n";
	out=value["SFDLRDB"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//是否代理人代办为空,默认为2-否
		sendMsg+="\r\n";
	}
	else
	{
		if(out!="1"&&out!="2")
		{
			reStr=RetMsg("016","是否代理人代办输入数据有误");
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
				//姓名为空
				reStr=RetMsg("034","姓名是必输项，不可为空");
				return reStr;
			}
			else 
			{
				if(out.length()<2)
				{
					reStr=RetMsg("034","代理人姓名：长度至少为2个字符(1个汉字为2个字符)");
					return reStr;
				}
				GtWriteTrace(EM_TraceDebug,"%s",out.c_str());
				//不能全由数字组成
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
						//GtWriteTrace(EM_TraceDebug,"代理人姓名:不能全由数字组成");
					reStr=RetMsg("018","代理人姓名:不能全由数字组成");
					return reStr;
				}
				sendMsg+=out.c_str();
				sendMsg+="\r\n";
			}
			zjlx=value["DLRZJLX"].asString();
			if(zjlx.empty()!=0) //true 1 false 0
			{
				//证件类型为空
				reStr=RetMsg("035","证件类型是必输项，不可为空");
				return reStr;
			}
			else 
			{
				if(ZJLX.find(zjlx)==-1)
				{
					reStr=RetMsg("014","汇款人证件类型不存在");
					return reStr;
				}
				sendMsg+=zjlx.c_str();
			}
			out=value["DLRZJHM"].asString();
			if(out.empty()!=0) //true 1 false 0
			{
				//证件号码为空
				reStr=RetMsg("036","证件号码是必输项，不可为空");
				return reStr;
			}
			else 
			{
				if(zjlx=="10")
				{
					if(out.length()!=18)
					{
						reStr=RetMsg("016","身份证只能18位");
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
				//联系电话为空
				reStr=RetMsg("037","联系电话是必输项，不可为空");
				return reStr;
			}
			else 
			{
				if(out.length()<9)
				{
					reStr=RetMsg("016","联系电话：请输入至少9位字符(含区号)");
					return reStr;
				}
				sendMsg+=out.c_str();
			}
		}
	}
	reStr=RetMsg("000","发送成功");
	return reStr;
}

CString Json_101005_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json解析
	std::string out="";
	std::string zjlx="";
	std::string hkfs="";
	std::string zfzff="";
	sendMsg+=out.c_str();
	CString reStr="";

	hkfs=value["HKFS"].asString();
	if(hkfs.empty()!=0) //true 1 false 0
	{
		//汇款方式为空
		reStr=RetMsg("014","汇款方式是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(hkfs!="1"&&hkfs!="2")
		{
			reStr=RetMsg("017","汇款方式：输入数据不存在");
			//写日志
			return reStr;
		}
		sendMsg+=hkfs.c_str();
	}
	zfzff=value["ZFZFF"].asString();
	if(zfzff.empty()!=0) //true 1 false 0
	{
		//资费支付方为空
		reStr=RetMsg("014","资费支付方是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(zfzff!="1"&&zfzff!="2")
		{
			reStr=RetMsg("017","资费支付方：输入数据不存在");
			//写日志
			return reStr;
		}
		sendMsg+=zfzff.c_str();
	}
	out=value["SKDWMC"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//收款单位名称为空
		reStr=RetMsg("015","收款单位名称是必输项，不可为空");
		//写日志
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
		//收款单位账号为空
		reStr=RetMsg("016","收款单位账号是必输项，不可为空");
		//写日志
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
		//汇款金额为空
		reStr=RetMsg("017","汇款金额是必输项，不可为空");
		//写日志
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
		//转账类型为空
		reStr=RetMsg("018","转账类型是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(atoi(out.c_str())<1||atoi(out.c_str())>3)
		{
			reStr=RetMsg("021","转账类型:输入数据不存在");
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
			//支票入账流水号为空
			reStr=RetMsg("018","支票入账流水号是必输项，不可为空");
			//写日志
			return reStr;
		}
		else
		{
			if(out.length()!=17)
			{
				//支票入账流水号长度错误
				reStr=RetMsg("018","支票入账流水号:请输入17位数字或字母");
				return reStr;
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}
		out=value["RZRQ"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//入账日期为空
			reStr=RetMsg("018","入账日期是必输项，不可为空");
			//写日志
			return reStr;
		}
		else
		{
			//入账日期不能大于交易日期
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
		}*/
		out=value["ZPLSHBZ"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//支票流水号标志为空
			reStr=RetMsg("018","支票流水号标志是必输项，不可为空");
			return reStr;
		}
		else
		{
			if(out!="ZPLSH_BZ")
			{
				//支票流水号标志为空
				reStr=RetMsg("018","支票流水号标志输入有误");
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
			//协议号为空
			reStr=RetMsg("018","协议号是必输项，不可为空");
			//写日志
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
		//汇款人姓名为空
		reStr=RetMsg("018","汇款人姓名是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(out.length()<2)
		{
			reStr=RetMsg("018","汇款人姓名:长度至少为2个字符(1个汉字两个字符)");
			//写日志
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
	}

	out=value["HKRDZDH"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//汇款人地址/电话为空
		reStr=RetMsg("04","汇款人地址/电话是必输项，不可为空");
		//写日志
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
		//跳过是否输入辅助证件
		sendMsg+="\r\n";
	}
	else
	{
		if(ZJLX.find(zjlx)==-1)
		{
			reStr=RetMsg("014","汇款人证件类型不存在");
			return reStr;
		}
		sendMsg+=zjlx.c_str();
		sendMsg+="\r\n";
		out=value["HKRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 
		{
			//姓名为空
			reStr=RetMsg("05","汇款人证件号码是必输项，不可为空");
			//写日志
			return reStr;
		}
		else
		{
			if(zjlx=="10")
			{
				if(out.length()!=18 && out.length()!=15)
				{
					reStr=RetMsg("016","身份证只能15位或18位");
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
		//新老客户标志为空
		reStr=RetMsg("05","新老客户标志是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(0==strcmp(out.c_str(),"0"))
		{
			Json::Reader tmp_reader;//json解析
			Json::Value tmp_json_rtn;//表示一个json格式的对象 
			std::string tmp_out;
			//新客户
			reStr=Json_060104_SendMsg(value);
			if(tmp_reader.parse(reStr.GetBuffer(), tmp_json_rtn))//解析出json放到json中区
			{
				tmp_out=tmp_json_rtn["XYM"].asString();
				GtWriteTrace(EM_TraceDebug, "响应码=[%s]", tmp_out.c_str());
				if(0 != strcmp(tmp_out.c_str(), "000"))
				{
					return reStr;
				}
			}
			else
			{
				reStr=RetMsg("999","解析失败");
				return reStr;
			}
			sendMsg+='&';
		}
		else
		{
			//老客户
			/*if(::MessageBox(NULL,"老客户客户录入是否覆盖","提示",MB_OK|MB_OKCANCEL)==IDOK)
			{
				reStr=Json_060104_SendMsg(value);
				return reStr;
			}*/
			sendMsg+='&';
		}
	}
	//跳过是否输入辅助证件
	sendMsg+="\r\n";

	out=value["DLRXM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//是否代理人代办为空,默认为2-否
		sendMsg+="\r\n";
	}
	else
	{
		if(out.length()<2)
		{
			reStr=RetMsg("025","代理人姓名：长度至少为2个字符(1个汉字两个字符)");
			return reStr;
		}
		//不能全由数字组成
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
				//GtWriteTrace(EM_TraceDebug,"代理人姓名:不能全由数字组成");
			reStr=RetMsg("018","代理人姓名:不能全由数字组成");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+="\r\n";
		zjlx=value["DLRZJLX"].asString();
		if(zjlx.empty()!=0) //true 1 false 0
		{
			//证件类型为空
			reStr=RetMsg("025","证件类型是必输项，不可为空");
			return reStr;
		}
		else 
		{
			if(ZJLX.find(zjlx)==-1)
			{
				reStr=RetMsg("014","汇款人证件类型不存在");
				return reStr;
			}
			sendMsg+=zjlx.c_str();
		}
		out=value["DLRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//证件号码为空
			reStr=RetMsg("026","证件号码是必输项，不可为空");
			return reStr;
		}
		else 
		{
			if(zjlx=="10")
			{
				if(out.length()!=18 && out.length()!=15)
				{
					reStr=RetMsg("016","身份证只能为18位");
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
			//联系电话为空
			reStr=RetMsg("027","联系电话是必输项，不可为空");
			return reStr;
		}
		else 
		{
			if(out.length()<9)
			{
				reStr=RetMsg("027","联系电话:请输入至少9位字符(含区号)");
				return reStr;
			}
			sendMsg+=out.c_str();
		}
	}

	reStr=RetMsg("000","发送成功");
	return reStr;
}

CString Json_101006_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json解析
	std::string out="";
	std::string zjlx="";
	sendMsg+=out.c_str();
	std::string zfzff="";
	CString reStr="";	

	zfzff=value["ZFZFF"].asString();
	if(zfzff.empty()!=0) //true 1 false 0
	{
		//资费支付方为空
		reStr=RetMsg("014","资费支付方是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(zfzff!="1"&&zfzff!="2")
		{
			reStr=RetMsg("017","资费支付方：输入数据不存在");
			//写日志
			return reStr;
		}
		sendMsg+=zfzff.c_str();
		sendMsg+="\r\n";
	}
	out=value["HCZHKHBZ"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//汇出账户卡号标志为空
		reStr=RetMsg("015","汇出账户卡号标志是必输项，不可为空");
		//写日志
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
			reStr=RetMsg("017","汇出账户卡号标志输入有误");
			//写日志
			return reStr;
		}
	}
	out=value["SKDWMC"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//收款单位名称为空
		reStr=RetMsg("016","收款单位名称是必输项，不可为空");
		//写日志
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
		//收款单位账号为空
		reStr=RetMsg("017","收款单位账号是必输项，不可为空");
		//写日志
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
		//汇款金额为空
		reStr=RetMsg("017","汇款金额是必输项，不可为空");
		//写日志
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
		//转账类型为空
		reStr=RetMsg("018","转账类型是必输项，不可为空");
		//写日志
		return reStr;
	}
	else
	{
		if(atoi(out.c_str())<1||atoi(out.c_str())>3)
		{
			reStr=RetMsg("021","转账类型:输入数据不存在");
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
			//协议号为空
			reStr=RetMsg("018","协议号是必输项，不可为空");
			//写日志
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
		//汇款人证件类型为空，直接跳到是否输入辅助证件
		sendMsg+="\r\n";
		sendMsg+='&';
	}
	else
	{
		if(ZJLX.find(zjlx)==-1)
		{
			reStr=RetMsg("014","汇款人证件类型不存在");
			return reStr;
		}
		sendMsg+=zjlx.c_str();
		out=value["HKRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//汇款人证件号码为空
			reStr=RetMsg("023","汇款人证件号码是必输项，不可为空");
			//写日志
			return reStr;
		}
		else
		{
			if(zjlx=="10")
			{
				if(out.length()!=18 && out.length()!=15)
				{
					reStr=RetMsg("016","身份证只能15位或18位");
					return reStr;
				}
			}
			sendMsg+=out.c_str();
			sendMsg+="\r\n";
			sendMsg+='&';
			//GtWriteTrace(EM_TraceDebug,"%s",sendMsg);
		}
	}
	//跳过是否输入辅助证件
	sendMsg+="\r\n";
/*	out=value["SFSRFZZJ"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//是否输入辅助证件为空,默认为2-否
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
			//是否输入辅助证件为空,默认为2-否
			sendMsg+="\r\n";
			sendMsg+='&';
		}
	}
	*/
	out=value["DLRXM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//是否代理人代办为空,默认为2-否
		sendMsg+="\r\n";
	}
	else
	{
		if(out.length()<2)
		{
			reStr=RetMsg("025","代理人姓名：长度至少为2个字符(1个汉字两个字符)");
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
				//GtWriteTrace(EM_TraceDebug,"代理人姓名:不能全由数字组成");
			reStr=RetMsg("018","代理人姓名:不能全由数字组成");
			return reStr;
		}
		sendMsg+=out.c_str();
		sendMsg+="\r\n";
		sendMsg+="\r\n";
		zjlx=value["DLRZJLX"].asString();
		if(zjlx.empty()!=0) //true 1 false 0
		{
			//证件类型为空
			reStr=RetMsg("025","证件类型是必输项，不可为空");
			return reStr;
		}
		else 
		{
			if(ZJLX.find(zjlx)==-1)
			{
				reStr=RetMsg("014","汇款人证件类型不存在");
				return reStr;
			}
			sendMsg+=zjlx.c_str();
		}
		out=value["DLRZJHM"].asString();
		if(out.empty()!=0) //true 1 false 0
		{
			//证件号码为空
			reStr=RetMsg("026","证件号码是必输项，不可为空");
			return reStr;
		}
		else 
		{
			if(zjlx=="10")
			{
				if(out.length()!=18 && out.length()!=15)
				{
					reStr=RetMsg("016","身份证只能为18位");
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
			//联系电话为空
			reStr=RetMsg("027","联系电话是必输项，不可为空");
			return reStr;
		}
		else 
		{
			if(out.length()<9)
			{
				reStr=RetMsg("027","联系电话:请输入至少9位字符(含区号)");
				return reStr;
			}
			sendMsg+=out.c_str();
		}
	}
	reStr=RetMsg("000","发送成功");
	return reStr;
}

CString Json_970101_SendMsg(Json::Value &value)
{
	Json::Reader reader;//json解析
	std::string out="";
	sendMsg+=out.c_str();
	CString reStr="";	

	out=value["XM"].asString();
	if(out.empty()!=0) //true 1 false 0
	{
		//姓名为空
		reStr=RetMsg("014","姓名是必输项，不可为空");
		//写日志
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
		//证件类型为空
		reStr=RetMsg("015","证件类型是必输项，不可为空");
		//写日志
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
		//证件号码为空
		reStr=RetMsg("016","证件号码是必输项，不可为空");
		//写日志
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
	reStr=RetMsg("000","发送成功");
	return reStr;
}
int SendToWindows()
{
	// 自动填单交易进行处理
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
		//获取窗口句柄
		if(wname.GetBuffer() == "")
		{
			::MessageBox(NULL,"目标窗口未配置","提示",MB_OK);
			return -1;
		}

		hWnd = ::FindWindow(NULL,wname.GetBuffer());
		if(NULL==hWnd){
			::MessageBox(NULL,"没有找到窗口","提示",MB_OK);
			return -1;
		}

		//目标窗口线程与当前线程关联
		DWORD curtid = GetCurrentThreadId();
		DWORD tid = GetWindowThreadProcessId(hWnd, NULL);
		DWORD error = 0;
	/**关联两个线程，使目标线程与当前线程共用消息队列，这步很关键，
		成功后可设置目标线程窗口为活动窗口，并获取焦点位置的句柄.*/
		hCurFocus = NULL;
		//目标窗口置顶
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
			CString errStr = "关联线程失败" + error;
			::MessageBox(NULL,errStr,"提示",MB_OK);
			return -1;
		}

	
		//目标窗口置顶
		//::SetWindowPos(hWnd,HWND_TOPMOST,20,20,820,420,SWP_SHOWWINDOW);
		//发送跨线程消息
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

		//结束线程关联
		if(!AttachThreadInput(tid, curtid, false))
		{
			::MessageBox(NULL,"取消线程关联失败！","提示",MB_OK);
			return -1;
		}
		//取消置顶
		//::SetWindowPos(hWnd,HWND_NOTOPMOST,100,100,900,500,SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
	//	::SetWindowPos(hWnd,HWND_NOTOPMOST,100,100,900,500,SWP_SHOWWINDOW);
		return 0;
}

/*
	JSON转换成发送给第三方系统的字符串报文
	CString JsonToSendMsg(string str,CString &sendMsg)
	输入参数：
		str:收到的报文
		sendMsg:返回的字符串报文
	输出参数：
		返回的CString类型的json应答报文
*/
CString JsonToSendMsg(string str)
{
	Json::Reader reader;//json解析
	Json::Value jValue;//表示一个json格式的对象
	std::string out="";
	CString reStr="";
	if(reader.parse(str,jValue))//解析出json放到json中区
	{
		out=jValue["JYDM"].asString();
		GtWriteTrace(EM_TraceDebug,"%s",out.c_str());
		//活期开户
		if(0==strcmp(out.c_str(),"010101"))
		{
			sendMsg="";
			sendMsg+=out.c_str();
			reStr=Json_010101_SendMsg(jValue);
			return reStr;
		}
		//个人现金到账户汇款
		if(0==strcmp(out.c_str(),"101003"))
		{
			sendMsg="";
			sendMsg+=out.c_str();
			reStr=Json_101003_SendMsg(jValue);
			
			return reStr;
		}
		//个人账户到账户汇款
		if(0==strcmp(out.c_str(),"101004"))
		{
			sendMsg="";
			sendMsg+=out.c_str();
			reStr=Json_101004_SendMsg(jValue);
			
			return reStr;
		}
		//个人现金/支票到对公账户汇款
		if(0==strcmp(out.c_str(),"101005"))
		{
			sendMsg="";
			sendMsg+=out.c_str();
			reStr=Json_101005_SendMsg(jValue);
			
			return reStr;
		}
		//个人账户到对公账户汇款
		if(0==strcmp(out.c_str(),"101006"))
		{
			sendMsg="";
			sendMsg+=out.c_str();
			reStr=Json_101006_SendMsg(jValue);
			
			return reStr;
		}
		//电子银行渠道开通
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
			//未知交易码
			reStr=RetMsg("001","未知交易码");
			return reStr;
		}		
	}
	else
	{
		GtWriteTrace(EM_TraceDebug,"error = [%s]", (reader.getFormatedErrorMessages()).c_str());
		//解析失败
		reStr=RetMsg("999","解析失败");
		return reStr;
	}
}