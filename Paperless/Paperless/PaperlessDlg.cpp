
// PaperlessDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Paperless.h"
#include "PaperlessDlg.h"
#include "afxdialogex.h"
#include "utils.h"
#include "MyTTrace.h"
#include "GHook/GHook.h"
#include "Centerm/CENT_IDCard.h"
#include "CBaseReadIDCard.h"
#include "CCentOneCamera.h"
#include "CGeitCamera.h"
#include "CCentOneReader.h"
#include "CCentReader.h"
#include "ScreenshotDlg.h"
#include "ManualInputDlg.h"
#include "SettingDlg.h"
#include "CBaseReadIDCard.h"
#include "Network/SocketComm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// 消息映射函数
BEGIN_MESSAGE_MAP(CPaperlessDlg, CDialogEx)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_MESSAGE(WM_TRAYNOTIFY, OnTrayNotify)
	ON_MESSAGE(WM_SCREENSHOT, OnScreenshot)
	ON_MESSAGE(WM_SCREENDLG_MSG, OnScreenDlgMessage)
	ON_MESSAGE(WM_CONTINUE_INPUT, OnContinueInput)
	ON_COMMAND(ID_QUIT_MENU, OnQuit)
	ON_COMMAND(ID_INPUT_SQ_MENU, OnManualInput)
	ON_COMMAND(ID_SETTING_MENU, OnSettingWin)
	// 系统命令处理
	ON_WM_SYSCOMMAND()
	// 关闭主窗口事件
	ON_MESSAGE_VOID(WM_CLOSE, OnMyClose)
END_MESSAGE_MAP()


CPaperlessDlg::CPaperlessDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPaperlessDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	// TODO: 在此添加成员初始化代码
	pScreenshotDlg = NULL;
	pInputDlg = NULL;
	pSettingDlg = NULL;
	pBaseReadIDCardInfo = NULL;
	pBaseSaveCameraPic = NULL;
	nReadIDDevice = -1;
	nDeskCameraDevice = -1;
	isLastTimeExitZoomed = false;
	isFirstDbClickMenu = true;
}


CPaperlessDlg::~CPaperlessDlg()
{
	//GtWriteTrace(30, "%s:%d: ~CPaperlessDlg!", __FUNCTION__, __LINE__);
	// 清空托盘图标
	if(m_nid.hIcon)
		::DestroyIcon(m_nid.hIcon);
	// 清空菜单
	if(m_menu)
		::DestroyMenu(m_menu);
	// 删除托盘相关结构体
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
	// 清空自己创建的
	if (pScreenshotDlg != NULL)
	{
		delete pScreenshotDlg;
		pScreenshotDlg = NULL;
	}
	if (pInputDlg != NULL)
	{
		delete pInputDlg;
		pInputDlg = NULL;
	}
	if (pSettingDlg != NULL)
	{
		delete pSettingDlg;
		pSettingDlg = NULL;
	}
	if (pBaseReadIDCardInfo != NULL)
	{
		delete pBaseReadIDCardInfo;
		pBaseReadIDCardInfo = NULL;
	}
	if (pBaseSaveCameraPic != NULL)
	{
		delete pBaseSaveCameraPic;
		pBaseSaveCameraPic = NULL;
	}
}


// 点击托盘菜单的退出程序，
void CPaperlessDlg::OnQuit()
{
	//GtWriteTrace(30, "%s:%d: OnQuit!", __FUNCTION__, __LINE__);
	// 退出程序，保存窗口位置到配置文件中
	SaveFrmPosToFile();
	// 清空托盘图标
	if(m_nid.hIcon)
		::DestroyIcon(m_nid.hIcon);
	// 清空菜单
	if(m_menu)
		::DestroyMenu(m_menu);
	// 删除托盘相关结构体
	Shell_NotifyIcon(NIM_DELETE, &m_nid);

	// 清空自己创建的
	if (pScreenshotDlg != NULL)
	{
		delete pScreenshotDlg;
		pScreenshotDlg = NULL;
	}
	if (pInputDlg != NULL)
	{
		delete pInputDlg;
		pInputDlg = NULL;
	}
	if (pSettingDlg != NULL)
	{
		delete pSettingDlg;
		pSettingDlg = NULL;
	}
	if (pBaseReadIDCardInfo != NULL)
	{
		delete pBaseReadIDCardInfo;
		pBaseReadIDCardInfo = NULL;
	}
	if (pBaseSaveCameraPic != NULL)
	{
		delete pBaseSaveCameraPic;
		pBaseSaveCameraPic = NULL;
	}
	DestroyWindow();
}


void CPaperlessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

// 创建对话框
int CPaperlessDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	//GtWriteTrace(30, "%s:%d: OnCreate!", __FUNCTION__, __LINE__);
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	/************* 创建状态栏 ***************/ 
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
	/************* 创建托盘图标 ***************/ 
	//启动就隐藏到右下角
	memset(&m_nid,0,sizeof(m_nid));
	m_nid.cbSize = sizeof(m_nid);
	m_nid.hWnd = this->m_hWnd;
	m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	m_nid.uCallbackMessage = WM_TRAYNOTIFY;
	CString strToolTip = "免填单系统后台服务";
	strcpy(m_nid.szTip,strToolTip.GetBuffer());
	m_nid.uID = IDR_MAINFRAME;
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nid.hIcon = hIcon;
	Shell_NotifyIcon(NIM_ADD, &m_nid);
	if(hIcon)
		::DestroyIcon(hIcon);
	BOOL flag = m_menu.LoadMenu(IDR_POPMENU); 

	/************* 钩子启动 ***************/ 
	// 钩子启动标志，初始为FALSE未启动状态
	m_hookFlag = FALSE;
	// 启动钩子
	StartKeyBoardHook();

	/************* 初始化iocp服务器 ***************/ 
	// 初始化iocp服务器
	InitIocpService(this);
	return 0;
}


typedef struct S_WNDINFO
{
	DWORD dwProcessId;
	HWND hWnd;
}WNDINFO, *LPWNDINFO;
BOOL CALLBACK MyEnumProc(HWND hwnd, LPARAM lParam)
{
	DWORD dwProcessId;
	GetWindowThreadProcessId(hwnd, &dwProcessId);
	LPWNDINFO pInfo = (LPWNDINFO)lParam;
	char buf[256]; 
	GetWindowText(hwnd, buf, sizeof(buf)); 
	GtWriteTrace(30, "%s:%d: 查找ID: %ld, 此ID: %ld, 名称: %s", __FUNCTION__, __LINE__,
		pInfo->dwProcessId, dwProcessId, buf);
	if (pInfo->dwProcessId == dwProcessId)
	{
		GtWriteTrace(30, "%s:%d: +++++++++++++++++找到窗口++++++++++++++++++", __FUNCTION__, __LINE__);
		return false;
	}
	return true;
}

// 
BOOL CPaperlessDlg::OnInitDialog()
{
	//GtWriteTrace(30, "%s:%d: OnInitDialog!", __FUNCTION__, __LINE__);
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	/************* 初始化身份证识读仪，高拍仪文拍摄像头，高拍仪环境摄像头 ***************/ 
	InitDevice();

	/************* 从配置文件中恢复上次退出窗口位置 ***************/ 
	InitFrmPosFromFile();

//	ShellExecute(NULL, _T("open"), _T("IExplore.exe"), _T("http://www.baidu.com/"), NULL, SW_SHOWNORMAL);
	STARTUPINFO si = { sizeof(si) }; 
	PROCESS_INFORMATION pi; 

	si.dwFlags = STARTF_USESHOWWINDOW; 
	si.wShowWindow = TRUE; //TRUE表示显示创建的进程的窗口
	BOOL bRet = ::CreateProcess ( 
		NULL,
		TEXT("c://program files//internet explorer//iexplore.exe http://www.baidu.com"), //在Unicode版本中此参数不能为常量字符串，因为此参数会被修改	 
		NULL, 
		NULL, 
		FALSE, 
		CREATE_NEW_CONSOLE, 
		NULL, 
		NULL, 
		&si, 
		&pi); 

	int error = GetLastError();
	if(bRet) 
	{
		WNDINFO wi;
		GtWriteTrace(30, "%s:%d: 新进程的进程ID号：%ld!", __FUNCTION__, __LINE__, pi.dwProcessId);
		GtWriteTrace(30, "%s:%d: 新进程的主线程ID号：%ld!", __FUNCTION__, __LINE__, pi.dwThreadId);
		wi.dwProcessId = pi.dwProcessId;
		wi.hWnd = NULL;
		// 等待新进程初始化完毕
		WaitForInputIdle(pi.hProcess, 5000);
		EnumWindows(MyEnumProc, (LPARAM)&wi);

		::CloseHandle (pi.hThread); 
		::CloseHandle (pi.hProcess); 
	} 
	else
	{
		GtWriteTrace(30, "%s:%d: error code:%d!", __FUNCTION__, __LINE__, error);
	}
	// 除非将焦点设置到控件，否则返回 TRUE
	return TRUE;
}


