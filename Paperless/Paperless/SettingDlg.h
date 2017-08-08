#pragma once
#include "GHook/GHook.h"


// SettingDlg dialog

class SettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(SettingDlg)

public:
	SettingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SettingDlg();

// Dialog Data
	enum { IDD = IDD_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonCancel();
	void MyInit();
private:
	COMBKEY combKey;
	int keyDown(BYTE key, char *kyeName);
	int keyFirstUp(BYTE key);
	BOOL isSpecialKey(BYTE key);
	BOOL isKeyUp;
	DECLARE_MESSAGE_MAP()
};
