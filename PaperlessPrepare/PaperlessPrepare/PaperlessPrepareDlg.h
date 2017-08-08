
// PaperlessPrepareDlg.h : ͷ�ļ�
//

#pragma once

// �̺߳���
UINT ThreadFunc(LPVOID pParm);
// ��ʱ���ص�����
void CALLBACK TimerProc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime);
// ˢ����ʾ��ʱ��
#define TIMEER_NO_REFLUSH 1


// CPaperlessPrepareDlg �Ի���
class CPaperlessPrepareDlg : public CDialogEx
{
// ����
public:
	CPaperlessPrepareDlg(CWnd* pParent = NULL);	// ��׼���캯��
	virtual ~CPaperlessPrepareDlg();	// ��������

// �Ի�������
	enum { IDD = IDD_PAPERLESSPREPARE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	int nTime;
	// ��ʱ���Ƿ�������
	BOOL bTimerIsRun;
private:
	// ���͸��±����߳�
	CWinThread *m_pThread;
	// ���԰�ť
	CButton* pBtnRetry;
	// ȡ����ť
	CButton* pBtnCancel;
	// ״̬��ʾ
	CStatic* pStatic;

public:
	void MySetFlushTimer();
	void MyDelFlushTimer();
	void MyInitWin();
	void MyRetryWin(const char *sTip);

	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedRetry();
};