/*********************** VS2010自动生成函数 *******************************/
// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。
void CPaperlessDlg::OnPaint()
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
HCURSOR CPaperlessDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
// CMainFrame 诊断

#ifdef _DEBUG
void CPaperlessDlg::AssertValid() const
{
	CDialogEx::AssertValid();
}

void CPaperlessDlg::Dump(CDumpContext& dc) const
{
	CDialogEx::Dump(dc);
}
#endif //_DEBUG
/*********************** VS2010自动生成函数 *******************************/


// 窗口大小改变响应函数
void CPaperlessDlg::OnSize(UINT nType, int cx, int cy)
{
	//GtWriteTrace(30, "%s:%d: OnSize(%d, %d)!", __FUNCTION__, __LINE__, cx, cy);
	CDialog::OnSize(nType, cx, cy);
	if(nType==1) return;//最小化则什么都不做
	// TODO: Add your message handler code here
	CStatic *pStatic = (CStatic *)GetDlgItem(IDC_STATIC_HTML);
	if (pStatic)
	{
		//GtWriteTrace(30, "%s:%d: 更新html窗口位置!", __FUNCTION__, __LINE__, cx, cy);
		// 把编辑控件的大小设为(cx, cy)，位置不变
		pStatic->MoveWindow(0, 0, cx, cy);
		// 设置CStatic子控件 html 窗体大小，发送消息设置
	}
}


