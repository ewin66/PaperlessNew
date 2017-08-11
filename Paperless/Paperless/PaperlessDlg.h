// PaperlessDlg.h : 头文件
//
#include "ScreenshotDlg.h"
#include "ManualInputDlg.h"
#include "SettingDlg.h"
#include "CBaseReadIDCard.h"

#pragma once


// CPaperlessDlg 对话框
class CPaperlessDlg : public CDialogEx
{
// 构造
public:
	CPaperlessDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~CPaperlessDlg();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
// 对话框数据
	enum { IDD = IDD_PAPERLESS_DIALOG};

protected:
	// DDX/DDV 支持
	virtual void DoDataExchange(CDataExchange* pDX);
	// 初始化对话框处理
	virtual BOOL OnInitDialog();
	// 处理系统命令函数
	virtual void OnSysCommand( UINT nID, LPARAM lParam);
	// 窗口大小改变响应函数
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// 创建对话框
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	// ??
	afx_msg void OnPaint();
	// ??
	afx_msg HCURSOR OnQueryDragIcon();

	// 开启全局钩子
	void StartKeyBoardHook();
	// 关闭全局钩子
	void StopKeyBoardHook();
	// 托盘气泡提示
	BOOL ShowBalloonTip(LPCTSTR szMsg, LPCTSTR szTitle, UINT uTimeout, DWORD dwInfoFlags);
	// 收到钩子发送的截屏消息的处理
	afx_msg LRESULT OnScreenshot(WPARAM wParam, LPARAM lParam);
	// 截图界面传回的消息处理
	afx_msg LRESULT OnScreenDlgMessage(WPARAM wParam, LPARAM iParam);
	// 收到钩子发送的继续自动填单消息处理
	afx_msg LRESULT OnContinueInput(WPARAM wParam, LPARAM iParam);

	// 添加托盘消息响应函数的声明
	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);
	// 收到托盘菜单点击 手动输入流水号 时窗体显示
	afx_msg void OnManualInput();
	// 初始化手动输入流水号窗体时显示提示
	afx_msg void ShowManualInput(char *tip);
	// 收到托盘菜单点击 设置 时窗体显示
	afx_msg void OnSettingWin();
	// 托盘菜单点击退出按钮的事件处理
	afx_msg void OnQuit();

	// 关闭主窗体事件的处理
	afx_msg void OnMyClose();

// 实现
protected:
	// ？？
	HICON m_hIcon;
	// 控件条嵌入成员
	CStatusBar m_wndStatusBar;
	// 定义托盘菜单
	CMenu m_menu;
public:
	// 定义托盘的相关结构体
	NOTIFYICONDATA m_nid;

public:
	HANDLE handle_memery;
	// 身份证识读仪设备编号
	int nReadIDDevice;
	// 高拍仪摄像头设备编号
	int nDeskCameraDevice;
	// 是否设置钩子标志
	BOOL m_hookFlag;
	// 截屏窗口
	CScreenshotDlg *pScreenshotDlg;
	// 手动输入流水号窗口
	CManualInputDlg *pInputDlg;
	// 设置窗口
	SettingDlg *pSettingDlg;
	// 升腾高拍仪窗口
	CDialogEx *pCentCameraDlg;
	// 身份证识读器实体类，多态
	CBaseReadIDCardInfo *pBaseReadIDCardInfo;
	// 高拍仪摄像头实体类，多态
	CBaseSaveCamera *pBaseSaveCameraPic;
	// 是否是第一次左键双击托盘菜单
	bool isFirstDbClickMenu;
	// 上次退出时窗口是否是最大化状态
	bool isLastTimeExitZoomed;

public:
	// 保存主窗口位置到配置文件中，供下次启动时恢复
	BOOL SaveFrmPosToFile();
	// 读取配置文件配置，恢复窗口位置
	BOOL InitFrmPosFromFile();
	// 初始化身份证识读仪、高拍仪设备
	void InitDevice();

	DECLARE_MESSAGE_MAP()
};
