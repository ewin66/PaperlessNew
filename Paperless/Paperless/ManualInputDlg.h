#pragma once


// CManualInputDlg dialog

class CManualInputDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CManualInputDlg)

public:
	CManualInputDlg(CString tip, CWnd* pParent = NULL);   // standard constructor
	virtual ~CManualInputDlg();

// Dialog Data
	enum { IDD = IDD_MANUALINPUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

public:
	afx_msg void OnBnClickedBtnSend();
	afx_msg void OnBnClickedBtnCancel();

public:
	void SetTip(char *pStrTip);
	void ResetEdit();

private:
	CString tip;

	DECLARE_MESSAGE_MAP()
};
