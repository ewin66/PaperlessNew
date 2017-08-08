
// PaperlessRestartDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PaperlessRestart.h"
#include "PaperlessRestartDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPaperlessRestartDlg 对话框


CPaperlessRestartDlg::CPaperlessRestartDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPaperlessRestartDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	// 配置文件读取配置
	CString strSettingDir = GetAppPath()+"\\win.ini";
	// 获取等待启动时间
	char sWaitTime[32] = {0};
	memset(sAppDir, 0, sizeof(sAppDir));
	sprintf_s(sAppDir, sizeof(sAppDir)-1, "%s\\XXbgService.exe", GetAppPath().GetBuffer());
	//GetPrivateProfileString("Setting", "AppDir", "./Paperless.exe", sAppDir, sizeof(sAppDir)-1, strSettingDir);
	GetPrivateProfileString("Information", "WaitTime", "3", sWaitTime, sizeof(sWaitTime)-1, strSettingDir);
	nTime = atoi(sWaitTime);
}

void CPaperlessRestartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPaperlessRestartDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CPaperlessRestartDlg 消息处理程序

BOOL CPaperlessRestartDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	// TODO: 在此添加额外的初始化代码
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_STATIC);
	CString sTip;
	if (((CPaperlessRestartApp*)AfxGetApp())->sParam == "EXCEPTION_RESTART")
	{
		// 判断重启时间
		if (nTime <= 0)
		{
			// 直接重启
			RestartPaperlessApp();
		}
		else
		{
			// 如果是通过主程序异常调起的程序，则进行重启操作
			sTip.Format(_T("应用程序异常退出，将在%d秒后重新启动！"), nTime);
			pEdit->SetWindowText(sTip);
			nTime--;
			// 添加定时器
			SetTimer(1, 1000, TimerProc);
		}
	}
	else
	{
		// 重启程序主动打开无效
		sTip.Format(_T("重启程序主动打开无效！"), nTime);
		pEdit->SetWindowText(sTip);
		// IDCANCEL
		CPaperlessRestartDlg *pDlg= (CPaperlessRestartDlg*)AfxGetApp()->m_pMainWnd;
		CButton* pButton = (CButton*)pDlg->GetDlgItem(IDCANCEL);
		pButton->SetWindowText("确定");

	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPaperlessRestartDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CPaperlessRestartDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// 重启应用程序
int CPaperlessRestartDlg::RestartPaperlessApp()
{
	CPaperlessRestartDlg *pDlg= (CPaperlessRestartDlg*)AfxGetApp()->m_pMainWnd;
	CEdit* pEdit = (CEdit*)pDlg->GetDlgItem(IDC_STATIC);
	CString sTip;
	sTip.Format(_T("应用程序发生异常，正在重新启动..."), pDlg->nTime);
	pEdit->SetWindowText(sTip);

	// 启动主程序
	//ShellExecute(NULL, "open", sAppDir, (LPCSTR)"EXCEPTION_RESTART", NULL, SW_SHOWNORMAL);
	ShellExecute(NULL, "open", sAppDir, NULL, NULL, SW_SHOWNORMAL);
	pDlg->KillTimer(1);
	pDlg->DestroyWindow();

	return 0;
}


// 定时器回调函数
void CALLBACK TimerProc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime)
{
	CPaperlessRestartDlg *pDlg= (CPaperlessRestartDlg*)AfxGetApp()->m_pMainWnd;
	CEdit* pEdit = (CEdit*)pDlg->GetDlgItem(IDC_STATIC);
	CString sTip;
	if (pDlg->nTime > 0)
	{
		sTip.Format(_T("应用程序异常退出，将在%d秒后重新启动！"), pDlg->nTime);
		pEdit->SetWindowText(sTip);
		pDlg->nTime--;
	}
	else
	{
		pDlg->RestartPaperlessApp();
	}
}


/* 获取当前程序执行路径
*/
CString GetAppPath()
{
	char szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	CString strPath = szPath;
	int nPos = strPath.ReverseFind('\\');
	int nLen = strPath.GetLength();
	if(nPos > 0)
		strPath = strPath.Left(nPos);
	return strPath;
}