void CPaperlessDlg::InitDevice()
{
	// 配置文件获取设备配置信息
	// 当前程序运行路径
	CString sIniFilePath;
	sIniFilePath = GetAppPath() + "\\win.ini";
	// 临时字符串
	char sTmpString[32] = {0};
	// 身份证识读仪设备
	GetPrivateProfileString("Device", "ReadIDDevice", "-1", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	nReadIDDevice = atoi(sTmpString);
	// 文拍摄像头设备
	GetPrivateProfileString("Device", "DesktopCameraDevice", "-1", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	nDeskCameraDevice = atoi(sTmpString);

	GtWriteTrace(30, "%s:%d: 设备配置参数，ReadIDDevice=[%d], sDeskCameraDevice=[%d]!", __FUNCTION__
		, __LINE__, nReadIDDevice, nDeskCameraDevice);
	// 判断 身份证识读仪设备类型，加载对应类

	switch (nReadIDDevice)
	{
	case -1:
		::MessageBox(NULL, "身份证识读仪设备未配置！请检查！", "警告", MB_OK);
		break;
	case 1:
		pBaseReadIDCardInfo = new CCentOneReader();
		break;
	case 2:
		pBaseReadIDCardInfo = new CCentReader();
		break;
	default:
		::MessageBox(NULL, "配置的身份证识读仪不存在！请检查！", "警告", MB_OK);
		break;
	}
	if (nDeskCameraDevice == -1)
	{
		::MessageBox(NULL, "高拍仪摄像头设备未配置！请检查！", "警告", MB_OK);
	}
	else if (nDeskCameraDevice == 1)
	{
		pBaseSaveCameraPic = new CCentOneCamera(this);
	}
	else if (nDeskCameraDevice == 2)
	{
		pBaseSaveCameraPic = new CGeitCamera();
	}
	else
	{
		::MessageBox(NULL, "配置的高拍仪摄像头设备不存在！请检查！", "警告", MB_OK);
	}
}


// 保存主窗口位置到配置文件中
BOOL CPaperlessDlg::SaveFrmPosToFile()
{
	// 配置文件路径
	CString iniDir = GetAppPath()+"\\win.ini";
	// 判断当前窗体状态，最大化、最小化、正常？
	// 不在此处保存最大化、最小化位置是因为最大化最小化状态取不到其之前的信息
	if (this->IsZoomed())
	{
		// 最大化，保存状态到配置文件中
		// 最大化状态：1-是
		// 不需保存最大化前的位置，因为点击最大化时已经保存过
		WritePrivateProfileString("Information", "IsZoomed", "1", iniDir);
	}
	else if (this->IsIconic())
	{
		// 最小化，不需保存最小化前的位置，，因为在点击最小化之前已经保存过
	}
	else
	{
		// 非最大化，保存窗口位置到配置文件中
		RECT rect = {0};
		CString tmp;
		// 获取当前窗口位置
		::GetWindowRect(this->m_hWnd, &rect);
		tmp.Format("%d", rect.left);
		WritePrivateProfileString("Information", "WinXPos", tmp, iniDir);
		tmp.Format("%d", rect.top);
		WritePrivateProfileString("Information", "WinYPos", tmp, iniDir);
		tmp.Format("%d", rect.right - rect.left);
		WritePrivateProfileString("Information", "WinWidth", tmp, iniDir);
		tmp.Format("%d", rect.bottom - rect.top);
		WritePrivateProfileString("Information", "WinHigh", tmp, iniDir);
		// 最大化状态：0-否
		WritePrivateProfileString("Information", "IsZoomed", "0", iniDir);
	}
	return TRUE;
}


// 读取配置文件配置，恢复窗口位置
BOOL CPaperlessDlg::InitFrmPosFromFile()
{
	GtWriteTrace(30, "%s:%d: 初始化窗口位置开始......", __FUNCTION__, __LINE__);
	// 左上角x坐标
	int nWinXPos = 0;
	// 左上角y坐标
	int nWinYPos = 0;
	// 窗口宽
	int nWinWidth = 0;
	// 窗口高
	int nWinHigh = 0;
	// 临时字符串
	char strTmp[32] = {0};
	CString iniDir = GetAppPath()+"\\win.ini";

	// 获取配置文件中的窗口显示的位置
	GetPrivateProfileString("Information", "WinXPos", "-1", strTmp, sizeof(strTmp)-1, iniDir);
	nWinXPos = atoi(strTmp);

	GetPrivateProfileString("Information", "WinYPos", "-1", strTmp, sizeof(strTmp)-1, iniDir);
	nWinYPos = atoi(strTmp);

	GetPrivateProfileString("Information", "WinWidth", "-1", strTmp, sizeof(strTmp)-1, iniDir);
	nWinWidth = atoi(strTmp);

	GetPrivateProfileString("Information", "WinHigh", "-1", strTmp, sizeof(strTmp)-1, iniDir);
	nWinHigh = atoi(strTmp);

	// 获取配置文件中上次退出的窗口是否是最大化状态，是，则下次显示需要恢复最大化
	GetPrivateProfileString("Information", "IsZoomed", "0", strTmp, sizeof(strTmp)-1, iniDir);
	if (0 == strcmp(strTmp, "1"))
	{
		// 上次退出为最大化状态，设置下次显示为最大化
		isLastTimeExitZoomed = true;
	}
	// 不是最大化状态的位置
	if (nWinXPos < 0 || nWinYPos < 0 || nWinWidth <= 0 || nWinHigh <= 0)
	{
		// 配置文件无数据，默认位置，屏幕右下方四分之一的位置
		GtWriteTrace(30, "%s:%d: \t默认位置，窗口在屏幕右下方四分之一!", __FUNCTION__, __LINE__);
		// 获取任务栏位置，只考虑任务栏在底端的情况
		CRect rect;
		::GetWindowRect(::FindWindow(_T("Shell_TrayWNd"), NULL), &rect);
		// 设置下次显示的位置，设置屏幕右下方四分之一的位置
		this->SetWindowPos(&wndTop, rect.right / 2, rect.top / 2, rect.right / 2, rect.top / 2, SWP_HIDEWINDOW);
	}
	else
	{
		// 设置下次显示的位置，按配置文件位置显示
		GtWriteTrace(30, "%s:%d: \t按配置文件位置设置窗口位置!", __FUNCTION__, __LINE__);
		this->SetWindowPos(&wndTop, nWinXPos, nWinYPos, nWinWidth, nWinHigh, SWP_HIDEWINDOW);
	}
	GtWriteTrace(30, "%s:%d: 初始化窗口位置正常退出。", __FUNCTION__, __LINE__);
	return TRUE;
}


void CPaperlessDlg::OnMyClose()
{
	//MessageBox("OnMyClose!");
	SaveFrmPosToFile();
	this->ShowWindow(SW_HIDE);
}


// 启用钩子
void CPaperlessDlg::StartKeyBoardHook()
{
	if(!m_hookFlag)
	{
		// 待设置的快捷键
		COMBKEY combKey[4];
		HWND myHwnd[4];
		UINT myMsgType[4];
		memset(combKey, 0, sizeof(COMBKEY) * 4);
		memset(myHwnd, 0, sizeof(HWND) * 4);
		memset(myMsgType, 0, sizeof(UINT) * 4);

		char tmpKey[32] = {0};
		char sHotKeyValue[32] = {0};
		// 配置文件读取截图热键值
		GetPrivateProfileString("Information", "HotKeyValue", "3|162|164|83|0|", sHotKeyValue,
			sizeof(sHotKeyValue)-1, GetAppPath()+"\\win.ini");
		// 截屏快捷键
		memset(tmpKey, 0, sizeof(tmpKey));
		splitString(tmpKey, sHotKeyValue, 0);
		combKey[0].count = atoi(tmpKey);
		memset(tmpKey, 0, sizeof(tmpKey));
		splitString(tmpKey, sHotKeyValue, 1);
		combKey[0].keys[0] = atoi(tmpKey);
		memset(tmpKey, 0, sizeof(tmpKey));
		splitString(tmpKey, sHotKeyValue, 2);
		combKey[0].keys[1] = atoi(tmpKey);
		memset(tmpKey, 0, sizeof(tmpKey));
		splitString(tmpKey, sHotKeyValue, 3);
		combKey[0].keys[2] = atoi(tmpKey);
		memset(tmpKey, 0, sizeof(tmpKey));
		splitString(tmpKey, sHotKeyValue, 4);
		combKey[0].keys[3] = atoi(tmpKey);
		myHwnd[0] = m_hWnd;
		myMsgType[0] = WM_SCREENSHOT;

		// 自动填单继续输入快捷键
		combKey[1].count = 2;
		combKey[1].keys[0] = VK_LCONTROL;
		combKey[1].keys[1] = 80;
		combKey[1].keys[2] = 0;
		combKey[1].keys[3] = 0;
		myHwnd[1] = m_hWnd;
		myMsgType[1] = WM_CONTINUE_INPUT;


		// 设置通知窗口句柄和通知消息类型
		SetHWndAndMsgType(myHwnd, myMsgType);
		// 设置快捷键
		SetCombKey(combKey);
		// 设置全局键盘钩子
		SetGHook_KEYBOARD();
	}
	else
	{
		FreeGHook_KEYBOARD();
	}
}


// 启动截图
LRESULT CPaperlessDlg::OnScreenshot(WPARAM wParam, LPARAM lParam)
{
	//	MessageBox("收到截屏消息!");
	//	m_ScreenshotDlg->ShowWindow(SW_SHOW);

	if (pScreenshotDlg == NULL)
	{
		pScreenshotDlg = new CScreenshotDlg();
		pScreenshotDlg->DoModal();
		delete pScreenshotDlg;
		pScreenshotDlg = NULL;
	}
	else
	{
		pScreenshotDlg->ShowWindow(SW_SHOWNORMAL);
	}

	return 0;
}


// 收到截图对话框的消息
LRESULT CPaperlessDlg::OnScreenDlgMessage(WPARAM wParam, LPARAM iParam)
{
	char *str = (char *)wParam;
	GtWriteTrace(30, "[MainFrm]Receive pScreenshotDlg message = [%s]", str);

	// 判断收到的消息
	if (0 == strcmp(str, RECOGNIZE_PICTURE_FAILED))
	{
		// 二维码识别失败，弹出手动输入二维码编号
		ShowManualInput("二维码识别失败，请手动输入流水号或重新截取二维码！");
	}
	else if (0 == strcmp(str, QR_CODE_NOT_EXIST))
	{
		// 流水号不存在，提示是否重新输入二维码编号
		ShowManualInput("流水号不存在，请确认后重新输入！");
	}
	return 0;
}


LRESULT CPaperlessDlg::OnContinueInput(WPARAM wParam, LPARAM iParam)
{
	SendToWindows();
	//MessageBox("tsadfd");
	return 0;
}


LRESULT CPaperlessDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam)
{
	switch(lParam)
	{
	case WM_RBUTTONDOWN:
		{  
			// 右键起来时弹出菜单  
			CMenu *pMenu = m_menu.GetSubMenu(0);
			if(pMenu){
				LPPOINT lpoint = new tagPOINT;  
				// 得到鼠标位置  
				::GetCursorPos(lpoint);
				SetForegroundWindow();
				pMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, lpoint->x ,lpoint->y, AfxGetMainWnd());   
				delete lpoint;
			}	  
			break;  
		}  
	case WM_LBUTTONDBLCLK:
		// 双击左键的处理    
		// 第一次双击的处理，获取配置文件退出前是否是最大化，是则最大化显示
		if (isFirstDbClickMenu && isLastTimeExitZoomed)
		{
			// 将第一次单击置为否
			isFirstDbClickMenu = false;
			// 第一次单击，且上次退出程序是最大化状态，恢复最大化
			this->ShowWindow(SW_SHOWMAXIMIZED);
		}
		else
		{
			// 将第一次单击置为否
			isFirstDbClickMenu = false;
			if (this->IsZoomed())
			{
				// 最大化，恢复最大化
				this->ShowWindow(SW_SHOWMAXIMIZED);
			}
			else if (this->IsIconic())
			{
				// 最小化，恢复原先状态
				this->ShowWindow(SW_SHOWNOACTIVATE);
			}
			else
			{
				this->ShowWindow(SW_SHOWNORMAL);
			}
		}
		break;  
	}
	return 0;
}


