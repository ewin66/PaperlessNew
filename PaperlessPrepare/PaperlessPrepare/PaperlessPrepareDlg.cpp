
// PaperlessPrepareDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "PaperlessPrepare.h"
#include "PaperlessPrepareDlg.h"
#include "afxdialogex.h"
#include "Json/json.h"
#include "Network/HttpComm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPaperlessPrepareDlg 对话框
CPaperlessPrepareDlg::CPaperlessPrepareDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPaperlessPrepareDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	nTime = 0;
	bTimerIsRun = FALSE;
	m_pThread = FALSE;
	// 重试按钮
	pBtnRetry = FALSE;
	// 取消按钮
	pBtnCancel = FALSE;
	// 状态提示
	pStatic = FALSE;
}

CPaperlessPrepareDlg::~CPaperlessPrepareDlg()
{
	MyDelFlushTimer();
}

void CPaperlessPrepareDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPaperlessPrepareDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CANCEL, &CPaperlessPrepareDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_RETRY, &CPaperlessPrepareDlg::OnBnClickedRetry)
END_MESSAGE_MAP()


// CPaperlessPrepareDlg 消息处理程序

BOOL CPaperlessPrepareDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	// 重试按钮
	pBtnRetry = (CButton*) GetDlgItem(IDC_RETRY);
	// 取消按钮
	pBtnCancel = (CButton*) GetDlgItem(IDC_CANCEL);
	// 状态提示
	pStatic = (CStatic*) GetDlgItem(IDC_STATIC);

	MyInitWin();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPaperlessPrepareDlg::OnPaint()
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
HCURSOR CPaperlessPrepareDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// 设置界面刷新定时器
void CPaperlessPrepareDlg::MySetFlushTimer()
{
	if (!bTimerIsRun)
	{
		SetTimer(TIMEER_NO_REFLUSH, 500, TimerProc);
		bTimerIsRun = TRUE;
	}
}


// 清除界面刷新定时器
void CPaperlessPrepareDlg::MyDelFlushTimer()
{
	if (bTimerIsRun)
	{
		KillTimer(TIMEER_NO_REFLUSH);
		bTimerIsRun = FALSE;
	}
}


// 重置界面数据
void CPaperlessPrepareDlg::MyInitWin()
{
	// 隐藏按钮
	pBtnRetry->ShowWindow(SW_HIDE);
	pBtnCancel->ShowWindow(SW_HIDE);

	// 设置提示字体
	CFont *font = new CFont();
	font->CreatePointFont(150, "宋体");
	pStatic->SetFont(font);
	delete font;
	font = NULL;

	// 设置更新提示
	pStatic->SetWindowTextA("检测更新中，请稍后   ");
	nTime = 0;

	// 添加定时器，刷新提示
	MySetFlushTimer();
	// 开启线程，发送程序更新请求
	m_pThread = AfxBeginThread(ThreadFunc, this);
}


// 失败时界面的处理
void CPaperlessPrepareDlg::MyRetryWin(const char *sTip)
{
	// 关闭定时器
	MyDelFlushTimer();

	// 显示按钮
	pBtnRetry->ShowWindow(SW_SHOW);
	pBtnCancel->ShowWindow(SW_SHOW);

	// 设置提示字体
	CFont *font = new CFont();
	font->CreatePointFont(120, "宋体");
	pStatic->SetFont(font);
	delete font;
	font = NULL;
	// 设置更新提示
	pStatic->SetWindowTextA(sTip);
	nTime = 0;
}


// 定时器回调函数
void CALLBACK TimerProc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime)
{
	CPaperlessPrepareDlg *pDlg= (CPaperlessPrepareDlg*)AfxGetApp()->m_pMainWnd;
	CStatic* pEdit = (CStatic*)pDlg->GetDlgItem(IDC_STATIC);
	CString sTip;

	pDlg->nTime = (pDlg->nTime + 1) % 4;
	switch (pDlg->nTime)
	{
	case 0:
		sTip.Format(_T("检测更新中，请稍后   "));
		break;
	case 1:
		sTip.Format(_T("检测更新中，请稍后.  "));
		break;
	case 2:
		sTip.Format(_T("检测更新中，请稍后.. "));
		break;
	case 3:
		sTip.Format(_T("检测更新中，请稍后..."));
		break;
	}
	pEdit->SetWindowText(sTip);
}

// 线程函数
UINT ThreadFunc(LPVOID pParm)
{
	CPaperlessPrepareDlg *pDlg = (CPaperlessPrepareDlg *)pParm;
	
	// 读取配置文件，获取当前程序版本
	char sVersion[32] = {0};
	// 获取更新程序http请求的url
	char sUrl[256] = {0};
	// 发送函数返回值
	int nRet = 0;
	// 发送失败，存放失败原因的位置
	char sSendRet[128] = {0};

	GetPrivateProfileString("Information", "Version", "1.0", sVersion, sizeof(sVersion)-1, GetFilePath()+"\\win.ini");
	GetPrivateProfileString("Information", "POST_URL_UPDATE", "0", sUrl, sizeof(sUrl)-1, GetFilePath()+"\\win.ini");

	// 参数判断
	if (strcmp(sUrl, "0") == 0)
	{
		pDlg->MyRetryWin("检测不到服务器地址配置，请检查！");
	}
	else
	{
		// 拼请求报文
		Json::Value jsonBuff;
		jsonBuff["type"] = "2";
		jsonBuff["string"] = "Paperless.exe";
		jsonBuff["version"] = sVersion;
		jsonBuff["desVersion"] = "0";
		jsonBuff["other"] = "";
		string sData = jsonBuff.toStyledString();
		
		// 报文存放临时文件名
		sprintf_s(g_sTmpFilePath, 256, "%s\\HttpRecvData.loadtmp", GetFilePath().GetBuffer());
		// 删除原有文件
		DeleteFile(g_sTmpFilePath);
		// 发送报文，并将返回报文存入文件 g_sTmpFilePath 中
		nRet = SendData(sUrl, sData.length(), sData.c_str(), sizeof(sSendRet), sSendRet);
		char sTip[128] = {0};
		if (nRet != 0)
		{
			// 报文发送失败
			sprintf_s(sTip, sizeof(sTip)-1, "连接服务器失败：%s", sSendRet);
			pDlg->MyRetryWin(sTip);
		}
		else
		{
			// 数据接收完成，并且已保存到本地临时文件
			// 读取文件，解析报文并处理
			AnalyzeData();
		}
	}
	return 0;
}



void CPaperlessPrepareDlg::OnBnClickedCancel()
{
	MyDelFlushTimer();
	CDialogEx::OnCancel();
}


void CPaperlessPrepareDlg::OnBnClickedRetry()
{
	MyInitWin();
}


BOOL CPaperlessPrepareDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_RETURN)
	{
		return TRUE;
	}
	if(pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_ESCAPE)
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
