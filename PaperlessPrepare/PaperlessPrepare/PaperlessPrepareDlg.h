
// PaperlessPrepareDlg.h : 头文件
//

#pragma once

// 线程函数
UINT ThreadFunc(LPVOID pParm);
// 定时器回调函数
void CALLBACK TimerProc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime);
// 刷新提示定时器
#define TIMEER_NO_REFLUSH 1


// CPaperlessPrepareDlg 对话框
class CPaperlessPrepareDlg : public CDialogEx
{
// 构造
public:
	CPaperlessPrepareDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~CPaperlessPrepareDlg();	// 析构函数

// 对话框数据
	enum { IDD = IDD_PAPERLESSPREPARE_DIALOG };

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

	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	int nTime;
	// 定时器是否在运行
	BOOL bTimerIsRun;
private:
	// 发送更新报文线程
	CWinThread *m_pThread;
	// 重试按钮
	CButton* pBtnRetry;
	// 取消按钮
	CButton* pBtnCancel;
	// 状态提示
	CStatic* pStatic;

public:
	void MySetFlushTimer();
	void MyDelFlushTimer();
	void MyInitWin();
	void MyRetryWin(const char *sTip);

	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedRetry();
};