void CPaperlessDlg::OnSettingWin()
{
	if (pSettingDlg == NULL)
	{
		pSettingDlg = new SettingDlg();
		pSettingDlg->Create(IDD_SETTING, this);
		pSettingDlg->ShowWindow(SW_SHOWNA);
	}
	else
	{
		pSettingDlg->MyInit();
		pSettingDlg->ShowWindow(SW_SHOWNA);
	}
}


// 显示二维码流水号对话框
void CPaperlessDlg::ShowManualInput(char *tip)
{
	if (pInputDlg == NULL)
	{
		pInputDlg = new CManualInputDlg(tip, this);
		pInputDlg->Create(IDD_MANUALINPUT, this);
		pInputDlg->ShowWindow(SW_SHOWNA);
	}
	else
	{
		pInputDlg->SetTip(tip);
		pInputDlg->ResetEdit();
		pInputDlg->ShowWindow(SW_SHOWNA);
	}
}


// 托盘菜单->手动输入
void CPaperlessDlg::OnManualInput()
{
	//int j = 0;
	//volatile int i = 10 / j;
	ShowManualInput("");
}


void CPaperlessDlg::OnSysCommand( UINT nID, LPARAM lParam)
{
	switch (nID)
	{
	case SC_MAXIMIZE:
		// 最大化按钮点击事件
		// 保存窗口位置信息，是否最大化信息
		this->SaveFrmPosToFile();
		break;
	case SC_MINIMIZE:
		// 最小化按钮点击事件
		// 保存窗口位置信息，是否最大化信息
		this->SaveFrmPosToFile();
		break;
	default:
		break;
	}
	CDialogEx::OnSysCommand(nID, lParam);
}
