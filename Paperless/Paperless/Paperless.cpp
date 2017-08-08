
// Paperless.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "Paperless.h"
#include "PaperlessDlg.h"

#include "ScreenshotDlg.h"
#include "MyTTrace.h"
#include "utils.h"
#include <DbgHelp.h>
#pragma comment(lib,"DbgHelp.Lib")  //MiniDumpWriteDump链接时用到

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPaperlessApp
BEGIN_MESSAGE_MAP(CPaperlessApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPaperlessApp 构造
CPaperlessApp::CPaperlessApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
	m_pMainWnd = NULL;
	m_hwndDlg = NULL;
	m_pHttpThread = NULL;
	bIsHttpThreadRun = FALSE;
	nSystemBit = 0;
}


// 唯一的一个 CPaperlessApp 对象
CPaperlessApp theApp;


// CPaperlessApp 初始化
BOOL CPaperlessApp::InitInstance()
{
	// 创建log文件夹，截图文件夹
	// 获取当前程序运行相对路径
	CString strDir = GetAppPath();
	// 判断截图文件存储目录是否存在，不存在则创建
	CString strPicDir = strDir + "\\CatchScreen";
	if (!PathIsDirectory(strPicDir))
	{
		//创建目录,已有的话不影响
		::CreateDirectory(strPicDir, NULL);
	}
	// 判断log文件存储目录是否存在，不存在则创建
	CString strLogDir = strDir + "\\log";
	if (!PathIsDirectory(strLogDir))
	{
		//创建目录,已有的话不影响
		::CreateDirectory(strLogDir, NULL);
	}
	// 判断身份证正反面文件存储目录是否存在，不存在则创建
	CString strIDDir = strDir + "\\IDPicture";
	if (!PathIsDirectory(strIDDir))
	{
		//创建目录,已有的话不影响
		::CreateDirectory(strIDDir, NULL);
	}

	// 日志文件初始化
	GtSetTraceFilePrefix(GetFilePath()+"\\log\\Paperless");
	GtSetTraceLevel(EM_TraceDebug);

	// 单一实例处理
	HANDLE m_hMutex = CreateMutex(NULL, FALSE, this->m_pszAppName);// 检查错误代码
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		// 如果已有互斥量存在则释放句柄并复位互斥量　
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		// 程序退出
		::MessageBoxA(NULL, "程序正在运行！不允许重复启动！", "警告", MB_OK);
		return FALSE;
	}

	// 关联异常处理程序
	SetUnhandledExceptionFilter(FreeEIM_UnhandledExceptionFilter);


	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

// 	// 初始化 OLE 库
// 	if (!AfxOleInit())
// 	{
// 		AfxMessageBox(IDP_OLE_INIT_FAILED);
// 		return FALSE;
// 	}

//	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("国通科技"));

	CPaperlessDlg *dlg = new CPaperlessDlg();
	dlg->Create(IDD_PAPERLESS_DIALOG);
	m_pMainWnd = dlg;
	m_pMainWnd->UpdateWindow();

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 设置开机启动
	//SetAutoRun();

	// 删除临时目录下超过 M天 的文件，配置文件获取清理的天数，获取不到默认保留7天的日志（不包括当天）
	//PreCleanUpFiles();

	// 设置IE的版本
	//SetIEVersion();

	// 启动应用程序的消息泵。
	return TRUE;
}


int CPaperlessApp::ExitInstance()
{
	//TODO: 处理可能已添加的附加资源
	//AfxOleTerm(FALSE);
	if (m_pMainWnd)
	{
		delete m_pMainWnd;
		m_pMainWnd = NULL;
	}


	return CWinApp::ExitInstance();
}


