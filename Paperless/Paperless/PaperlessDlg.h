// PaperlessDlg.h : ͷ�ļ�
//
#include "ScreenshotDlg.h"
#include "ManualInputDlg.h"
#include "SettingDlg.h"
#include "CBaseReadIDCard.h"

#pragma once


// CPaperlessDlg �Ի���
class CPaperlessDlg : public CDialogEx
{
// ����
public:
	CPaperlessDlg(CWnd* pParent = NULL);	// ��׼���캯��
	virtual ~CPaperlessDlg();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
// �Ի�������
	enum { IDD = IDD_PAPERLESS_DIALOG};

protected:
	// DDX/DDV ֧��
	virtual void DoDataExchange(CDataExchange* pDX);
	// ��ʼ���Ի�����
	virtual BOOL OnInitDialog();
	// ����ϵͳ�����
	virtual void OnSysCommand( UINT nID, LPARAM lParam);
	// ���ڴ�С�ı���Ӧ����
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// �����Ի���
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	// ??
	afx_msg void OnPaint();
	// ??
	afx_msg HCURSOR OnQueryDragIcon();

	// ����ȫ�ֹ���
	void StartKeyBoardHook();
	// �ر�ȫ�ֹ���
	void StopKeyBoardHook();
	// ����������ʾ
	BOOL ShowBalloonTip(LPCTSTR szMsg, LPCTSTR szTitle, UINT uTimeout, DWORD dwInfoFlags);
	// �յ����ӷ��͵Ľ�����Ϣ�Ĵ���
	afx_msg LRESULT OnScreenshot(WPARAM wParam, LPARAM lParam);
	// ��ͼ���洫�ص���Ϣ����
	afx_msg LRESULT OnScreenDlgMessage(WPARAM wParam, LPARAM iParam);
	// �յ����ӷ��͵ļ����Զ����Ϣ����
	afx_msg LRESULT OnContinueInput(WPARAM wParam, LPARAM iParam);

	// ���������Ϣ��Ӧ����������
	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);
	// �յ����̲˵���� �ֶ�������ˮ�� ʱ������ʾ
	afx_msg void OnManualInput();
	// ��ʼ���ֶ�������ˮ�Ŵ���ʱ��ʾ��ʾ
	afx_msg void ShowManualInput(char *tip);
	// �յ����̲˵���� ���� ʱ������ʾ
	afx_msg void OnSettingWin();
	// ���̲˵�����˳���ť���¼�����
	afx_msg void OnQuit();

	// �ر��������¼��Ĵ���
	afx_msg void OnMyClose();

// ʵ��
protected:
	// ����
	HICON m_hIcon;
	// �ؼ���Ƕ���Ա
	CStatusBar m_wndStatusBar;
	// �������̲˵�
	CMenu m_menu;
public:
	// �������̵���ؽṹ��
	NOTIFYICONDATA m_nid;

public:
	HANDLE handle_memery;
	// ���֤ʶ�����豸���
	int nReadIDDevice;
	// ����������ͷ�豸���
	int nDeskCameraDevice;
	// �Ƿ����ù��ӱ�־
	BOOL m_hookFlag;
	// ��������
	CScreenshotDlg *pScreenshotDlg;
	// �ֶ�������ˮ�Ŵ���
	CManualInputDlg *pInputDlg;
	// ���ô���
	SettingDlg *pSettingDlg;
	// ���ڸ����Ǵ���
	CDialogEx *pCentCameraDlg;
	// ���֤ʶ����ʵ���࣬��̬
	CBaseReadIDCardInfo *pBaseReadIDCardInfo;
	// ����������ͷʵ���࣬��̬
	CBaseSaveCamera *pBaseSaveCameraPic;
	// �Ƿ��ǵ�һ�����˫�����̲˵�
	bool isFirstDbClickMenu;
	// �ϴ��˳�ʱ�����Ƿ������״̬
	bool isLastTimeExitZoomed;

public:
	// ����������λ�õ������ļ��У����´�����ʱ�ָ�
	BOOL SaveFrmPosToFile();
	// ��ȡ�����ļ����ã��ָ�����λ��
	BOOL InitFrmPosFromFile();
	// ��ʼ�����֤ʶ���ǡ��������豸
	void InitDevice();

	DECLARE_MESSAGE_MAP()
};
