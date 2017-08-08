
// Paperless.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CPaperlessApp:
// 有关此类的实现，请参阅 Paperless.cpp
//

class CPaperlessApp : public CWinApp
{
public:
	CPaperlessApp();

// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);


public:
	int nSystemBit;
	HWND m_hwndDlg;
	// 发送http报文线程
	CWinThread *m_pHttpThread;
	volatile BOOL bIsHttpThreadRun;
private:
	// 设置开机启动
	BOOL SetAutoRun();
	// 清理日志、截图、身份证照等文件，配置文件获取保留几天文件
	BOOL PreCleanUpFiles();
	// 清理sDir目录下超过nDay的文件
	BOOL CleanUpFiles(CString sDir, int nDay);
	// 通过写注册表，设置IE版本
	BOOL SetIEVersion();
	// 写注册表，设置IE版本
	BOOL SetIEVersionChild(const char *pKeyName, int pKeyValue);

	DECLARE_MESSAGE_MAP()
};

// 程序崩溃处理程序
LONG WINAPI FreeEIM_UnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo);
extern CPaperlessApp theApp;