// 设置开机启动
BOOL CPaperlessApp::SetAutoRun()
{
	GtWriteTrace(30, "%s:%d: 开始设置开机启动......", __FUNCTION__, __LINE__);
	HKEY rootKeyDir = HKEY_LOCAL_MACHINE;
	char subKeyDir[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	char sValue[256] = {0};

	// 程序名称，key值
	char sMyAppName[] = "Paperless";
	if (!MyReadRegedit(rootKeyDir, subKeyDir, sMyAppName, sValue, 256, REG_SZ))
	{
		// 读取注册表信息失败，设置开机启动
		char pFileName[MAX_PATH] = {0};
		// 得到程序自身的全路径
		DWORD dwRet = GetModuleFileName(NULL, pFileName, MAX_PATH);
		if (!MyWriteRegedit(rootKeyDir, subKeyDir, sMyAppName, pFileName, REG_SZ))
		{
			GtWriteTrace(30, "%s:%d: \t设置开机启动失败！", __FUNCTION__, __LINE__);
			::MessageBox(NULL, "设置开机自启失败！", "警告", MB_OK);
		}
		else
		{
			GtWriteTrace(30, "%s:%d: \t设置开机启动成功！", __FUNCTION__, __LINE__);
			::MessageBox(NULL, "设置开机自启成功！", "提示", MB_OK);
		}
	}
	else
	{
		GtWriteTrace(30, "%s:%d: \t已设置开机启动，应用程序位置：%s", __FUNCTION__, __LINE__, sValue);
	}
	GtWriteTrace(30, "%s:%d: 设置开机启动结束\n", __FUNCTION__, __LINE__);
	return TRUE;
}


// 清理日志、截图、身份证照等文件，配置文件获取保留几天文件
BOOL CPaperlessApp::PreCleanUpFiles()
{
	// 临时文件保留天数
	int nDay = 0;
	// 临时字符串
	CString tmpString;
    // 获取当前程序运行目录
	CString sPath = GetAppPath();
	// 配置文件位置
	CString iniDir = sPath + "\\win.ini";
	// 待清理日期的临时目录
	CString sTmpPath;

	// 配置文件获取 临时文件 保留天数
	GetPrivateProfileString("Information", "FilesSaveDay", "7", tmpString.GetBuffer(31), 31, iniDir);
	tmpString.ReleaseBuffer();
	// CString转int
	nDay = _ttoi(tmpString);

	// 清理截图文件
	sTmpPath = sPath + "\\CatchScreen\\";
	CleanUpFiles(sTmpPath, nDay);

	// 清理身份证照片
	sTmpPath = sPath + "\\IDPicture\\";
	CleanUpFiles(sTmpPath, nDay);

	// 清理日志
	sTmpPath = sPath + "\\log\\";
	CleanUpFiles(sTmpPath, nDay);

	// 测试目录
	sTmpPath = sPath + "\\test\\";
	CleanUpFiles(sTmpPath, nDay);

	return TRUE;
}


// 清理sDir目录下超过nDay的文件
BOOL CPaperlessApp::CleanUpFiles(CString sDir, int nDay)
{
	GtWriteTrace(30, "%s:%d: 开始清理sDir目录下过期文件......", __FUNCTION__, __LINE__);
	if (sDir.IsEmpty() || nDay <= 0)
	{
		GtWriteTrace(30, "%s:%d: 入参检查失败！\n.", __FUNCTION__, __LINE__);
		return FALSE;
	}
	GtWriteTrace(30, "%s:%d: \t即将清理目录[%s]下超过[%d]天的文件", __FUNCTION__, __LINE__, sDir.GetBuffer(), nDay);
	// 时间差
	double lDiffTime = 0.00;
	// 临时文件保存时间，秒为单位
	double lSaveDays = nDay * 24 * 60 * 60;
	// 当前时间
	struct tm sNowTime;
	// 当前时间指针
	struct tm *pNowTime = &sNowTime;
	// 以秒为单位的当前时间
	time_t nowTime;
	// 以秒为单位的当天0时0分0秒时间，用于与文件创建时间做比较，删除大于nDay的文件
	time_t beginTime;
	
	// 获取当前时间
    time(&nowTime);
	// 转化为日常日期格式
	localtime_s(pNowTime, &nowTime);
	GtWriteTrace(30, "%s:%d: \t当前时间[%04d-%02d-%02d %02d:%02d:%02d]", __FUNCTION__, __LINE__, nDay, 
		pNowTime->tm_year + 1900, pNowTime->tm_mon + 1, pNowTime->tm_mday, pNowTime->tm_hour,
		pNowTime->tm_min, pNowTime->tm_sec);
	// 日期转换到当天 0时0分0秒
	pNowTime->tm_hour = 0;
	pNowTime->tm_min = 0;
	pNowTime->tm_sec = 0;
	GtWriteTrace(30, "%s:%d: \t当天零时时间[%04d-%02d-%02d %02d:%02d:%02d]", __FUNCTION__, __LINE__, nDay, 
		pNowTime->tm_year + 1900, pNowTime->tm_mon + 1, pNowTime->tm_mday, pNowTime->tm_hour,
		pNowTime->tm_min, pNowTime->tm_sec);
	// 日常日期格式转换成日历时间（秒为单位）
	beginTime = mktime(pNowTime);
	

	// 遍历目录下的文件
	CFileFind ff;
	if (sDir.Right(1) != "\\")
	{
		sDir += "\\";
	}
	sDir += "*.*";
	BOOL ret = ff.FindFile(sDir);
	while (ret)
	{
		ret = ff.FindNextFile();
		// 不是目录，包括 ".", ".."
		if (!ff.IsDirectory() && !ff.IsDots())
		{
			CString sFilePath = ff.GetFilePath();
			CString sFileName = ff.GetFileName();
			struct _stati64 stat;

			//GtWriteTrace(30, "[XXbgService]CleanUpFiles() 查询文件：[%s]", sFileName.GetBuffer());
			if (FALSE == GetFileAttributes(&stat, sFilePath))
			{
				GtWriteTrace(30, "%s:%d: \t获取文件[%d]创建时间失败！", __FUNCTION__, __LINE__, sFileName.GetBuffer());
				continue;
			}
			// 计算时间差值，判断是否需要删除(当天零点减去文件创建时间的值，是否超过nDay天)
			struct tm tmpTime;
			struct tm *pTmpTime = &tmpTime;
			// 转化为日常日期格式
			localtime_s(pTmpTime, &stat.st_ctime);
			lDiffTime = difftime(beginTime, stat.st_ctime);
			if (lDiffTime > lSaveDays)
			//if (lDiffTime > 120)
			{
				// 超过设置的保存时间，删除此文件
				if (DeleteFile(sFilePath))
				{
					GtWriteTrace(30, "%s:%d: \t清理文件[%s]成功！", __FUNCTION__, __LINE__, sFileName.GetBuffer());
				}
				else
				{
					GtWriteTrace(30, "%s:%d: \t清理文件[%s]失败！", __FUNCTION__, __LINE__, sFileName.GetBuffer());
				}
			}
		}
	}
	GtWriteTrace(30, "%s:%d: 清理sDir目录下过期文件正常退出\n", __FUNCTION__, __LINE__);
	return true;
}


// 通过写注册表，设置本程序启动的IE版本
BOOL CPaperlessApp::SetIEVersion()
{
	GtWriteTrace(30, "%s:%d: 进入设置IE版本函数......", __FUNCTION__, __LINE__);
	int nIEVersion = 0;
	// 注册表查询当前IE最高版本
	HKEY rootKey = HKEY_LOCAL_MACHINE;
	char subKeyIEVersion[] = "SOFTWARE\\Microsoft\\Internet Explorer\\Version Vector";
	char keyNameIE[] = "IE";
	char keyValueIE[256] = {0};
	if (MyReadRegedit(rootKey, subKeyIEVersion, keyNameIE, keyValueIE, sizeof(keyValueIE), REG_SZ))
	{
		nIEVersion = atoi(keyValueIE) * 1000;
		GtWriteTrace(30, "%s:%d: \t注册表查询到当前IE版本[%s]，设置[%d]\n", __FUNCTION__, __LINE__, keyValueIE, nIEVersion);
	}
	else
	{
		nIEVersion = 6 * 1000;
		GtWriteTrace(30, "%s:%d: \t注册表查询当前IE版本失败，设置默认版本：IE6，设置[%d]\n", __FUNCTION__, __LINE__, nIEVersion);
	}

	// 设置IE版本键值
	// 首先检查注册表是否已经存在值
	char subKeyWrite[] = "SOFTWARE\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\FEATURE_BROWSER_EMULATION";
	// 程序名称，key名称
	char sMyAppName[] = "Paperless.exe";
	DWORD sValue = 0;

	// 未设置键值 或者 设置了键值，但不为当前IE最新版本
	if ( (!MyReadRegedit(rootKey, subKeyWrite, sMyAppName, (void *)&sValue, sizeof(DWORD), REG_DWORD)) && (sValue != nIEVersion) )
	{
		// 不存在该键值则添加
		if (!MyWriteRegedit(rootKey, subKeyWrite, sMyAppName, (void *)&nIEVersion, REG_DWORD))
		{
			GtWriteTrace(30, "%s:%d: \t设置IE版本[%s]失败！\n", __FUNCTION__, __LINE__, "FEATURE_BROWSER_EMULATION");
		}
		else
		{
			GtWriteTrace(30, "%s:%d: \t设置IE版本[%s]成功！\n", __FUNCTION__, __LINE__, "FEATURE_BROWSER_EMULATION");
		}
	}
	else
	{
		GtWriteTrace(30, "%s:%d: \t已设置IE版本[%d]\n", __FUNCTION__, __LINE__, sValue);
	}

	// 设置其他键值
	if (SetIEVersionChild("FEATURE_ACTIVEX_REPURPOSEDETECTION", 1) == false || 
		SetIEVersionChild("FEATURE_BLOCK_LMZ_IMG", 1) == false || 
		SetIEVersionChild("FEATURE_BLOCK_LMZ_OBJECT", 1) == false || 
		SetIEVersionChild("FEATURE_BLOCK_LMZ_SCRIPT", 1) == false || 
		SetIEVersionChild("FEATURE_ENABLE_SCRIPT_PASTE_URLACTION_IF_PROMPT", 1) == false || 
		SetIEVersionChild("FEATURE_LOCALMACHINE_LOCKDOWN", 1) == false ||
		SetIEVersionChild("FEATURE_GPU_RENDERING", 1) == false || 
		SetIEVersionChild("FEATURE_Cross_Domain_Redirect_Mitigation", 1) == false )
	{
		GtWriteTrace(30, "%s:%d: \t子键设置失败！", __FUNCTION__, __LINE__);
	}


	GtWriteTrace(30, "%s:%d: 设置IE版本函数正常退出\n", __FUNCTION__, __LINE__);
	return TRUE;
}


// 写注册表，设置IE版本
BOOL CPaperlessApp::SetIEVersionChild(const char *pKeyName, int pKeyValue)
{
	GtWriteTrace(30, "%s:%d: 进入设置IE版本其他key函数......", __FUNCTION__, __LINE__, pKeyName);
	if (pKeyName == false)
	{
		GtWriteTrace(30, "%s:%d: 入参检查失败！\n", __FUNCTION__, __LINE__, pKeyName);
		return FALSE;
	}
	// 首先检查注册表是否已经存在值
	HKEY rootKey = HKEY_LOCAL_MACHINE;
	CString subKeyDir = "SOFTWARE\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\";
	subKeyDir += pKeyName;

	// 首先创建子key，因为可能其不存在
	MyAddRegedit(rootKey, subKeyDir.GetBuffer());

	// 程序名称，key值
	char sMyAppName[] = "Paperless.exe";
	DWORD sValue = 0;

	// 
	if (!MyReadRegedit(rootKey, subKeyDir.GetBuffer(), sMyAppName, (void *)&sValue, sizeof(DWORD), REG_DWORD))
	{
		// 不存在该键值则添加
		if (!MyWriteRegedit(rootKey, subKeyDir.GetBuffer(), sMyAppName, (void *)&pKeyValue, REG_DWORD))
		{
			GtWriteTrace(30, "%s:%d: \t设置key[%s]失败！\n", __FUNCTION__, __LINE__, pKeyName);
			return FALSE;
		}
		else
		{
			GtWriteTrace(30, "%s:%d: \t设置key[%s]成功！", __FUNCTION__, __LINE__, pKeyName);
		}
	}
	else
	{
		GtWriteTrace(30, "%s:%d: \t已存在此键[%s], 值：%d", __FUNCTION__, __LINE__, pKeyName, sValue);
	}
	GtWriteTrace(30, "%s:%d: 设置IE版本其他key函数正常退出！\n", __FUNCTION__, __LINE__, pKeyName);
	return TRUE;
}

//********************************************************************************
#define SHIFTED 0x8000 
//********************************************************************************
BOOL CPaperlessApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(m_hwndDlg!=NULL)
	{
		// 只接收到键盘事件？鼠标事件 lpMsg->wParam = 0；
		//判断消息，如果消息是从对话框发出的或者其子控件发出的，就进行处理
		if((lpMsg->hwnd==m_hwndDlg) || ::IsChild(m_hwndDlg,lpMsg->hwnd))
		{
			//如果消息是WM_KEYDOWN
			//用方向键调整位置
			CScreenshotDlg * pDlg = ((CPaperlessDlg*)theApp.m_pMainWnd)->pScreenshotDlg;
			if (pDlg != NULL)
			{
				if(lpMsg->message==WM_KEYDOWN)
				{
					CRect rect(0,0,0,0);
					rect=pDlg->m_rectTracker.m_rect;
					// 判断
					if(pDlg->m_bFirstDraw)
					{
						//如果Shift键按下则方向键调整大小
						BOOL isShifeDowm=FALSE;
						int nVirtKey;
						nVirtKey = GetKeyState(VK_SHIFT); 
						if (nVirtKey & SHIFTED) 
							isShifeDowm=TRUE;

						switch(lpMsg->wParam)
						{
						case VK_UP:
							//如果按下Shift,则只调整一边
							if(!isShifeDowm)
								rect.top-=1;
							rect.bottom-=1;
							pDlg->m_rectTracker.m_rect=rect;
							pDlg->PaintWindow();
							break;
						case VK_DOWN:
							if(!isShifeDowm)
								rect.top+=1;
							rect.bottom+=1;
							pDlg->m_rectTracker.m_rect=rect;
							pDlg->PaintWindow();
							break;
						case VK_LEFT:
							if(!isShifeDowm)
								rect.left-=1;
							rect.right-=1;
							pDlg->m_rectTracker.m_rect=rect;
							pDlg->PaintWindow();
							break;
						case VK_RIGHT:
							if(!isShifeDowm)
								rect.left+=1;
							rect.right+=1;
							pDlg->m_rectTracker.m_rect=rect;
							pDlg->PaintWindow();
							break;
						}
						// 边缘调整，重新 定位显示 确认、取消按钮
						pDlg->MoveShowOkCancelBtn();
					}
				}
			}
			
		}
	} 
	return CWinApp::ProcessMessageFilter(code, lpMsg);
}
//********************************************************************************


