
// PaperlessRestartDlg.h : 头文件
//

#pragma once

// 定时器回调函数
void CALLBACK TimerProc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime);
// 获取当前程序运行目录
CString GetAppPath();

// CPaperlessRestartDlg 对话框
class CPaperlessRestartDlg : public CDialogEx
{
// 构造
public:
	CPaperlessRestartDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_PAPERLESSRESTART_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	int nTime;
	// 待启动程序路径
	char sAppDir[256];

public:
	int RestartPaperlessApp();
};

