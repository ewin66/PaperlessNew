
#include "./iocp.h"
#include "../Json/json.h"
#include "../CBaseReadIDCard.h"
#include <string>
using namespace std;

#pragma once

// 初始化IOCP服务
int InitIocpService(LPVOID lpVoid);

void __stdcall AcceptEvt(LPVOID lpParam, HANDLE hAcceptor, PCHAR buf, DWORD len);

void __stdcall WriteEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len);

void __stdcall ExceptEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len);

void __stdcall CloseEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len);

void __stdcall ReadEvt(LPVOID lpParam, HANDLE hObject, PCHAR buf, DWORD len);

// 身份证识读仪获取身份证信息响应码内转外
void ReadIDCardCodeTrans(int nRet, char *sRetCode, char *sRetMsg);

// 摄像头获取照片响应码内转外
void ReadPicCodeTrans(int nRet, char *sRetCode, char *sRetMsg);

// 通过 PERSONINFO 结构体组身份证信息json报文
void getIDCardInfoJson(Json::Value &jsonBuff, CString strDir, MYPERSONINFO *pPerson, int nRet);

/* 功能：通过身份证正反面照组json报文
 * 入参：flag 0-正面 1-反面
 *		strDir 照片本地路径
 *		nRet 调本函数之前，保存身份图片是否成功
 * 出参：jsonBuff：返回报文 
*/ 
void getIDPicJson(Json::Value &jsonBuff, int flag, CString strDir, int nRet);

/* 功能：通过人像照片组返回报文
 * 入参：strDir 照片本地路径
 *		pFilename 照片名称
 *		nRet 调本函数之前，保存人像照是否成功
 *		jsonBuff：返回报文 
*/ 
void getJsonFromPersonPic(Json::Value &jsonBuff, CString strDir, char *pFilename, int nRet);

/* 功能：通过人像照片组返回报文
 * 入参：strDir 照片本地绝对路径
 *		jsonBuff：返回报文 
*/ 
void getJsonFromPic(Json::Value &jsonBuff, CString strDir);

// 返回json数据报文
void SendJsonMsg(Json::Value &jsonBuff, PIO_OP_KEY pOpKey);


CString RetMsg(string xym,string xynr);

CString Json_010101_SendMsg(Json::Value &value);

CString Json_101004_SendMsg(Json::Value &value);

int SendToWindows();

/*
	JSON转换成发送给第三方系统的字符串报文
	CString JsonToSendMsg(string str,CString &sendMsg)
	输入参数：
		str:收到的报文
		sendMsg:返回的字符串报文
	输出参数：
		返回的CString类型的json应答报文
*/
CString JsonToSendMsg(string str);