// 程序崩溃处理程序
LONG WINAPI FreeEIM_UnhandledExceptionFilter(LPEXCEPTION_POINTERS pExceptionInfo)
{
	//AfxMessageBox("程序发生异常，即将退出程序！");	//调用回调函数成功

	static int flag = 0;
	// 写dump文件
	SYSTEMTIME st;
	GetLocalTime(&st);
	CString sDmpFilename;
	sDmpFilename.Format("ExceDump-%04d%02d%02d-%02d%02d%02d-%03d.dmp", 
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	LPCSTR pPath = sDmpFilename.GetBuffer();
	sDmpFilename.ReleaseBuffer();
	//MultiByteToWideChar(CP_ACP, 0, path.c_str(), size, buffer, size * sizeof(wchar_t));
	//buffer[size] = 0;  //确保以 '\0' 结尾 
	HANDLE hFile = ::CreateFile(pPath, GENERIC_WRITE,
		FILE_SHARE_WRITE, NULL, CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{   
		MINIDUMP_EXCEPTION_INFORMATION exptInfo;
		exptInfo.ThreadId = ::GetCurrentThreadId();
		exptInfo.ExceptionPointers = pExceptionInfo;
		//将内存堆栈dump到文件中
		//MiniDumpWriteDump需引入dbghelp头文件
		BOOL bOK = ::MiniDumpWriteDump(::GetCurrentProcess(),
			::GetCurrentProcessId(),hFile, MiniDumpNormal,
			&exptInfo, NULL, NULL);
	}

	if (flag == 0)
	{
		// 删除托盘图标
// 		NOTIFYICONDATA *pNid = &((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_nid;
// 		Shell_NotifyIcon(NIM_DELETE, pNid);
		flag = 1;
		// 调用重启/错误提交程序（视情况而定）
		LPCSTR lpcsOperate = _T("open");
		LPCSTR lpcsDir = _T("H:\\个人文件\\gotop\\其他项目\\程序包_20170709\\XXbgService\\Debug\\PaperlessRestart.exe");
		//WinExec(lpcsDir, SW_SHOWNORMAL);
		ShellExecute(NULL, lpcsOperate, lpcsDir, (LPCSTR)"EXCEPTION_RESTART", NULL, SW_SHOWNORMAL);
	}

	return EXCEPTION_EXECUTE_HANDLER;	//返回本回调函数的处理结果
}