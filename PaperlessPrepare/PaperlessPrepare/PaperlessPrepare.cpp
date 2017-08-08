
// PaperlessPrepare.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "PaperlessPrepare.h"
#include "PaperlessPrepareDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPaperlessPrepareApp

BEGIN_MESSAGE_MAP(CPaperlessPrepareApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPaperlessPrepareApp 构造

CPaperlessPrepareApp::CPaperlessPrepareApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CPaperlessPrepareApp 对象

CPaperlessPrepareApp theApp;


// CPaperlessPrepareApp 初始化

BOOL CPaperlessPrepareApp::InitInstance()
{
	CWinApp::InitInstance();

	CString strDir = GetFilePath();
	// 判断log文件存储目录是否存在，不存在则创建
	CString strLogDir = strDir + "\\log";
	if (!PathIsDirectory(strLogDir))
	{
		//创建目录,已有的话不影响
		::CreateDirectory(strLogDir, NULL);
	}
	// 日志文件初始化
	GtSetTraceFilePrefix(GetFilePath()+"\\log\\Update");
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

	AfxEnableControlContainer();

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

	CPaperlessPrepareDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

