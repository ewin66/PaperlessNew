#include "CCentOneReader.h"
#include "MyTTrace.h"
#include "utils.h"


// 重写，获取身份证芯片信息
int CCentOneReader::MyReadIDCardInfo(const char *pSaveHeadPicFilenm, MYPERSONINFO *pPersonInfo)
{
	GtWriteTrace(30, "%s:%d: 进入获取身份证芯片信息函数!", __FUNCTION__, __LINE__);
	if (pSaveHeadPicFilenm == NULL)
	{
		return -1;
	}
	// 升腾身份证识读信息结构体
	PERSONINFO pPerson;
	memset( &pPerson, 0, sizeof(PERSONINFO) );
	int nRet = 0;
	// 读取配置文件中的配置
	char sDiv[32] = {0};
	char sBp[32] = {0};
	char sBaud[32] = {0};
	char sTimeOut[32] = {0};
	// 当前程序运行路径
	CString sIniFilePath;
	sIniFilePath = GetAppPath() + "\\win.ini";
	// 临时字符串
	char sTmpString[32] = {0};
	// 设备连接端口
	GetPrivateProfileString("CentermOneMachine", "Device", "HID", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sDiv, (const char*)sTmpString, sizeof(sDiv)-1);
	// BP口
	GetPrivateProfileString("CentermOneMachine", "BpNo", "9", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sBp, (const char*)sTmpString, sizeof(sBp)-1);
	// 串口波特率
	GetPrivateProfileString("CentermOneMachine", "Baud", "115200", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sBaud, (const char*)sTmpString, sizeof(sBaud)-1);
	// 寻卡超时时间
	GetPrivateProfileString("CentermOneMachine", "TimeOut", "5", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sTimeOut, (const char*)sTmpString, sizeof(sTimeOut)-1);

	GtWriteTrace(30, "%s:%d: 参数: sDiv=[%s], sBp=[%s], sBaud=[%s], sTimeOut=[%s], pSaveHeadPicFilenm=[%s]",
		__FUNCTION__, __LINE__, sDiv, sBp, sBaud, sTimeOut, pSaveHeadPicFilenm);
	// 调高拍仪接口获取身份证信息
	nRet = CT_ReadIDCard( sDiv, sBp, atoi(sBaud), atoi(sTimeOut), (char *)pSaveHeadPicFilenm, &pPerson );
	GtWriteTrace(30, "%s:%d: 调身份证识读接口返回值 nRet = [%d]!",  __FUNCTION__, __LINE__, nRet);
	switch (nRet)
	{
	case 0:
		// 读取成功
		nRet = 0;
		// 个人信息结构体赋值
		memcpy(pPersonInfo, &pPerson, sizeof(PERSONINFO));
		break;
	case -34:
		// 未检测到身份证
		nRet = 204;
		break;
	case -36:
		// 未检测到设备
		nRet = 203;
		break;
	default:
		break;
	}

	return nRet;
}